
/***************************************************************************
                          files.cpp  - work with data and source files
                             -------------------
    begin                : Mon May 2 2016
    copyright            : (C) 2016 by Keith Isdale
    email                : keithisdale@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/* We want skip most of these includes when building documentation */

#include "files.h"
#include "xsldbg.h"
#include <stdio.h>
#include <libxml/entities.h>
#include <libxml/tree.h>
#include <libxml/catalog.h>
#include <libxml/encoding.h>    /* needed by filesTranslate, filesEncoding functions */
#include <libxml/uri.h>    /* needed for  xmlURIUnescapeString */
#include "debugXSL.h"
#include "utils.h"
#include "options.h"
#include <QDir>
#include <QDebug>
#include <QMap>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QUrl>

#ifndef WIN32
#include <unistd.h>
#endif
#include "xsldbgthread.h"
#ifdef WIN32
#include <direct.h>
#endif

/* top xml document */
static xmlDocPtr topDocument;

/* temporary xml document */
static xmlDocPtr tempDocument;

/* used as a scratch pad for temporary results*/
static xmlChar filesBuffer[DEBUG_BUFFER_SIZE];

/* top stylsheet */
static xsltStylesheetPtr topStylesheet;

/* what is the base path for top stylesheet */
static QString stylePathName;

/* what is the path for current working directory*/
static QString workingDirPath;

static arrayListPtr entityNameList = NULL;

/* Current encoding to use for standard output*/
static xmlCharEncodingHandlerPtr stdoutEncoding = NULL;

/* input and output buffers for encoding*/
static xmlBufferPtr encodeInBuff = NULL;
static xmlBufferPtr encodeOutBuff = NULL;

/* Current line number and URI for xsldbg*/
static int currentLineNo = -1;
static xmlChar *currentUrl = NULL;

/* -----------------------------------------
   Private function declarations for files.c
 -------------------------------------------*/

/**
 * Try to guess what the complete file/URI is. If successful the search
 *   info will be set to found and the search data will contain the
 *   file name found. We are given our payload via walkStylesheets
 *
 * @param payload: valid xsltStylesheetPtr
 * @param data: valid searchInfoPtr of type SEARCH_NODE
 * @param name: not used
 *
 */
static void guessStylesheetHelper(void *payload, void *data, xmlChar * name);


/**
 * Try to guess what the complete file/URI is. If successful the search
 *   info will be set to found and the search data will contain the
 *   file name found. We are given our payload via walkIncludes
 *
 * @param payload: valid xmlNodePtr of the included stylesheet
 * @param data: valid searchInfoPtr of type SEARCH_NODE
 * @param name: not used
 *
 */
static void guessStylesheetHelper2(void *payload, void *data, xmlChar * name);


entityInfoPtr filesNewEntityInfo(const xmlChar * SystemID, const xmlChar * PublicID);

void filesFreeEntityInfo(entityInfoPtr info);

void filesAddEntityName(const xmlChar * SystemID, const xmlChar * PublicID);


/* -------------------------------------
    End private functions
---------------------------------------*/


FILE *terminalIO;

/* No longer needed
   static FILE *oldStdin, *oldStdout, *oldStderr;*/


//static char *ttyName = NULL;           /* what is the name of the default terminal */
static char *termName = NULL;   /* what is the name of terminal we are redirected to */


int openTerminal(xmlChar * device)
{
    int result = 0;

    if (!device) {              /* Failed; there's no device */
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL argument provided\n");
#endif
        return result;
    }

    /*
     * On RISC OS, you get one terminal - the screen.
     * we assume that the parameter is meant to be an output device as
     * per normal - we can use vdu:, rawvdu: or :tt, or a filename for
     * normal VDU output, VDU output without newline expansion,
     * C terminal output with control code escaping, or a raw file
     * respectively.
     * The name passed is expected to be in native file format - no
     * URI escaping here.
     * One assumes that you might use a socket or a pipe here.
     */

    if (terminalIO) {
        fclose(terminalIO);
        terminalIO = NULL;
    }


    switch (device[0]) {
    case '\0':
    case '0':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        /* look like we are supposed to close the terminal
             * but we've already done that
             */
        break;

    case '1':
        if (termName) {
            terminalIO = fopen((char *) termName, "w");
            if (terminalIO != NULL) {
                xmlFree(termName);
                termName = xmlMemStrdup((char *) device);
                result = 1;
            } else {
                xsldbgGenericErrorFunc(QObject::tr("Error: Unable to open terminal %1.\n").arg(xsldbgText(termName)));
            }
        } else {
            xsldbgGenericErrorFunc(QObject::tr("Error: Did not previously open terminal.\n"));
        }
        break;

    case '2':
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Warning: Terminal level 2 not implemented\n");
#endif
        break;


    default:
        terminalIO = fopen((char *) device, "w");
        if (terminalIO != NULL) {
            if (termName)
                xmlFree(termName);
            termName = xmlMemStrdup((char *) device);
            result = 1;
        } else {
            xsldbgGenericErrorFunc(QObject::tr("Error: Unable to open terminal %1.\n").arg(xsldbgText(device)));
        }

    }

    return result;
}


void guessStylesheetHelper(void *payload, void *data,
                           xmlChar * name)
{
    Q_UNUSED(name);
    xsltStylesheetPtr style = (xsltStylesheetPtr) payload;
    searchInfoPtr searchCriteria = (searchInfoPtr) data;
    nodeSearchDataPtr searchData = NULL;

    if (!style || !style->doc || !searchCriteria || !searchCriteria->data
            || (searchCriteria->type != SEARCH_NODE))
        return;

    searchData = (nodeSearchDataPtr) searchCriteria->data;
    if (searchData->nameInput && (searchData->absoluteNameMatch == NULL)) {
        xmlChar * fixedDocURL = fixResolveFilePath(xmlStrdup(style->doc->URL));
        /* at this point we know that we have not made an absolute match
         * but we may have made a relative match */
        if (xmlStrCmp(fixedDocURL, searchData->nameInput) == 0) {
            /* absolute path match great! */
            searchData->absoluteNameMatch =
                    (xmlChar *) xmlMemStrdup((char *)fixedDocURL);
            searchData->node = (xmlNodePtr) style->doc;
            searchCriteria->found = 1;
            xmlFree(fixedDocURL);
            return;
        }

        /* try to guess we assume that the files are unique */
        xmlStrCpy(filesBuffer, "__#!__");

        /* try relative to top stylesheet directory */
        if (!stylePath().isEmpty()) {
            QChar pathSeparator = QDir::separator();
            QChar lastChar = stylePath().at(stylePath().length() - 1);

            if (stylePath().indexOf(':') != -1) { // if URI then always use forward slash
                pathSeparator = URISEPARATORCHAR;
            }
            xmlStrCpy(filesBuffer, stylePath().toUtf8().constData());
            if (lastChar != pathSeparator) { //ensure that there is a final path separator present
                QString pathSeparatorString(pathSeparator);
                xmlStrCat(filesBuffer, pathSeparatorString.toUtf8().constData());
            }
            xmlStrCat(filesBuffer, searchData->nameInput);
        }
        if (xmlStrCmp(fixedDocURL, filesBuffer) == 0) {
            /* guessed right! */
            searchData->guessedNameMatch =
                    (xmlChar *) xmlMemStrdup((char *) filesBuffer);
            searchData->node = (xmlNodePtr) style->doc;
            searchCriteria->found = 1;
            xmlFree(fixedDocURL);
            return;
        }
        if (!workingPath().isEmpty()) {
            /* try relative to working directory */
            QChar pathSeparator = QDir::separator();
            QChar lastChar = workingPath().at(workingPath().length() - 1);

            xmlStrCpy(filesBuffer, workingPath().toUtf8().constData());
            if (lastChar != pathSeparator) { //ensure that there is a final path separator present
                QString pathSeparatorString(pathSeparator);
                xmlStrCat(filesBuffer, pathSeparatorString.toUtf8().constData());
            }
            xmlStrCat(filesBuffer, searchData->nameInput);
        }
        if (xmlStrCmp(fixedDocURL, filesBuffer) == 0) {
            /* guessed right! */
            searchData->guessedNameMatch =
                    (xmlChar *) xmlMemStrdup((char *) filesBuffer);
            searchData->node = (xmlNodePtr) style->doc;
            searchCriteria->found = 1;
            xmlFree(fixedDocURL);
            return;
        }

        /* Find the last separator of the document's URL */
        xmlChar pathSeparator = PATHCHAR;

        if (xmlStrChr(fixedDocURL, ':') != 0) { // if URI then always use forward slash
            pathSeparator = URISEPARATORCHAR;
        }

        xmlChar* lastSlash = (xmlChar*)xmlStrrChr(fixedDocURL, pathSeparator);
        xmlChar* lastSlashSearch = (xmlChar*)xmlStrrChr(filesBuffer, pathSeparator);
        if (!lastSlashSearch) {
            lastSlashSearch = filesBuffer;
        } else {
            lastSlashSearch++;
        }
        if (!lastSlash) {
            lastSlash = (xmlChar*)fixedDocURL;
        } else {
            lastSlash++;
        }
        if (xmlStrCmp(lastSlash, lastSlashSearch) == 0) {
            /* guessed right! */
            searchData->guessedNameMatch =
                    (xmlChar *) xmlStrdup(fixedDocURL);
            searchData->node = (xmlNodePtr) style->doc;
            searchCriteria->found = 1;
        }
        xmlFree(fixedDocURL);
    }
}


void guessStylesheetHelper2(void *payload, void *data,
                            xmlChar * name)
{
    Q_UNUSED(name);
    xmlNodePtr node = (xmlNodePtr) payload;
    searchInfoPtr searchCriteria = (searchInfoPtr) data;
    nodeSearchDataPtr searchData = NULL;

    if (!node || !node->doc || !searchCriteria || !searchCriteria->data ||
            (searchCriteria->type != SEARCH_NODE))
        return;

    searchData = (nodeSearchDataPtr) searchCriteria->data;
    if (searchData->nameInput && (searchData->absoluteNameMatch == NULL)) {
        /* at this point we know that we have not made an absolute match
         * but we may have made a relative match */
        if (xmlStrCmp(node->doc->URL, searchData->nameInput) == 0) {
            /* absolute path match great! */
            searchData->absoluteNameMatch =
                    (xmlChar *) xmlMemStrdup((char *) node->doc->URL);
            searchData->node = node;
            searchCriteria->found = 1;
            return;
        }


        /* try to guess we assume that the files are unique */
        xmlStrCpy(filesBuffer, "__#!__");
        /* try relative to top stylesheet directory */
        if (!stylePath().isEmpty()) {
            QChar pathSeparator = QDir::separator();
            QChar lastChar = stylePath().at(stylePath().length() - 1);

            if (stylePath().indexOf(':') != -1) { // if URI then always use forward slash
                pathSeparator = URISEPARATORCHAR;
            }
            xmlStrCpy(filesBuffer, stylePath().toUtf8().constData());
            if (lastChar != pathSeparator) { //ensure that there is a final path separator present
                QString pathSeparatorString(pathSeparator);
                xmlStrCat(filesBuffer, pathSeparatorString.toUtf8().constData());
            }
            xmlStrCat(filesBuffer, searchData->nameInput);
        }
        if (xmlStrCmp(node->doc->URL, filesBuffer) == 0) {
            /* guessed right! */
            searchData->guessedNameMatch =
                    (xmlChar *) xmlMemStrdup((char *) filesBuffer);
            searchData->node = node;
            searchCriteria->found = 1;
            return;
        }

        if (!workingPath().isEmpty()) {
            /* try relative to working directory */
            QChar pathSeparator = QDir::separator();
            QChar lastChar = workingPath().at(workingPath().length() - 1);

            xmlStrCpy(filesBuffer, workingPath().toUtf8().constData());
            if (lastChar != pathSeparator) { //ensure that there is a final path separator present
                QString pathSeparatorString(pathSeparator);
                xmlStrCat(filesBuffer, pathSeparatorString.toUtf8().constData());
            }
            xmlStrCat(filesBuffer, searchData->nameInput);
        }
        if (xmlStrCmp(node->doc->URL, filesBuffer) == 0) {
            /* guessed right! */
            searchData->guessedNameMatch =
                    (xmlChar *) xmlMemStrdup((char *) filesBuffer);
            searchData->node = node;
            searchCriteria->found = 1;
            return;
        }


        /* Find the last separator of the document's URL */
        xmlChar pathSeparator = PATHCHAR;

        if (xmlStrChr(node->doc->URL, ':') != 0) { // if URI then always use forward slash
            pathSeparator = URISEPARATORCHAR;
        }

        xmlChar* lastSlash = (xmlChar*)xmlStrrChr(node->doc->URL, pathSeparator);
        xmlChar* lastSlashSearch = (xmlChar*)xmlStrrChr(filesBuffer, pathSeparator);
        if (!lastSlashSearch) {
            lastSlashSearch = filesBuffer;
        } else {
            lastSlashSearch++;
        }
        if (!lastSlash) {
            lastSlash = (xmlChar*)node->doc->URL;
        } else {
            lastSlash++;
        }
        if (xmlStrCmp(lastSlash, lastSlashSearch) == 0) {
            /* guessed right! */
            searchData->guessedNameMatch =
                    (xmlChar *) xmlStrdup(node->doc->URL);
            searchData->node = node;
            searchCriteria->found = 1;
        }
    }
}


void guessStylesheetName(searchInfoPtr searchCriteria)
{
    nodeSearchDataPtr searchData;

    if (!searchCriteria)
        return;

    searchData = (nodeSearchDataPtr) searchCriteria->data;
    if (searchData->nameInput == NULL)
        return;                 /* must supply name of file to look for */

    walkStylesheets((xmlHashScanner) guessStylesheetHelper,
                    searchCriteria, filesGetStylesheet());
    if (!searchCriteria->found) {
        /* try looking in the included stylesheets */
        walkIncludes((xmlHashScanner) guessStylesheetHelper2,
                     searchCriteria, filesGetStylesheet());
    }
}


QString stylePath(void)
{
    return stylePathName;
}


QString workingPath(void)
{
    return workingDirPath;
}


int changeDir(QString path)
{
    int result = 0;
    QString expandedName;

    path = path.trimmed();
    if (path.indexOf("file:/") != -1) {
        QUrl url(path);
        path = url.toLocalFile();
    }

    if (!path.isEmpty()) {
        // expand the path but do not add the "file:// prefix"
        expandedName = filesExpandName(path, false);
    } else {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: Empty path provided to changeDir");
#endif
        return result;
    }

    if (expandedName.isEmpty()){
        xsldbgGenericErrorFunc(QObject::tr("Error: Unable to change to directory %1 which was expanded to %2.\n").arg(path).arg(expandedName));
        return result;
    }

    QDir d;
    QString newDir(expandedName);

    if (d.cd(newDir) && ::chdir(QFile::encodeName(newDir)) == 0) {
        newDir = QDir::currentPath();
        /* must have path char at end of path name */
        workingDirPath = newDir + PATHCHAR;
#ifdef Q_OS_WIN32
        workingDirPath = workingDirPath.replace("/", "\\");
#endif
        optionsSetStringOption(OPTIONS_CWD, workingDirPath);
        result = 1;
    }
    if (!result) {
        if (path != expandedName)
            xsldbgGenericErrorFunc(QObject::tr("Error: Unable to change to directory \"%1\" which was expanded to \"%2\".\n").arg(path).arg(expandedName));
        else
            xsldbgGenericErrorFunc(QObject::tr("Error: Unable to change to directory \"%1\".\n").arg(path));
    } else {

        if (xslDebugStatus != DEBUG_NONE)
            xsldbgGenericErrorFunc(QObject::tr("Changed to directory %1.\n").arg(workingDirPath));
    }
    return result;
}

int filesLoadXmlFile(const xmlChar * path, FileTypeEnum fileType)
{
    int result = 0;

    if (!filesFreeXmlFile(fileType))
        return result;

    switch (fileType) {
    case FILES_XMLFILE_TYPE:
        if (path && xmlStrLen(path)) {
            if (optionsGetIntOption(OPTIONS_SHELL)) {
                xsldbgGenericErrorFunc(QObject::tr("Setting XML Data file name to %1.\n").arg(xsldbgText(path)));
            }
            optionsSetStringOption(OPTIONS_DATA_FILE_NAME, xsldbgText(path));
        }
        topDocument = xsldbgLoadXmlData();
        if (topDocument)
            result = 1;
        break;

    case FILES_SOURCEFILE_TYPE:
        if (path && xmlStrLen(path)) {
            if (optionsGetIntOption(OPTIONS_SHELL)) {
                xsldbgGenericErrorFunc(QObject::tr("Setting stylesheet file name to %1.\n").arg(xsldbgText(path)));
            }
            optionsSetStringOption(OPTIONS_SOURCE_FILE_NAME, xsldbgText(path));
        }
        topStylesheet = xsldbgLoadStylesheet();
        if (topStylesheet && topStylesheet->doc) {
            /* look for last slash (or baskslash) of URL */
            QString orginalDocURL = xsldbgText(topStylesheet->doc->URL);
            QString docUrl = fixResolveFilePath(orginalDocURL);
            int lastSlash;
            char pathChar = URISEPARATORCHAR;
            if (!docUrl.contains(":/")) {
                pathChar = PATHCHAR;
            }
            lastSlash = docUrl.lastIndexOf(pathChar);
            result = 1;
            if (!docUrl.isEmpty() && lastSlash) {
                stylePathName = docUrl.mid(0, lastSlash);
                if (optionsGetIntOption(OPTIONS_SHELL)) {
                    xsldbgGenericErrorFunc(QObject::tr("Setting stylesheet base path to'%1'.\n").arg(stylePathName));
                }
            } else {
                /* ie for *nix this becomes "./" */
                stylePathName = '.' + PATHCHAR;
            }

            /* try to find encoding for this stylesheet */
            if (optionsGetIntOption(OPTIONS_AUTOENCODE))
                filesSetEncoding((char *) topStylesheet->encoding);
        }
        break;

    case FILES_TEMPORARYFILE_TYPE:
        if (!path || !xmlStrLen(path)) {
            xsldbgGenericErrorFunc(QObject::tr("Missing file name.\n"));
            break;
        }
        topDocument = xsldbgLoadXmlTemporary(path);
        if (tempDocument)
            result = 1;
        break;
    }
    return result;
}


int filesFreeXmlFile(FileTypeEnum fileType)
{
    int result = 0, type = fileType;

    switch (type) {
    case FILES_XMLFILE_TYPE:
        if (topDocument)
            xmlFreeDoc(topDocument);
        topDocument = NULL;
        result = 1;
        break;

    case FILES_SOURCEFILE_TYPE:
        if (topStylesheet)
            xsltFreeStylesheet(topStylesheet);
        topStylesheet = NULL;
        result = 1;
        break;

    case FILES_TEMPORARYFILE_TYPE:
        if (tempDocument)
            xmlFreeDoc(tempDocument);
        tempDocument = NULL;
        result = 1;
        break;
    }
    return result;
}


xsltStylesheetPtr filesGetStylesheet(void)
{
    return topStylesheet;
}


xmlDocPtr filesGetTemporaryDoc(void)
{
    return tempDocument;
}


xmlDocPtr filesGetMainDoc(void)
{
    return topDocument;
}


int filesReloaded(int reloaded)
{
    static int changed = 0;

    if (reloaded >= 0) {
        changed = reloaded;
    }

    return changed;
}



int filesInit(void)
{
    int result = 0;

    terminalIO = NULL;
#ifdef __riscos
    ttyName = ":tt";            /* Default tty */
#endif
#ifdef HAVE_UNISTD
    ttyName = ttyname(fileno(stdin));
    /* save out io for when/if we send debugging to a terminal */
    oldStdin = stdin;
    oldStdout = stdout;
    oldStderr = stderr;
#endif
    topDocument = NULL;
    tempDocument = NULL;
    topStylesheet = NULL;
    entityNameList = arrayListNew(4, (freeItemFunc) filesFreeEntityInfo);
#if defined(HAVE_INCLUDE_FIX) && (LIBXML_VERSION < 20508)
    xmlSetEntityReferenceFunc(filesEntityRef);
#endif

    /* setup the encoding */
    encodeInBuff = xmlBufferCreate();
    encodeOutBuff = xmlBufferCreate();

    /* check the result so far and lastly perform platform specific
     * initialization */
    if (entityNameList && encodeInBuff && encodeOutBuff &&
            filesPlatformInit())
        result = 1;
    return result;
}

void filesFree(void)
{
    int result;

    if (terminalIO) {
        fclose(terminalIO);
        terminalIO = NULL;
    }
    if (termName) {
        xmlFree(termName);
        termName = NULL;
    }

    result = filesFreeXmlFile(FILES_SOURCEFILE_TYPE);
    if (result)
        result = filesFreeXmlFile(FILES_XMLFILE_TYPE);
    if (result)
        result = filesFreeXmlFile(FILES_TEMPORARYFILE_TYPE);
    if (!result){
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: Unable to free memory used by XML/XSLT files\n");
#endif
    }

    if (entityNameList) {
        arrayListFree(entityNameList);
        entityNameList = NULL;
    }

    /* Free memory used by encoding related structures */
    if (encodeInBuff)
        xmlBufferFree(encodeInBuff);
    encodeInBuff = 0;

    if (encodeOutBuff)
        xmlBufferFree(encodeOutBuff);
    encodeOutBuff = 0;

    /* close current encoding */
    filesSetEncoding(NULL);

    if (currentUrl)
        xmlFree(currentUrl);
    currentUrl = 0;

    /* free any memory used by platform specific files module */
    filesPlatformFree();
}


int filesIsSourceFile(xmlChar * fileName)
{
    return strstr((char *) fileName, ".xsl") ||
            strstr((char *) fileName, ".Xsl") ||
            strstr((char *) fileName, ".XSL");
}



entityInfoPtr filesNewEntityInfo(const xmlChar * SystemID, const xmlChar * PublicID)
{

    entityInfoPtr result = (entityInfoPtr) xmlMalloc(sizeof(entityInfo));

    if (result) {
        if (SystemID)
            result->SystemID = xmlStrdup(SystemID);
        else
            result->SystemID = xmlStrdup(BAD_CAST "");

        if (PublicID)
            result->PublicID = xmlStrdup(PublicID);
        else
            result->PublicID = xmlStrdup(BAD_CAST "");

        result->ResolvedURI = xmlStrdup(BAD_CAST "");
    }
    return result;
}

void filesFreeEntityInfo(entityInfoPtr info)
{
    if (!info)
        return;

    if (info->SystemID)
        xmlFree(info->SystemID);

    if (info->PublicID)
        xmlFree(info->PublicID);

    if (info->ResolvedURI)
        xmlFree(info->ResolvedURI);

    xmlFree(info);
}

void filesAddEntityName(const xmlChar * SystemID, const xmlChar * PublicID)
{
    int entityIndex = 0;
    entityInfoPtr tempItem;

    if (!SystemID || !filesEntityList())
        return;

    for (entityIndex = 0;
         entityIndex < arrayListCount(filesEntityList()); entityIndex++) {
        tempItem =
                (entityInfoPtr) arrayListGet(filesEntityList(), entityIndex);
        if (tempItem && xmlStrEqual(SystemID, tempItem->SystemID)) {
            /* name aready exits so don't add it */
            return;
        }

    }

    tempItem = filesNewEntityInfo(SystemID, PublicID);
    arrayListAdd(filesEntityList(), tempItem);
}

void filesEntityRef(xmlEntityPtr ent, xmlNodePtr firstNode, xmlNodePtr lastNode)
{
    xmlNodePtr node = firstNode;
    if (!firstNode || !ent || !ent->SystemID ||
            (ent->etype != XML_EXTERNAL_GENERAL_PARSED_ENTITY) )
        return;

    xmlChar *fixedURI = 0;
    if (ent->ExternalID) {
        fixedURI = fixResolveFilePath(xmlStrdup(ent->SystemID));
        filesAddEntityName(ent->SystemID, ent->ExternalID);
    } else {
        fixedURI = fixResolveFilePath(xmlStrdup(ent->URI));
        filesAddEntityName(ent->URI, BAD_CAST "");
    }

    if (fixedURI) {
        while (node){
            filesSetBaseUri(node, fixedURI);
            if (node != lastNode)
                node = node->next;
            else
                node = NULL;
        }
        xmlFree(fixedURI);
    }
}



int filesSetBaseUri(xmlNodePtr node, const xmlChar * uri)
{
    int result = 0;

    if (!node || !uri)
        return result;
    else {
        if (node->type == XML_ELEMENT_NODE){
            xmlChar *xsldbgUrlCopy = xmlGetProp(node, BAD_CAST "xsldbg:uri");
            if (!xsldbgUrlCopy) {
                xmlNewProp(node, BAD_CAST "xsldbg:uri", uri);
            } else {
                xmlFree(xsldbgUrlCopy);
            }

        }
        result = 1;
    }
    return result;
}


xmlChar * filesGetBaseUri(xmlNodePtr node)
{
    xmlChar *result = NULL;

    if (!node || !node->doc)
        return result;

    while (node && node->parent) {
        /*
         * result =  xmlGetNsProp(node, BAD_CAST "uri", XSLDBG_XML_NAMESPACE);
         */
        if (node->type == XML_ELEMENT_NODE) {
            result = xmlGetProp(node, BAD_CAST "xsldbg:uri");
            if (result)
                break;
        }
        node = node->parent;
    }

    if (!result && node->doc && node->doc->URL)
        result = fixResolveFilePath(xmlStrdup(node->doc->URL));

    return result;
}




arrayListPtr filesEntityList(void)
{
    return entityNameList;
}


int filesLoadCatalogs(void)
{
    int result = 0;
    QString catalogs;

    xmlCatalogCleanup();
    if (optionsGetIntOption(OPTIONS_CATALOGS)) {
        if (optionsGetStringOption(OPTIONS_CATALOG_NAMES).isEmpty()) {
            /* use the SGML catalog */
#ifdef __riscos
            catalogs = getenv("SGML$CatalogFiles");
#else
            catalogs = getenv("SGML_CATALOG_FILES");
#endif
            if (catalogs == NULL) {
#ifdef __riscos
                xsldbgGenericErrorFunc("Warning: Environment variable SGML$CatalogFiles is not set.\n");
#else
                xsldbgGenericErrorFunc("Warning: Environment variabe SGML_CATALOG_FILES FILES not set.\n");
#endif
            } else
                /* copy the current catalog name(s) for user to see */
                optionsSetStringOption(OPTIONS_CATALOG_NAMES, catalogs);
        } else
            /* Use the current catalog settings from users*/
            catalogs = optionsGetStringOption(OPTIONS_CATALOG_NAMES);

        result = 1;
    }

    if (!catalogs.isEmpty()){
        /* Load the new cataog selection */
        xmlLoadCatalogs(catalogs.toUtf8().constData());
    }else{
        /* Use default catalogs */
        xmlInitializeCatalog();
    }
    return result;
}




xmlChar * filesEncode(const xmlChar * text)
{
    xmlChar *result = NULL;

    if (!text)
        return result;

    if (!stdoutEncoding || !encodeInBuff || !encodeOutBuff)
        return xmlStrdup(text); /* no encoding active return as UTF-8 */

    xmlBufferEmpty(encodeInBuff);
    xmlBufferEmpty(encodeOutBuff);
    xmlBufferCat(encodeInBuff, text);

    if (xmlCharEncOutFunc(stdoutEncoding, encodeOutBuff, encodeInBuff)
            >= 0) {
        result = xmlStrdup(xmlBufferContent(encodeOutBuff));
    } else {
        xsldbgGenericErrorFunc(QObject::tr("Encoding of text failed.\n"));
        return xmlStrdup(text); /*  panic,  return as UTF-8 */
    }
    return result;
}



xmlChar * filesDecode(const xmlChar * text)
{
    xmlChar *result = NULL;

    if (!text)
        return result;

    if (!stdoutEncoding || !encodeInBuff || !encodeOutBuff)
        return xmlStrdup(text); /* no encoding active return as UTF-8 */

    xmlBufferEmpty(encodeInBuff);
    xmlBufferEmpty(encodeOutBuff);
    xmlBufferCat(encodeInBuff, text);

    if (xmlCharEncInFunc(stdoutEncoding, encodeOutBuff, encodeInBuff)
            >= 0) {
        result = xmlStrdup(xmlBufferContent(encodeOutBuff));
    } else {
        xsldbgGenericErrorFunc(QObject::tr("Encoding of text failed.\n"));
        return xmlStrdup(text); /*  panic,  return @text unchanged */
    }
    return result;
}


int filesSetEncoding(const char *encoding)
{
    int result = 0;

    if (encoding) {
        /* don't switch encoding unless we've found a valid encoding */
        xmlCharEncodingHandlerPtr tempEncoding =
                xmlFindCharEncodingHandler(encoding);
        if (tempEncoding) {
            filesSetEncoding(NULL);     /* re-use code to close encoding */
            stdoutEncoding = tempEncoding;
            result =
                    (xmlCharEncOutFunc(stdoutEncoding, encodeOutBuff, NULL)
                     >= 0);
            if (!result) {
                xmlCharEncCloseFunc(stdoutEncoding);
                stdoutEncoding = NULL;
                xsldbgGenericErrorFunc(QObject::tr("Unable to initialize encoding %1.").arg(xsldbgText(encoding)));
            } else
                optionsSetStringOption(OPTIONS_ENCODING, xsldbgText(encoding));
        } else {
            xsldbgGenericErrorFunc(QObject::tr("Invalid encoding %1.\n").arg(xsldbgText(encoding)));
        }
    } else {
        /* close encoding and use UTF-8 */
        if (stdoutEncoding)
            result = (xmlCharEncCloseFunc(stdoutEncoding) >= 0);
        else
            result = 1;
        stdoutEncoding = NULL;
    }
    return result;
}



int filesMoreFile(const xmlChar * fileName, FILE * file)
{
    int result = 0;
    int openedFile = 0;
    int lineCount;
    int reachedEof = 0;

    if (fileName && !file) {
#ifdef __riscos
        /* convert into RISC OS format a *nix style file name */
        fileName = (const xmlChar *) riscosfilename((char *) fileName);
#endif
        file = fopen((char *) fileName, "r");
        openedFile = 1;         /* since we opened the file we must close it */
    }
    if (file) {
        while (!feof(file) && !reachedEof) {
            lineCount = 0;
            while (!feof(file) && (lineCount < FILES_NO_LINES) &&
                   !reachedEof) {
                if (fgets((char *) filesBuffer, sizeof(filesBuffer), file)) {
                    xsltGenericError(xsltGenericErrorContext, "%s",
                                     filesBuffer);
                    lineCount++;
                } else {
                    reachedEof = 1;
                }
            }

            if (!feof(file) && !reachedEof) {
                xsldbgGenericErrorFunc(QObject::tr(" ----- more ---- \n"));
                fflush(stderr);
                if (fgets((char *) filesBuffer, sizeof(filesBuffer), stdin)) {
                    if ((*filesBuffer == 'q') || (*filesBuffer == 'Q'))
                        reachedEof = 1;
                } else {
                    reachedEof = 1;
                }
            }
        }

        if (openedFile) {
            fclose(file);
        }
        xsltGenericError(xsltGenericErrorContext, "\n");
        result = 1;
    } else {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: No valid file provided to print\n");
#endif
    }


    return result;
}


QString filesSearchResultsPath()
{
    QString result;

    if (!optionsGetStringOption(OPTIONS_SEARCH_RESULTS_PATH).isEmpty()) {
        result = optionsGetStringOption(OPTIONS_SEARCH_RESULTS_PATH);
    }

    if (result.isEmpty()) {
        result = QDir::homePath();
    }

#ifdef Q_OS_WIN32
    // for Win32 always return paths with '\'
    result = result.replace("/", "\\");
#endif

    return result;
}


xmlChar *filesURItoFileName(const xmlChar* uri)
{
    xmlChar *result = NULL;

    if (uri){
        if (xmlStrnCmp(uri, "file:/", 6 )){
            QUrl URL((const char*)uri);
            QString localPath;
#ifdef Q_OS_WIN32
            // must return path using correct file separator
            localPath = URL.toLocalFile().replace("/", "\\");
#else
            localPath = URL.toLocalFile();
#endif
            result = (xmlChar*) xmlStrdup((xmlChar*)localPath.toLocal8Bit().constData());
        }else{
            result = (xmlChar*) xmlStrdup(uri);
        }
    }

    return result;
}


/* TODO in xsldbg 3.x rename these to use files prefix */

void xsldbgUpdateFileDetails(xmlNodePtr node)
{
    if ((node != NULL) && (node->doc != NULL)) {
        if (currentUrl != NULL)
            xmlFree(currentUrl);
        currentUrl =  filesGetBaseUri(node);
        currentLineNo = xmlGetLineNo(node);
    }
}


int xsldbgLineNo(void)
{
    return currentLineNo;
}


xmlChar * xsldbgUrl(void)
{
    if (currentUrl != NULL)
        return (xmlChar *) xmlMemStrdup((char *) currentUrl);
    else
        return NULL;
}

QMap<QString, QStringList> _fileData;

void filesDataClear()
{
    _fileData.clear();
}

QStringList filesDataReadFile(const QString uri)
{
    QStringList result;
    QString filePath = uri;
    if (filePath.contains("file:/")) {
        QUrl url(uri);
        filePath = url.toLocalFile();
    }

    if (!_fileData.contains(filePath)) {
        QFile fileIn(filePath);
        if (fileIn.open(QIODevice::ReadOnly)) {
            QTextStream ts(&fileIn);
            QString firstLine = ts.readLine(255);
            //qWarning() << "First Line read:" << firstLine;
            QString regExText("^(<.xml[^?]+encoding\\s*=\\s*\")([^\"]+)(.*)$");
            QRegExp encodingName(regExText);
            QString readEncoding;
            if (encodingName.exactMatch(firstLine) && encodingName.captureCount() == 3) {
                //qWarning() << encodingName.capturedTexts() << encodingName.captureCount();
                readEncoding = encodingName.cap(2);
                QByteArray encName = readEncoding.toLocal8Bit();
                if (QTextCodec::availableCodecs().contains(encName)){
                    ts.setCodec(encName);
                    //qWarning() << "Current codec:" << ts.codec()->name() << "requested" << encName;
                } else {
                    qWarning() << "Did not find requested TextCodec" << encName;
                    qWarning() << "Available codecs are" << QTextCodec::availableCodecs();
                }
            }else {
                if (optionsGetIntOption(OPTIONS_VERBOSE)) {
                    qWarning() << "No match for encoding RegEx " << encodingName.pattern() << "in" << firstLine;
                }
            }
            ts.seek(0); // return to start of file

            QStringList readData = ts.readAll().split('\n');
            _fileData[filePath] = readData;
            fileIn.close();
            result = _fileData[filePath];
        }
    } else {
        result = _fileData[filePath];
    }

    return result;
}
