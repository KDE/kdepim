#ifndef ksync_addressbookpart_h
#define ksync_addressbookpart_h

#include <kglobal.h>
#include <kabc/resource.h>

#include <manipulatorpart.h>

class KAboutData;
class KConfig;
class AddressBookConfigBase;
class KSimpleConfig;

namespace KABC {
    class AddressBook;
}

namespace KSync {

    class Syncee;
    class SyncEntry;
    class AddressBookSyncee;
    class AddressBookPart : public ManipulatorPart{
        Q_OBJECT
    public:
        AddressBookPart(QWidget* parent,  const char* name,
                        QObject* obj = 0, const char* name2 =0,
                        const QStringList& = QStringList() );
        virtual ~AddressBookPart();
        static KAboutData *createAboutData();

        QString type()const;
        QString name()const;
        QString description()const;
        QPixmap *pixmap();
        QString iconName()const;
        bool configIsVisible()const;
        bool canSync()const;
        void sync( const SynceeList& , SynceeList& );
    public:
        void slotConfigOk();
    private:
        AddressBookSyncee* load();
        void doMeta( Syncee*,  const QString& path );
        void writeMeta( KABC::AddressBook*, const QString& path );
        void save( AddressBookSyncee*, const QString& metapath );

        AddressBookSyncee* book2syncee( KABC::AddressBook* );
        QPixmap m_pixmap;
        QString m_path;
        bool m_evo:1;
        bool m_configured:1;
        KABC::Resource* resource( const QString& type );
    };
};

#endif
