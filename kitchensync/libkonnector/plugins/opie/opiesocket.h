
#ifndef opiesocket_h
#define opiesocket_h

#include <qcstring.h>
#include <qhostaddress.h>
#include <qobject.h>


#include <ksyncentry.h>
#include <konnector.h>

class OpieSocket : public QObject
{
Q_OBJECT
 public:
  OpieSocket(QObject *obj, const char *name );
  void setUser(const QString &user );
  void setPassword(const QString &pass );
  void setSrcIP( const QHostAddress & );
  void setDestIP(const QHostAddress & );
  void setMeta( bool );
  void startUp();
  bool startSync();
  bool isConnected();
  QByteArray retrFile(const QString &path );
  bool insertFile(const QString &fileName );
  void write(const QString &, const QByteArray & );
  void write(QPtrList<KSyncEntry> );
  void write(QValueList<KOperations> );
 signals:
  void sync( QPtrList<KSyncEntry> );
  void errorKonnector(int, QString );

 private:
  class OpieSocketPrivate;
  OpieSocketPrivate *d;

 private slots:
  void slotError(int );
  void slotConnected( );
  void slotClosed();
  void process(); // ready read
  void slotNOOP();
  void slotStartSync();
  void manageCall(const QString &line );
  void parseCategory(const QString &tempFile );
  QString categoryById(const QString &id, const QString &app = QString::null );
};


#endif

