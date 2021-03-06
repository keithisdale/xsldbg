
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


#include <QApplication>
#include <QTimer>
#include <QFile>
#include <QTimerEvent>
#include <QEvent>
#include <QUrl>
#include <QDebug>
#include <QTextStream>
#include <QTextCodec>

#include <libxslt/xsltInternals.h>

#include "../libxsldbg/xsldbgevent.h"
#include "xsldbgdebuggerbase.h"
#include "../libxsldbg/arraylist.h"
#include "../libxsldbg/breakpoint.h"
#include "../libxsldbg/xsldbgmsg.h"
#include "../libxsldbg/xsldbgthread.h"
#include "../libxsldbg/options.h"
#include "../libxsldbg/files.h"

QString updateText;

XsldbgEventData::XsldbgEventData()
{
    int column;

    for (column = 0; column < XSLDBGEVENT_COLUMNS; column++){
        textValues[column].clear();
    }

    for (column = 0; column < XSLDBGEVENT_COLUMNS; column++){
        intValues[column] = -1;
    }

}

XsldbgEventData::~XsldbgEventData()
{
}

void XsldbgEventData::setText(int column, const QString & text)
{
    if ((column >= 0) && (column < XSLDBGEVENT_COLUMNS))
        textValues[column] = text;
}


QString XsldbgEventData::getText(int column)
{
    if ((column >= 0) && (column < XSLDBGEVENT_COLUMNS))
        return textValues[column];
    else
        return QString();
}


void XsldbgEventData::setInt(int column, int value)
{
    if ((column >= 0) && (column < XSLDBGEVENT_COLUMNS))
        intValues[column] = value;
}


int XsldbgEventData::getInt(int column)
{
    if ((column >= 0) && (column < XSLDBGEVENT_COLUMNS))
        return intValues[column];
    else
        return -1;
}


XsldbgEvent::XsldbgEvent(XsldbgMessageEnum type, const void *data)
    : QEvent(QEvent::User)
{
    XsldbgEventData *eventData;
    this->data = data;
    debugger = 0L;
    beenCreated = false;

    if (type == XSLDBG_MSG_LIST){             /* 23 : As list of messages  */
        notifyMessageListPtr msgList = (notifyMessageListPtr)data;
        void *msgData;
        /* If this is an included source message, which just continues a source message,
       we don't need to add an empty XsldbgEventData */
        if (msgList->type != XSLDBG_MSG_INCLUDED_SOURCE_CHANGED){
            /* add an empty event data item which indicates the start of a list */
            eventData  = new XsldbgEventData();
            if (eventData != 0L)
                list.append(eventData);
        }
        for (int index = 0; index < arrayListCount(msgList->list); index++){
            msgData = ::arrayListGet(msgList->list, index);
            eventData = createEventData(msgList->type, msgData);
            if (eventData != 0L)
                list.append(eventData);
        }
        ::arrayListFree(msgList->list);
        msgList->list = 0L;
        itemType = msgList->type;
    }else{
        eventData = createEventData(type, data);
        if (eventData != 0L)
            list.append(eventData);
        itemType = type;
    }

    beenCreated = true;
    /* remove any knowledge of orginal data */
    this->data = 0L;
}


XsldbgEvent::~XsldbgEvent()
{
    XsldbgEventData *eventData;
    for (int index = 0; index < list.count(); ++index){
        eventData = list.takeAt(index);
        delete eventData;
    }
}


XsldbgEventData *XsldbgEvent::createEventData(XsldbgMessageEnum type, const void *msgData)
{
    XsldbgEventData *result = new XsldbgEventData();

    if (result == 0L)
        return result;

    switch (type){

    case XSLDBG_MSG_THREAD_NOTUSED:  /* 0:  Thread are not to be used */
    case XSLDBG_MSG_THREAD_INIT:     /* 1: The xsldbg thread is initializing */
    case XSLDBG_MSG_THREAD_RUN:      /* 2: The xsldbg thread is running */
    case XSLDBG_MSG_THREAD_STOP:     /* 3: The xsldbg thread is about to die */
    case XSLDBG_MSG_THREAD_DEAD:     /* 4: The xsldbg thread died */
        /* we don't need to do anything extra with this message */
        break;

        /* input status ( once thread is running) */
    case XSLDBG_MSG_AWAITING_INPUT:  /* 5: Waiting for user input */
        /* we don't need to do anything extra with this message */
        break;

    case XSLDBG_MSG_READ_INPUT:      /* 6: Read user input */
        /* we don't need to do anything extra with this message */
        break;

    case XSLDBG_MSG_PROCESSING_INPUT:        /* 7: Processing user's request */
        /* we don't need to do anything extra with this message */
        break;

        /* provide more informatiom about state of xsldbg (optional) */
    case XSLDBG_MSG_PROCESSING_RESULT:       /* 8: An error occurred performing command
                                         * requested command */
        if (msgData != 0L){
            xsldbgErrorMsgPtr msg = (xsldbgErrorMsgPtr)msgData;
            if (msg->text)
                result->setText(0, XsldbgDebuggerBase::fromUTF8(msg->text));
        }
        break;

    case XSLDBG_MSG_LINE_CHANGED:    /* 9: Changed to new line number
                                 * ie a step */
        handleLineNoChanged(result, msgData);
        break;

    case XSLDBG_MSG_FILE_CHANGED:    /* 10: Changed selection for source/data/output file */
        // not used
        break;

    case XSLDBG_MSG_BREAKPOINT_CHANGED:      /* 11: Response to a showbreak command */
        handleBreakpointItem(result, msgData);
        break;

    case XSLDBG_MSG_PARAMETER_CHANGED:       /* 12: Response to showparam command */
        break;

    case XSLDBG_MSG_TEXTOUT:         /* 13 : Free form text from xsldg */
        /* this is going to be most common and it is so simple we can handle
       it here */
        result->setText(0, XsldbgDebuggerBase::fromUTF8((xmlChar*)msgData));
        break;

    case XSLDBG_MSG_FILEOUT:         /* 14 : Response to cat command, ie
                                 * Free form text in file */
        /* this is actualy the  file to load */
    {
        QUrl url(XsldbgDebuggerBase::fromUTF8FileName((xmlChar*)msgData));
        if (url.scheme() == QLatin1String("file")){
            qDebug() << "Remote path to temp file" << url.toString() << " is unsupported, unable to read message from xsldbg";
            break;
        }

        QString fileName = url.path();
        QString outputText;
        if (!fileName.isNull()){
            QFile file (fileName);
            if (file.open(QIODevice::ReadOnly)){
                QTextStream textFile(&file);
                QString textIn = "";
                textFile.setCodec(QTextCodec::codecForName("UTF8"));
                while (1){
                    textIn = textFile.readLine();
                    if (textIn.isNull())
                        break;
                    outputText.append(textIn).append("\n");
                }
                file.close();
            }
            outputText.append("\n");
            result->setText(0, outputText);
        }
    }
        break;

    case XSLDBG_MSG_LOCALVAR_CHANGED:        /* 15 : Response to locals command ie a
                                         * local variable */
        handleLocalVariableItem(result, msgData);
        break;

    case XSLDBG_MSG_GLOBALVAR_CHANGED:       /* 16 : Response to globals command
                                         * ie a global variable */
        handleGlobalVariableItem(result, msgData);
        break;

    case XSLDBG_MSG_TEMPLATE_CHANGED:        /* 17 : Response to templates command
                                         * ie template details */
        handleTemplateItem(result, msgData);
        break;

    case XSLDBG_MSG_SOURCE_CHANGED:  /* 18 : Response to stylesheets command,
                                 * a normal stylesheet */
        handleSourceItem(result, msgData);
        break;

    case XSLDBG_MSG_INCLUDED_SOURCE_CHANGED: /* 19: Response to stylesheets
                                         * command, a xmlNodeptr of
                                         * a included stylesheet */
        handleIncludedSourceItem(result, msgData);
        break;

    case XSLDBG_MSG_CALLSTACK_CHANGED:       /* 20: Response to where command,
                                         * ie a item on the call stack */
        handleCallStackItem(result, msgData);
        break;

    case XSLDBG_MSG_ENTITIY_CHANGED: /* 21: Response to entities
                                 * command */
        handleEntityItem(result, msgData);
        break;

    case XSLDBG_MSG_RESOLVE_CHANGE:  /* 22: Response to system or
                                 * public command */
        handleResolveItem(result, msgData);
        break;

    case XSLDBG_MSG_COMPLETED_TRANSFORMATION:    /* 26:  Completed XSLT exexcution */
        break;

    case XSLDBG_MSG_FIRST_LINE_REACHED:    /* 27:  Reached first break point after starting */
        break;
	
    case XSLDBG_MSG_FIRST_TEMPLATE_REACHED:    /* 28:  Reached first template break point reached after starting */
	break;
	
    default:
        qDebug("Unhandled type in createEventData %d", type);

    }
    return result;
}

void XsldbgEvent::emitMessage(XsldbgDebuggerBase *debugger)
{
    this->debugger = debugger;

    for (int index = 0; index < list.count(); ++index){
        emitMessage(list[index]);
    }

    /* make sure that we only temporarily set the value for debugger*/
    this->debugger = 0L;
}


void XsldbgEvent::emitMessage(XsldbgEventData *eventData)
{

    if ((eventData == 0L) || (debugger == 0L)){
        qDebug("emitMessage failed");
        if (eventData == 0L)
            qDebug("Event data == NULL");
        if (debugger == 0L)
            qDebug("Debugger == NULL");
        return;
    }

    /*
     Method use will end up like

    emit debugger->lineNoChanged("", 1, false);

  */

    switch (itemType){

    case XSLDBG_MSG_THREAD_NOTUSED:  /* 0:  Thread are not to be used */
    case XSLDBG_MSG_THREAD_INIT:     /* 1: The xsldbg thread is initializing */
    case XSLDBG_MSG_THREAD_RUN:      /* 2: The xsldbg thread is running */
        /* we don't need to do anything extra with this message */
        break;

    case XSLDBG_MSG_THREAD_STOP:     /* 3: The xsldbg thread is about to die */
    case XSLDBG_MSG_THREAD_DEAD:     /* 4: The xsldbg thread died */
        /* the debugger has stopped is about to stop */
        debugger->setInitialized(false);
        emit debugger->debuggerStopped();
        break;

        /* input status ( once thread is running) */
    case XSLDBG_MSG_AWAITING_INPUT:  /* 5: Waiting for user input */
        debugger->setInitialized(true); // if it is waiting input then system is ready

        if  ((getInputReady() == 0) && (debugger->commandQueue().count() > 0)){
            qDebug("Command queue not empty");
            QTimerEvent *e = new QTimerEvent(debugger->getUpdateTimerID());
            QApplication::postEvent(debugger, e);
        }
        if (!updateText.isEmpty()){
            debugger->queueMessage(updateText);
            updateText = "";
        }
        break;

    case XSLDBG_MSG_READ_INPUT:      /* 6: Read user input */
        /* we don't need to do anything extra with this message */
        break;

    case XSLDBG_MSG_PROCESSING_INPUT:        /* 7: Processing user's request */
        /* we don't need to do anything extra with this message */
        break;

        /* provide more informatiom about state of xsldbg (optional) */
    case XSLDBG_MSG_PROCESSING_RESULT:       /* 8: An error occurred performing command
                                         * requested command */
        debugger->setInitialized(true);
        if (!eventData->getText(0).isNull())
            updateText.append(eventData->getText(0));
        break;

    case XSLDBG_MSG_LINE_CHANGED:    /* 9: Changed to new line number
                                 * ie a step */
        handleLineNoChanged(eventData, 0L);
        break;

    case XSLDBG_MSG_FILE_CHANGED:    /* 10: Loaded source/data file */
        if (beenCreated == false){
            /* Empty data */
        }else{
            emit debugger->fileDetailsChanged();
        }
        break;

    case XSLDBG_MSG_BREAKPOINT_CHANGED:      /* 11: Response to a showbreak command */
        handleBreakpointItem(eventData, 0L);
        break;

    case XSLDBG_MSG_PARAMETER_CHANGED:       /* 12: Response to showparam command */
        break;

    case XSLDBG_MSG_TEXTOUT:         /* 13 : Free form text from xsldg */
        /* this is going to be most common and it is so simple we can handle
       it here */
        /*
      emit debugger->showMessage(eventData->getText(0));
      */
        if (!eventData->getText(0).isNull())
            updateText.append(eventData->getText(0));
        break;

    case XSLDBG_MSG_FILEOUT:         /* 14 : Response to cat command */
        if (!eventData->getText(0).isNull())
            updateText.append(eventData->getText(0));
        break;

    case XSLDBG_MSG_LOCALVAR_CHANGED:        /* 15 : Response to locals command ie a
                                         * local variable */
        handleLocalVariableItem(eventData, 0L);
        break;

    case XSLDBG_MSG_GLOBALVAR_CHANGED:       /* 16 : Response to globals command
                                         * ie a global variable */
        handleGlobalVariableItem(eventData, 0L);
        break;

    case XSLDBG_MSG_TEMPLATE_CHANGED:        /* 17 : Response to templates command
                                         * ie template details */
        handleTemplateItem(eventData, 0L);
        break;

    case XSLDBG_MSG_SOURCE_CHANGED:  /* 18 : Response to stylesheets command,
                                 * a normal stylesheet */
        handleSourceItem(eventData, 0L);
        break;

    case XSLDBG_MSG_INCLUDED_SOURCE_CHANGED: /* 19: Response to stylesheets
                                         * command, a xmlNodeptr of
                                         * a included stylesheet */
        handleIncludedSourceItem(eventData, 0L);
        break;

    case XSLDBG_MSG_CALLSTACK_CHANGED:       /* 20: Response to where command,
                                         * ie a item on the call stack */
        handleCallStackItem(eventData, 0L);
        break;

    case XSLDBG_MSG_ENTITIY_CHANGED: /* 21: Response to entities
                                 * command */
        handleEntityItem(eventData, 0L);
        break;

    case XSLDBG_MSG_RESOLVE_CHANGE:  /* 22: Response to system or
                                 * public command */
        handleResolveItem(eventData, 0L);
        break;

    case XSLDBG_MSG_COMPLETED_TRANSFORMATION:    /* 26:  Completed XSLT exexcution */
        emit debugger->completedTransform();
        break;

    case XSLDBG_MSG_FIRST_LINE_REACHED:    /* 27:  Reached first break point after starting */
        emit debugger->reachedFirstLine();
        break;

    case XSLDBG_MSG_FIRST_TEMPLATE_REACHED:    /* 28:  Reached first template break point reached after starting */
	emit debugger->reachedFirstTemplate();
	break;

    default:
        qDebug("Unhandled type in emitMessage %d", itemType);
    }
}


void XsldbgEvent::handleLineNoChanged(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (xsldbgUrl() != 0L){
                eventData->setText(0, XsldbgDebuggerBase::fromUTF8FileName(xsldbgUrl()));
                eventData->setInt(0, xsldbgLineNo());
                eventData->setInt(1, msgData != 0L);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->lineNoChanged(eventData->getText(0), eventData->getInt(0),
                                         eventData->getInt(1));
        }
    }
}


void XsldbgEvent::handleShowMessage(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                eventData->setText(0, XsldbgDebuggerBase::fromUTF8((xmlChar*)msgData));
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->showMessage(eventData->getText(0));
        }
    }
}


void XsldbgEvent::handleBreakpointItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                breakPointPtr breakItem = (breakPointPtr)msgData;
                /* set the file name*/
                eventData->setText(0, XsldbgDebuggerBase::fromUTF8FileName(breakItem->url));
                /* line number*/
                eventData->setInt(0, (int)breakItem->lineNo);

                /* templateName */
                eventData->setText(1, XsldbgDebuggerBase::fromUTF8(breakItem->templateName));

                /* modeName */
                eventData->setText(2, XsldbgDebuggerBase::fromUTF8(breakItem->modeName));
                /*  enabled state */
                eventData->setInt(1, (int)(breakItem->flags & BREAKPOINT_ENABLED));

                /* id value */
                eventData->setInt(2, (int)breakItem->id);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->breakpointItem(eventData->getText(0), /* file name*/
                                          eventData->getInt(0), /* line number*/
                                          eventData->getText(1), /* template name */
                                          eventData->getText(2), /* mode name */
                                          eventData->getInt(1),  /* enabled state*/
                                          eventData->getInt(2)  /* id*/);
        }
    }
}


void XsldbgEvent::handleGlobalVariableItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                xsltStackElemPtr item = (xsltStackElemPtr)msgData;
                QString name, fileName, selectXPath;
                int lineNumber = -1;

                /* variable name*/
                if (item->nameURI)
                    name = (XsldbgDebuggerBase::fromUTF8FileName(item->nameURI)).append(":");
                name.append(XsldbgDebuggerBase::fromUTF8(item->name));



                if (item->computed && item->comp && item->comp->inst && item->comp->inst->doc){
                    fileName = XsldbgDebuggerBase::fromUTF8FileName(item->comp->inst->doc->URL);
                    lineNumber= xmlGetLineNo(item->comp->inst);
                }

                if (item->select)
                    selectXPath   = XsldbgDebuggerBase::fromUTF8(item->select);

                eventData->setText(0, name);
                eventData->setText(1, "");
                eventData->setText(2, fileName);
                eventData->setText(3, selectXPath);
                eventData->setInt(0, lineNumber);
                eventData->setInt(1, 0);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->globalVariableItem(eventData->getText(0), /* variable name*/
                                        eventData->getText(2), /* file name */
                                        eventData->getInt(0), /* line number */
                                        eventData->getText(3) /* select XPath */);
        }
    }
}


void XsldbgEvent::handleLocalVariableItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                xsltStackElemPtr item = (xsltStackElemPtr)msgData;
                QString name, templateContext, fileName, selectXPath;
                int lineNumber = -1;
                xmlNodePtr varXmlNode;

                /* variable name */
                if (item->nameURI)
                    name = (XsldbgDebuggerBase::fromUTF8FileName(item->nameURI)).append(":");
                name.append(XsldbgDebuggerBase::fromUTF8(item->name));

                if (item->computed && item->comp && item->comp->inst){
                    varXmlNode = item->comp->inst;

                    /* try to find out what template this variable belongs to */
                    if (varXmlNode->parent && IS_XSLT_NAME(varXmlNode->parent, "template")) {
                        xmlChar* value = xmlGetProp(varXmlNode->parent, (xmlChar *) "name");
                        if (value) {
                            templateContext   = XsldbgDebuggerBase::fromUTF8(value);
                            xmlFree(value);
                        }else{
                            value = xmlGetProp(varXmlNode->parent, (xmlChar *) "match");
                            if (value) {
                                templateContext   = XsldbgDebuggerBase::fromUTF8(value);
                                xmlFree(value);
                            }
                        }
                    }

                    if (varXmlNode->doc)  {
                        fileName = XsldbgDebuggerBase::fromUTF8FileName(varXmlNode->doc->URL);
                        lineNumber = xmlGetLineNo(varXmlNode);
                    }

                    if (item->select)
                        selectXPath   = XsldbgDebuggerBase::fromUTF8(item->select);

                    eventData->setText(0, name);
                    eventData->setText(1, templateContext);
                    eventData->setText(2, fileName);
                    eventData->setText(3, selectXPath);
                    eventData->setInt(0, lineNumber);
                    eventData->setInt(1, 1);
                }
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->localVariableItem(eventData->getText(0), /* variable name*/
                                        eventData->getText(1), /* templatecontext*/
                                        eventData->getText(2), /* file name */
                                        eventData->getInt(0), /* line number */
                                        eventData->getText(3) /* select XPath */);
        }
    }
}


void XsldbgEvent::handleTemplateItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if(msgData != 0L){
                xsltTemplatePtr item = (xsltTemplatePtr)msgData;
                QString name, mode,  fileName;
                int lineNumber = -1;

                if (item->nameURI)
                    name.append(XsldbgDebuggerBase::fromUTF8FileName(item->nameURI)).append(":");

                if (item->name)
                    name.append(XsldbgDebuggerBase::fromUTF8(item->name));
                else if (item->match)
                    name.append(XsldbgDebuggerBase::fromUTF8(item->match));

                mode = XsldbgDebuggerBase::fromUTF8(item->mode);

                if (item->elem && item->elem->doc){
                    fileName = XsldbgDebuggerBase::fromUTF8FileName(item->elem->doc->URL);
                    lineNumber = xmlGetLineNo(item->elem);
                }
                eventData->setText(0, name);
                eventData->setText(1, mode);
                eventData->setText(2, fileName);
                eventData->setInt(0, lineNumber);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->templateItem(eventData->getText(0), /* tempalte name*/
                                        eventData->getText(1), /* mode*/
                                        eventData->getText(2), /* file name*/
                                        eventData->getInt(0) /* line number*/);
        }
    }

}


void XsldbgEvent::handleIncludedSourceItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                xmlNodePtr item = (xmlNodePtr)msgData;
                QString name,  fileName;
                int lineNumber = -1;

                if (item->doc)
                    name = XsldbgDebuggerBase::fromUTF8FileName(item->doc->URL);

                if (item->parent && item->parent->doc){
                    fileName = XsldbgDebuggerBase::fromUTF8FileName(item->parent->doc->URL);
                    lineNumber = xmlGetLineNo((xmlNodePtr)item->parent->doc);
                }
                eventData->setText(0, name);
                eventData->setText(1, fileName);
                eventData->setInt(0, lineNumber);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->sourceItem(eventData->getText(0), /* file name*/
                                      eventData->getText(1), /* parent file name*/
                                      eventData->getInt(0) /* parent line number*/);
        }
    }
}

void XsldbgEvent::handleSourceItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                xsltStylesheetPtr item = (xsltStylesheetPtr)msgData;
                QString name,  fileName;
                int lineNumber = -1;

                if (item->doc)
                    name = XsldbgDebuggerBase::fromUTF8FileName(item->doc->URL);

                if (item->parent && item->parent->doc){
                    fileName = XsldbgDebuggerBase::fromUTF8FileName(item->parent->doc->URL);
                    lineNumber = xmlGetLineNo((xmlNodePtr)item->parent->doc);
                }

                eventData->setText(0, name);
                eventData->setText(1, fileName);
                eventData->setInt(0, lineNumber);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->sourceItem(eventData->getText(0), /* name*/
                                      eventData->getText(1), /* parent file name*/
                                      eventData->getInt(0) /* parent line number*/);
        }
    }
}

void XsldbgEvent::handleCallStackItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                callPointPtr item = (callPointPtr)msgData;
                QString templateName,  fileName;
                int lineNumber = -1;

                /* template name */
                if (item->info){
                    templateName = XsldbgDebuggerBase::fromUTF8(item->info->templateName);
                    fileName = XsldbgDebuggerBase::fromUTF8FileName(item->info->url);
                    lineNumber = item->lineNo;
                }

                eventData->setText(0, templateName);
                eventData->setText(1, fileName);
                eventData->setInt(0, lineNumber);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->callStackItem(eventData->getText(0), /* template name*/
                                         eventData->getText(1), /* fileName */
                                         eventData->getInt(0) /* line number*/);
        }
    }
}


void XsldbgEvent::handleEntityItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                QString PublicID, SystemID, URI;

                entityInfoPtr info =  (entityInfoPtr)msgData;
                PublicID = XsldbgDebuggerBase::fromUTF8(info->PublicID);
                SystemID = XsldbgDebuggerBase::fromUTF8(info->SystemID);
                URI = XsldbgDebuggerBase::fromUTF8(info->ResolvedURI);
                eventData->setText(0, PublicID);
                eventData->setText(1, SystemID);
                eventData->setText(2, URI);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->entityItem(eventData->getText(0), /* PublicID*/
                                      eventData->getText(1), /* SystemID*/
                                      eventData->getText(2)  /* URI */);
        }
    }
}


void XsldbgEvent::handleResolveItem(XsldbgEventData *eventData, const  void *msgData)
{
    if (eventData != 0L){
        if (beenCreated == false){
            /* add our specific data to eventData*/
            if (msgData != 0L){
                QString URI = XsldbgDebuggerBase::fromUTF8FileName((const xmlChar*)msgData);

                eventData->setText(0, URI);
            }
        }else{
            /* emit the event data via debugger*/
            emit debugger->resolveItem(eventData->getText(0) /* URI */);
        }
    }
}

