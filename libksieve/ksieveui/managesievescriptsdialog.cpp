
#include "managesievescriptsdialog.h"
#include "editor/sievetextedit.h"
#include "editor/sieveeditor.h"
#include "widgets/sievetreewidgetitem.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kwindowsystem.h>
#include <kinputdialog.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

#include <akonadi/agentinstance.h>
#include <kmanagesieve/sievejob.h>
#include <ksieveui/util.h>

#include <QApplication>
#include <QButtonGroup>
#include <QMenu>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>

#include <errno.h>

using namespace KSieveUi;

ManageSieveTreeView::ManageSieveTreeView(QWidget *parent)
    : QTreeWidget(parent),
      mImapFound(true)
{
    connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()),
             this, SLOT(slotGeneralFontChanged()));
    connect( KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
             this, SLOT(slotGeneralPaletteChanged()));
}

ManageSieveTreeView::~ManageSieveTreeView()
{
}

void ManageSieveTreeView::slotGeneralPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha( 128 );
    mTextColor = color;
}

void ManageSieveTreeView::slotGeneralFontChanged()
{
    setFont( KGlobalSettings::generalFont() );
}

void ManageSieveTreeView::setImapFound(bool found)
{
    if (mImapFound != found) {
        mImapFound = found;
        update();
    }
}

void ManageSieveTreeView::paintEvent( QPaintEvent *event )
{
    if ( mImapFound ) {
        QTreeWidget::paintEvent(event);
    } else {
        QPainter p( viewport() );

        QFont font = p.font();
        font.setItalic( true );
        p.setFont( font );

        if (!mTextColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        p.setPen( mTextColor );

        p.drawText( QRect( 0, 0, width(), height() ), Qt::AlignCenter, i18n( "No imap server configured..." ) );
    }
}

ManageSieveScriptsDialog::ManageSieveScriptsDialog( QWidget * parent )
    : QDialog( parent ),
      mSieveEditor( 0 ),
      mIsNewScript( false ),
      mWasActive( false ),
      mBlockSignal( false ),
      mClearAll( false )
{
    setWindowTitle( i18n( "Manage Sieve Scripts" ) );
    setModal( false );
    setAttribute( Qt::WA_GroupLeader );
    setAttribute( Qt::WA_DeleteOnClose );
    KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)) );
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFrame *frame =new QFrame;
    mainLayout->addWidget( frame );
    QVBoxLayout * vlay = new QVBoxLayout( frame );
    vlay->setSpacing( 0 );
    vlay->setMargin( 0 );

    mListView = new ManageSieveTreeView( frame);
    mListView->setContextMenuPolicy(Qt::CustomContextMenu);
    mListView->setHeaderLabel( i18n( "Available Scripts" ) );
    mListView->setRootIsDecorated( true );
    mListView->setAlternatingRowColors( true );
    mListView->setSelectionMode( QAbstractItemView::SingleSelection );
#ifndef QT_NO_CONTEXTMENU
    connect( mListView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotContextMenuRequested(QPoint)) );
#endif
    connect( mListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(slotDoubleClicked(QTreeWidgetItem*)) );
    connect( mListView, SIGNAL(itemSelectionChanged()),
             this, SLOT(slotUpdateButtons()) );
    connect( mListView, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
             this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    vlay->addWidget( mListView );

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    vlay->addLayout( buttonLayout );

    mNewScript = new KPushButton( i18nc( "create a new sieve script", "New..." ) );
    connect( mNewScript, SIGNAL(clicked()), SLOT(slotNewScript()) );
    buttonLayout->addWidget( mNewScript );

    mEditScript = new KPushButton( i18n( "Edit..." ) );
    connect( mEditScript, SIGNAL(clicked()), SLOT(slotEditScript()) );
    buttonLayout->addWidget( mEditScript );

    mDeleteScript = new KPushButton( i18n( "Delete" ) );
    connect( mDeleteScript, SIGNAL(clicked()), SLOT(slotDeleteScript()) );
    buttonLayout->addWidget( mDeleteScript );

    mDeactivateScript = new KPushButton( i18n( "Deactivate" ) );
    connect( mDeactivateScript, SIGNAL(clicked()), SLOT(slotDeactivateScript()) );
    buttonLayout->addWidget( mDeactivateScript );

    KPushButton *close = new KPushButton( KStandardGuiItem::close() );
    connect( close, SIGNAL(clicked()), this, SLOT(accept()) );
    buttonLayout->addWidget( close );

    KConfigGroup group( KGlobal::config(), "ManageSieveScriptsDialog" );
    const QSize size = group.readEntry( "Size", QSize() );
    if ( size.isValid() ) {
        resize( size );
    } else {
        resize( sizeHint().width(), sizeHint().height() );
    }

    slotRefresh();
}

ManageSieveScriptsDialog::~ManageSieveScriptsDialog()
{
    clear();
    KConfigGroup group( KGlobal::config(), "ManageSieveScriptsDialog" );
    group.writeEntry( "Size", size() );
}

void ManageSieveScriptsDialog::killAllJobs()
{
    mClearAll = true;
    QMap<KManageSieve::SieveJob*,QTreeWidgetItem*>::const_iterator it = mJobs.constBegin();
    while (it != mJobs.constEnd()) {
        it.key()->kill();
        ++it;
    }
    mClearAll = false;
    mJobs.clear();
}

bool ManageSieveScriptsDialog::serverHasError(QTreeWidgetItem *item) const
{
    const QVariant variant = item->data( 0, SIEVE_SERVER_ERROR );
    if ( variant.isValid() && variant.toBool()==true )
        return true;
    return false;
}

void ManageSieveScriptsDialog::slotItemChanged(QTreeWidgetItem *item, int col)
{
    if (!item || mBlockSignal || (col != 0) ) {
        return;
    }
    if ( !isFileNameItem( item ) )
        return;
    QTreeWidgetItem *parent = item->parent();
    if ( (mSelectedItems[parent] != item) && itemIsActived( item )) {
        mSelectedItems[parent] = item;
        changeActiveScript( parent, true );
    } else {
        mSelectedItems[parent] = item;
        changeActiveScript( parent, false );
    }
}

void ManageSieveScriptsDialog::slotUpdateButtons()
{
    QTreeWidgetItem * item = mListView->currentItem();

    bool enabled = true;
    if ( !item )
        enabled = false;
    else if ( !item->parent() && !mUrls.count( item ))
        enabled = false;
    if ( !enabled ) {
        mNewScript->setEnabled( false );
        mEditScript->setEnabled( false );
        mDeleteScript->setEnabled( false );
        mDeactivateScript->setEnabled( false );
    } else {
        if ( serverHasError(item) || !mJobs.keys(item).isEmpty())
            mNewScript->setEnabled( false );
        else
            mNewScript->setEnabled( mUrls.count( item ) );
        enabled = isFileNameItem( item );
        mEditScript->setEnabled( enabled );
        mDeleteScript->setEnabled( enabled );
        mDeactivateScript->setEnabled( enabled && itemIsActived( item ));
    }
}


void ManageSieveScriptsDialog::slotRefresh()
{
    mBlockSignal = true;
    clear();
    SieveTreeWidgetItem *last = 0;
    Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    bool imapFound = false;
    foreach ( const Akonadi::AgentInstance& type, lst ) {
        if ( type.status() == Akonadi::AgentInstance::Broken )
            continue;

        last = new SieveTreeWidgetItem( mListView, last );
        last->setText( 0, type.name() );
        last->setIcon( 0, SmallIcon( QLatin1String("network-server") ) );

        const KUrl u = KSieveUi::Util::findSieveUrlForAccount( type.identifier() );
        if ( u.isEmpty() ) {
            QTreeWidgetItem *item = new QTreeWidgetItem( last );
            item->setText( 0, i18n( "No Sieve URL configured" ) );
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
            mListView->expandItem( last );
        } else {
            KManageSieve::SieveJob * job = KManageSieve::SieveJob::list( u );
            connect( job, SIGNAL(item(KManageSieve::SieveJob*,QString,bool)),
                     this, SLOT(slotItem(KManageSieve::SieveJob*,QString,bool)) );
            connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
                     this, SLOT(slotResult(KManageSieve::SieveJob*,bool,QString,bool)) );
            mJobs.insert( job, last );
            mUrls.insert( last, u );
            last->startAnimation();
        }
        imapFound = true;
    }
    slotUpdateButtons();
    mListView->setImapFound(imapFound);
}

void ManageSieveScriptsDialog::slotResult( KManageSieve::SieveJob *job, bool success, const QString &, bool )
{
    if (mClearAll)
        return;
    QTreeWidgetItem * parent = mJobs[job];
    if ( !parent )
        return;
    (static_cast<SieveTreeWidgetItem*>(parent))->stopAnimation();
    if (success)
        parent->setData( 0, SIEVE_SERVER_CAPABILITIES, job->sieveCapabilities() );
    mJobs.remove( job );

    mListView->expandItem( parent );

    mBlockSignal = false;
    if ( success )
        return;

    parent->setData( 0, SIEVE_SERVER_ERROR, true );
    QTreeWidgetItem * item =
            new QTreeWidgetItem( parent );
    item->setText( 0, i18n( "Failed to fetch the list of scripts" ) );
    item->setFlags( item->flags() & ~Qt::ItemIsEnabled );

}

void ManageSieveScriptsDialog::slotItem( KManageSieve::SieveJob * job, const QString & filename, bool isActive )
{
    QTreeWidgetItem * parent = mJobs[job];
    if ( !parent )
        return;
    QTreeWidgetItem* item = new QTreeWidgetItem( parent );
    item->setFlags(item->flags() & (Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable));

    const bool oldBlockSignal = mBlockSignal;
    mBlockSignal = true; // don't trigger slotItemChanged
    item->setText(0,filename);
    item->setCheckState(0, isActive ? Qt::Checked : Qt::Unchecked);
    if ( isActive ) {
        mSelectedItems[parent] = item;
    }
    mBlockSignal = oldBlockSignal;
}

void ManageSieveScriptsDialog::slotContextMenuRequested( const QPoint& p )
{
    QTreeWidgetItem *item = mListView->itemAt( p );
    if ( !item )
        return;
    if ( !item->parent() && !mUrls.count( item ))
        return;
    QMenu menu;
    if ( isFileNameItem( item ) ) {
        // script items:
        menu.addAction( i18n( "Edit Script..." ), this, SLOT(slotEditScript()) );
        menu.addAction( i18n( "Delete Script" ), this, SLOT(slotDeleteScript()) );
        if ( itemIsActived( item ) ) {
            menu.addSeparator();
            menu.addAction( i18n( "Deactivate Script" ), this, SLOT(slotDeactivateScript()) );
        }
    } else if ( !item->parent() ) {
        // top-levels:
        if ( !serverHasError(item) && mJobs.keys(item).isEmpty())
            menu.addAction( i18n( "New Script..." ), this, SLOT(slotNewScript()) );
    }
    if ( !menu.actions().isEmpty() )
        menu.exec( mListView->viewport()->mapToGlobal(p) );
}

void ManageSieveScriptsDialog::slotDeactivateScript()
{
    QTreeWidgetItem * item = mListView->currentItem();
    if ( !isFileNameItem( item ) )
        return;
    QTreeWidgetItem *parent = item->parent();
    if ( itemIsActived( item ) ) {
        mSelectedItems[parent] = item;
        changeActiveScript( parent, false );
    }
}

void ManageSieveScriptsDialog::changeActiveScript( QTreeWidgetItem *item, bool activate )
{
    if ( !item )
        return;
    if ( !mUrls.count( item ) )
        return;
    if ( !mSelectedItems.count( item ) )
        return;
    KUrl u = mUrls[item];
    if ( u.isEmpty() )
        return;
    QTreeWidgetItem* selected = mSelectedItems[item];
    if ( !selected )
        return;
    u.setFileName( selected->text(0) );

    KManageSieve::SieveJob * job;
    if ( activate )
        job = KManageSieve::SieveJob::activate( u );
    else
        job = KManageSieve::SieveJob::deactivate( u );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotRefresh()) );
}

bool ManageSieveScriptsDialog::itemIsActived( QTreeWidgetItem *item ) const
{
    Q_ASSERT( item && item->parent() );
    return (item->checkState(0) == Qt::Checked);
}

bool ManageSieveScriptsDialog::isFileNameItem( QTreeWidgetItem *item ) const
{
    if ( !item || !item->parent() )
        return false;
    return (item->flags() & Qt::ItemIsEnabled);
}

void ManageSieveScriptsDialog::clear()
{
    killAllJobs();
    mSelectedItems.clear();
    mUrls.clear();
    mListView->clear();
}

void ManageSieveScriptsDialog::slotDoubleClicked( QTreeWidgetItem * item )
{
    if ( !isFileNameItem( item ) )
        return;

    slotEditScript();
}

void ManageSieveScriptsDialog::slotDeleteScript()
{
    QTreeWidgetItem * currentItem =  mListView->currentItem();
    if ( !isFileNameItem( currentItem ) )
        return;

    QTreeWidgetItem *parent = currentItem->parent();
    if ( !parent )
        return;

    if ( !mUrls.count( parent ) )
        return;

    KUrl u = mUrls[parent];
    if ( u.isEmpty() )
        return;

    u.setFileName( currentItem->text(0) );

    if ( KMessageBox::warningContinueCancel( this, i18n( "Really delete script \"%1\" from the server?", u.fileName() ),
                                             i18n( "Delete Sieve Script Confirmation" ),
                                             KStandardGuiItem::del() )
         != KMessageBox::Continue )
        return;
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::del( u );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotRefresh()) );
}

void ManageSieveScriptsDialog::slotEditScript()
{
    QTreeWidgetItem *currentItem = mListView->currentItem();
    if ( !isFileNameItem( currentItem ) )
        return;
    QTreeWidgetItem* parent = currentItem->parent();
    if ( !mUrls.count( parent ) )
        return;
    KUrl url = mUrls[parent];
    if ( url.isEmpty() )
        return;
    url.setFileName( currentItem->text(0) );
    mCurrentURL = url;
    mCurrentCapabilities = parent->data(0, SIEVE_SERVER_CAPABILITIES).toStringList();
    mIsNewScript = false;
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::get( url );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void ManageSieveScriptsDialog::slotNewScript()
{
    QTreeWidgetItem *currentItem = mListView->currentItem();
    if ( !currentItem )
        return;
    if ( currentItem->parent() )
        currentItem = currentItem->parent();
    if ( !currentItem )
        return;

    if ( !mUrls.count( currentItem ) )
        return;

    KUrl u = mUrls[currentItem];
    if ( u.isEmpty() )
        return;

    bool ok = false;
    const QString name = KInputDialog::getText( i18n( "New Sieve Script" ),
                                                i18n( "Please enter a name for the new Sieve script:" ),
                                                i18n( "unnamed" ), &ok, this );
    if ( !ok || name.isEmpty() )
        return;

    u.setFileName( name );

    mCurrentCapabilities = currentItem->data(0, SIEVE_SERVER_CAPABILITIES).toStringList();

    QTreeWidgetItem * parentItem = currentItem;
    if (parentItem) {
        const int numberOfElement(parentItem->childCount());
        for (int i = 0; i <numberOfElement; ++i) {
            if (parentItem->child(i)->text(0) == name) {
                KMessageBox::error(
                            this,
                            i18n( "Script name already used \"%1\".", name ),
                            i18n( "New Script" ) );
                return;
            }
        }
    }

    QTreeWidgetItem *newItem = new QTreeWidgetItem( currentItem );
    newItem->setFlags(newItem->flags() & (Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable));
    newItem->setText(0,name);
    newItem->setCheckState(0,Qt::Unchecked);
    mCurrentURL = u;
    mIsNewScript = true;
    slotGetResult( 0, true, QString(), false );
}

void ManageSieveScriptsDialog::slotGetResult( KManageSieve::SieveJob *, bool success, const QString & script, bool isActive )
{
    if ( !success )
        return;

    if ( mSieveEditor )
        return;

    disableManagerScriptsDialog(true);
    mSieveEditor = new SieveEditor;
    mSieveEditor->setScriptName( mCurrentURL.fileName() );
    mSieveEditor->setSieveCapabilities(mCurrentCapabilities);
    mSieveEditor->setScript( script );
    connect( mSieveEditor, SIGNAL(okClicked()), this, SLOT(slotSieveEditorOkClicked()) );
    connect( mSieveEditor, SIGNAL(cancelClicked()), this, SLOT(slotSieveEditorCancelClicked()) );
    connect( mSieveEditor, SIGNAL(checkSyntax()), this, SLOT(slotSieveEditorCheckSyntaxClicked()) );
    mSieveEditor->show();
    mWasActive = isActive;
}

void ManageSieveScriptsDialog::slotSieveEditorCheckSyntaxClicked()
{
    if ( !mSieveEditor )
        return;
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::put( mCurrentURL,mSieveEditor->script(), mWasActive, mWasActive );
    job->setInteractive( false );
    connect( job, SIGNAL(errorMessage(KManageSieve::SieveJob*,bool,QString)),
             this, SLOT(slotPutResultDebug(KManageSieve::SieveJob*,bool,QString)) );
}

void ManageSieveScriptsDialog::slotSieveEditorOkClicked()
{
    if ( !mSieveEditor )
        return;
    disableManagerScriptsDialog(false);
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::put( mCurrentURL,mSieveEditor->script(), mWasActive, mWasActive );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotPutResult(KManageSieve::SieveJob*,bool)) );
}

void ManageSieveScriptsDialog::slotSieveEditorCancelClicked()
{
    disableManagerScriptsDialog(false);
    mSieveEditor->deleteLater();
    mSieveEditor = 0;
    mCurrentURL = KUrl();
    if ( mIsNewScript )
        slotRefresh();
}

void ManageSieveScriptsDialog::slotPutResultDebug(KManageSieve::SieveJob *,bool success ,const QString &errorMsg)
{
    if ( success ) {
        addOkMessage( i18n( "No errors found." ) );
    } else {
        if ( errorMsg.isEmpty() )
            addFailedMessage( i18n( "An unknown error was encountered." ) );
        else
            addFailedMessage( errorMsg );
    }
    mSieveEditor->resultDone();
}

void ManageSieveScriptsDialog::slotPutResult( KManageSieve::SieveJob *, bool success )
{
    if ( success ) {
        KMessageBox::information( this, i18n( "The Sieve script was successfully uploaded." ),
                                  i18n( "Sieve Script Upload" ) );
        mSieveEditor->deleteLater();
        mSieveEditor = 0;
        mCurrentURL = KUrl();
    } else {
        mSieveEditor->show();
    }
}

void ManageSieveScriptsDialog::addFailedMessage( const QString &err )
{
    addMessageEntry( err, QColor( Qt::darkRed ) );
}

void ManageSieveScriptsDialog::addOkMessage( const QString &err )
{
    addMessageEntry( err, QColor( Qt::darkGreen ) );
}

void ManageSieveScriptsDialog::addMessageEntry( const QString &errorMsg, const QColor &color )
{
    mSieveEditor->setDebugColor( color );
    mSieveEditor->setDebugScript( errorMsg );
}

void ManageSieveScriptsDialog::disableManagerScriptsDialog(bool disable)
{
    setDisabled(disable);
}


#include "managesievescriptsdialog.moc"
