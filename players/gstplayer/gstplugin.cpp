/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generating the factories so that GStreamer can be used as and audio plug in.
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ******************************************************************************/

#include <kgenericfactory.h>

#include "gstreamerplayer.h"

K_EXPORT_COMPONENT_FACTORY( libkttsd_gstplugin, KGenericFactory<GStreamerPlayer>("kttsd") );

