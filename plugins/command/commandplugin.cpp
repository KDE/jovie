/***************************************************** vim:set ts=4 sw=4 sts=4:
  -------------------
  Copyright : (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de>
  Copyright : (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gunnar Schmi Dt <kmouth@schmi-dt.de>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kgenericfactory.h>

#include "commandconf.h"
#include "commandproc.h"

typedef K_TYPELIST_2( CommandProc, CommandConf ) Command;
K_EXPORT_COMPONENT_FACTORY( libkttsd_commandplugin, KGenericFactory<Command>("kttsd") )

