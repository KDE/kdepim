
#ifndef opieplugin_h
#define opieplugin_h

#include <qiconset.h>
#include <qptrlist.h>
#include <konnectorplugin.h>

namespace KSync {

    class OpiePlugin : public KonnectorPlugin
    {
        Q_OBJECT
    public:
        OpiePlugin(QObject *obj, const char *name, const QStringList );
        ~OpiePlugin();

        virtual void setUDI(const QString & );
        virtual QString udi()const;
        virtual Kapabilities capabilities( );
        virtual void setCapabilities( const KSync::Kapabilities &kaps );
        virtual bool startSync();
        virtual bool connectDevice() { return true; }
        virtual void disconnectDevice() { }
        virtual bool isConnected();
        virtual bool insertFile(const QString &fileName );
        virtual QByteArray retrFile(const QString &path );
        virtual Syncee* retrEntry(const QString &path);
        virtual QString metaId()const;
        virtual QIconSet iconSet() const;
        virtual QString iconName()const;
        virtual QString id()const { return QString::fromLatin1("Opie-1"); };
    public slots:
        virtual void slotWrite(const QString &, const QByteArray & ) ;
        virtual void slotWrite(Syncee::PtrList ) ;
        virtual void slotWrite(KOperations::ValueList ) ;

    private:
        class OpiePluginPrivate;
        OpiePluginPrivate *d;
    signals:
        void sync(const QString&, Syncee::PtrList );
        void errorKonnector(const QString&, int, const QString& );
        void stateChanged( const QString&,  bool );
    private slots:
        void slotSync( Syncee::PtrList );
        void slotErrorKonnector(int , QString );
        void slotChanged( bool );
    };
};

#endif
