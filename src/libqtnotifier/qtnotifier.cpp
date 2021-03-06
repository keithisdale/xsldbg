
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

// For this this to be ignored by moc 

#if 0



#include "qtnotifier.h"
#include <libxsldbg/xsldbgthread.h>

QTNotifier xsldbgNotifier;


int notifyXsldbgApp(XsldbgMessageEnum type, const void *data)
{
    int result = 0;
    if (getThreadStatus() == XSLDBG_MSG_THREAD_NOTUSED){
      return 1;
    }

    /* clear the input ready flag as quickly as possible*/
     if ( getInputStatus() == XSLDBG_MSG_READ_INPUT)
     	  setInputReady(0);

    /* we'll do something with QT signals/slots to let the KDE world process the
     * state of the thread */
      xsldbgNotifier.doNotify(type, data);

    if (getThreadStatus() == XSLDBG_MSG_THREAD_STOP)
        xsldbgThreadCleanup();        /* thread has died so cleanup after it */

    result++; /* at the moment this function will always work */

    return result;
}




QTNotifier::QTNotifier(void)
  : QObject::QObject(0, "QTNotifier")
{
}


QTNotifier::~QTNotifier(void)
{
}

void QTNotifier::connectNotifier(QObject *listener){
  QObject::connect(&xsldbgNotifier, SIGNAL(notifyXsldbgApp(XsldbgMessageEnum, const void *)), 
				      listener, SLOT(processNotify(XsldbgMessageEnum, const void *)));
}

void QTNotifier::doNotify(XsldbgMessageEnum type, const void *data)
{
	emit notifyXsldbgApp(type, data);
}

#endif
