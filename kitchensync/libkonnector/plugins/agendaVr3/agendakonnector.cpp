#include "agendakonnector.h"

#include <qvaluelist.h>
#include <qpair.h>
#include <qptrlist.h>
#include <qhostaddress.h>
#include <kapabilities.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>

typedef KGenericFactory<KSync::AgendaPlugin, QObject>  agendaKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libagendakonnector,  agendaKonnectorPlugin );

using namespace KSync;

AgendaPlugin::AgendaPlugin(QObject *obj, const char *name, const QStringList ) : KonnectorPlugin(obj, name )
{
}
AgendaPlugin::~AgendaPlugin()
{
}
void AgendaPlugin::setUDI(const QString &udi )
{
}

QString AgendaPlugin::udi()const
{
}

QIconSet AgendaPlugin::iconSet()const
{
}

QString AgendaPlugin::iconName()const
{
};

Kapabilities AgendaPlugin::capabilities( )
{
}

void AgendaPlugin::setCapabilities( const Kapabilities &kaps )
{
}
bool AgendaPlugin::startSync()
{
}
bool AgendaPlugin::isConnected()
{
}
bool AgendaPlugin::insertFile(const QString &fileName )
{
}
QByteArray AgendaPlugin::retrFile(const QString &path )
{
}
void AgendaPlugin::slotWrite(const QString &path, const QByteArray &array )
{
}
void AgendaPlugin::slotWrite(Syncee::PtrList entry)
{
};
void AgendaPlugin::slotSync(Syncee::PtrList entry )
{
}
void AgendaPlugin::slotErrorKonnector( int mode, QString error )
{
}
void AgendaPlugin::slotWrite(KOperations::ValueList operations )
{
}
void AgendaPlugin::slotChanged( bool b)
{
}
Syncee* AgendaPlugin::retrEntry( const QString& path )
{
}
QString AgendaPlugin::metaId()const
{
}

#include "agendakonnector.moc"
