#include <kgenericfactory.h>

#include "festivalcsconf.h"
#include "festivalcsproc.h"

typedef K_TYPELIST_2( FestivalCSProc, FestivalCSConf ) Festival;
K_EXPORT_COMPONENT_FACTORY( libfestivalcsplugin, KGenericFactory<Festival> );

