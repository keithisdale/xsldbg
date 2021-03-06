
/***************************************************************************
    begin                : Sat Dec 22 2001
    copyright            : (C) 2001-2018 by Keith Isdale
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

#ifndef XSLCALLSTACK_H
#define XSLCALLSTACK_H


#include "breakpoint.h"

#ifdef WITH_XSLT_DEBUG
#ifndef WITH_XSLT_DEBUG_BREAKPOINTS
#define WITH_XSLT_DEBUG_BREAKPOINTS
#endif
#endif


typedef struct _callPointInfo callPointInfo;
typedef callPointInfo *callPointInfoPtr;
struct _callPointInfo {
    xmlChar *templateName, *templateURI;
    xmlChar *modeName, *modeURI;
    xmlChar *url;
    callPointInfoPtr next;
};

typedef struct _callPoint callPoint;
typedef callPoint *callPointPtr;
struct _callPoint {
    callPointInfoPtr info;
    long lineNo;
    callPointPtr next;
};


/**
 * Initialized the call stack
 *
 * @returns 1 if callStack has been initialized properly and all
 *               memory required has been obtained,
 *         0 otherwise
 */
int callStackInit(void);


/**
 *
 * Free all memory used by callStack
 */
void callStackFree(void);


/**
 * Add template "call" to call stack
 *
 * @param templ The current template being applied 
 * @param source The source node being processed
 *
 * @returns 1 on success, 
 *          0 otherwise
 */
int callStackAdd(xsltTemplatePtr templ, xmlNodePtr source);


/**
 * Drop the topmost item off the call stack
 */
void callStackDrop(void);


/**
 * Set the frame depth to step up to
 *
 * @returns 1 on success,
 *          0 otherwise
 *
 * @param depth The frame depth to step up to  
 *            0 < @p depth <= callDepth()
 */
int callStackStepup(int depth);


/**
 * Set the frame depth to step down to
 *
 * @returns 1 on success, 
 *          0 otherwise
 *
 * @param depth The frame depth to step down to  
 *            0 < @p depth <= callDepth()
 */
int callStackStepdown(int depth);


/**
 * Retrieve the call point at specified call depth 

 * @returns Non-null a if depth is valid,
 *          NULL otherwise 
 *
 * @param depth 0 < @p depth <= callDepth()
 */
callPointPtr callStackGet(int depth);


/**
 * Get the top item in the call stack
 *
 * @returns The top of the call stack
 */
callPointPtr callStackGetTop(void);


/**
 * @returns the depth of call stack
 */
int callStackGetDepth(void);

#endif
