
#include <qcheckbox.h>
#include <qdir.h>
#include <qlineedit.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kurlrequester.h>

#include <kabc/stdaddressbook.h>
#include <kabc/resourcefile.h>

#include <kparts/genericfactory.h>

#include <kabc/resourcefile.h>

#include <ksync_mainwindow.h>

#include <addressbooksyncee.h>

#include "addressbookbase.h"
#include "ksync_addressbookpart.h"

typedef KParts::GenericFactory< KSync::AddressBookPart> AddressBookPartFactory;
K_EXPORT_COMPONENT_FACTORY( libaddressbookpart,  AddressBookPartFactory );

using namespace KSync;



AddressBookPart::AddressBookPart( QWidget* parent,  const char* name,
                                  QObject* obj,  const char* name2,
                                  const QStringList & )
    : ManipulatorPart( parent ? parent : obj ,  name )
{
    setInstance( AddressBookPartFactory::instance() );
    m_pixmap = KGlobal::iconLoader()->loadIcon("kaddressbook",  KIcon::Desktop,  48 );
    m_widget = 0;
}
AddressBookPart::~AddressBookPart()
{
}
KAboutData *AddressBookPart::createAboutData()
{
  return new KAboutData("KSyncAddressBookPart", I18N_NOOP("Sync AddressBook Part"), "0.0" );
}
QPixmap* AddressBookPart::pixmap()
{
    return &m_pixmap;
}
QWidget* AddressBookPart::widget()
{
    return 0l;
}
QWidget* AddressBookPart::configWidget()
{
    Profile prof = core()->currentProfile();
    QString path = prof.path("AddressBook");

    m_widget = new AddressBookConfigBase();
    if ( QString::fromLatin1("evolution") == path ) {
        m_widget->ckbEvo->setChecked( true );
    }else {
        m_widget->ckbEvo->setChecked( false );
        m_widget->urlReq->setURL( path );
    }

    return m_widget;
}
/*
 * SYnc it aye?
 * 1. get the currentProfile + Konnector
 * 2. get the paths + the path to the meta data
 * 3. search our AddressBookSyncee
 * 4. load the File
 * 5. do meta
 * 6. sync
 * 7. write Meta
 * 8. save
 * 9. write back
 * 10. party
 */
void AddressBookPart::processEntry( const Syncee::PtrList& in,
                                    Syncee::PtrList& out )
{
    kdDebug() << "processEntry in AddressBookPart aye" << endl;
    /* 1. */
    Profile prof = core()->currentProfile();
    KonnectorProfile kon = core()->konnectorProfile();

    /* 2. */
    QString path = prof.path("AddressBook");
    QString meta = kon.uid() + "/" + prof.uid() + "addressbook.rc";
    bool met = kon.kapabilities().isMetaSyncingEnabled();
    kdDebug() << "Is meta syncing enabled? " << met << endl;

    /* 3. */
    Syncee* syncee = 0l;
    AddressBookSyncee* aBook=0l;
    QPtrListIterator<Syncee> syncIt( in );
    for ( ; syncIt.current(); ++syncIt ) {
        syncee = syncIt.current();
        if (syncee->type() == QString::fromLatin1("AddressBookSyncee") ) {
	    kdDebug() << "Found our syncee" << endl;
            aBook = (AddressBookSyncee*) syncee;
            break;
        }

    }
    if (!aBook) return;

    /* 4. */
    AddressBookSyncee* ourbook;
    ourbook = load(path );

    /* 5 */
    if (met)
        doMeta( ourbook, meta );

    /* 6. */
    Syncer sync( core()->syncUi(), core()->syncAlgorithm() );
    sync.addSyncee( aBook );
    sync.addSyncee( ourbook );
    sync.sync();

    /* 7. */
    if (met)
        writeMeta( ourbook, meta );

    /* 8. */
    save( ourbook, path );

    /* writeback */
    out.append( ourbook );
}
void AddressBookPart::slotConfigOk()
{
    Profile prof = core()->currentProfile();
    if ( m_widget->ckbEvo->isChecked() ) {
        prof.setPath( "AddressBook", "evolution" );
    }else {
        prof.setPath("AddressBook",m_widget->urlReq->url() );
    }
    core()->profileManager()->replaceProfile( prof );
    core()->profileManager()->setCurrentProfile( prof );


}
/*
 * let's load it
 * if path is empty or default Take KStdAddressBook
 * otherwise load the file
 */
AddressBookSyncee* AddressBookPart::load( const QString& path ) {
    kdDebug() << "load abook" << path << endl;
    KABC::AddressBook* book;
    AddressBookSyncee* sync;
    if ( pathIsDefault( path ) ) {
        kdDebug() << "use default one " << endl;
        book =  KABC::StdAddressBook::self();
        sync = book2syncee( book );
        return sync;
    }else {
        kdDebug() << "use non default" << endl;
        book = new KABC::AddressBook();
        KABC::ResourceFile *res = new KABC::ResourceFile( book, path );
        book->addResource(res);
        book->load();
        sync = book2syncee( book );
        delete book;
        return sync;
    }
}
void AddressBookPart::doMeta( Syncee* syncee, const QString& path ) {
    kdDebug() << "Do Meta" << endl;
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/konnector-" + path;
    if (!QFile::exists( str ) ) {
        kdDebug() << "Path does not exist ->First Sync" << endl;
	kdDebug() << "Path was " << str << "  " << path << endl;
        syncee->setFirstSync( true );
        syncee->setSyncMode( Syncee::MetaMode );
        return;
    }
    syncee->setSyncMode( Syncee::MetaMode );
    KSimpleConfig conf( str );

    SyncEntry* entry;
    QString timestmp;
    QStringList ids;

    /* mod + added */
    for (entry= syncee->firstEntry(); entry; entry = syncee->nextEntry() ) {
        ids << entry->id();
	kdDebug() << "Entry " << entry->name() << endl << "Entry id" << entry->id() << endl;
        if ( conf.hasGroup( entry->id() )  ) {
            conf.setGroup( entry->id() );
            timestmp = conf.readEntry("time");
	    kdDebug() << "Timestamp Old" << timestmp << endl;
	    kdDebug() << "Timestamp New" << entry->timestamp() << endl;
            if ( timestmp != entry->timestamp() )
                entry->setState( SyncEntry::Modified );
        }
        /* added */
        else {
	    kdDebug() << "Entry added" << endl;
            entry->setState( SyncEntry::Added );
        }
    }
    /* find removed item... through reversed mapping */
    QStringList groups = conf.groupList();
    QStringList::Iterator it;
    for (it = groups.begin(); it != groups.end(); ++it ) {
        // removed items if ids is not present
        if (!ids.contains( (*it) ) ) {
	    kdDebug() << "Entry removed from abook" << (*it) << endl;
            KABC::Addressee adr;
            adr.setUid( (*it) );
            AddressBookSyncEntry* entry;
            entry = new AddressBookSyncEntry( adr );
            entry->setState( SyncEntry::Removed );
            syncee->addEntry( entry );
        }
    }
}
void AddressBookPart::writeMeta( Syncee* syncee, const QString& path ) {
    kdDebug() << "WriteMeta AddressBookPart " << endl;
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/konnector-" + path;
    if (!QFile::exists( str ) ) {
        kdDebug() << "Path does not exist " << endl;
	kdDebug() << "Path = " << str << endl;
        KonnectorProfile kon = core()->konnectorProfile();
        QDir dir;
        dir.mkdir( dir.homeDirPath() + "/.kitchensync");
        dir.mkdir( dir.homeDirPath() + "/.kitchensync/meta");
        dir.mkdir( dir.homeDirPath() + "/.kitchensync/meta/konnector-" + kon.uid() );
	kdDebug() << "Kon UID " << kon.uid() << endl;
    }
    KSimpleConfig conf( str );
    QStringList grpList = conf.groupList();
    QStringList::Iterator it;
    for ( it = grpList.begin(); it != grpList.end(); ++it ) {
        conf.deleteGroup( (*it) );
    }
    SyncEntry* entry;
    for (entry = syncee->firstEntry(); entry; entry= syncee->nextEntry() ) {
        if ( entry->state() == SyncEntry::Removed )
            continue;
        kdDebug() << "Name " << entry->name() << endl;
        kdDebug() << "UID  " << entry->id() << endl;
        kdDebug() << "Timestamp " << entry->timestamp() << endl;
        conf.setGroup( entry->id() );
        conf.writeEntry( "time", entry->timestamp() );
    }
}
void AddressBookPart::save( AddressBookSyncee* sync, const QString& path) {
    AddressBookSyncEntry* entry;
    kdDebug() << "save" << endl;
    if ( pathIsDefault( path ) ) { // save to the std. addressbook
        KABC::AddressBook *book = KABC::StdAddressBook::self();
        book->clear();
        for ( entry = (AddressBookSyncEntry*)sync->firstEntry();
              entry;
              entry= (AddressBookSyncEntry*) sync->nextEntry() ) {
            kdDebug() << "Writing addresse " << endl;
	    if( entry->state() != SyncEntry::Removed )
                book->insertAddressee( entry->addressee() );
        }
	kdDebug() << "dumping abook " << endl;
	book->dump();
        book->saveAll();
	kdDebug() << "dumped abook " << endl;
    }else {
        kdDebug() << "non default save" << endl;
        KABC::AddressBook book;
        KABC::ResourceFile *file = new KABC::ResourceFile(&book, path );
        book.addResource(file );
	/* work around */
	/*book.load();
	book.clear();*/
	/* work around end */
        for ( entry = (AddressBookSyncEntry*)sync->firstEntry();
              entry;
              entry= (AddressBookSyncEntry*) sync->nextEntry() ) {
	    kdDebug() << "Entry" << entry->name() << endl;
	    if( entry->state() != SyncEntry::Removed )
                book.insertAddressee( entry->addressee() );
        }
        KABC::Ticket* ticket = book.requestSaveTicket(file);
        book.save(ticket );

    }
}
bool AddressBookPart::pathIsDefault( const QString& path ) {
    if ( path.isEmpty() ) return true;
    if ( path.stripWhiteSpace() == QString::fromLatin1("default") )
        return true;

    kdDebug() << "Path is not default" << endl;
    return false;
}
AddressBookSyncee* AddressBookPart::book2syncee( KABC::AddressBook* book) {
    AddressBookSyncee* syncee = new AddressBookSyncee();
    AddressBookSyncEntry* entry=0l;
    KABC::AddressBook::Iterator it = book->begin();
    for ( ; it != book->end(); ++it ) {
        entry = new AddressBookSyncEntry( (*it) );
        syncee->addEntry( entry );
    }
    return syncee;
}
#include "ksync_addressbookpart.moc"
