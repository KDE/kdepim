
#ifndef agendaplugin_h
#define agendaplugin_h

#include <qiconset.h>
#include <qptrlist.h>
#include <konnectorplugin.h>

namespace KSync {

    class AgendaPlugin : public KonnectorPlugin
    {
        Q_OBJECT
    public:
        AgendaPlugin(QObject *obj, const char *name, const QStringList );
        ~AgendaPlugin();

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
        /* FIXME get rid of inline without inline */
        virtual QString id()const { return QString::fromLatin1("Agenda"); };
        virtual ConfigWidget* configWidget( const Kapabilities&, QWidget*,  const char* ) { return 0l; }
        virtual ConfigWidget* configWidget( QWidget*, const char* ) { return 0l; }

    public slots:
        virtual void slotWrite(const QString &, const QByteArray & ) ;
        virtual void slotWrite(Syncee::PtrList ) ;
        virtual void slotWrite(KOperations::ValueList ) ;

    private:
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
