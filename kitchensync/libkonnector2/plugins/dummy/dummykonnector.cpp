#include <kgenericfactory.h>

#include <konnectorinfo.h>
#include <kapabilities.h>

#include "dummykonnector.h"

typedef KGenericFactory<KSync::DummyKonnector, QObject>  DummyKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libdummykonnector,  DummyKonnectorPlugin );

using namespace KSync;


DummyKonnector::DummyKonnector( QObject* obj, const char* name,const QStringList )
    : Konnector( obj, name )
{
}

DummyKonnector::~DummyKonnector()
{
}

KSync::Kapabilities DummyKonnector::capabilities()
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

void DummyKonnector::setCapabilities( const KSync::Kapabilities& caps )
{
}

bool DummyKonnector::startSync()
{
    return true;
}

bool DummyKonnector::startBackup(const QString& path)
{
    error ( StdError::backupNotSupported() );
    return false;
}

bool DummyKonnector::startRestore( const QString& path )
{
    error ( StdError::backupNotSupported() );
    return false;
}

bool DummyKonnector::connectDevice()
{
    return true;
}

bool DummyKonnector::disconnectDevice()
{
    return true;
}

KSync::KonnectorInfo DummyKonnector::info() const
{
    return KonnectorInfo( i18n("Dummy Konnector"),
                          QIconSet(),
                          QString::fromLatin1("dummykonnector"),  // same as the .desktop file
                          "Dummy Konnector",
                          "agenda", // icon name
                          false );
}

void DummyKonnector::download( const QString& )
{
    error( StdError::downloadNotSupported() );
}

KSync::ConfigWidget *DummyKonnector::configWidget( const KSync::Kapabilities& cap, QWidget* parent,
                                                   const char* name )
{
  return 0;
}

KSync::ConfigWidget *DummyKonnector::configWidget( QWidget* parent, const char* name )
{
  return 0;
}

void DummyKonnector::write( Syncee::PtrList lst )
{
}


#include "dummykonnector.moc"
