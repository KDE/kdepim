
#include "ksync_fileviewwidget.h"

#include <qlayout.h>

#include <ktrader.h>
#include <kinstance.h>
#include <klibloader.h>

#include <manipulatorpart.h>

using namespace KitchenSync;


/*

 Todo:
 - welches part embedded
 - toolbar mergen
 - segfault am ende

*/

KSyncFileviewWidget::KSyncFileviewWidget(  QWidget* parent , const char* name , WFlags fl )
    : KParts::MainWindow( parent, name, fl ) {
    //: QWidget( parent, name, fl ) {

      setInstance(KGlobal::instance() );

    if ( !name )
        setName( "FileviewWidget" );

    QBoxLayout * l = new QHBoxLayout( this );

    m_manager = new KParts::PartManager( this );
    connect ( m_manager,  SIGNAL ( activePartChanged ( KParts::Part * ) ),  this,  SLOT ( createGUI ( KParts::Part *)) );
    m_splitter = new QSplitter( this );
    //setView( m_splitter );

    KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("inode/directory"),  QString::null);
    KService::Ptr ptr = offers.first();
    KLibFactory *factory = KLibLoader::self()->factory( ptr->library() );
    if (factory) {
        m_part1 = static_cast<KParts::ReadOnlyPart *>(factory->create(m_splitter, ptr->name(), "KParts::ReadOnlyPart"));
        m_part2 = static_cast<KParts::ReadOnlyPart *>(factory->create(m_splitter, ptr->name(), "KParts::ReadOnlyPart"));
    } else {
        //   kdFatal() << "No suitable part found! " << endl;
    }

    m_manager->addPart(m_part1, true);
    m_manager->addPart(m_part2, false);

    m_splitter->setMinimumSize(300, 200);
    m_splitter->show();

    l->addWidget( m_splitter );
}

void KSyncFileviewWidget::openURLHost(const KURL & url) {
    m_part1->openURL( url );
}


void KSyncFileviewWidget::openURLClient(const KURL & url) {
    m_part2->openURL( url );
}

KSyncFileviewWidget::~KSyncFileviewWidget() {
     delete m_part1;
     delete m_part2;
}
#include "ksync_fileviewwidget.moc"
#include "fileviewwidget.moc"
