/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generating the factories so Sentence Boundary Detection Filter can be used
  as plug in.
  -------------------
  Copyright : (C) 2005 Gary Cramblitt
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

#include "sbdconf.h"
#include "sbdproc.h"

typedef K_TYPELIST_2( SbdProc, SbdConf ) SbdPlugin;
K_EXPORT_COMPONENT_FACTORY( libkttsd_sbdplugin,
    KGenericFactory<SbdPlugin>("kttsd") );

