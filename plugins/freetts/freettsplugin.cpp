/****************************************************************************
	Factory generation for the FreeTTS plugin so it can actually be used
	as such.	
	-------------------
	Copyright : (C) 2004 Paul Giannaros
	-------------------
	Original author: Paul Giannaros <ceruleanblaze@gmail.com>
	Current Maintainer: Paul Giannaros <ceruleanblaze@gmail.com>
 ******************************************************************************/

/***************************************************************************
 *																					*
 *	 This program is free software; you can redistribute it and/or modify	*
 *	 it under the terms of the GNU General Public License as published by	*
 *	 the Free Software Foundation; version 2 of the License.				 *
 *																					 *
 ***************************************************************************/

#include <kgenericfactory.h>

#include "freettsconf.h"
#include "freettsproc.h"

typedef K_TYPELIST_2( FreeTTSProc, FreeTTSConf ) FreeTTS;
K_EXPORT_COMPONENT_FACTORY( libkttsd_freettsplugin, KGenericFactory<FreeTTS>("kttsd_freetts"))

