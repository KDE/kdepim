#ifndef OPIE_QTOPIA_KONNECTOR_H
#define OPIE_QTOPIA_KONNECTOR_H

#include <qiconset.h>
#include <qptrlist.h>

#include <konnectorplugin.h>

namespace KSync {
    class QtopiaPlugin : public KonnectorPlugin {
        Q_OBJECT
    public:
        QtopiaPlugin( QObject*, const char*,  const QStringList );
        ~QtopiaPlugin();

        void setUDI( const QString& udi );
        QString udi()const;
        Kapabilities capabilities();
        void setCapabilities( const KSync::Kapabilities& );
        bool startSync();
        bool startBackup( const QString& path );
        bool startRestore( const QString& path );
        bool connectDevice();
        void disconnectDevice();
        bool isConnected();
        bool insertFile( const QString& );
        QByteArray retrFile( const QString& );
        Syncee* retrEntry( const QString& path );
        QString metaId()const;
        QIconSet iconSet()const;
        QString iconName()const;

        QString id()const;
        ConfigWidget* configWidget( const Kapabilities&, QWidget*, const char* );
        ConfigWidget* configWidget( QWidget*, const char* );

    public slots:
        void slotWrite( const QString&, const QByteArray& );
        void slotWrite( Syncee::PtrList );
        void slotWrite( KOperations::ValueList );
    signals:
        void backup();
        void restore();
        void stateChanged( const QString&, bool );

    private:
        class Private;
        /* for compiling purposes */
        Private* d;

    private slots:
        void slotSync(Syncee::PtrList );
        void slotError(int, QString );
        void slotChanged(bool);

    };
};


#endif
