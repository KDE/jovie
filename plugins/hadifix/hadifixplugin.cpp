/***************************************************************************
    begin                : Mon Okt 14 2002
    copyright            : (C) 2002 by Gunnar Schmi Dt
    email                : gunnar@schmi-dt.de
    current mainainer:   : Gary Cramblitt <garycramblitt@comcast.net> 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <kgenericfactory.h>

#include "hadifixconf.h"
#include "hadifixproc.h"

typedef K_TYPELIST_2( HadifixProc, HadifixConf ) Hadifix;
K_EXPORT_COMPONENT_FACTORY( libkttsd_hadifixplugin, KGenericFactory<Hadifix> );

