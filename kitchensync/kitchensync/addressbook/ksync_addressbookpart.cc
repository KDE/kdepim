
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

#include <kparts/genericfactory.h>

#include <kaddressbooksyncentry.h>
#include <kabc/resourcefile.h>

#include <ksync_mainwindow.h>

#include <ksync_sync.h>

#include "addressbookbase.h"
#include "ksync_addressbookpart.h"

typedef KParts::GenericFactory< KitchenSync::AddressBookPart> AddressBookPartFactory;
K_EXPORT_COMPONENT_FACTORY( libaddressbookpart,  AddressBookPartFactory );

using namespace KitchenSync;

namespace {
    void dump(const QString& prefix, const QValueList<KABC::Addressee>& list  ) {
        QValueList<KABC::Addressee>::ConstIterator it;
        for (it = list.begin(); it != list.end(); ++it ) {
            kdDebug() << prefix << (*it).givenName() << endl;
        }
    }

};

AddressBookPart::AddressBookPart( QWidget* parent,  const char* name,
                                  QObject* obj,  const char* name2,
                                  const QStringList & )
    : ManipulatorPart( parent,  name )
{
    setInstance( AddressBookPartFactory::instance() );
    m_pixmap = KGlobal::iconLoader()->loadIcon("kaddressbook",  KIcon::Desktop,  48 );
    m_widget = 0;
    m_config = new KConfig( "KitchenSyncAddressBookPart");
    m_configured = false;
}
AddressBookPart::~AddressBookPart()
{
    delete m_config;
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
    if (!m_configured ) {
        m_config->setGroup( prof.name() );
        m_path = m_config->readEntry("Path");
        m_evo = m_config->readBoolEntry("Evo");
        m_configured = true;
    }
    m_widget = new AddressBookConfigBase();
    m_widget->lnePath->setText( m_path );
    m_widget->ckbEvo->setChecked( m_evo );
    return m_widget;
}
void AddressBookPart::processEntry( const KSyncEntry::List& in,
                                    KSyncEntry::List& out )
{
    QPtrList<KAddressbookSyncEntry> our;
    QPtrListIterator<KSyncEntry> it( in );
    KSyncEntry *entry;
    KAddressbookSyncEntry* entry2 = 0;
    while ( (entry = it.current() ) != 0 ) {
        ++it;
        kdDebug() << entry->type() << endl;
        if (entry->type() == QString::fromLatin1("KAddressbookSyncEntry") ) {
            kdDebug() << "KAddressbookSyncEntry found type" << endl;
            entry2 = (KAddressbookSyncEntry*)  entry->clone();
            break;
        }
    }
    if (entry2 == 0 )
        return;

    KAddressbookSyncEntry *met  = meta();
    if ( met == 0l )
        return;
    kdDebug() << "meta addressbook" << endl;
    dump("Opie Added",  entry2->added() );
    dump("Opie Mod",  entry2->modified() );
    dump("Opie Del",  entry2->deleted() );
    Profile prof = core()->currentProfile();
    Kapabilities cap = prof.caps();
    // write back
    KURL url(m_path );
    QString newPath;
    if ( !url.isLocalFile() ) { // download
        ;
    }else{
        newPath = url.path();
    }
    KSyncEntry::List one;
    one.append( entry2 );
    KSyncEntry::List two;
    two.append( met );
    SyncManager manager( this,  "SyncManager" );
    SyncReturn ret = manager.sync( SYNC_INTERACTIVE,  one,  two );
    one = ret.synced();
    delete entry2;
    delete met;
    entry2 = 0;
    for ( entry = one.first(); entry != 0; entry = one.next() ) {
        if (entry->type() == QString::fromLatin1("KAddressbookSyncEntry") ) {
            entry2 = (KAddressbookSyncEntry*) entry;
            kdDebug() << "After syncing found " << endl;
            break;
        }
    }
    if ( entry2 == 0)
        return;
    KABC::ResourceFile *res2 = new KABC::ResourceFile(entry2->addressbook(),
                                                      newPath);
    entry2->addressbook()->addResource(res2);
    KABC::Ticket *tick = entry2->addressbook()->requestSaveTicket( res2 );
    entry2->addressbook()->save(tick);
    //delete res2;
    if ( cap.isMetaSyncingEnabled() ) {
        kdDebug() << "Meta enabled" << endl;
        m_config->setGroup( prof.name() );
        QString metaPath = m_config->readEntry("metaPath");
        KABC::ResourceFile *res = new KABC::ResourceFile(entry2->addressbook(),
                                                         QDir::homeDirPath() + "/.kitchensync/meta/"+ metaPath + "/addressb.vcf");
        entry2->addressbook()->addResource( res );
        tick = entry2->addressbook()->requestSaveTicket( res );
        entry2->addressbook()->save(tick );
        QDateTime time = QDateTime::currentDateTime();
        m_config->writeEntry("time", time );
        delete res;
    }
    delete res2;

    //
    out.append(entry2  );
}
void AddressBookPart::slotConfigOk()
{
    Profile prof = core()->currentProfile();
    m_path = m_widget->lnePath->text();
    m_evo = m_widget->ckbEvo->isChecked();
    m_config->writeEntry( "Path",  m_path );
    m_config->writeEntry( "Evo",  m_evo );
}
KAddressbookSyncEntry* AddressBookPart::meta()
{
    KAddressbookSyncEntry* entry = new KAddressbookSyncEntry();
    KABC::AddressBook *book = new KABC::AddressBook();
    entry->setAddressbook( book );

    if ( m_path.isEmpty() ) {
        m_path = "file:" + locateLocal("data",  "kabc/std.vcf");
    }
    KURL url(m_path );
    QString newPath;
    if ( !url.isLocalFile() ) { // download
        ;
    }else{
        newPath = url.path();
    }
    KABC::ResourceFile *file = new KABC::ResourceFile( book,  newPath );
    book->addResource(file );
    if (!book->load() ) {
        delete entry;
        return 0l;
    }
    delete file;
    Profile prof = core()->currentProfile();
    Kapabilities cap = prof.caps();
    entry->setSyncMode( KSyncEntry::SYNC_NORMAL );
    // meta
    if ( cap.isMetaSyncingEnabled() ) {
        kdDebug() << "Gathering metadata " << endl;
        entry->setSyncMode( KSyncEntry::SYNC_META );
        m_config->setGroup( prof.name() );
        QString metaPath = m_config->readEntry("metaPath");

        if (metaPath.isEmpty() ) {
            entry->setFirstSync( true );
            QString meta = kapp->randomString( 6 );
            meta.append("abpart");
            m_config->writeEntry("metaPath",  meta );
            m_config->sync();
            metaPath = meta;
            QDir dir;
            dir.mkdir( QDir::homeDirPath() + "/.kitchensync/");
            dir.mkdir( QDir::homeDirPath() + "/.kitchensync/meta");
            dir.mkdir( QDir::homeDirPath() + "/.kitchensync/meta/" + metaPath );
        }else {
            entry->setFirstSync( false );
            QDateTime date = m_config->readDateTimeEntry("time");
            KABC::AddressBook bookMeta;
            KABC::ResourceFile file(&bookMeta, QDir::homeDirPath() + "/.kitchensync/meta/"+ metaPath + "/addressb.vcf");
            bookMeta.addResource( &file );
            // loaded get meta
            // book the current Abook
            // bookMeta the currentMeta Abook
            if ( bookMeta.load() ) {
                kdDebug() << "not loaded" << endl;
                KABC::AddressBook::Iterator itNew;
                KABC::AddressBook::Iterator itOld;
                QValueList<KABC::Addressee> added;
                QValueList<KABC::Addressee> removed;
                QValueList<KABC::Addressee> mod;
                bool found;
                for ( itNew = book->begin(); itNew != book->end(); ++itNew ) { // new + modified
                    found = false;
                    for ( itOld = bookMeta.begin(); itOld != bookMeta.end(); ++itOld ) {
                        if ( (*itOld).uid() == (*itNew).uid() ) {
                            //if ( date < (*itNew).revision() ) // modified broken use the one down
                            if ( (*itOld) == (*itNew) ) {
                                ; // FIXMER
                    	    }else
                                mod.append( (*itNew) );
                            found = true;
                            break;
                        }
                    }
                    if (!found ) { // gues it's new
                        added.append( (*itNew ) );
                    }
                }
                // for oldIt and newIt to search for removed entries
                for ( itOld = bookMeta.begin(); itOld != bookMeta.end(); ++itOld ) {
                    found = false;
                    for ( itNew = book->begin(); itNew != book->end(); ++itNew ) {
                        if ( (*itNew).uid() == (*itOld).uid() ) {
                            found = true;
                            break;
                        }
                    }
                    if (!found ) {
                        removed.append( (*itOld) );
                    }
                }
                // add it to the entry
                entry->setModified( mod );
                dump("KDE Mod",  mod );
                entry->setAdded( added );
                dump("KDE added",  added );
                entry->setDeleted( removed );
                dump("KDE del ",  removed );
            }else
                kdDebug() << "Could not load meta data" << endl;
        }
    }
    return entry;
};
#include "ksync_addressbookpart.moc"
