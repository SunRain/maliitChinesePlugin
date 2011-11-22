#include <qdeclarative.h>

#include "plugin.h"
#include "engine.h"

void IMEnginePlugin::registerTypes( const char *uri ) {
    qmlRegisterType<engine::Engine>( uri, 1, 0, "Engine" ) ;
}

Q_EXPORT_PLUGIN2( imengineplugin, IMEnginePlugin ) ;

