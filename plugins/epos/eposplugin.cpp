/***************************************************** vim:set ts=4 sw=4 sts=4:
  eposplugin.cpp
  Generating the factories so Epos can be used as plug in.
  -------------------
  Copyright : (C) 2004 Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
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

#include "eposconf.h"
#include "eposproc.h"

typedef K_TYPELIST_2( EposProc, EposConf ) Epos;
K_EXPORT_COMPONENT_FACTORY( libkttsd_eposplugin, KGenericFactory<Epos>("plugin_Epos") );

