/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generating the factories so that GStreamer can be used as and audio plug in.
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

#include "gstreamerplayer.h"

K_EXPORT_COMPONENT_FACTORY( libkttsd_gstplugin, KGenericFactory<GStreamerPlayer>("kttsd") );

