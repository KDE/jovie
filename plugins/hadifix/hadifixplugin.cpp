#include <kgenericfactory.h>

#include "hadifixconf.h"
#include "hadifixproc.h"

typedef K_TYPELIST_2( HadifixProc, HadifixConf ) Hadifix;
K_EXPORT_COMPONENT_FACTORY( libhadifixplugin, KGenericFactory<Hadifix> );

