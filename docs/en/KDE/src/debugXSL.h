
/* *************************************************************************
                          debugXSL.h  -  describe the core function of xsldbg
                             -------------------
    begin                : Fri Dec 7 2001
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
 ************************************************************************ **/

/*
 * Orinal file : debugXML.h : This is a set of routines used for
 *    debugging the tree   produced by the XML parser.
 *
 * New file : debugXSL.h : Debug support version
 *
 * See Copyright for the status of this software.
 *
 * Daniel Veillard <daniel@veillard.com>
 *
 * Permission abtained to modify the LGPL'd code and extend to include 
 *   break points, inspections of stylesheet source, xml data, stylesheet
 *    variables   Keith Isdale <k_isdale@tpg.com.au>
 */

#ifndef DEBUGXSL_H
#define DEBUGXSL_H

/**
 * Provide a set of core commands for user
 *
 * @short debugger core logic 
 *
 * @author Keith Isdale <k_isdale@tpg.com.au> 
 */

#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/debugXML.h>

#include <libxslt/xsltInternals.h>
#include <libxml/xpath.h>
#include "breakpointInternals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_BUFFER_SIZE 500   /*used by xslDbgShell */


/****************************************************************
 *								*
 *	 The XSL shell related structures and functions		*
 *								*
 ****************************************************************/

/**
 * @returns 1 if @p c is a blank character
 *
 * @param c An UNICODE value (int)
 *
 * Macro to check the following production in the XML spec
 *
 * [3] S ::= (#x20 | #x9 | #xD | #xA)+
 */
#define IS_BLANK(c) (((c) == 0x20) || ((c) == 0x09) || ((c) == 0xA) ||	\
                     ((c) == 0x0D))

/**
 * Remove leading and trailing spaces off @p text
 *         stores result back into @p text
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param text A valid string with leading or trailing spaces
 */
    int trimString(xmlChar * text);


/**
 * Spit string by white space and put into @p out
 * 
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param textIn The string to split
 * @param maxStrings The max number of strings to put into @p out
 * @param out Is valid and at least the size of @p maxStrings
 */
    int splitString(xmlChar * textIn, int maxStrings, xmlChar ** out);


/**
 * Present the xsldbg shell to user and process entered commands
 *
 * @param source Current stylesheet instruction being executed
 * @param doc Current document node being processed
 * @param filename Not used
 * @param input The function to call to when reading commands from stdio
 * @param output Where to put the results
 * @param styleCtxt Is valid 
 */
    void xslDbgShell(xmlNodePtr source, xmlNodePtr doc,
                     xmlChar * filename,
                     xmlShellReadlineFunc input,
                     FILE * output, xsltTransformContextPtr styleCtxt);


/**
 * Print stylesheets that can be found in loaded stylsheet
 *
 * @param arg The stylesheets of interests, is NULL for all stylsheets
 *
 */
    void xslDbgPrintStyleSheets(xmlChar * arg);



/* -----------------------------------------

   Break Point related commands

  ------------------------------------------- */


/**
 * Set a "frame" break point either up or down from here
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param arg Is valid 
 * @param stepup If != 1 then we step up, otherwise step down
 */
    int xslDbgShellFrameBreak(xmlChar * arg, int stepup);


/**
 * Add break point specified by arg
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param arg Is valid
 * @param style Is valid
 * @param ctxt Is valid
 * 
 */
    int xslDbgShellBreak(xmlChar * arg, xsltStylesheetPtr style,
                         xsltTransformContextPtr ctxt);


/**
 * Delete break point specified by arg
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param arg Is valid
 */
    int xslDbgShellDelete(xmlChar * arg);


/**
 * Enable/disable break points via use of scan of break points
 *
 * @param payload Is valid xslBreakPointPtr
 * @param data Enable type, a pointer to an integer 
 *         for a value of 
 *              @li   1 enable break point
 *              @li   0 disable break point
 *              @li   -1 toggle enabling of break point 
 * @param name Not used
*/
    void xslDbgEnableBreakPoint(void *payload, void *data, xmlChar * name);


/**
 * Print data given by scan of break points 
 *
 * @param payload Is valid xslBreakPointPtr
 * @param data Not used
 * @param name Not used
*/
    void xslDbgPrintBreakPoint(void *payload, void *data, xmlChar * name);




/* -----------------------------------------

   Template related commands

  ------------------------------------------- */

/** 
 * Print out the list of template names found that match critieria   
 *
 * @param styleCtxt Is valid 
 * @param ctxt Not used
 * @param arg Not used
 * @param verbose If 1 then print extra messages about templates found,
 *            otherwise print normal messages only 
 * @param allFiles If 1 then look for all templates in stylsheets found in 
 *                 @p styleCtxt
 *             otherwise look in the stylesheet found by 
 *                 xslDebugBreak function
 */
    void xslDbgPrintTemplateNames(xsltTransformContextPtr styleCtxt,
                                  xmlShellCtxtPtr ctxt,
                                  xmlChar * arg, int verbose,
                                  int allFiles);

/**
 * This displays the templates in the same order as they are in the 
 *   stylesheet. If verbose is 1 then print more information
 *   For each template found @p templateCount is increased
 *   For each printed template @p printCount is increased
 *
 * @param templ Is valid
 * @param verbose Either 1 or 0
 * @param templateCount Is valid
 * @param printCount Is valid
 * @param templateName The template name to print, may be NULL
 */
    void xslDbgPrintTemplateHelper(xsltTemplatePtr templ, int verbose,
                                   int *templateCount, int *printCount,
                                   xmlChar * templateName);

/**
 * getTemplate:
 * 
 * @returns the last template node found, if any
 */
    xsltTemplatePtr getTemplate(void);


/* -----------------------------------------

   Node viewing related commands

  ------------------------------------------- */

/**
 * Print list of nodes in either ls or dir format
 *
 * @param ctxt The current shell context
 * @param arg What xpath to display
 * @param dir If 1 print in dir mode, 
 *        otherwise ls mode
 */
    void xslDbgShellPrintList(xmlShellCtxtPtr ctxt, xmlChar * arg,
                              int dir);


/** 
 * Print the result of an xpath expression. This can include variables
 *        if styleCtxt is not NULL
 *
 * @param styleCtxt Current stylesheet context
 * @param ctxt Current shell context
 * @param arg The xpath to print
 */
    void xslDbgShellCat(xsltTransformContextPtr styleCtxt,
                        xmlShellCtxtPtr ctxt, xmlChar * arg);


/**
 *  Print the value variable specified by args.
 *
 * @param styleCtxt The current stylesheet context 
 * @param arg The name of variable to look for '$' prefix is optional
 * @param type Is valid VariableTypeEnum
 */
    void xslDbgShellPrintVariable(xsltTransformContextPtr styleCtxt,
                                  xmlChar * arg, VariableTypeEnum type);




/* -----------------------------------------

   Operating system related commands

  ------------------------------------------- */

/**
 * @returns 1 if able to change xsldbg working direcorty to @p path
 *          0 otherwise
 *
 * @param path Operating system path(directory) to change to
 */
    int xslDbgShellChangeWd(const xmlChar * path);


/**
 * @returns 1 if able to execute command @p name,
 *          0 otherwise
 *
 * @param name The name of command string to be executed 
 *            by operating system shell
 * @param verbose If 1 then print extra debugging messages,
 *            normal messages otherwise
 */
    int xslDbgShellExecute(xmlChar * name, int verbose);




/* -----------------------------------------

   libxslt parameter related commands

  ------------------------------------------- */

/* 
 * Add a parameter to be sent to libxslt later on
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param arg A string comprised of two words separated by
 *          one or more spaces. 
 */
    int xslDbgShellAddParam(xmlChar * arg);


/**
 *
 * @returns 1 if able to delete parameter @p name,
 *          0 otherwise
 *
 * @param arg A single white space trimmed libxslt parameter number to look for
 */
    int xslDbgShellDelParam(xmlChar * arg);


/**
 * Print list of current paramters
 *
 * @returns 1 on success,
 *         0 otherwise
 *
 * @param arg Not used
 */
    int xslDbgShellShowParam(xmlChar * arg);


/* -----------------------------------------

   Tracing related commands

  ------------------------------------------- */

/**
 * Start the tracing of the stylesheet. First need to restart it.
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param arg Not used
 */
    int xslDbgShellTrace(xmlChar * arg);


/**
 * Start walking through the stylesheet.
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param arg An interger between 0 and 9 indicate the speed of walk
 */
    int xslDbgShellWalk(xmlChar * arg);



/* -----------------------------------------
   
   Seach related commands
   
  ------------------------------------------- */

/**
 * @returns 1 if able to run query with @p arg, 
 *          0 otherwise
 *
 * @param styleCtxt Is valid
 * @param style Is valid
 * @param arg The xpath query to use for searching dataBase
 */
    int xslDbgShellSearch(xsltTransformContextPtr styleCtxt,
                          xsltStylesheetPtr style, xmlChar * arg);


#ifdef __cplusplus
}
#endif
#endif
 /* DEBUGXSL_H */