
#include "konnectorinfo.h"
#include "konnectorplugin.h"

using namespace KSync;

KonnectorPlugin::KonnectorPlugin( QObject* obj, const char* name,
                                  const QStringList& )
    : QObject( obj, name )
{
}

KonnectorPlugin::~KonnectorPlugin()
{
}

void KonnectorPlugin::setUDI( const QString& udi )
{
    m_udi = udi;
}

QString KonnectorPlugin::udi() const
{
    return m_udi;
}

void KonnectorPlugin::add( const QString& res )
{
    m_adds << res;
}

void KonnectorPlugin::remove( const QString& res )
{
    m_adds.remove( res );
}

QStringList KonnectorPlugin::resources() const
{
    return m_adds;
}

bool KonnectorPlugin::isConnected() const
{
    return  info().isConnected();
}

void KonnectorPlugin::progress( const Progress& prog )
{
    emit sig_progress( m_udi, prog );
}

void KonnectorPlugin::error( const Error& err )
{
    emit sig_error( m_udi, err );
}

ConfigWidget* KonnectorPlugin::configWidget( const Kapabilities&, QWidget*, const char* )
{
    return 0;
}

ConfigWidget* KonnectorPlugin::configWidget( QWidget*, const char* )
{
    return 0;
}

QStringList KonnectorPlugin::builtIn() const
{
    return QStringList();
}

void KonnectorPlugin::doWrite( Syncee::PtrList list )
{
    write( list );
}

#include "konnectorplugin.moc"
