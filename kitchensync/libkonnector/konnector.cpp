#include "kdevice.h"
#include "konnectorplugin.h"
#include "konnector.h"
#include "kapabilities.h"

class KOperations{
public:
  KOperations(){ };

};

class Konnector::KonnectorPrivate{
public:
  KonnectorPrivate(){

  }
  QMap<QString, KonnectorPlugin*> m_konnectors;
  QValueList<KDevice> m_devices;
};

Konnector::Konnector( QObject *object, const char *name ) : QObject( object, name )
{
  // initialize
}
Konnector::~Konnector()
{

};
QValueList<KDevice> Konnector::query(const QString &category )
{
  QValueList<KDevice> dev;
  return dev;
}
QString Konnector::registerKonnector(const QString &Devie )
{
  return QString::null;
}
QString Konnector::registerKonnector(const KDevice &Device )
{
  return QString::null;
}
Kapabilities Konnector::capabilities (const QString& ) const 
{
  return Kapabilities();
}
void Konnector::setCapabilities(const QString &, const Kapabilities& )
{

}
QByteArray Konnector::file(const QString &udi, const QString &path )
{
  return QByteArray();
}
void Konnector::retrieveFile(const QString &udi, const QString &file )
{

}
void Konnector::write( const QString &udi, QPtrList<KSyncEntry> )
{

}
void Konnector::write( const QString &udi, QValueList<KOperations> )
{

}
void Konnector::write( const QString &udi, const QString &dest, const QByteArray & )
{

}
bool Konnector::isConnected(const QString &udi ){
  return false;
}






