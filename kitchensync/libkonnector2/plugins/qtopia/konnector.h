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

        Kapabilities capabilities();
        void setCapabilities( const KSync::Kapabilities& );
        bool startSync();
        bool startBackup( const QString& path );
        bool startRestore( const QString& path );
        bool connectDevice();
        bool disconnectDevice();

        KonnectorInfo info()const;
        void download( const QString& );
    protected:
        QString metaId()const;
        QIconSet iconSet()const;
        QString iconName()const;


    public slots:
        void slotWrite( const Syncee::PtrList& );
    signals:
        void backup();
        void restore();

    private:
        class Private;
        /* for compiling purposes */
        Private* d;

    private slots:
        void slotSync(Syncee::PtrList );
        void slotError( const Error& );
        void slotProg( const Progress& );

    };
};


#endif
