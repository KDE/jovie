/**************************************************************************
  festivalplugin.cpp
  Generating the factories so festival can be used as plug in.
  ------------------- 
  Copyright : (C) 2002 by José Pablo Ezequiel "Pupeno" Fernández
              (C) 2003 by José Pablo Ezequiel "Pupeno" Fernández
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

#include <kgenericfactory.h>

#include "festivalconf.h"
#include "festivalproc.h"

typedef K_TYPELIST_2( FestivalProc, FestivalConf ) Festival;
K_EXPORT_COMPONENT_FACTORY( libfestivalplugin, KGenericFactory<Festival> );

