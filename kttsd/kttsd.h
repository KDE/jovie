/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.h
  KTTSD main class
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// $Id$

#ifndef _KTTSD_H_
#define _KTTSD_H_

#include <kdedmodule.h>

class KTTSD : public KDEDModule{
    Q_OBJECT
    K_DCOP

    public:
        KTTSD(const QCString &obj);
//        void idle();

    k_dcop:
        QString world();
//        void registerMe(const QCString &app);
};

#endif // _KTTSD_H_
