/*
  This file is part of KAddressBook.

  Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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

#include "mainwidget.h"
#include "contactswitcher.h"
#include "globalcontactmodel.h"
#include "modelcolumnmanager.h"
#include "printing/printingwizard.h"
#include "merge/mergecontactsdialog.h"
#include "merge/mergecontactduplicatecontactdialog.h"
#include "quicksearchwidget.h"
#include "settings.h"
#include "xxportmanager.h"
#include "utils.h"
#include "kaddressbookadaptor.h"
#include "categoryselectwidget.h"
#include "categoryfilterproxymodel.h"

#include "sendmail/mailsenderjob.h"

#include "kaddressbookgrantlee/formatter/grantleecontactformatter.h"
#include "kaddressbookgrantlee/formatter/grantleecontactgroupformatter.h"
#include "grantleetheme/grantleethememanager.h"
#include "grantleetheme/globalsettings_base.h"

#include "libkdepim/misc/uistatesaver.h"

#include <pimcommon/acl/collectionaclpage.h>
#include <pimcommon/acl/imapaclattribute.h>

#include <AkonadiWidgets/ETMViewStateSaver>
#include <AkonadiCore/CollectionFilterProxyModel>
#include <AkonadiCore/CollectionModel>
#include <AkonadiCore/Control>
#include <AkonadiCore/EntityMimeTypeFilterModel>
#include <AkonadiWidgets/EntityTreeView>
#include <AkonadiWidgets/ItemView>
#include <AkonadiCore/MimeTypeChecker>
#include <AkonadiCore/AttributeFactory>
#include <AkonadiWidgets/CollectionPropertiesDialog>
#include <pimcommon/baloodebug/baloodebugdialog.h>

#include <QtCore/QPointer>

#include <Akonadi/Contact/ContactDefaultActions>
#include <Akonadi/Contact/ContactGroupEditorDialog>
#include <Akonadi/Contact/ContactGroupViewer>
#include <Akonadi/Contact/ContactsFilterProxyModel>
#include <Akonadi/Contact/ContactsTreeModel>
#include <Akonadi/Contact/ContactViewer>
#include <Akonadi/Contact/StandardContactActionManager>

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <QDebug>
#include <KCmdLineArgs>
#include <QAction>
#include <KActionCollection>
#include <KActionMenu>
#include <KApplication>
#include <KCheckableProxyModel>
#include <kdescendantsproxymodel.h> //krazy:exclude=camelcase TODO wait for kdelibs4.9
#include <KLocalizedString>
#include <KSelectionProxyModel>
#include <QTextBrowser>
#include <KToggleAction>
#include <KCMultiDialog>
#include <kdeprintdialog.h>
#include <KPrintPreview>
#include <KXMLGUIClient>
#include <KIconLoader>
#include <KMessageBox>

#include <QAction>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPrinter>
#include <QPrintDialog>
#include <QSplitter>
#include <QStackedWidget>
#include <QDBusConnection>
#include <QDesktopServices>

namespace
{
static bool isStructuralCollection(const Akonadi::Collection &collection)
{
    QStringList mimeTypes;
    mimeTypes << KContacts::Addressee::mimeType() << KContacts::ContactGroup::mimeType();
    const QStringList collectionMimeTypes = collection.contentMimeTypes();
    foreach (const QString &mimeType, mimeTypes) {
        if (collectionMimeTypes.contains(mimeType)) {
            return false;
        }
    }
    return true;
}

class StructuralCollectionsNotCheckableProxy : public KCheckableProxyModel
{
public:
    StructuralCollectionsNotCheckableProxy(QObject *parent)
        : KCheckableProxyModel(parent)
    {
    }

    /* reimp */QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid()) {
            return QVariant();
        }

        if (role == Qt::CheckStateRole) {
            // Don't show the checkbox if the collection can't contain incidences
            const Akonadi::Collection collection =
                index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (collection.isValid() && isStructuralCollection(collection)) {
                return QVariant();
            }
        }
        return KCheckableProxyModel::data(index, role);
    }
};

}

MainWidget::MainWidget(KXMLGUIClient *guiClient, QWidget *parent)
    : QWidget(parent), mAllContactsModel(0), mXmlGuiClient(guiClient), mGrantleeThemeManager(0), mQuickSearchAction(0)
{

    (void) new KaddressbookAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/KAddressBook"), this);

    mXXPortManager = new XXPortManager(this);
    Akonadi::AttributeFactory::registerAttribute<PimCommon::ImapAclAttribute>();

    setupGui();
    setupActions(guiClient->actionCollection());

    /*
    *  The item models, proxies and views have the following structure:
    *
    *                               mItemView
    *                                   ^
    *                                   |
    *                           mContactsFilterModel
    *                                   ^
    *                                   |
    * mCategorySelectWidget --> mCategoryFilterModel
    *                                   ^
    *                                   |
    *                               mItemTree
    *                                   ^
    *                                   |
    *                                   |           mAllContactsModel
    *                                   |                  ^
    *                                   |                  |
    *      mCollectionView     selectionProxyModel  descendantsModel
    *            ^               ^      ^                  ^
    *            |               |      |                  |
    *            |       selectionModel |                  |
    *            |               |      |                  |
    *        proxyModel ---------'      |                  |
    *            ^                      |                  |
    *            |                      |                  |
    *      mCollectionTree              |                  |
    *            ^                      |                  |
    *            |                      |   _______________/
    *             \                    /  /
    *            GlobalContactModel::instance()
    *
    *
    *  GlobalContactModel::instance():  The global contact model (contains collections and items)
    *                 mCollectionTree:  Filters out all items
    *                      proxyModel:  Allows the user to select collections by checkboxes
    *                  selectionModel:  Represents the selected collections that have been
    *                                   selected in proxyModel
    *                 mCollectionView:  Shows the collections (address books) in a view
    *             selectionProxyModel:  Filters out all collections and items that are no children
    *                                   of the collections currently selected in selectionModel
    *                       mItemTree:  Filters out all collections
    *           mCategorySelectWidget:  Selects a list of categories for filtering
    *            mCategoryFilterModel:  Filters the contacts by the selected categories
    *            mContactsFilterModel:  Filters the contacts by the content of mQuickSearchWidget
    *                       mItemView:  Shows the items (contacts and contact groups) in a view
    *
    *                descendantsModel:  Flattens the item/collection tree to a list
    *               mAllContactsModel:  Provides a list of all available contacts from all
    *                                   address books
    */

    mCollectionTree = new Akonadi::EntityMimeTypeFilterModel(this);
    mCollectionTree->setDynamicSortFilter(true);
    mCollectionTree->setSortCaseSensitivity(Qt::CaseInsensitive);
    mCollectionTree->setSourceModel(GlobalContactModel::instance()->model());
    mCollectionTree->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
    mCollectionTree->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);

    mCollectionSelectionModel = new QItemSelectionModel(mCollectionTree);
    StructuralCollectionsNotCheckableProxy *checkableProxyModel =
        new StructuralCollectionsNotCheckableProxy(this);
    checkableProxyModel->setSelectionModel(mCollectionSelectionModel);
    checkableProxyModel->setSourceModel(mCollectionTree);

    mCollectionView->setModel(checkableProxyModel);
    mCollectionView->setXmlGuiClient(guiClient);
    mCollectionView->header()->setDefaultAlignment(Qt::AlignCenter);
    mCollectionView->header()->setSortIndicatorShown(false);

    connect(mCollectionView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(slotCheckNewCalendar(QModelIndex,int,int)));

    connect(mCollectionView, SIGNAL(currentChanged(Akonadi::Collection)),
            mXXPortManager, SLOT(setDefaultAddressBook(Akonadi::Collection)));

    KSelectionProxyModel *selectionProxyModel =
        new KSelectionProxyModel(mCollectionSelectionModel, this);
    selectionProxyModel->setSourceModel(GlobalContactModel::instance()->model());
    selectionProxyModel->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);

    mItemTree = new Akonadi::EntityMimeTypeFilterModel(this);
    mItemTree->setSourceModel(selectionProxyModel);
    mItemTree->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
    mItemTree->setHeaderGroup(Akonadi::EntityTreeModel::ItemListHeaders);

    mCategoryFilterModel = new CategoryFilterProxyModel(this);
    mCategoryFilterModel->setSourceModel(mItemTree);
    mCategoryFilterModel->setFilterCategories(mCategorySelectWidget->filterTags());
    mCategoryFilterModel->setFilterEnabled(true);

    connect(mCategorySelectWidget, SIGNAL(filterChanged(QList<Akonadi::Tag::Id>)),
            mCategoryFilterModel, SLOT(setFilterCategories(QList<Akonadi::Tag::Id>)));

    mContactsFilterModel = new Akonadi::ContactsFilterProxyModel(this);
    mContactsFilterModel->setSourceModel(mCategoryFilterModel);

    connect(mQuickSearchWidget, SIGNAL(filterStringChanged(QString)),
            mContactsFilterModel, SLOT(setFilterString(QString)));
    connect(mQuickSearchWidget, SIGNAL(filterStringChanged(QString)),
            this, SLOT(selectFirstItem()));
    connect(mQuickSearchWidget, SIGNAL(arrowDownKeyPressed()),
            mItemView, SLOT(setFocus()));

    mItemView->setModel(mContactsFilterModel);
    mItemView->setXmlGuiClient(guiClient);
    mItemView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mItemView->setRootIsDecorated(false);
    mItemView->header()->setDefaultAlignment(Qt::AlignCenter);

    mXXPortManager->setSelectionModel(mItemView->selectionModel());

    mActionManager = new Akonadi::StandardContactActionManager(guiClient->actionCollection(), this);
    mActionManager->setCollectionSelectionModel(mCollectionView->selectionModel());
    mActionManager->setItemSelectionModel(mItemView->selectionModel());

    QList<Akonadi::StandardActionManager::Type> standardActions;
    standardActions << Akonadi::StandardActionManager::CreateCollection
                    << Akonadi::StandardActionManager::CopyCollections
                    << Akonadi::StandardActionManager::DeleteCollections
                    << Akonadi::StandardActionManager::SynchronizeCollections
                    << Akonadi::StandardActionManager::CollectionProperties
                    << Akonadi::StandardActionManager::CopyItems
                    << Akonadi::StandardActionManager::Paste
                    << Akonadi::StandardActionManager::DeleteItems
                    << Akonadi::StandardActionManager::CutItems
                    << Akonadi::StandardActionManager::CutCollections
                    << Akonadi::StandardActionManager::CreateResource
                    << Akonadi::StandardActionManager::DeleteResources
                    << Akonadi::StandardActionManager::ResourceProperties
                    << Akonadi::StandardActionManager::SynchronizeResources
                    << Akonadi::StandardActionManager::SynchronizeCollectionsRecursive;

    Q_FOREACH (Akonadi::StandardActionManager::Type standardAction, standardActions) {
        mActionManager->createAction(standardAction);
    }

    QList<Akonadi::StandardContactActionManager::Type> contactActions;
    contactActions << Akonadi::StandardContactActionManager::CreateContact
                   << Akonadi::StandardContactActionManager::CreateContactGroup
                   << Akonadi::StandardContactActionManager::EditItem;

    Q_FOREACH (Akonadi::StandardContactActionManager::Type contactAction, contactActions) {
        mActionManager->createAction(contactAction);
    }
    static bool pageRegistered = false;

    if (!pageRegistered) {
        Akonadi::CollectionPropertiesDialog::registerPage(new PimCommon::CollectionAclPageFactory);
        pageRegistered = true;
    }

    const QStringList pages =
        QStringList() << QLatin1String("Akonadi::CollectionGeneralPropertiesPage")
        << QLatin1String("Akonadi::CachePolicyPage")
        << QLatin1String("PimCommon::CollectionAclPage");

    mActionManager->setCollectionPropertiesPageNames(pages);

    connect(mItemView, SIGNAL(currentChanged(Akonadi::Item)),
            this, SLOT(itemSelected(Akonadi::Item)));
    connect(mItemView, SIGNAL(doubleClicked(Akonadi::Item)),
            mActionManager->action(Akonadi::StandardContactActionManager::EditItem),
            SLOT(trigger()));
    connect(mItemView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(itemSelectionChanged(QModelIndex,QModelIndex)));

    // show the contact details view as default
    mDetailsViewStack->setCurrentWidget(mContactDetails);

    mContactSwitcher->setView(mItemView);

    Akonadi::Control::widgetNeedsAkonadi(this);

    mModelColumnManager = new ModelColumnManager(GlobalContactModel::instance()->model(), this);
    mModelColumnManager->setWidget(mItemView->header());
    mModelColumnManager->load();

    QMetaObject::invokeMethod(this, "delayedInit", Qt::QueuedConnection);
    updateQuickSearchText();
}

void MainWidget::configure()
{
    KCMultiDialog dlg(this);
    dlg.addModule(QLatin1String("akonadicontact_actions.desktop"));
    dlg.addModule(QLatin1String("kcmldap.desktop"));

    dlg.exec();
}

bool MainWidget::handleCommandLine()
{
    bool doneSomething = false;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("import")) {
        for (int i = 0; i < args->count(); ++i) {
            importManager()->importFile(args->url(i));
        }
    }
    args->clear();
    return doneSomething;
}

XXPortManager *MainWidget::importManager() const
{
    return mXXPortManager;
}

void MainWidget::updateQuickSearchText()
{
    mQuickSearchWidget->updateQuickSearchText(i18nc("@label Search contacts in list", "Search...<%1>", mQuickSearchAction->shortcut().toString()));
}

void MainWidget::delayedInit()
{
    setViewMode(0);                                        // get default from settings

    const KConfigGroup group(Settings::self()->config(), "UiState_ContactView");
    KPIM::UiStateSaver::restoreState(mItemView, group);

#if defined(HAVE_PRISON)
    mXmlGuiClient->
    actionCollection()->
    action(QLatin1String("options_show_qrcodes"))->setChecked(showQRCodes());
#endif

    connect(GlobalContactModel::instance()->model(), SIGNAL(modelAboutToBeReset()),
            SLOT(saveState()));
    connect(GlobalContactModel::instance()->model(), SIGNAL(modelReset()),
            SLOT(restoreState()));
    connect(qApp, &QApplication::aboutToQuit, this, &MainWidget::saveState);

    restoreState();
    updateQuickSearchText();
}

MainWidget::~MainWidget()
{
    mModelColumnManager->store();
    saveSplitterStates();

    KConfigGroup group(Settings::self()->config(), "UiState_ContactView");
    KPIM::UiStateSaver::saveState(mItemView, group);

    saveState();
    delete mGrantleeThemeManager;

    Settings::self()->save();
}

void MainWidget::restoreState()
{
    // collection view
    {
        Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
        saver->setView(mCollectionView);

        const KConfigGroup group(Settings::self()->config(), "CollectionViewState");
        saver->restoreState(group);
    }

    // collection view
    {
        Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
        saver->setSelectionModel(mCollectionSelectionModel);

        const KConfigGroup group(Settings::self()->config(), "CollectionViewCheckState");
        saver->restoreState(group);
    }

    // item view
    {
        Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
        saver->setView(mItemView);
        saver->setSelectionModel(mItemView->selectionModel());

        const KConfigGroup group(Settings::self()->config(), "ItemViewState");
        saver->restoreState(group);
    }
}

void MainWidget::saveState()
{
    // collection view
    {
        Akonadi::ETMViewStateSaver saver;
        saver.setView(mCollectionView);

        KConfigGroup group(Settings::self()->config(), "CollectionViewState");
        saver.saveState(group);
        group.sync();
    }

    // collection view
    {
        Akonadi::ETMViewStateSaver saver;
        saver.setSelectionModel(mCollectionSelectionModel);

        KConfigGroup group(Settings::self()->config(), "CollectionViewCheckState");
        saver.saveState(group);
        group.sync();
    }

    // item view
    {
        Akonadi::ETMViewStateSaver saver;
        saver.setView(mItemView);
        saver.setSelectionModel(mItemView->selectionModel());

        KConfigGroup group(Settings::self()->config(), "ItemViewState");
        saver.saveState(group);
        group.sync();
    }
}

void MainWidget::setupGui()
{
    // the horizontal main layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    // Splitter 1 contains the two main parts of the GUI:
    //  - collection and item view splitter 2 on the left (see below)
    //  - details pane on the right, that contains
    //   - details view stack on the top
    //   - contact switcher at the bottom
    mMainWidgetSplitter1 = new QSplitter(Qt::Horizontal);
    mMainWidgetSplitter1->setObjectName(QLatin1String("MainWidgetSplitter1"));
    layout->addWidget(mMainWidgetSplitter1);

    // Splitter 2 contains the remaining parts of the GUI:
    //  - collection view on either the left or the top
    //  - item view on either the right or the bottom
    // The orientation of this splitter is changed for either
    // a three or two column view;  in simple mode it is hidden.
    mMainWidgetSplitter2 = new QSplitter(Qt::Vertical);
    mMainWidgetSplitter2->setObjectName(QLatin1String("MainWidgetSplitter2"));
    mMainWidgetSplitter1->addWidget(mMainWidgetSplitter2);

    // the collection view
    mCollectionView = new Akonadi::EntityTreeView();
    mMainWidgetSplitter2->addWidget(mCollectionView);

    // the items view
    mItemView = new Akonadi::EntityTreeView();
    mItemView->setObjectName(QLatin1String("ContactView"));
    mItemView->setDefaultPopupMenu(QLatin1String("akonadi_itemview_contextmenu"));
    mItemView->setAlternatingRowColors(true);
    mMainWidgetSplitter2->addWidget(mItemView);

    // the details pane that contains the details view stack and contact switcher
    mDetailsPane = new QWidget;
    mMainWidgetSplitter1->addWidget(mDetailsPane);

    mMainWidgetSplitter1->setStretchFactor(1, 9);          // maximum width for detail
    mMainWidgetSplitter2->setStretchFactor(1, 9);          // for intuitive resizing
    mMainWidgetSplitter2->setChildrenCollapsible(false);
    mMainWidgetSplitter1->setChildrenCollapsible(false);

    QVBoxLayout *detailsPaneLayout = new QVBoxLayout(mDetailsPane);
    detailsPaneLayout->setMargin(0);

    // the details view stack
    mDetailsViewStack = new QStackedWidget();
    detailsPaneLayout->addWidget(mDetailsViewStack);

    // the details widget for contacts
    mContactDetails = new Akonadi::ContactViewer(mDetailsViewStack);
    mDetailsViewStack->addWidget(mContactDetails);

    // the details widget for contact groups
    mContactGroupDetails = new Akonadi::ContactGroupViewer(mDetailsViewStack);
    mDetailsViewStack->addWidget(mContactGroupDetails);

    // the details widget for empty items
    mEmptyDetails = new QTextBrowser(mDetailsViewStack);
    mDetailsViewStack->addWidget(mEmptyDetails);

    // the contact switcher for the simple gui mode
    mContactSwitcher = new ContactSwitcher;
    detailsPaneLayout->addWidget(mContactSwitcher);
    mContactSwitcher->setVisible(false);

    // the quick search widget which is embedded in the toolbar action
    mQuickSearchWidget = new QuickSearchWidget;

    // the category filter widget which is embedded in the toolbar action
    mCategorySelectWidget = new CategorySelectWidget;

    // setup the default actions
    Akonadi::ContactDefaultActions *actions = new Akonadi::ContactDefaultActions(this);
    actions->connectToView(mContactDetails);
    actions->connectToView(mContactGroupDetails);
    mFormatter = new KAddressBookGrantlee::GrantleeContactFormatter;

    mContactDetails->setContactFormatter(mFormatter);

    mGroupFormatter = new KAddressBookGrantlee::GrantleeContactGroupFormatter;

    mContactGroupDetails->setContactGroupFormatter(mGroupFormatter);
}

void MainWidget::setupActions(KActionCollection *collection)
{
    mGrantleeThemeManager = new GrantleeTheme::GrantleeThemeManager(GrantleeTheme::GrantleeThemeManager::Addressbook, QStringLiteral("theme.desktop"), collection, QLatin1String("kaddressbook/viewertemplates/"));
    mGrantleeThemeManager->setDownloadNewStuffConfigFile(QLatin1String("kaddressbook_themes.knsrc"));
    connect(mGrantleeThemeManager, &GrantleeTheme::GrantleeThemeManager::grantleeThemeSelected, this, &MainWidget::slotGrantleeThemeSelected);
    connect(mGrantleeThemeManager, &GrantleeTheme::GrantleeThemeManager::updateThemes, this, &MainWidget::slotGrantleeThemesUpdated);

    KActionMenu *themeMenu  = new KActionMenu(i18n("&Themes"), this);
    collection->addAction(QLatin1String("theme_menu"), themeMenu);

    initGrantleeThemeName();
    QActionGroup *group = new QActionGroup(this);
    mGrantleeThemeManager->setThemeMenu(themeMenu);
    mGrantleeThemeManager->setActionGroup(group);

    QAction *action = KStandardAction::print(this, SLOT(print()), collection);
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "Print the complete address book or a selected number of contacts."));

    if (KPrintPreview::isAvailable()) {
        KStandardAction::printPreview(this, SLOT(printPreview()), collection);
    }

    QWidgetAction *quicksearch = new QWidgetAction(this);
    quicksearch->setText(i18n("Quick search"));
    quicksearch->setDefaultWidget(mQuickSearchWidget);
    collection->addAction(QLatin1String("quick_search"), quicksearch);

    QWidgetAction *categoryFilter = new QWidgetAction(this);
    categoryFilter->setText(i18n("Category filter"));
    categoryFilter->setDefaultWidget(mCategorySelectWidget);
    collection->addAction(QLatin1String("category_filter"), categoryFilter);

    action = collection->addAction(QLatin1String("select_all"));
    action->setText(i18n("Select All"));
    collection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_A));
    action->setWhatsThis(i18n("Select all contacts in the current address book view."));
    connect(action, &QAction::triggered, mItemView, &Akonadi::EntityTreeView::selectAll);

#if defined(HAVE_PRISON)
    KToggleAction *qrtoggleAction;
    qrtoggleAction = collection->add<KToggleAction>(QLatin1String("options_show_qrcodes"));
    qrtoggleAction->setText(i18n("Show QR Codes"));
    qrtoggleAction->setWhatsThis(i18n("Show QR Codes in the contact."));
    connect(qrtoggleAction, &KToggleAction::toggled, this, &MainWidget::setQRCodeShow);
#endif

    mViewModeGroup = new QActionGroup(this);

    QAction *act = new QAction(i18nc("@action:inmenu", "Simple (one column)"), mViewModeGroup);
    act->setCheckable(true);
    act->setData(1);
    collection->setDefaultShortcut(act, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_1));
    act->setWhatsThis(i18n("Show a simple mode of the address book view."));
    collection->addAction(QLatin1String("view_mode_simple"), act);

    act = new QAction(i18nc("@action:inmenu", "Two Columns"), mViewModeGroup);
    act->setCheckable(true);
    act->setData(2);
    collection->addAction(QLatin1String("view_mode_2columns"), act);
    collection->setDefaultShortcut(act,  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_2));

    act = new QAction(i18nc("@action:inmenu", "Three Columns"), mViewModeGroup);
    act->setCheckable(true);
    act->setData(3);
    collection->setDefaultShortcut(act, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_3));
    collection->addAction(QLatin1String("view_mode_3columns"), act);

    connect(mViewModeGroup, SIGNAL(triggered(QAction*)), SLOT(setViewMode(QAction*)));

    // import actions
    action = collection->addAction(QLatin1String("file_import_vcard"));
    action->setText(i18n("Import vCard..."));
    action->setWhatsThis(i18n("Import contacts from a vCard file."));
    mXXPortManager->addImportAction(action, QLatin1String("vcard30"));

    action = collection->addAction(QLatin1String("file_import_csv"));
    action->setText(i18n("Import CSV file..."));
    action->setWhatsThis(i18n("Import contacts from a file in comma separated value format."));
    mXXPortManager->addImportAction(action, QLatin1String("csv"));

    action = collection->addAction(QLatin1String("file_import_ldif"));
    action->setText(i18n("Import LDIF file..."));
    action->setWhatsThis(i18n("Import contacts from an LDIF file."));
    mXXPortManager->addImportAction(action, QLatin1String("ldif"));

    action = collection->addAction(QLatin1String("file_import_ldap"));
    action->setText(i18n("Import From LDAP server..."));
    action->setWhatsThis(i18n("Import contacts from an LDAP server."));
    mXXPortManager->addImportAction(action, QLatin1String("ldap"));

    action = collection->addAction(QLatin1String("file_import_gmx"));
    action->setText(i18n("Import GMX file..."));
    action->setWhatsThis(i18n("Import contacts from a GMX address book file."));
    mXXPortManager->addImportAction(action, QLatin1String("gmx"));

    // export actions
    action = collection->addAction(QLatin1String("file_export_vcard40"));
    action->setText(i18n("Export vCard 4.0..."));
    action->setWhatsThis(i18n("Export contacts to a vCard 4.0 file."));
    mXXPortManager->addExportAction(action, QLatin1String("vcard40"));

    action = collection->addAction(QLatin1String("file_export_vcard30"));
    action->setText(i18n("Export vCard 3.0..."));
    action->setWhatsThis(i18n("Export contacts to a vCard 3.0 file."));
    mXXPortManager->addExportAction(action, QLatin1String("vcard30"));

    action = collection->addAction(QLatin1String("file_export_vcard21"));
    action->setText(i18n("Export vCard 2.1..."));
    action->setWhatsThis(i18n("Export contacts to a vCard 2.1 file."));
    mXXPortManager->addExportAction(action, QLatin1String("vcard21"));

    action = collection->addAction(QLatin1String("file_export_csv"));
    action->setText(i18n("Export CSV file..."));
    action->setWhatsThis(i18n("Export contacts to a file in comma separated value format."));
    mXXPortManager->addExportAction(action, QLatin1String("csv"));

    action = collection->addAction(QLatin1String("file_export_ldif"));
    action->setText(i18n("Export LDIF file..."));
    action->setWhatsThis(i18n("Export contacts to an LDIF file."));
    mXXPortManager->addExportAction(action, QLatin1String("ldif"));

    action = collection->addAction(QLatin1String("file_export_gmx"));
    action->setText(i18n("Export GMX file..."));
    action->setWhatsThis(i18n("Export contacts to a GMX address book file."));
    mXXPortManager->addExportAction(action, QLatin1String("gmx"));

    KToggleAction *actTheme = mGrantleeThemeManager->actionForTheme();
    if (actTheme) {
        actTheme->setChecked(true);
    }

    action = collection->addAction(QLatin1String("merge_contacts"));
    action->setText(i18n("Merge Contacts..."));
    connect(action, &QAction::triggered, this, &MainWidget::mergeContacts);

    action = collection->addAction(QLatin1String("search_duplicate_contacts"));
    action->setText(i18n("Search Duplicate Contacts..."));
    connect(action, &QAction::triggered, this, &MainWidget::slotSearchDuplicateContacts);

    mQuickSearchAction = new QAction(i18n("Set Focus to Quick Search"), this);
    //If change shortcut change in quicksearchwidget->lineedit->setPlaceholderText
    collection->addAction(QLatin1String("focus_to_quickseach"), mQuickSearchAction);
    connect(mQuickSearchAction, &QAction::triggered, mQuickSearchWidget, &QuickSearchWidget::slotFocusQuickSearch);
    collection->setDefaultShortcut(mQuickSearchAction, QKeySequence(Qt::ALT + Qt::Key_Q));

    action = collection->addAction(QLatin1String("send_mail"));
    action->setText(i18n("Send an email..."));
    action->setIcon(KIconLoader::global()->loadIcon(QLatin1String("mail-message-new"), KIconLoader::Small));
    connect(action, &QAction::triggered, this, &MainWidget::slotSendMail);

    if (!qgetenv("KDEPIM_BALOO_DEBUG").isEmpty()) {
        action = collection->addAction(QLatin1String("debug_baloo"));
        //Don't translate it. It's just for debug
        action->setText(QLatin1String("Debug baloo..."));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(slotDebugBaloo()));
    }
}

void MainWidget::printPreview()
{
    QPrinter printer;
    printer.setDocName(i18n("Address Book"));
    printer.setOutputFileName(Settings::self()->defaultFileName());
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setCollateCopies(true);

    KPrintPreview previewdlg(&printer, this);
    KABPrinting::PrintingWizard wizard(&printer, mItemView->selectionModel(), this);
    wizard.setDefaultAddressBook(currentAddressBook());

    const int result = wizard.exec();
    if (result) {
        Settings::self()->setDefaultFileName(printer.outputFileName());
        Settings::self()->setPrintingStyle(wizard.printingStyle());
        Settings::self()->setSortOrder(wizard.sortOrder());
        previewdlg.exec();
    }
}

void MainWidget::print()
{
    QPrinter printer;
    printer.setDocName(i18n("Address Book"));
    printer.setOutputFileName(Settings::self()->defaultFileName());
    printer.setCollateCopies(true);

    QPointer<QPrintDialog> printDialog = KdePrint::createPrintDialog(&printer, this);

    printDialog->setWindowTitle(i18n("Print Contacts"));
    if (!printDialog->exec() || !printDialog) {
        delete printDialog;
        return;
    }
    KABPrinting::PrintingWizard wizard(&printer, mItemView->selectionModel(), this);
    wizard.setDefaultAddressBook(currentAddressBook());

    wizard.exec(); //krazy:exclude=crashy

    Settings::self()->setDefaultFileName(printer.outputFileName());
    Settings::self()->setPrintingStyle(wizard.printingStyle());
    Settings::self()->setSortOrder(wizard.sortOrder());
}

void MainWidget::newContact()
{
    mActionManager->action(Akonadi::StandardContactActionManager::CreateContact)->trigger();
}

void MainWidget::newGroup()
{
    mActionManager->action(Akonadi::StandardContactActionManager::CreateContactGroup)->trigger();
}

/**
 * Depending on the mime type of the selected item, this method
 * brings up the right view on the detail view stack and sets the
 * selected item on it.
 */
void MainWidget::itemSelected(const Akonadi::Item &item)
{
    if (Akonadi::MimeTypeChecker::isWantedItem(item, KContacts::Addressee::mimeType())) {
        mDetailsViewStack->setCurrentWidget(mContactDetails);
        mContactDetails->setContact(item);
    } else if (Akonadi::MimeTypeChecker::isWantedItem(item, KContacts::ContactGroup::mimeType())) {
        mDetailsViewStack->setCurrentWidget(mContactGroupDetails);
        mContactGroupDetails->setContactGroup(item);
    }
}

/**
 * Catch when the selection has gone ( e.g. an empty address book has been selected )
 * clear the details view in this case.
 */
void MainWidget::itemSelectionChanged(const QModelIndex &current, const QModelIndex &)
{
    if (!current.isValid()) {
        mDetailsViewStack->setCurrentWidget(mEmptyDetails);
    }
}

void MainWidget::selectFirstItem()
{
    // Whenever the quick search has changed, we select the first item
    // in the item view, so that the detailsview is updated
    if (mItemView && mItemView->selectionModel()) {
        mItemView->selectionModel()->setCurrentIndex(mItemView->model()->index(0, 0),
                QItemSelectionModel::Rows |
                QItemSelectionModel::ClearAndSelect);
    }
}

bool MainWidget::showQRCodes()
{
#if defined(HAVE_PRISON)
    KConfig config(QLatin1String("akonadi_contactrc"));
    KConfigGroup group(&config, QLatin1String("View"));
    return group.readEntry("QRCodes", true);
#else
    return true;
#endif
}

void MainWidget::setQRCodeShow(bool on)
{
#if defined(HAVE_PRISON)
    // must write the configuration setting first before updating the view.
    KConfig config(QLatin1String("akonadi_contactrc"));
    KConfigGroup group(&config, QLatin1String("View"));
    group.writeEntry("QRCodes", on);
    if (mItemView->model()) {
        mItemView->setCurrentIndex(mItemView->model()->index(0, 0));
    }
#else
    Q_UNUSED(on);
#endif
}

Akonadi::Collection MainWidget::currentAddressBook() const
{
    if (mCollectionView->selectionModel() && mCollectionView->selectionModel()->hasSelection()) {
        const QModelIndex index = mCollectionView->selectionModel()->selectedIndexes().first();
        const Akonadi::Collection collection =
            index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

        return collection;
    }

    return Akonadi::Collection();
}

QAbstractItemModel *MainWidget::allContactsModel()
{
    if (!mAllContactsModel) {
        KDescendantsProxyModel *descendantsModel = new KDescendantsProxyModel(this);
        descendantsModel->setSourceModel(GlobalContactModel::instance()->model());

        mAllContactsModel = new Akonadi::EntityMimeTypeFilterModel(this);
        mAllContactsModel->setSourceModel(descendantsModel);
        mAllContactsModel->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
        mAllContactsModel->setHeaderGroup(Akonadi::EntityTreeModel::ItemListHeaders);
    }

    return mAllContactsModel;
}

void MainWidget::setViewMode(QAction *action)
{
    setViewMode(action->data().toInt());
}

void MainWidget::setViewMode(int mode)
{
    int currentMode = Settings::self()->viewMode();
    //qDebug() << "cur" << currentMode << "new" << mode;
    if (mode == currentMode) {
        return;    // nothing to do
    }

    if (mode == 0) {
        mode = currentMode;// initialisation, no save
    } else {
        saveSplitterStates();                                // for 2- or 3-column mode
    }
    if (mode == 1) {                                          // simple mode
        mMainWidgetSplitter2->setVisible(false);
        mDetailsPane->setVisible(true);
        mContactSwitcher->setVisible(true);
    } else {
        mMainWidgetSplitter2->setVisible(true);
        mContactSwitcher->setVisible(false);

        if (mode == 2) {                                          // 2 columns
            mMainWidgetSplitter2->setOrientation(Qt::Vertical);
        } else if (mode == 3) {                                // 3 columns
            mMainWidgetSplitter2->setOrientation(Qt::Horizontal);
        }
    }

    Settings::self()->setViewMode(mode);                  // save new mode in settings
    restoreSplitterStates();                                // restore state for new mode
    mViewModeGroup->actions().at(mode - 1)->setChecked(true);

    if (mItemView->model()) {
        mItemView->setCurrentIndex(mItemView->model()->index(0, 0));
    }
}

void MainWidget::saveSplitterStates() const
{
    // The splitter states are saved separately for each column view mode,
    // but only if not in simple mode (1 column).
    int currentMode = Settings::self()->viewMode();
    if (currentMode == 1) {
        return;
    }

    const QString groupName = QStringLiteral("UiState_MainWidgetSplitter_%1").arg(currentMode);
    //qDebug() << "saving to group" << groupName;
    KConfigGroup group(Settings::self()->config(), groupName);
    KPIM::UiStateSaver::saveState(mMainWidgetSplitter1, group);
    KPIM::UiStateSaver::saveState(mMainWidgetSplitter2, group);
}

void MainWidget::restoreSplitterStates()
{
    // The splitter states are restored as appropriate for the current
    // column view mode, but not for simple mode (1 column).
    int currentMode = Settings::self()->viewMode();
    if (currentMode == 1) {
        return;
    }

    const QString groupName = QStringLiteral("UiState_MainWidgetSplitter_%1").arg(currentMode);
    //qDebug() << "restoring from group" << groupName;
    KConfigGroup group(Settings::self()->config(), groupName);
    KPIM::UiStateSaver::restoreState(mMainWidgetSplitter1, group);
    KPIM::UiStateSaver::restoreState(mMainWidgetSplitter2, group);
}

void MainWidget::initGrantleeThemeName()
{
    QString themeName = GrantleeTheme::GrantleeSettings::self()->grantleeAddressBookThemeName();
    if (themeName.isEmpty()) {
        themeName = QLatin1String("default");
    }
    mFormatter->setGrantleeTheme(mGrantleeThemeManager->theme(themeName));
    mGroupFormatter->setGrantleeTheme(mGrantleeThemeManager->theme(themeName));
}

void MainWidget::slotGrantleeThemeSelected()
{
    initGrantleeThemeName();
    if (mItemView->model()) {
        mItemView->setCurrentIndex(mItemView->model()->index(0, 0));
    }
}

void MainWidget::slotGrantleeThemesUpdated()
{
    if (mItemView->model()) {
        mItemView->setCurrentIndex(mItemView->model()->index(0, 0));
    }
}

void MainWidget::mergeContacts()
{
    const Akonadi::Item::List lst = Utils::collectSelectedContactsItem(mItemView->selectionModel());
    QPointer<KABMergeContacts::MergeContactsDialog> dlg = new KABMergeContacts::MergeContactsDialog(lst, this);
    dlg->exec();
    delete dlg;
}

void MainWidget::slotSearchDuplicateContacts()
{
    const Akonadi::Item::List lst = Utils::collectSelectedContactsItem(mItemView->selectionModel());
    QPointer<KABMergeContacts::MergeContactDuplicateContactDialog> dlg = new KABMergeContacts::MergeContactDuplicateContactDialog(lst, this);
    dlg->exec();
    delete dlg;
}

Akonadi::EntityTreeModel *MainWidget::entityTreeModel() const
{
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel *>(mCollectionView->model());
    while (proxy) {
        Akonadi::EntityTreeModel *etm = qobject_cast<Akonadi::EntityTreeModel *>(proxy->sourceModel());
        if (etm) {
            return etm;
        }
        proxy = qobject_cast<QAbstractProxyModel *>(proxy->sourceModel());
    }

    qWarning() << "Couldn't find EntityTreeModel";
    return 0;
}

void MainWidget::slotCheckNewCalendar(const QModelIndex &parent, int begin, int end)
{
    // HACK: Check newly created calendars
    Akonadi::EntityTreeModel *etm = entityTreeModel();
    if (etm && etm->isCollectionTreeFetched()) {
        for (int row = begin; row <= end; ++row) {
            QModelIndex index = mCollectionView->model()->index(row, 0, parent);
            if (index.isValid()) {
                mCollectionView->model()->setData(index, Qt::Checked, Qt::CheckStateRole);
            }
        }
        if (parent.isValid()) {
            mCollectionView->setExpanded(parent, true);
        }
    }
}

void MainWidget::slotSendMail()
{
    const Akonadi::Item::List lst = Utils::collectSelectedAllContactsItem(mItemView->selectionModel());
    if (!lst.isEmpty()) {
        KABMailSender::MailSenderJob *mailSender = new KABMailSender::MailSenderJob(lst, this);
        connect(mailSender, &KABMailSender::MailSenderJob::sendMails, this, &MainWidget::slotSendMails);
        mailSender->start();
    } else {
        KMessageBox::sorry(this, i18n("You have not selected any contacts."));
    }
}

void MainWidget::slotSendMails(const QStringList &emails)
{
    if (!emails.isEmpty()) {
        QUrl url;
        url.setScheme(QLatin1String("mailto"));
        url.setPath(emails.join(QLatin1String(";")));
        QDesktopServices::openUrl(url);
    }
}

void MainWidget::slotDebugBaloo()
{
    const Akonadi::Item::List lst = Utils::collectSelectedAllContactsItem(mItemView->selectionModel());
    if (lst.isEmpty()) {
        return;
    }
    QPointer<PimCommon::BalooDebugDialog> dlg = new PimCommon::BalooDebugDialog;
    dlg->setAkonadiId(lst.at(0).id());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setSearchType(PimCommon::BalooDebugSearchPathComboBox::Contacts);
    dlg->doSearch();
    dlg->show();
}
