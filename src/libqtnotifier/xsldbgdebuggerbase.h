
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


#ifndef XSLDBGDEBUGGERBASE_H
#define XSLDBGDEBUGGERBASE_H

#if defined WIN32
#  include <libxsldbg/xsldbgwin32config.h>
#else
#  ifndef  XSLDBG_SO_API
#     define XSLDBG_SO_API
#  endif
#endif

#include <QObject>
#include <QStringList>
#include <libxml/tree.h>

class XsldbgEvent;
class QApplication;
class QStringList;

/** 
    We delegate the task of emitting our signals to XsldbgEvent , so 
    Xsldbg must become our friend
*/
class XsldbgDebuggerBase : public QObject
{
    Q_OBJECT

    friend class XsldbgEvent;

 public:
  XsldbgDebuggerBase(void);
  ~XsldbgDebuggerBase(void);

  void setInitialized(bool state) {initialized = state; };
  bool getInitialized(void) {return initialized ;};
void setUpdateTimerID(int ID) { updateTimerID = ID;}; int getUpdateTimerID(void) {return updateTimerID ;};


  /** Convert from libxslt UTF8 to a QString */	
  static QString fromUTF8(const char *text);
  //static QString fromUTF8FileName(const char *text);
	
  /** Convert from libxslt UTF8 to a QString */	
  static QString fromUTF8(const xmlChar *text);
  static QString fromUTF8FileName(const xmlChar *text);

  /* list of command yet to be processed */
  QStringList commandQueue(void) {return _commandQueue ;};
  
  void queueMessage(const QString &text);

  /* helper function to handle differences between URI's in Qt/KDE and libxml2 */
  static QString fixLocalPaths(const QString & file);
  static bool isLocalPath(const QString & file);

protected:
    QString updateText;

 private:
  bool initialized;
  int updateTimerID;
  QStringList _commandQueue;	

signals: // Signals
   /** line number and/or file name changed */
   void lineNoChanged(QString fileName, int lineNumber, bool /* breakpoint */);

   /** emitted when first line is reached after starting**/
   void reachedFirstLine();

   /** emitted when first template is reached after starting**/
   void reachedFirstTemplate();
   
   /** Show a message in debugger window */
   void showMessage(QString /* msg*/);

   /** Add breakpoint to view, First parameter is QString()
       to indicate start of breakpoint list notfication */
   void breakpointItem(QString fileName, int lineNumber,
               QString templateName, QString modeName,
              bool enabled, int id);

  /** Add global variable to view, First parameter is QString()
      to indicate start of global variable list notfication */
  void globalVariableItem(QString name, QString fileName, int lineNumber, QString XPath);

  /** Add local variable to view, First parameter is QString()
      to indicate start of local variable list notfication */
  void localVariableItem(QString name, QString templateContext,
             QString fileName, int lineNumber, QString XPath);
  
  /** Add template to view, First parameter is QString()
      to indicate start of template list notfication */
  void templateItem(QString name, QString mode, QString fileName, int lineNumber);

  /** Add source to view, First parameter is QString()
      to indicate start of source list notfication */
  void sourceItem(QString fileName, QString parentFileName, int lineNumber);

  /** Add parameter to view, First parameter is QString()
      to indicate start of parameter list notfication */
  void parameterItem(QString name, QString value);

  /** Add callStack to view, First parameter is QString()
      to indicate start of callstack list notfication */
  void callStackItem(QString templateName, QString fileName, int lineNumber);
	
  /** Add entity to view, First parameter is QString()
      to indicate start of entity list notfication */
  void entityItem(QString SystemID,  QString PublicID, QString URI);
	
  /* Show the URI for SystemID or PublicID requested */
  void resolveItem(QString URI);
  
  /* Display a integer option value First parameter is QString()
     to indicate start of option list notification */
  void intOptionItem(QString name, int value);

  /* Display a string option value. First parameter is QString()
     to indicate start of option list notification */
  void stringOptionItem(QString name, QString value);

  /* Cause the names for source, data and output files to be reload from xsldbg */
  void fileDetailsChanged();

  void debuggerReady();
  void debuggerStopped();
  void completedTransform();

};

#endif
