
#ifndef opieplugin_h
#define opieplugin_h

#include <qptrlist.h>
#include <konnectorplugin.h>

class OpiePlugin : public KonnectorPlugin
{
Q_OBJECT
 public:
  OpiePlugin(QObject *obj, const char *name, const QStringList );
  ~OpiePlugin();

  virtual void setUDI(const QString & );
  virtual QString udi()const;
  virtual Kapabilities capabilities( );
  virtual void setCapabilities( const Kapabilities &kaps );
  virtual bool startSync();
  virtual bool isConnected();
  virtual bool insertFile(const QString &fileName );
  virtual QByteArray retrFile(const QString &path );
    virtual KSyncEntry* retrEntry(const QString &path) { return 0l;};
 public slots:
  virtual void slotWrite(const QString &, const QByteArray & ) ;
  virtual void slotWrite(QPtrList<KSyncEntry> ) ;
  virtual void slotWrite(QValueList<KOperations> ) ;

 private:
  class OpiePluginPrivate;
  OpiePluginPrivate *d;

 private slots:
  void slotSync( QPtrList<KSyncEntry> );
  void slotErrorKonnector(int , QString );
};


#endif
