/******************************************************************************
  -------------------
  Copyright : (C) 2003 by José Pablo Ezequiel Fernández <pupeno@pupeno.com>
  -------------------
  Original author: José Pablo Ezequiel Fernández <pupeno@pupeno.com>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#include <kgenericfactory.h>

#include "festivalcsconf.h"
#include "festivalcsproc.h"

typedef K_TYPELIST_2( FestivalCSProc, FestivalCSConf ) Festival;
K_EXPORT_COMPONENT_FACTORY( libfestivalcsplugin, KGenericFactory<Festival> );

