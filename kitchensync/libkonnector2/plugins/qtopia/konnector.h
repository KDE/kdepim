#ifndef OPIE_QTOPIA_KONNECTOR_H
#define OPIE_QTOPIA_KONNECTOR_H

#include <qiconset.h>
#include <qptrlist.h>

#include <konnector.h>

namespace KSync {
    class QtopiaPlugin : public Konnector {
        Q_OBJECT
    public:
        QtopiaPlugin( QObject*, const char*,  const QStringList = QStringList() );
        ~QtopiaPlugin();

        Kapabilities capabilities();
        void setCapabilities( const KSync::Kapabilities& );
        bool readSyncees();
        bool writeSyncees();
        bool connectDevice();
        bool disconnectDevice();

        KonnectorInfo info()const;
        void download( const QString& );

        ConfigWidget* configWidget( const Kapabilities&, QWidget* parent, const char* name );
        ConfigWidget* configWidget( QWidget* parent, const char* name );
    protected:
        QString metaId()const;
        QIconSet iconSet()const;
        QString iconName()const;
    signals:
        void backup();
        void restore();

    private:
        class Private;
        /* for compiling purposes */
        Private* d;

    private slots:
        void slotSync(SynceeList );
        void slotError( const Error& );
        void slotProg( const Progress& );

    };
};


#endif
