#include <kgenericfactory.h>

#include "commandconf.h"
#include "commandproc.h"

typedef K_TYPELIST_2( CommandProc, CommandConf ) Command;
K_EXPORT_COMPONENT_FACTORY( libkttsd_commandplugin, KGenericFactory<Command>("plugin_Command") );

