
/***************************************************************************
              trace_cmds.cpp  - user commands to trace libxslt execution
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


#include "xsldbg.h"
#include "debugXSL.h"
#include "options.h"


/* -----------------------------------------

   Tracing related commands

  ------------------------------------------- */


int xslDbgShellTrace(xmlChar * arg)
{
    Q_UNUSED(arg);
    optionsSetIntOption(OPTIONS_TRACE, TRACE_ON);
    xslDebugStatus = DEBUG_TRACE;
    return 1;
}


int xslDbgShellWalk(xmlChar * arg)
{
    int result = 0;

    long speed = WALKSPEED_NORMAL;

    if (xmlStrLen(arg)
        && (!sscanf((char *) arg, "%ld", &speed) || ((speed < 0) || (speed > 9)))) {
        xsldbgGenericErrorFunc(QObject::tr("Error: Invalid arguments to command %1.\n").arg(QString("walk")));
        xsldbgGenericErrorFunc(QObject::tr("Warning: Assuming normal speed.\n"));
        speed = WALKSPEED_NORMAL;
    }
    result = 1;
    optionsSetIntOption(OPTIONS_WALK_SPEED, speed);
    xslDebugStatus = DEBUG_WALK;

    return result;
}
