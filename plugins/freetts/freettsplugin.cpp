#include <kgenericfactory.h>

#include "freettsconf.h"
#include "freettsproc.h"

typedef K_TYPELIST_2( FreeTTSProc, FreeTTSConf ) FreeTTS;
K_EXPORT_COMPONENT_FACTORY( libfreettsplugin, KGenericFactory<FreeTTS> );

