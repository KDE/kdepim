
#include <konnectorplugin.h>

class OpiePlugin : public KonnectorPlugin
{
 public:
  OpiePlugin(QWidget *obj, const char *name);
  ~OpiePlugin();

  virtual void setUDI(const QString & );
  virtual QString udi()const;
  virtual Kapabilities capabilities( );
  virtual void setCapabilities( const Kapabilities &kaps );
  virtual bool startSync();
  virtual bool isConnected();
  virtual bool insertFile(const QString &fileName );
  virtual QByteArray retrFile(const QString &path );
 public slots:
  virtual void slotWrite(const QString &, const QByteArray & ) = 0;
  virtual void slotWrite(QPtrList<KSyncEntry> ) = 0;
  virtual void slotWrite(QValueList<KOperations> ) = 0;

};
