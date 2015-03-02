/*
  Filter Dialog
  Author: Marc Mutz <mutz@kde.org>
  based upon work by Stefan Taferner <taferner@kde.org>

  Copyright (c) 2011-2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kmfilterdialog.h"
#include "kmfilterlistbox.h"

#include "filteractions/filteractiondict.h"
#include "filteractions/filteractionwidget.h"
#include "filterimporterexporter.h"
#include "filterselectiondialog.h"
#include "kmfilteraccountlist.h"
using MailCommon::FilterImporterExporter;
#include "filtermanager.h"
#include "folderrequester.h"
#include "kernel/mailkernel.h"
#include "util/mailutil.h"
#include "search/searchpatternedit.h"
#include "filterconverter/filterconverttosieve.h"

#include <Akonadi/ItemFetchJob>

#include <KConfigGroup>
#include <KDebug>
#include <KIconLoader>
#include <KInputDialog>
#include <KJob>
#include <KKeySequenceWidget>
#include <KListWidgetSearchLine>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KTabWidget>
#include <KWindowSystem>
#include <KIconButton>

#include <QApplication>
#include <QHeaderView>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QRadioButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QSplitter>
#include <QPointer>
#include <QKeyEvent>

Q_DECLARE_METATYPE(MailCommon::FilterImporterExporter::FilterType)
using namespace MailCommon;

namespace MailCommon {

//=============================================================================
//
// class KMFilterDialog (the filter dialog)
//
//=============================================================================



KMFilterDialog::KMFilterDialog( const QList<KActionCollection*> &actionCollection,
                                QWidget *parent, bool createDummyFilter )
    : KDialog( parent ),
      mFilter( 0 ),
      mDoNotClose( false ),
      mIgnoreFilterUpdates( true )
{
    setCaption( i18n( "Filter Rules" ) );
    setButtons( Help|Ok|Apply|Cancel|User1|User2|User3);
    setModal( false );
    setButtonFocus( Ok );
    KWindowSystem::setIcons( winId(),
                             qApp->windowIcon().pixmap( IconSize( KIconLoader::Desktop ),
                                                        IconSize( KIconLoader::Desktop ) ),
                             qApp->windowIcon().pixmap( IconSize( KIconLoader::Small ),
                                                        IconSize( KIconLoader::Small ) ) );
    setHelp( QLatin1String("filters"), QLatin1String("kmail") );
    setButtonText( User1, i18n( "Import..." ) );
    setButtonText( User2, i18n( "Export..." ) );
    setButtonText( User3, i18n( "Convert to..." ) );
    QMenu *menu = new QMenu();

    QAction *act = new QAction( i18n( "KMail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::KMailFilter) );
    menu->addAction( act );

    act = new QAction( i18n( "Thunderbird filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::ThunderBirdFilter) );
    menu->addAction( act );

    act = new QAction( i18n( "Evolution filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::EvolutionFilter) );
    menu->addAction( act );

    act = new QAction( i18n( "Sylpheed filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::SylpheedFilter) );
    menu->addAction( act );

    act = new QAction( i18n( "Procmail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::ProcmailFilter) );
    menu->addAction( act );

    act = new QAction( i18n( "Balsa filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::BalsaFilter) );
    menu->addAction( act );

    act = new QAction( i18n( "Claws Mail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::ClawsMailFilter) );
    menu->addAction( act );

    act = new QAction( i18n( "Icedove Mail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::IcedoveFilter) );
    menu->addAction( act );
#if 0
    //KF5 add i18n
    act = new QAction( QLatin1String( "GMail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::GmailFilter) );
    menu->addAction( act );
#endif
    connect( menu, SIGNAL(triggered(QAction*)), SLOT(slotImportFilter(QAction*)) );

    button( KDialog::User1 )->setMenu( menu );

    menu = new QMenu();

    act = new QAction( i18n( "Sieve script" ), this );
    connect(act, SIGNAL(triggered(bool)), SLOT(slotExportAsSieveScript()));
    menu->addAction( act );
    button( KDialog::User3 )->setMenu( menu );


    connect( this, SIGNAL(user2Clicked()),
             this, SLOT(slotExportFilters()) );
    enableButtonApply( false );

    QWidget *w = new QWidget( this );
    setMainWidget( w );
    QVBoxLayout *topVLayout = new QVBoxLayout( w );
    QHBoxLayout *topLayout = new QHBoxLayout;
    topVLayout->addLayout( topLayout );
    topLayout->setSpacing( spacingHint() );
    topLayout->setMargin( 0 );
    QVBoxLayout *vbl2 = 0;

    QSplitter *splitter = new QSplitter;
    splitter->setChildrenCollapsible(false);
    topLayout->addWidget(splitter);

    mFilterList = new KMFilterListBox( i18n( "Available Filters" ) );
    splitter->addWidget(mFilterList);
    KTabWidget *tabWidget = new KTabWidget;
    splitter->addWidget(tabWidget);

    QWidget *page1 = new QWidget( tabWidget );
    tabWidget->addTab( page1, i18nc( "General mail filter settings.", "General" ) );
    QHBoxLayout * hbl = new QHBoxLayout( page1 );
    hbl->setSpacing( spacingHint() );
    hbl->setMargin( marginHint() );

    QWidget *page2 = new QWidget( tabWidget );
    tabWidget->addTab( page2, i18nc( "Advanced mail filter settings.","Advanced" ) );
    vbl2 = new QVBoxLayout( page2 );
    vbl2->setSpacing( spacingHint() );
    vbl2->setMargin( marginHint() );

    QVBoxLayout *vbl = new QVBoxLayout();
    hbl->addLayout( vbl );
    vbl->setSpacing( spacingHint() );
    hbl->setStretchFactor( vbl, 2 );

    QGroupBox *patternGroupBox = new QGroupBox( i18n( "Filter Criteria" ), page1 );
    QHBoxLayout *layout = new QHBoxLayout( patternGroupBox );
    mPatternEdit =
            new MailCommon::SearchPatternEdit(
                patternGroupBox, MailCommon::SearchPatternEdit::MatchAllMessages );
    layout->addWidget( mPatternEdit );

    vbl->addWidget( patternGroupBox, 0, Qt::AlignTop );

    QGroupBox *agb = new QGroupBox( i18n( "Filter Actions" ), page1 );
    QHBoxLayout *layout2 = new QHBoxLayout;
    mActionLister = new MailCommon::FilterActionWidgetLister( agb );
    layout2->addWidget( mActionLister );
    agb->setLayout( layout2 );
    vbl->addWidget( agb, 0, Qt::AlignTop );

    mAdvOptsGroup = new QGroupBox( i18n( "Advanced Options" ), page2 );
    {
        QGridLayout *gl = new QGridLayout();
        QVBoxLayout *vbl3 = new QVBoxLayout();
        gl->addLayout( vbl3, 0, 0 );
        vbl3->setSpacing( spacingHint() );
        vbl3->addStretch( 1 );

        mApplyOnIn = new QCheckBox( i18n( "Apply this filter to incoming messages:" ), mAdvOptsGroup );
        vbl3->addWidget( mApplyOnIn );

        QButtonGroup *bg = new QButtonGroup( mAdvOptsGroup );

        mApplyOnForAll = new QRadioButton( i18n( "from all accounts" ), mAdvOptsGroup );
        bg->addButton( mApplyOnForAll );
        vbl3->addWidget( mApplyOnForAll );

        mApplyOnForTraditional =
                new QRadioButton( i18n( "from all but online IMAP accounts" ), mAdvOptsGroup );
        bg->addButton( mApplyOnForTraditional );
        vbl3->addWidget( mApplyOnForTraditional );

        mApplyOnForChecked =
                new QRadioButton( i18n( "from checked accounts only" ), mAdvOptsGroup );
        bg->addButton( mApplyOnForChecked );
        vbl3->addWidget( mApplyOnForChecked );
        vbl3->addStretch( 2 );

        mAccountList = new KMFilterAccountList( mAdvOptsGroup );
        gl->addWidget( mAccountList, 0, 1, 4, 3 );

        mApplyOnOut =
                new QCheckBox( i18n( "Apply this filter to &sent messages" ), mAdvOptsGroup );
        mApplyOnOut->setToolTip(
                    i18n( "<p>The filter will be triggered <b>after</b> the message is sent "
                          "and it will only affect the local copy of the message.</p>"
                          "<p>If the recipient's copy also needs to be modified, "
                          "please use \"Apply this filter <b>before</b> sending messages\".</p>" ) );
        gl->addWidget( mApplyOnOut, 4, 0, 1, 4 );

        mApplyBeforeOut =
                new QCheckBox( i18n( "Apply this filter &before sending messages" ), mAdvOptsGroup );
        mApplyBeforeOut->setToolTip(
                    i18n( "<p>The filter will be triggered <b>before</b> the message is sent "
                          "and it will affect both the local copy and the sent copy of the message.</p>"
                          "<p>This is required if the recipient's copy also needs to be modified.</p>" ) );
        gl->addWidget( mApplyBeforeOut, 5, 0, 1, 4 );


        mApplyOnCtrlJ =
                new QCheckBox( i18n( "Apply this filter on manual &filtering" ), mAdvOptsGroup );
        gl->addWidget( mApplyOnCtrlJ, 6, 0, 1, 4 );

        mStopProcessingHere =
                new QCheckBox( i18n( "If this filter &matches, stop processing here" ), mAdvOptsGroup );
        gl->addWidget( mStopProcessingHere, 7, 0, 1, 4 );

        mConfigureShortcut =
                new QCheckBox( i18n( "Add this filter to the Apply Filter menu" ), mAdvOptsGroup );
        gl->addWidget( mConfigureShortcut, 8, 0, 1, 2 );

        QLabel *keyButtonLabel = new QLabel( i18n( "Shortcut:" ), mAdvOptsGroup );
        keyButtonLabel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
        gl->addWidget( keyButtonLabel, 8, 2, 1, 1 );

        mKeySeqWidget = new KKeySequenceWidget( mAdvOptsGroup );
        mKeySeqWidget->setObjectName( QLatin1String("FilterShortcutSelector") );
        gl->addWidget( mKeySeqWidget, 8, 3, 1, 1 );
        mKeySeqWidget->setEnabled( false );
        mKeySeqWidget->setModifierlessAllowed( true );
        mKeySeqWidget->setCheckActionCollections( actionCollection );

        mConfigureToolbar =
                new QCheckBox( i18n( "Additionally add this filter to the toolbar" ), mAdvOptsGroup );
        gl->addWidget( mConfigureToolbar, 9, 0, 1, 4 );
        mConfigureToolbar->setEnabled( false );

        KHBox *hbox = new KHBox( mAdvOptsGroup );
        mFilterActionLabel = new QLabel( i18n( "Icon for this filter:" ), hbox );
        mFilterActionLabel->setEnabled( false );

        mFilterActionIconButton = new KIconButton( hbox );
        mFilterActionLabel->setBuddy( mFilterActionIconButton );
        mFilterActionIconButton->setIconType( KIconLoader::NoGroup, KIconLoader::Action, false );
        mFilterActionIconButton->setIconSize( 16 );
        mFilterActionIconButton->setIcon( QLatin1String("system-run") );
        mFilterActionIconButton->setEnabled( false );

        gl->addWidget( hbox, 10, 0, 1, 4 );

        mAdvOptsGroup->setLayout( gl );
    }
    vbl2->addWidget( mAdvOptsGroup, 0, Qt::AlignTop );

    QHBoxLayout *applySpecificFiltersLayout = new QHBoxLayout;
    QLabel *lab = new QLabel( i18n( "Run selected filter(s) on: " ) );
    applySpecificFiltersLayout->addWidget( lab );
    mFolderRequester = new MailCommon::FolderRequester;
    mFolderRequester->setNotAllowToCreateNewFolder(true);
    applySpecificFiltersLayout->addWidget( mFolderRequester );
    connect( mFolderRequester, SIGNAL(folderChanged(Akonadi::Collection)),
             this, SLOT(slotFolderChanged(Akonadi::Collection)) );
    mRunNow = new KPushButton( i18n( "Run Now" ) );
    mRunNow->setEnabled( false );
    applySpecificFiltersLayout->addWidget( mRunNow );
    connect( mRunNow, SIGNAL(clicked()), this, SLOT(slotRunFilters()) );
    topVLayout->addLayout( applySpecificFiltersLayout );
    // spacer:
    vbl->addStretch( 1 );

    // load the filter parts into the edit widgets
    connect( mFilterList, SIGNAL(filterSelected(MailCommon::MailFilter*)),
             this, SLOT(slotFilterSelected(MailCommon::MailFilter*)) );

    // transfer changes from the 'Apply this filter on...'
    // combo box to the filter
    connect( mApplyOnIn, SIGNAL(clicked()),
             this, SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnForAll, SIGNAL(clicked()),
             this, SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnForTraditional, SIGNAL(clicked()),
             this, SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnForChecked, SIGNAL(clicked()),
             this, SLOT(slotApplicabilityChanged()) );
    connect( mApplyBeforeOut, SIGNAL(clicked()),
             this, SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnOut, SIGNAL(clicked()),
             this, SLOT(slotApplicabilityChanged()) );
    connect( mApplyOnCtrlJ, SIGNAL(clicked()),
             this, SLOT(slotApplicabilityChanged()) );
    connect( mAccountList, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
             this, SLOT(slotApplicableAccountsChanged()) );

    // transfer changes from the 'stop processing here'
    // check box to the filter
    connect( mStopProcessingHere, SIGNAL(toggled(bool)),
             this, SLOT(slotStopProcessingButtonToggled(bool)) );

    connect( mConfigureShortcut, SIGNAL(toggled(bool)),
             this, SLOT(slotConfigureShortcutButtonToggled(bool)) );

    connect( mKeySeqWidget, SIGNAL(keySequenceChanged(QKeySequence)),
             this, SLOT(slotShortcutChanged(QKeySequence)) );

    connect( mConfigureToolbar, SIGNAL(toggled(bool)),
             this, SLOT(slotConfigureToolbarButtonToggled(bool)) );

    connect( mFilterActionIconButton, SIGNAL(iconChanged(QString)),
             this, SLOT(slotFilterActionIconChanged(QString)) );

    // reset all widgets here
    connect( mFilterList, SIGNAL(resetWidgets()),
             this, SLOT(slotReset()) );

    connect( mFilterList, SIGNAL(applyWidgets()),
             this, SLOT(slotUpdateFilter()) );

    // support auto-naming the filter
    connect( mPatternEdit, SIGNAL(maybeNameChanged()),
             mFilterList, SLOT(slotUpdateFilterName()) );

    // save filters on 'Apply' or 'OK'
    connect( this, SIGNAL(buttonClicked(KDialog::ButtonCode)),
             mFilterList, SLOT(slotApplyFilterChanges(KDialog::ButtonCode)) );
    connect( button( KDialog::Apply ), SIGNAL(clicked(bool)), this, SLOT(slotApply()) );

    // save dialog size on 'OK'
    connect( this, SIGNAL(okClicked()),
             this, SLOT(slotSaveSize()) );

    // destruct the dialog on close and Cancel
    connect( this, SIGNAL(closeClicked()),
             this, SLOT(slotFinished()) );
    connect( this, SIGNAL(cancelClicked()),
             this, SLOT(slotFinished()) );

    // disable closing when user wants to continue editing
    connect( mFilterList, SIGNAL(abortClosing()),
             this, SLOT(slotDisableAccept()) );

    connect( mFilterList, SIGNAL(filterCreated()), this, SLOT(slotDialogUpdated()) );
    connect( mFilterList, SIGNAL(filterRemoved(QList<MailCommon::MailFilter*>)),
             this, SLOT(slotDialogUpdated()) );
    connect( mFilterList, SIGNAL(filterUpdated(MailCommon::MailFilter*)),
             this, SLOT(slotDialogUpdated()) );
    connect( mFilterList, SIGNAL(filterOrderAltered()), this, SLOT(slotDialogUpdated()) );
    connect( mPatternEdit, SIGNAL(patternChanged()), this, SLOT(slotDialogUpdated()) );
    connect( mActionLister, SIGNAL(widgetAdded(QWidget*)), this, SLOT(slotDialogUpdated()) );
    connect( mActionLister, SIGNAL(widgetRemoved()), this, SLOT(slotDialogUpdated()) );
    connect( mActionLister, SIGNAL(filterModified()), this, SLOT(slotDialogUpdated()) );
    connect( mActionLister, SIGNAL(clearWidgets()), this, SLOT(slotDialogUpdated()) );
    KConfigGroup myGroup( KernelIf->config(), "Geometry" );
    const QSize size = myGroup.readEntry( "filterDialogSize", QSize() );
    if ( size != QSize() ) {
        resize( size );
    } else {
        adjustSize();
    }

    // load the filter list (emits filterSelected())
    mFilterList->loadFilterList( createDummyFilter );
    mIgnoreFilterUpdates = false;
}

void KMFilterDialog::createFilter(const QByteArray &field, const QString &value)
{
    mFilterList->createFilter( field, value );
}

void KMFilterDialog::accept()
{
    if ( mDoNotClose ) {
        mDoNotClose = false; // only abort current close attempt
    } else {
        KDialog::accept();
        slotFinished();
    }
}

bool KMFilterDialog::event(QEvent* e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride || e->type() == QEvent::KeyPress ) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->ignore();
            return true;
        }
    }
    return KDialog::event(e);
}


void KMFilterDialog::slotApply()
{
    enableButtonApply( false );
}

void KMFilterDialog::slotFinished()
{
    deleteLater();
}

void KMFilterDialog::slotFolderChanged( const Akonadi::Collection &collection )
{
    mRunNow->setEnabled( collection.isValid() );
}

void KMFilterDialog::slotRunFilters()
{
    if ( !mFolderRequester->collection().isValid() ) {
        KMessageBox::information(
                    this,
                    i18nc( "@info",
                           "Unable to apply this filter since there are no folders selected." ),
                    i18n( "No folder selected." ) );
        return;
    }

    if ( isButtonEnabled( KDialog::Apply ) ) {
        KMessageBox::information(
                    this,
                    i18nc( "@info",
                           "Some filters were changed and not saved yet. "
                           "You must save your filters before they can be applied." ),
                    i18n( "Filters changed." ) );
        return;
    }
    SearchRule::RequiredPart requiredPart = SearchRule::Envelope;
    const QStringList selectedFiltersId = mFilterList->selectedFilterId( requiredPart, mFolderRequester->collection().resource() );
    if ( selectedFiltersId.isEmpty() ) {
        KMessageBox::information(
                    this,
                    i18nc( "@info",
                           "Unable to apply a filter since there are no filters currently selected." ),
                    i18n( "No filters selected." ) );
        return;
    }
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( mFolderRequester->collection(), this );
    job->setProperty( "requiredPart", QVariant::fromValue( requiredPart ) );
    job->setProperty( "listFilters", QVariant::fromValue( selectedFiltersId ) );

    connect( job, SIGNAL(result(KJob*)),
             this, SLOT(slotFetchItemsForFolderDone(KJob*)) );

    mRunNow->setEnabled( false ); //Disable it
}

void KMFilterDialog::slotFetchItemsForFolderDone( KJob *job )
{
    Akonadi::ItemFetchJob *fjob = dynamic_cast<Akonadi::ItemFetchJob*>( job );
    Q_ASSERT( fjob );

    QStringList filtersId;
    if ( fjob->property( "listFilters" ).isValid() ) {
        filtersId = fjob->property( "listFilters" ).toStringList();
    }

    SearchRule::RequiredPart requiredPart = SearchRule::Envelope;
    if ( fjob->property( "requiredPart" ).isValid() ) {
        requiredPart = fjob->property( "requiredPart" ).value<SearchRule::RequiredPart>();
    }
    Akonadi::Item::List items = fjob->items();
    mRunNow->setEnabled( true );
    MailCommon::FilterManager::instance()->filter( items, requiredPart, filtersId );
}

void KMFilterDialog::slotSaveSize() {
    KConfigGroup myGroup( KernelIf->config(), "Geometry" );
    myGroup.writeEntry( "filterDialogSize", size() );
    myGroup.sync();
}

void KMFilterDialog::slotFilterSelected( MailFilter *aFilter )
{
    Q_ASSERT( aFilter );
    mIgnoreFilterUpdates = true;
    mActionLister->setActionList( aFilter->actions() );

    mAdvOptsGroup->setEnabled( true );

    mPatternEdit->setSearchPattern( aFilter->pattern() );
    mFilter = aFilter;

    kDebug() << "apply on inbound ==" << aFilter->applyOnInbound();
    kDebug() << "apply on outbound ==" << aFilter->applyOnOutbound();
    kDebug() << "apply before outbound == " << aFilter->applyBeforeOutbound();
    kDebug() << "apply on explicit ==" << aFilter->applyOnExplicit();

    // NOTE: setting these values activates the slot that sets them in
    // the filter! So make sure we have the correct values _before_ we
    // set the first one:
    const bool applyOnIn = aFilter->applyOnInbound();
    const bool applyOnForAll = aFilter->applicability() == MailFilter::All;
    const bool applyOnTraditional = aFilter->applicability() == MailFilter::ButImap;
    const bool applyBeforeOut = aFilter->applyBeforeOutbound();
    const bool applyOnOut = aFilter->applyOnOutbound();
    const bool applyOnExplicit = aFilter->applyOnExplicit();
    const bool stopHere = aFilter->stopProcessingHere();
    const bool configureShortcut = aFilter->configureShortcut();
    const bool configureToolbar = aFilter->configureToolbar();
    const QString icon = aFilter->icon();
    const KShortcut shortcut( aFilter->shortcut() );

    mApplyOnIn->setChecked( applyOnIn );
    mApplyOnForAll->setEnabled( applyOnIn );
    mApplyOnForTraditional->setEnabled( applyOnIn );
    mApplyOnForChecked->setEnabled( applyOnIn );
    mApplyOnForAll->setChecked( applyOnForAll );
    mApplyOnForTraditional->setChecked( applyOnTraditional );
    mApplyOnForChecked->setChecked( !applyOnForAll && !applyOnTraditional );
    mAccountList->setEnabled( mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() );
    slotUpdateAccountList();
    mApplyBeforeOut->setChecked( applyBeforeOut );
    mApplyOnOut->setChecked( applyOnOut );
    mApplyOnCtrlJ->setChecked( applyOnExplicit );
    mStopProcessingHere->setChecked( stopHere );
    mConfigureShortcut->setChecked( configureShortcut );
    mKeySeqWidget->setKeySequence( shortcut.primary(),
                                   KKeySequenceWidget::NoValidate );
    mConfigureToolbar->setChecked( configureToolbar );
    mFilterActionIconButton->setIcon( icon );
    mIgnoreFilterUpdates = false;
}

void KMFilterDialog::slotReset()
{
    mFilter = 0;
    mPatternEdit->reset();

    mActionLister->reset();
    mAdvOptsGroup->setEnabled( false );
    slotUpdateAccountList();
}

void KMFilterDialog::slotUpdateFilter()
{
    mPatternEdit->updateSearchPattern();
    mActionLister->updateActionList();
}

void KMFilterDialog::slotApplicabilityChanged()
{
    if ( mFilter ) {
        mFilter->setApplyOnInbound( mApplyOnIn->isChecked() );
        mFilter->setApplyBeforeOutbound( mApplyBeforeOut->isChecked() );
        mFilter->setApplyOnOutbound( mApplyOnOut->isChecked() );
        mFilter->setApplyOnExplicit( mApplyOnCtrlJ->isChecked() );
        if ( mApplyOnForAll->isChecked() ) {
            mFilter->setApplicability( MailFilter::All );
        } else if ( mApplyOnForTraditional->isChecked() ) {
            mFilter->setApplicability( MailFilter::ButImap );
        } else if ( mApplyOnForChecked->isChecked() ) {
            mFilter->setApplicability( MailFilter::Checked );
        }

        mApplyOnForAll->setEnabled( mApplyOnIn->isChecked() );
        mApplyOnForTraditional->setEnabled( mApplyOnIn->isChecked() );
        mApplyOnForChecked->setEnabled( mApplyOnIn->isChecked() );
        mAccountList->setEnabled( mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() );

        // Advanced tab functionality - Update list of accounts this filter applies to
        mAccountList->applyOnAccount(mFilter);

        // Enable the apply button
        slotDialogUpdated();

        kDebug() << "Setting filter to be applied at"
                 << ( mFilter->applyOnInbound() ? "incoming " : "" )
                 << ( mFilter->applyOnOutbound() ? "outgoing " : "" )
                 << ( mFilter->applyBeforeOutbound() ? "before_outgoing " : "" )
                 << ( mFilter->applyOnExplicit() ? "explicit CTRL-J" : "" );
    }
}

void KMFilterDialog::slotApplicableAccountsChanged()
{
    // Advanced tab functionality - Update list of accounts this filter applies to
    if ( mFilter && mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() ) {

        QTreeWidgetItemIterator it( mAccountList );

        while ( QTreeWidgetItem *item = *it ) {
            const QString id = item->text( 2 );
            mFilter->setApplyOnAccount( id, item->checkState( 0 ) == Qt::Checked );
            ++it;
        }

        // Enable the apply button
        slotDialogUpdated();
    }
}

void KMFilterDialog::slotStopProcessingButtonToggled( bool aChecked )
{
    if ( mFilter ) {
        mFilter->setStopProcessingHere( aChecked );

        // Enable the apply button
        slotDialogUpdated();
    }
}

void KMFilterDialog::slotConfigureShortcutButtonToggled( bool aChecked )
{
    if ( mFilter ) {
        mFilter->setConfigureShortcut( aChecked );
        mKeySeqWidget->setEnabled( aChecked );
        mConfigureToolbar->setEnabled( aChecked );
        mFilterActionIconButton->setEnabled( aChecked );
        mFilterActionLabel->setEnabled( aChecked );

        // Enable the apply button
        slotDialogUpdated();
    }
}

void KMFilterDialog::slotShortcutChanged( const QKeySequence &newSeq )
{
    if ( mFilter ) {
        mKeySeqWidget->applyStealShortcut();
        mFilter->setShortcut( KShortcut( newSeq ) );

        // Enable the apply button
        slotDialogUpdated();
    }
}

void KMFilterDialog::slotConfigureToolbarButtonToggled( bool aChecked )
{
    if ( mFilter ) {
        mFilter->setConfigureToolbar( aChecked );
        // Enable the apply button
        slotDialogUpdated();
    }
}

void KMFilterDialog::slotFilterActionIconChanged( const QString &icon )
{
    if ( mFilter ) {
        mFilter->setIcon( icon );
        // Enable the apply button
        slotDialogUpdated();
    }
}

void KMFilterDialog::slotUpdateAccountList()
{
    mAccountList->updateAccountList(mFilter);
}

void KMFilterDialog::slotImportFilter( QAction *act )
{
    if ( act ) {
        importFilters( act->data().value<MailCommon::FilterImporterExporter::FilterType>() );
    }
}

void KMFilterDialog::importFilters( MailCommon::FilterImporterExporter::FilterType type )
{
    FilterImporterExporter importer( this );
    bool canceled = false;
    QList<MailFilter *> filters = importer.importFilters( canceled, type );
    if ( canceled ) {
        return;
    }

    if ( filters.isEmpty() ) {
        KMessageBox::information( this, i18n( "No filter was imported." ) );
        return;
    }
    QStringList listOfFilter;
    QList<MailFilter*>::ConstIterator end( filters.constEnd() );

    for ( QList<MailFilter*>::ConstIterator it = filters.constBegin(); it != end; ++it ) {
        mFilterList->appendFilter( *it ); // no need to deep copy, ownership passes to the list
        listOfFilter << (*it)->name();
    }

    KMessageBox::informationList(
                this,
                i18n( "Filters which were imported:" ),
                listOfFilter );
}

void KMFilterDialog::slotExportFilters()
{
    FilterImporterExporter exporter( this );
    QList<MailFilter *> filters = mFilterList->filtersForSaving( false );
    exporter.exportFilters( filters );
}

void KMFilterDialog::slotDisableAccept()
{
    mDoNotClose = true;
}

void KMFilterDialog::slotDialogUpdated()
{
    kDebug() << "Detected a change in data bound to the dialog!";
    if ( !mIgnoreFilterUpdates ) {
        enableButtonApply( true );
    }
}

void KMFilterDialog::slotExportAsSieveScript()
{
    if ( isButtonEnabled( KDialog::Apply ) ) {
        KMessageBox::information(
                    this,
                    i18nc( "@info",
                           "Some filters were changed and not saved yet.<br>"
                           "You must save your filters before they can be exported." ),
                    i18n( "Filters changed." ) );
        return;
    }
    KMessageBox::information(this, i18n("We cannot convert all KMail filters to sieve scripts but we can try :)"), i18n("Convert KMail filters to sieve scripts"));
    QList<MailFilter *> filters = mFilterList->filtersForSaving( false );
    QPointer<FilterSelectionDialog> dlg = new FilterSelectionDialog( this );
    dlg->setFilters( filters );
    if ( dlg->exec() == QDialog::Accepted ) {
        QList<MailFilter*> lst = dlg->selectedFilters();
        if (!lst.isEmpty()) {
            FilterConvertToSieve convert(lst);
            convert.convert();
            qDeleteAll(lst);
        } else {
            KMessageBox::information(this, i18n("No filters selected."), i18n("Convert KMail filters to sieve scripts"));
        }
    }
    delete dlg;
}

}

