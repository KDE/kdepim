#ifndef VR3_KONNECTOR_H
#define VR3_KONNECTOR_H

#include <qiconset.h>
#include <qptrlist.h>

#include <konnector.h>


namespace KSync {
    /**
     * This plugin gets loaded by the KonnectorManager
     * this is the key to the KonnectorWorld
     * we need to implement the interface to fully support it...
     */
    class AgendaPlugin : public KSync::Konnector
    {
        Q_OBJECT
    public:
        /**
         * @param parent the Parent Object
         * @param name the name
         * @param QStringList() a QStringList which is not used but necessary for KGenericFactory
         */
        AgendaPlugin( QObject* parent, const char* name, const QStringList = QStringList() );
        ~AgendaPlugin();

        /** return our capabilities() */
        KSync::Kapabilities capabilities();

        /**
         * the user configured this konnector
         * apply his preferecnes
         */
        void setCapabilities( const KSync::Kapabilities& );

        bool readSyncees();
        bool writeSyncees();

        bool connectDevice();
        bool disconnectDevice();

        /** the state and some informations */
        KSync::KonnectorInfo info()const;

        /** download a resource/url/foobar */
        void download( const QString& );

        /** configuration widgets */
        KSync::ConfigWidget* configWidget( const KSync::Kapabilities&, QWidget* parent, const char* name );
        KSync::ConfigWidget* configWidget( QWidget* parent, const char* name );

    private:
        AgendaSocket* m_socket;

    private slots:
        /** bridge from socket->plugin->konnectormanager */
        void slotSync( SynceeList );
        void slotError( const Error& error );
        void slotProg( const Progress& );

    };
}


#endif
