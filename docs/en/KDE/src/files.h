
/***************************************************************************
                          files.h  -  define file related functions
                             -------------------
    begin                : Sat Nov 10 2001
    copyright            : (C) 2001 by Keith Isdale
    email                : k_isdale@tpg.com.au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/








/**
 * Provide a file support
 *
 * @short file support
 *
 * @author Keith Isdale <k_isdale@tpg.com.au> 
 */






    extern FILE *terminalIO;



    typedef struct _entityInfo entityInfo;
    typedef entityInfo *entityInfoPtr;
    struct _entityInfo {
        xmlChar *SystemID;
        xmlChar *PublicID;
    };



  /*-----------------------------------------------------------
                               File commands
    -----------------------------------------------------------*/






    int xslDbgEntities(void);





  /**
   * Print what a system file @p arg maps to via the current xml catalog
   *
   * @param arg Is valid in UTF-8
   * 
   * @returns 1 on sucess,
   *          0 otherwise
   */


    int xslDbgSystem(const xmlChar * arg);





  /**
   * Print what a public ID @p arg maps to via the current xml catalog
   *
   * @param arg Is valid PublicID in UTF-8
   * 
   * @returns 1 on sucess,
   *          0 otherwise
   */


    int xslDbgPublic(const xmlChar * arg);





  /**
   * Set current encoding to use for output to standard output
   *
   * @param arg Is valid encoding supported by libxml2
   *
   *
   * Returns 1 on sucess,
   */


    int xslDbgEncoding(xmlChar * arg);




  /*-----------------------------------------------------------
    General function for working with files
    -----------------------------------------------------------*/




  /**
   * Fixes the nodes from firstNode to lastNode so that debugging can occur
   *
   * @param uri Is valid as provided by libxslt
   * @param firstNode Is valid
   * @param lastNode Is Valid
   */


    void filesEntityRef(xmlEntityPtr ent, xmlNodePtr firstNode,
                        xmlNodePtr lastNode);





  /**
   * Return the list entity names used for documents loaded
   *
   * @returns The list entity names used for documents loaded
   */


    arrayListPtr filesEntityList(void);





  /**
   * Set the base uri for this node. Function is used when xml file
   *    has external entities in its DTD
   * 
   * @param node Is valid and has a doc parent
   * @param uri Is Valid
   * 
   * @returns 1 if successful,
   *          0 otherwise
   */


    int filesSetBaseUri(xmlNodePtr node, const xmlChar * uri);





  /**
   * Get a copy of the base uri for this node. Function is most usefull 
   *  used when xml file  has external entities in its DTD
   * 
   * @param node : Is valid and has a doc parent
   * 
   * @returns The a copy of the base uri for this node,
   *          NULL otherwise
   */


    xmlChar *filesGetBaseUri(xmlNodePtr node);





  /**
   * Return the name of tempfile requested.
   * @param fleNumber : Number of temp file required
   *     where @p fileNumber is 
   *      0 : file name used by cat command
   *      1 : file name used by profiling output
   *
   *  This is a platform specific interface
   *
   * Returns The name of temp file to be used for temporary results if sucessful,
   *         NULL otherwise
   */


    const char *filesTempFileName(int fileNumber);





  /**
   * Load the catalogs specifed by OPTIONS_CATALOG_NAMES if 
   *      OPTIONS_CATALOGS is enabled
   *
   * @returns 1 if sucessful
   *          0 otherwise   
   */


    int filesLoadCatalogs(void);





  /**
   * Return  A  string of converted @text
   *
   * @param text Is valid, text to translate from UTF-8, 
   *
   * Returns  A  string of converted @text, may be NULL
   */


    xmlChar *filesEncode(const xmlChar * text);





  /**
   * Return  A  string of converted @text
   *
   * @param test Is valid, text to translate from current encoding to UTF-8, 
   *
   * Returns  A  string of converted @text, may be NULL
   */


    xmlChar *filesDecode(const xmlChar * text);





  /**
   * Opens encoding for all standard output to @p encoding. If  @p encoding 
   *        is NULL then close current encoding and use UTF-8 as output encoding
   *
   * @param encoding Is a valid encoding supported by the iconv library or NULL
   *
   * Returns 1 if successful in setting the encoding of all standard output
   *           to @p encoding
   *         0 otherwise
   */


    int filesSetEncoding(const char *encoding);




    /* used by filesLoadXmlFile, filesFreeXmlFile functions */
    enum FileTypeEnum {
        FILES_XMLFILE_TYPE = 100,       /* pick a unique starting point */
        FILES_SOURCEFILE_TYPE,
        FILES_TEMPORARYFILE_TYPE
    };






  /**
   * Open communications to the terminal device @p device
   *
   * @param device Terminal to redirect i/o to , will not work under win32
   *
   * @returns 1 if sucessful
   *          0 otherwise
   */


    int openTerminal(xmlChar * device);





  /**
   * Try to find a matching stylesheet name
   * Sets the values in @p searchinf depending on outcome of search
   *
   * @param searchInf Is valid
   */


    void guessStylesheetName(searchInfoPtr searchInf);





  /**
   * Return the base path for the top stylesheet ie
   *        ie URL minus the actual file name
   *
   * @returns The base path for the top stylesheet ie
   *        ie URL minus the actual file name
   */


    xmlChar *stylePath(void);





  /** 
   * Return the working directory as set by changeDir function
   *
   * @return the working directory as set by changeDir function
   */


    xmlChar *workingPath(void);





  /**
   * Change working directory to path 
   *
   * @param path The operating system path(directory) to adopt as 
   *         new working directory
   *
   * @returns 1 on success,
   *          0 otherwise
   */


    int changeDir(const xmlChar * path);





  /**
   * Load specified file type, freeing any memory previously used 
   *
   * @returns 1 on success,
   *         0 otherwise 
   *
   * @param path The xml file to load
   * @param fileType A valid FileTypeEnum
   */


    int filesLoadXmlFile(const xmlChar * path, FileTypeEnum fileType);





  /**
   * Free memory associated with the xml file 
   *
   * @returns 1 on success,
   *         0 otherwise
   *
   * @param fileType : A valid FileTypeEnum
   */


    int filesFreeXmlFile(FileTypeEnum fileType);





  /**
   * Return the topmost stylesheet 
   *
   * @returns Non-null on success,
   *         NULL otherwise
   */


    xsltStylesheetPtr filesGetStylesheet(void);





  /** 
   * Return the current "temporary" document
   *
   * @returns non-null on success,
   *          NULL otherwise
   */


    xmlDocPtr filesGetTemporaryDoc(void);





  /** 
   * Return the main docment
   *
   * @returns the main document
   */


    xmlDocPtr filesGetMainDoc(void);





  /**
   * @returns 1 if stylesheet or its xml data file has been "flaged" as reloaded,
   *         0 otherwise
   *
   * @param reloaded If = -1 then ignore @p reloaded
   *             otherwise change the status of files to value of @p reloaded
   */


    int filesReloaded(int reloaded);





  /**
   * Initialize the file module
   *
   * @returns 1 on success,
   *          0 otherwise
   */


    int filesInit(void);





  /**
   * Free memory used by file related structures
   */


    void filesFree(void);







    int filesIsSourceFile(xmlChar * fileName);




  /*-----------------------------------------------------------
    Platform specific file functions
    -----------------------------------------------------------*/





  /**
   * Intialize the platform specific files module
   *
   *  This is a platform specific interface
   *
   * @returns 1 if sucessful
   *          0 otherwise  
   */


    int filesPlatformInit(void);





  /**
   * Free memory used by the platform specific files module
   *
   *  This is a platform specific interface
   *
   */


    void filesPlatformFree(void);



