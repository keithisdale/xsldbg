
/***************************************************************************
                          qtnotifier.h  - notify the qt app of changes
                             -------------------
    begin                : Sun Dec 23 2001
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

#ifndef QTNOTIFIER
#define QTNOTIFIER

#include <libxsldbg/xsldbgnotifier.h>
#include <qobject.h>


class QTNotifier : public QObject
{
 Q_OBJECT

 public: 
  QTNotifier();
  ~QTNotifier(); 

  void connectNotifier(QObject *listener);

 public slots:
  void doNotify(XsldbgMessageEnum type, const void *data);

  signals:
   void notifyXsldbgApp(XsldbgMessageEnum, const void *);   
};

#endif
