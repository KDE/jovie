/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.cpp
  KTTSD main class
  -------------------
  Copyright : (C) 2002-2003 by Jos�Pablo Ezequiel "Pupeno" Fern�dez
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernádez <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernádez <pupeno@kde.org>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

 // $Id$

#include <qcstring.h>

#include "kttsd.moc"

#include "kttsd.h"

class TestObject : public KShared{
    public:
        TestObject(const QCString &_app) : app(_app){
            qWarning("Creating TestObject belonging to '%s'", app.data());
        }
        ~TestObject(){
            qWarning("Destructing TestObject belonging to '%s'", app.data());
        }

    protected:
        QCString app;
};

KTTSD::KTTSD(const QCString &obj) : KDEDModule(obj){
    // Do stuff here
    setIdleTimeout(15); // 15 seconds idle timeout.
}

QString KTTSD::world(){
  return "Hello World!";
}

void KTTSD::idle(){
   qWarning("TestModule is idle.");
}

void KTTSD::registerMe(const QCString &app){
   insert(app, "kttsd", new TestObject(app));
   // When 'app' unregisters with DCOP, the TestObject will get deleted.
}

extern "C" {
    KDEDModule *create_KTTSD(const QCString &obj){
        return new KTTSD(obj);
    }
};
