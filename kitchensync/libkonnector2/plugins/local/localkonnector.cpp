#include <kgenericfactory.h>

#include <konnectorinfo.h>
#include <kapabilities.h>

#include "localkonnector.h"

typedef KGenericFactory<KSync::LocalKonnector, QObject> LocalKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( liblocalkonnector, LocalKonnectorPlugin );

using namespace KSync;


LocalKonnector::LocalKonnector( QObject* obj, const char* name,const QStringList )
    : Konnector( obj, name )
{
}

LocalKonnector::~LocalKonnector()
{
}

KSync::Kapabilities LocalKonnector::capabilities()
{
    KSync::Kapabilities caps;

    caps.setSupportMetaSyncing( false ); // we can meta sync
    caps.setSupportsPushSync( false ); // we can initialize the sync from here
    caps.setNeedsConnection( false ); // we need to have pppd running
    caps.setSupportsListDir( false ); // we will support that once there is API for it...
    caps.setNeedsIPs( false ); // we need the IP
    caps.setNeedsSrcIP( false ); // we do not bind to any address...
    caps.setNeedsDestIP( false ); // we need to know where to connect
    caps.setAutoHandle( false ); // we currently do not support auto handling
    caps.setNeedAuthentication( false ); // HennevL says we do not need that
    caps.setNeedsModelName( false ); // we need a name for our meta path!

    return caps;
}

void LocalKonnector::setCapabilities( const KSync::Kapabilities& caps )
{
}

bool LocalKonnector::startSync()
{
    return true;
}

bool LocalKonnector::startBackup(const QString& path)
{
    error ( StdError::backupNotSupported() );
    return false;
}

bool LocalKonnector::startRestore( const QString& path )
{
    error ( StdError::backupNotSupported() );
    return false;
}

bool LocalKonnector::connectDevice()
{
    return true;
}

bool LocalKonnector::disconnectDevice()
{
    return true;
}

KSync::KonnectorInfo LocalKonnector::info() const
{
    return KonnectorInfo( i18n("Dummy Konnector"),
                          QIconSet(),
                          QString::fromLatin1("LocalKonnector"),  // same as the .desktop file
                          "Dummy Konnector",
                          "agenda", // icon name
                          false );
}

void LocalKonnector::download( const QString& )
{
    error( StdError::downloadNotSupported() );
}

KSync::ConfigWidget *LocalKonnector::configWidget( const KSync::Kapabilities& cap, QWidget* parent,
                                                   const char* name )
{
  return 0;
}

KSync::ConfigWidget *LocalKonnector::configWidget( QWidget* parent, const char* name )
{
  return 0;
}

void LocalKonnector::write( Syncee::PtrList lst )
{
}


#include "localkonnector.moc"
