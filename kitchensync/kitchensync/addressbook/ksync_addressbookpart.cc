
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
    kdDebug(5228) << "processEntry in AddressBookPart aye" << endl;
    /* 1. */
    Profile prof = core()->currentProfile();
    KonnectorProfile kon = core()->konnectorProfile();

    /* 2. */
    QString path = prof.path("AddressBook");
    QString meta = kon.uid() + "/" + prof.uid() + "addressbook.rc";
    bool met = kon.kapabilities().isMetaSyncingEnabled();
    kdDebug(5228) << "Is meta syncing enabled? " << met << endl;

    /* 3. */
    Syncee* syncee = 0l;
    AddressBookSyncee* aBook=0l;
    QPtrListIterator<Syncee> syncIt( in );
    for ( ; syncIt.current(); ++syncIt ) {
        syncee = syncIt.current();
        if (syncee->type() == QString::fromLatin1("AddressBookSyncee") ) {
	    kdDebug(5228) << "Found our syncee" << endl;
            aBook = (AddressBookSyncee*) syncee;
            break;
        }

    }
    if (!aBook) return;

    /* 4. */
    AddressBookSyncee* ourbook;
    ourbook = load(path );

    /* 5. */
    if (met)
      doMeta( ourbook, meta );

    /* 6. */
    Syncer sync( core()->syncUi(), core()->syncAlgorithm() );
    sync.addSyncee( aBook );
    sync.addSyncee( ourbook );
    sync.sync();

    /* 7. KABC seems broken so we do meta from save*/
/*    if (met)
      writeMeta( ourbook, meta );
*/

    /* 8. */
    save( ourbook, path, met ? meta : QString::null );

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
    kdDebug(5228) << "load abook" << path << endl;
    KABC::AddressBook* book;
    AddressBookSyncee* sync;
    if ( pathIsDefault( path ) ) {
        kdDebug(5228) << "use default one " << endl;
        book =  KABC::StdAddressBook::self();
        sync = book2syncee( book );
        return sync;
    }else {
        kdDebug(5228) << "use non default" << endl;
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
    kdDebug(5228) << "Do Meta" << endl;
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/konnector-" + path;
    if (!QFile::exists( str ) ) {
        kdDebug(5228) << "Path does not exist ->First Sync" << endl;
	kdDebug(5228) << "Path was " << str << "  " << path << endl;
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
	kdDebug(5228) << "Entry " << entry->name() << endl << "Entry id" << entry->id() << endl;
        if ( conf.hasGroup( entry->id() )  ) {
            conf.setGroup( entry->id() );
            timestmp = conf.readEntry("time");
	    kdDebug(5228) << "Timestamp Old" << timestmp << endl;
	    kdDebug(5228) << "Timestamp New" << entry->timestamp() << endl;
            if ( timestmp != entry->timestamp() )
                entry->setState( SyncEntry::Modified );
        }
        /* added */
        else {
	    kdDebug(5228) << "Entry added" << endl;
            entry->setState( SyncEntry::Added );
        }
    }
    /* find removed item... through reversed mapping */
    QStringList groups = conf.groupList();
    QStringList::Iterator it;
    for (it = groups.begin(); it != groups.end(); ++it ) {
        // removed items if ids is not present
        if (!ids.contains( (*it) ) ) {
	    kdDebug(5228) << "Entry removed from abook" << (*it) << endl;
            KABC::Addressee adr;
            adr.setUid( (*it) );
            AddressBookSyncEntry* entry;
            entry = new AddressBookSyncEntry( adr );
            entry->setState( SyncEntry::Removed );
            syncee->addEntry( entry );
        }
    }
}
void AddressBookPart::writeMeta( KABC::AddressBook* book, const QString& path ) {
    /* no meta info to save */
    if (path.isEmpty() ) return;

    kdDebug(5228) << "WriteMeta AddressBookPart " << endl;
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/konnector-" + path;
    if (!QFile::exists( str ) ) {
        kdDebug(5228) << "Path does not exist " << endl;
	kdDebug(5228) << "Path = " << str << endl;
        KonnectorProfile kon = core()->konnectorProfile();
        QDir dir;
        dir.mkdir( dir.homeDirPath() + "/.kitchensync");
        dir.mkdir( dir.homeDirPath() + "/.kitchensync/meta");
        dir.mkdir( dir.homeDirPath() + "/.kitchensync/meta/konnector-" + kon.uid() );
	kdDebug(5228) << "Kon UID " << kon.uid() << endl;
    }
    KSimpleConfig conf( str );
    QStringList grpList = conf.groupList();
    QStringList::Iterator it;
    for ( it = grpList.begin(); it != grpList.end(); ++it ) {
        conf.deleteGroup( (*it) );
    }

    KABC::AddressBook::Iterator aIt;
    for ( aIt = book->begin(); aIt != book->end(); ++aIt ) {
        kdDebug(5228) << "Name " << (*aIt).realName() << endl;
        kdDebug(5228) << "UID  " << (*aIt).uid() << endl;
        kdDebug(5228) << "Timestamp " << (*aIt).revision().toString() << endl;
        conf.setGroup( (*aIt).uid() );
        conf.writeEntry( "time", (*aIt).revision().toString() );
    }
}
void AddressBookPart::save( AddressBookSyncee* sync, const QString& path,  const QString& meta) {
    bool pIsDefault = false;
    AddressBookSyncEntry* entry;
    KABC::AddressBook* book;

    pIsDefault = pathIsDefault( path );

    // save to the std. addressbook
    if ( pIsDefault ) {
        book = KABC::StdAddressBook::self();
    }else {
        book = new KABC::AddressBook() ;
        /* resource get's deleted for us */
        KABC::ResourceFile *file = new KABC::ResourceFile(book, path );
        book->addResource(file );
    }
    /* clear the old book first */
    book->clear();

    for ( entry = (AddressBookSyncEntry*)sync->firstEntry();
          entry;
          entry= (AddressBookSyncEntry*) sync->nextEntry() ) {
        if( entry->state() != SyncEntry::Removed )
            book->insertAddressee( entry->addressee() );
    }
    saveAll( book );
    kdDebug(5228) << "dumped abook " << endl;
    writeMeta( book, meta );

    if (!pIsDefault )
        delete book;
    else
        KABC::StdAddressBook::close();
}
bool AddressBookPart::pathIsDefault( const QString& path ) {
    if ( path.isEmpty() ) return true;
    if ( path.stripWhiteSpace() == QString::fromLatin1("default") )
        return true;

    kdDebug(5228) << "Path is not default" << endl;
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
void AddressBookPart::saveAll( KABC::AddressBook* ab) {
    KABC::Resource *res = 0l;
    QPtrList<KABC::Resource> list = ab->resources();
    for (uint i = 0; i < list.count(); ++i ) {
        res = list.at( i );
        if (!res->readOnly() ) {
            KABC::Ticket* ticket = ab->requestSaveTicket( res );
            if (ticket)
                ab->save( ticket );
        }
    }
}
#include "ksync_addressbookpart.moc"
