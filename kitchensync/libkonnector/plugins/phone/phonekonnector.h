
#ifndef mobileplugin_h
#define mobileplugin_h

#include <qiconset.h>
#include <qptrlist.h>
#include <konnectorplugin.h>

class PhonePlugin : public KonnectorPlugin
{
Q_OBJECT
 public:
  PhonePlugin(QObject *obj, const char *name, const QStringList );
  ~PhonePlugin();

  virtual void setUDI(const QString & );
  virtual QString udi()const;
  virtual Kapabilities capabilities( );
  virtual void setCapabilities( const Kapabilities &kaps );
  virtual bool startSync();
  virtual bool connectDevice(); 
  virtual void disconnectDevice() { }
  virtual bool isConnected();
  virtual bool insertFile(const QString &fileName );
  virtual QByteArray retrFile(const QString &path );
  virtual KSyncEntry* retrEntry(const QString &path);
  virtual QString metaId()const;
  virtual QIconSet iconSet() const { return QIconSet(); };
  virtual QString id()const { return QString::fromLatin1("Phone-1"); };
 
 public slots:
  virtual void slotWrite(const QString &, const QByteArray & ) ;
  virtual void slotWrite(KSyncEntryList ) ;
  virtual void slotWrite(QValueList<KOperations> ) ;

 private:
  class PhonePluginPrivate;
  PhonePluginPrivate *d;

  
signals:
    void sync(const QString&, KSyncEntryList );
    void errorKonnector(const QString&, int, const QString& );
    void stateChanged( const QString&,  bool );
 private slots:
  void slotSync( KSyncEntryList );
  void slotErrorKonnector(int , QString );
    void slotChanged( bool );
};


#endif
