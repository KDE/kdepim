/*
    This file is part of KJots.

    Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
    Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
    Copyright (C) 2007-2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kjotswidget.h"

// Qt
#include <QHBoxLayout>
#include <QInputDialog>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QPrintDialog>
#include <QPainter>
#include <QPointer>
#include <QPrinter>
#include <QAbstractTextDocumentLayout>
#include <QDBusConnection>

// Akonadi
#include <akonadi/control.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/collectioncreatejob.h>
#include <akonadi/changerecorder.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/etmviewstatesaver.h>
#include <akonadi/item.h>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

#include "akonadi/entityorderproxymodel.h"
#include "akonadi_next/note.h"
#include "akonadi_next/notecreatorandselector.h"

// Grantlee
#include <grantlee/template.h>
#include <grantlee/engine.h>
#include <grantlee/context.h>

// KDE
#include <KAction>
#include <KActionCollection>
#include <KBookmarkMenu>
#include <KFileDialog>
#include <KFind>
#include <KFindDialog>
#include <KLocale>
#include <KMessageBox>
#include <KReplaceDialog>
#include <kselectionproxymodel.h>
#include <KStandardDirs>
#include <KTextBrowser>
#include <KGlobalSettings>
#include <KXMLGUIClient>
#include <KProcess>
#include <KPrintPreview>

// KMime
#include <KMime/KMimeMessage>

// KJots
#include "kjotsbookmarks.h"
#include "kjotssortproxymodel.h"
#include "kjotsmodel.h"
#include "kjotsedit.h"
#include "kjotstreeview.h"
#include "kjotsconfigdlg.h"
#include "kjotsreplacenextdialog.h"
#include "KJotsSettings.h"
#include "kjotslockjob.h"

#include <kdebug.h>

#include <memory>
#include "noteshared/attributes/notelockattribute.h"
#include "localresourcecreator.h"
#include <krandom.h>
#include "kjotsbrowser.h"

#ifndef KDE_USE_FINAL
Q_DECLARE_METATYPE(QTextDocument*)
Q_DECLARE_METATYPE(QTextCursor)
#endif
using namespace Akonadi;
using namespace Grantlee;

KJotsWidget::KJotsWidget( QWidget * parent, KXMLGUIClient *xmlGuiClient, Qt::WindowFlags f )
    : QWidget( parent, f ), m_xmlGuiClient( xmlGuiClient )
{
  Akonadi::Control::widgetNeedsAkonadi( this );

  KConfigGroup migrationCfg( KGlobal::config(), "General" );
  const bool autoCreate = migrationCfg.readEntry( "AutoCreateResourceOnStart", true );
  migrationCfg.writeEntry("AutoCreateResourceOnStart", autoCreate);
  migrationCfg.sync();
  if (autoCreate) {
    LocalResourceCreator *creator = new LocalResourceCreator( this );
    creator->createIfMissing();
  }

  m_splitter = new QSplitter( this );

  m_splitter->setStretchFactor(1, 1);
  m_splitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin( 0 );

  KStandardDirs KStd;
  m_templateEngine = new Engine(this);
  m_templateEngine->setPluginPaths( KStd.findDirs( "lib", QString() ) );

  m_loader = FileSystemTemplateLoader::Ptr( new FileSystemTemplateLoader() );
  m_loader->setTemplateDirs( KStd.findDirs( "data", QLatin1String( "kjots/themes" ) ) );
  m_loader->setTheme( QLatin1String( "default" ) );

  m_templateEngine->addTemplateLoader( m_loader );

  treeview = new KJotsTreeView( xmlGuiClient, m_splitter );

  ItemFetchScope scope;
  scope.fetchFullPayload( true ); // Need to have full item when adding it to the internal data structure
  scope.fetchAttribute< EntityDisplayAttribute >();
  scope.fetchAttribute< NoteShared::NoteLockAttribute >();

  ChangeRecorder *monitor = new ChangeRecorder( this );
  monitor->fetchCollection( true );
  monitor->setItemFetchScope( scope );
  monitor->setCollectionMonitored( Collection::root() );
  monitor->setMimeTypeMonitored( Akonotes::Note::mimeType() );

  m_kjotsModel = new KJotsModel( monitor, this );

  m_sortProxyModel = new KJotsSortProxyModel( this );
  m_sortProxyModel->setSourceModel( m_kjotsModel );

  m_orderProxy = new EntityOrderProxyModel( this );
  m_orderProxy->setSourceModel( m_sortProxyModel );

  KConfigGroup cfg( KGlobal::config(), "KJotsEntityOrder" );

  m_orderProxy->setOrderConfig( cfg );

  treeview->setModel( m_orderProxy );
  treeview->setSelectionMode( QAbstractItemView::ExtendedSelection );
  treeview->setEditTriggers( QAbstractItemView::DoubleClicked );

  connect( treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)) );

  selProxy = new KSelectionProxyModel( treeview->selectionModel(), this );
  selProxy->setSourceModel( treeview->model() );

  // TODO: Write a QAbstractItemView subclass to render kjots selection.
  connect( selProxy, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(renderSelection()) );
  connect( selProxy, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(renderSelection()) );
  connect( selProxy, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(renderSelection()) );


  stackedWidget = new QStackedWidget( m_splitter );

  KActionCollection *actionCollection = xmlGuiClient->actionCollection();

  editor = new KJotsEdit( treeview->selectionModel(), stackedWidget );
  editor->createActions( actionCollection );
  stackedWidget->addWidget( editor );

  layout->addWidget( m_splitter );

  browser = new KJotsBrowser( treeview->selectionModel(), stackedWidget );
  stackedWidget->addWidget( browser );
  stackedWidget->setCurrentWidget( browser );


  KAction *action;

  action = actionCollection->addAction( QLatin1String("go_next_book") );
  action->setText( i18n( "Next Book" ) );
  action->setIcon( KIcon( QLatin1String("go-down") ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_D ) );
  connect( action, SIGNAL(triggered()), SLOT(nextBook()) );
  connect( this, SIGNAL(canGoNextBookChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( QLatin1String("go_prev_book") );
  action->setText( i18n( "Previous Book" ) );
  action->setIcon( KIcon( QLatin1String("go-up") ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_D ) );
  connect( action, SIGNAL(triggered()), SLOT(prevBook()) );
  connect( this, SIGNAL(canGoPreviousBookChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( QLatin1String("go_next_page") );
  action->setText( i18n( "Next Page" ) );
  action->setIcon( KIcon( QLatin1String("go-next") ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_PageDown ) );
  connect( action, SIGNAL(triggered()), SLOT(nextPage()));
  connect( this, SIGNAL(canGoNextPageChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( QLatin1String("go_prev_page") );
  action->setText( i18n( "Previous Page" ) );
  action->setIcon( KIcon( QLatin1String("go-previous") ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_PageUp ) );
  connect( action, SIGNAL(triggered()), SLOT(prevPage()) );
  connect( this, SIGNAL(canGoPreviousPageChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( QLatin1String("new_page") );
  action->setText( i18n( "&New Page" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  action->setIcon( KIcon( QLatin1String("document-new") ) );
  connect( action, SIGNAL(triggered()), SLOT(newPage()) );

  action = actionCollection->addAction(QLatin1String("new_book"));
  action->setText( i18n( "New &Book..." ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_N ) );
  action->setIcon( KIcon( QLatin1String("address-book-new") ) );
  connect( action, SIGNAL(triggered()), SLOT(newBook()) );

  action = actionCollection->addAction( QLatin1String("del_page") );
  action->setText( i18n( "&Delete Page" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Delete ) );
  action->setIcon( KIcon( QLatin1String("edit-delete-page") ) );
  connect( action, SIGNAL(triggered()), SLOT(deletePage()) );

  action = actionCollection->addAction( QLatin1String("del_folder") );
  action->setText( i18n( "Delete Boo&k" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Delete ) );
  action->setIcon( KIcon( QLatin1String("edit-delete") ) );
  connect( action, SIGNAL(triggered()), SLOT(deleteBook()) );

  action = actionCollection->addAction( QLatin1String("del_mult") );
  action->setText( i18n( "Delete Selected" ) );
  action->setIcon( KIcon( QLatin1String("edit-delete") ) );
  connect( action, SIGNAL(triggered()), SLOT(deleteMultiple()) );

  action = actionCollection->addAction( QLatin1String("manual_save") );
  action->setText( i18n( "Manual Save" ) );
  action->setIcon( KIcon( QLatin1String("document-save") ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );

  action = actionCollection->addAction( QLatin1String("auto_bullet") );
  action->setText( i18n( "Auto Bullets" ) );
  action->setIcon( KIcon( QLatin1String("format-list-unordered") ) );
  action->setCheckable( true );

  action = actionCollection->addAction( QLatin1String("auto_decimal") );
  action->setText( i18n( "Auto Decimal List" ) );
  action->setIcon( KIcon( QLatin1String("format-list-ordered") ) );
  action->setCheckable( true );

  action = actionCollection->addAction( QLatin1String("manage_link") );
  action->setText( i18n( "Link" ) );
  action->setIcon( KIcon( QLatin1String("insert-link") ) );

  action = actionCollection->addAction( QLatin1String("insert_checkmark") );
  action->setText( i18n( "Insert Checkmark" ) );
  action->setIcon( KIcon( QLatin1String("checkmark") ) );
  action->setEnabled( false );

  action = actionCollection->addAction( QLatin1String("rename_entry") );
  action->setText( i18n( "Rename..." ) );
  action->setIcon( KIcon( QLatin1String("edit-rename") ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ) );

  action = actionCollection->addAction( QLatin1String("insert_date") );
  action->setText( i18n( "Insert Date" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_I ) );
  action->setIcon( KIcon( QLatin1String("view-calendar-time-spent") ) );

  action = actionCollection->addAction( QLatin1String("change_color") );
  action->setIcon( KIcon( QLatin1String("format-fill-color") ) );
  action->setText( i18n( "Change Color..." ) );

  action = actionCollection->addAction( QLatin1String("copy_link_address") );
  action->setText( i18n( "Copy Link Address" ) );

  action = actionCollection->addAction( QLatin1String("lock") );
  action->setText(i18n( "Lock Selected" ) );
  action->setIcon( KIcon( QLatin1String("emblem-locked") ) );
  connect( action, SIGNAL(triggered()), SLOT(actionLock()) );

  action = actionCollection->addAction( QLatin1String("unlock") );
  action->setText( i18n( "Unlock Selected" ) );
  action->setIcon( KIcon( QLatin1String("emblem-unlocked") ) );
  connect( action, SIGNAL(triggered()), SLOT(actionUnlock()) );

  action = actionCollection->addAction( QLatin1String("sort_children_alpha") );
  action->setText( i18n( "Sort children alphabetically" ) );
  connect( action, SIGNAL(triggered()), SLOT(actionSortChildrenAlpha()) );

  action = actionCollection->addAction( QLatin1String("sort_children_by_date") );
  action->setText( i18n( "Sort children by creation date" ) );
  connect( action, SIGNAL(triggered()), SLOT(actionSortChildrenByDate()) );

  action = KStandardAction::cut( editor, SLOT(cut()), actionCollection );
  connect( editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  action->setEnabled( false );

  action = KStandardAction::copy( this, SLOT(copy()), actionCollection );
  connect( editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  connect( browser, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  action->setEnabled( false );

  KStandardAction::pasteText( editor, SLOT(paste()), actionCollection );

  KStandardAction::undo( editor, SLOT(undo()), actionCollection );
  KStandardAction::redo( editor, SLOT(redo()), actionCollection );
  KStandardAction::selectAll( editor, SLOT(selectAll()), actionCollection );

  action = actionCollection->addAction( QLatin1String("copyIntoTitle") );
  action->setText( i18n( "Copy &into Page Title" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ) );
  action->setIcon( KIcon( QLatin1String("edit-copy") ) );
  connect( action, SIGNAL(triggered()), SLOT(copySelectionToTitle()) );
  connect( editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  action->setEnabled( false );

  action = actionCollection->addAction( QLatin1String("paste_plain_text") );
  action->setText( i18nc( "@action Paste the text in the clipboard without rich text formatting.", "Paste Plain Text" ) );
  connect( action, SIGNAL(triggered()), editor, SLOT(pastePlainText()) );

  KStandardAction::preferences( this, SLOT(configure()), actionCollection );

  bookmarkMenu = actionCollection->add<KActionMenu>( QLatin1String("bookmarks") );
  bookmarkMenu->setText( i18n( "&Bookmarks" ) );
  KJotsBookmarks* bookmarks = new KJotsBookmarks( treeview );
  /*KBookmarkMenu *bmm =*/ new KBookmarkMenu(
      KBookmarkManager::managerForFile( KStandardDirs::locateLocal( "data",QLatin1String("kjots/bookmarks.xml") ), QLatin1String("kjots") ),
      bookmarks, bookmarkMenu->menu(), actionCollection );

  // "Add bookmark" and "make text bold" actions have conflicting shortcuts (ctrl + b)
  // Make add_bookmark use ctrl+shift+b to resolve that.
  KAction *bm_action = qobject_cast<KAction *>(actionCollection->action(QLatin1String("add_bookmark")));
  Q_ASSERT(bm_action);
  bm_action->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_B );

  KStandardAction::find( this, SLOT(onShowSearch()), actionCollection );
  action = KStandardAction::findNext( this, SLOT(onRepeatSearch()), actionCollection );
  action->setEnabled(false);
  KStandardAction::replace( this, SLOT(onShowReplace()), actionCollection );

  action = actionCollection->addAction( QLatin1String("save_to") );
  action->setText( i18n( "Rename..." ) );
  action->setIcon( KIcon( QLatin1String("edit-rename") ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ) );

  KActionMenu *exportMenu = actionCollection->add<KActionMenu>( QLatin1String("save_to") );
  exportMenu->setText( i18n( "Export" ) );
  exportMenu->setIcon( KIcon( QLatin1String("document-export") ) );

  action = actionCollection->addAction( QLatin1String("save_to_ascii") );
  action->setText( i18n( "To Text File..." ) );
  action->setIcon( KIcon( QLatin1String("text-plain") ) );
  connect( action, SIGNAL(triggered()), SLOT(exportSelectionToPlainText()) );
  exportMenu->menu()->addAction( action );

  action = actionCollection->addAction( QLatin1String("save_to_html") );
  action->setText( i18n( "To HTML File..." ) );
  action->setIcon( KIcon( QLatin1String("text-html") ) );
  connect( action, SIGNAL(triggered()), SLOT(exportSelectionToHtml()) );
  exportMenu->menu()->addAction( action );

  action = actionCollection->addAction( QLatin1String("save_to_book") );
  action->setText( i18n( "To Book File..." ) );
  action->setIcon( KIcon( QLatin1String("x-office-address-book") ) );
  connect( action, SIGNAL(triggered()), SLOT(exportSelectionToXml()) );
  exportMenu->menu()->addAction( action );

  KStandardAction::print(this, SLOT(printSelection()), actionCollection);
  KStandardAction::printPreview(this, SLOT(printPreviewSelection()), actionCollection);

  if ( !KJotsSettings::splitterSizes().isEmpty() )
  {
    m_splitter->setSizes( KJotsSettings::splitterSizes() );
  }

  QTimer::singleShot( 0, this, SLOT(delayedInitialization()) );

  connect( treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(updateMenu()) );
  connect( treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(updateCaption()) );

  connect( m_kjotsModel, SIGNAL(modelAboutToBeReset()), SLOT(saveState()));
  connect( m_kjotsModel, SIGNAL(modelReset()), SLOT(restoreState()));

  restoreState();

  QDBusConnection::sessionBus().registerObject( QLatin1String("/KJotsWidget"), this, QDBusConnection::ExportScriptableContents );
}

KJotsWidget::~KJotsWidget()
{
  saveState();
}

void KJotsWidget::restoreState()
{
  ETMViewStateSaver *saver = new ETMViewStateSaver;
  saver->setView( treeview );
  KConfigGroup cfg( KGlobal::config(), "TreeState" );
  saver->restoreState( cfg );
}

void KJotsWidget::saveState()
{
  ETMViewStateSaver saver;
  saver.setView( treeview );
  KConfigGroup cfg( KGlobal::config(), "TreeState" );
  saver.saveState( cfg );
  cfg.sync();
}

void KJotsWidget::delayedInitialization()
{
  migrateNoteData( QLatin1String("kjotsmigrator") );
  // Disable nigration of data from KNotes as that app still exists in 4.5.
//   migrateNoteData( "kres-migrator", "notes" );

  //TODO: Save previous searches in settings file?
  searchDialog = new KFindDialog ( this, 0, QStringList(), false );
  QGridLayout *layout = new QGridLayout(searchDialog->findExtension());
  layout->setMargin(0);
  searchAllPages = new QCheckBox(i18n("Search all pages"), searchDialog->findExtension());
  layout->addWidget(searchAllPages, 0, 0);

  connect(searchDialog, SIGNAL(okClicked()), this, SLOT(onStartSearch()) );
  connect(searchDialog, SIGNAL(cancelClicked()), this, SLOT(onEndSearch()) );
  connect(treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onUpdateSearch()) );
  connect(searchDialog, SIGNAL(optionsChanged()), SLOT(onUpdateSearch()) );
  connect(searchAllPages, SIGNAL(stateChanged(int)), SLOT(onUpdateSearch()) );

  replaceDialog = new KReplaceDialog ( this, 0, searchHistory, replaceHistory, false );
  QGridLayout *layout2 = new QGridLayout(replaceDialog->findExtension());
  layout2->setMargin(0);
  replaceAllPages = new QCheckBox(i18n("Search all pages"), replaceDialog->findExtension());
  layout2->addWidget(replaceAllPages, 0, 0);

  connect(replaceDialog, SIGNAL(okClicked()), this, SLOT(onStartReplace()) );
  connect(replaceDialog, SIGNAL(cancelClicked()), this, SLOT(onEndReplace()) );
  connect(replaceDialog, SIGNAL(optionsChanged()), SLOT(onUpdateReplace()) );
  connect(replaceAllPages, SIGNAL(stateChanged(int)), SLOT(onUpdateReplace()) );

  // Actions are enabled or disabled based on whether the selection is a single page, a single book
  // multiple selections, or no selection.
  //
  // The entryActions are enabled for all single pages and single books, and the multiselectionActions
  // are enabled when the user has made multiple selections.
  //
  // Some actions are in neither (eg, new book) and are available even when there is no selection.
  //
  // Some actions are in both, so that they are available for valid selections, but not available
  // for invalid selections (eg, print/find are disabled when there is no selection)

  KActionCollection *actionCollection = m_xmlGuiClient->actionCollection();

  // Actions for a single item selection.
  entryActions.insert( actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Find))) );
  entryActions.insert( actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Print))) );
  entryActions.insert( actionCollection->action(QLatin1String("rename_entry")) );
  entryActions.insert( actionCollection->action(QLatin1String("change_color")) );
  entryActions.insert( actionCollection->action(QLatin1String("save_to")) );
  entryActions.insert( actionCollection->action(QLatin1String("copy_link_address")) );

  // Actions that are used only when a page is selected.
  pageActions.insert( actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Cut))) );
  pageActions.insert( actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Paste))) );
  pageActions.insert( actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Replace))) );
  pageActions.insert( actionCollection->action(QLatin1String("del_page")) );
  pageActions.insert( actionCollection->action(QLatin1String("insert_date")) );
  pageActions.insert( actionCollection->action(QLatin1String("auto_bullet")) );
  pageActions.insert( actionCollection->action(QLatin1String("auto_decimal")) );
  pageActions.insert( actionCollection->action(QLatin1String("manage_link")) );
  pageActions.insert( actionCollection->action(QLatin1String("insert_checkmark")) );

  // Actions that are used only when a book is selected.
  bookActions.insert( actionCollection->action(QLatin1String("save_to_book")) );
  bookActions.insert( actionCollection->action(QLatin1String("del_folder")) );
  bookActions.insert( actionCollection->action(QLatin1String("sort_children_alpha")) );
  bookActions.insert( actionCollection->action(QLatin1String("sort_children_by_date")) );

  // Actions that are used when multiple items are selected.
  multiselectionActions.insert( actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Find))) );
  multiselectionActions.insert( actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Print))));
  multiselectionActions.insert( actionCollection->action(QLatin1String("del_mult")) );
  multiselectionActions.insert( actionCollection->action(QLatin1String("save_to")) );
  multiselectionActions.insert( actionCollection->action(QLatin1String("change_color")) );

  m_autosaveTimer = new QTimer(this);
  updateConfiguration();

  connect(m_autosaveTimer, SIGNAL(timeout()), editor, SLOT(savePage()));
  connect(treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), m_autosaveTimer, SLOT(start()) );

  treeview->delayedInitialization();
  editor->delayedInitialization( m_xmlGuiClient->actionCollection() );
  browser->delayedInitialization();


  connect( treeview->itemDelegate(), SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
      SLOT(bookshelfEditItemFinished(QWidget*,QAbstractItemDelegate::EndEditHint)) );

  connect( editor, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
      SLOT(currentCharFormatChanged(QTextCharFormat)) );
  updateMenu();
}

void KJotsWidget::bookshelfEditItemFinished( QWidget *, QAbstractItemDelegate::EndEditHint )
{
    // Make sure the editor gets focus again after naming a new book/page.
    activeEditor()->setFocus();
}

void KJotsWidget::currentCharFormatChanged(const QTextCharFormat & fmt)
{
    QString selectedAnchor = fmt.anchorHref();
    if (selectedAnchor != activeAnchor)
    {
        activeAnchor = selectedAnchor;
        if (!selectedAnchor.isEmpty())
        {
            QTextCursor c(editor->textCursor());
            editor->selectLinkText(&c);
            QString selectedText = c.selectedText();
            if (!selectedText.isEmpty())
            {
              emit activeAnchorChanged(selectedAnchor, selectedText);
            }
        } else {
            emit activeAnchorChanged(QString(), QString());
        }
    }
}

void KJotsWidget::migrateNoteData( const QString &migrator, const QString &type )
{
  // Akonadi migration
  KConfig config( migrator + QLatin1String("rc") );
  KConfigGroup migrationCfg( &config, "Migration" );
  const bool enabled = migrationCfg.readEntry( "Enabled", true );
  const bool completed = migrationCfg.readEntry( "Completed", false );
  const int currentVersion = migrationCfg.readEntry( "Version", 0 );
  const int targetVersion = migrationCfg.readEntry( "TargetVersion", 1 );
  if ( enabled && !completed && currentVersion < targetVersion ) {
    kDebug() << "Performing Akonadi migration. Good luck!";
    KProcess proc;
    QStringList args = QStringList() << QLatin1String("--interactive-on-change");
    if ( !type.isEmpty() )
      args << QLatin1String("--type") << type;

    const QString path = KStandardDirs::findExe( migrator );
    proc.setProgram( path, args );
    proc.start();
    bool result = proc.waitForStarted();
    if ( result ) {
      result = proc.waitForFinished();
    }
    if ( result && proc.exitCode() == 0 ) {
      kDebug() << "Akonadi migration has been successful";
    } else {
      // exit code 1 means it is already running, so we are probably called by a migrator instance
      kError() << "Akonadi migration failed!";
      kError() << "command was: " << proc.program();
      kError() << "exit code: " << proc.exitCode();
      kError() << "stdout: " << proc.readAllStandardOutput();
      kError() << "stderr: " << proc.readAllStandardError();
    }
    migrationCfg.writeEntry( "Version", targetVersion );
    migrationCfg.writeEntry( "Completed", true );
    migrationCfg.sync();
  }
}

inline QTextEdit* KJotsWidget::activeEditor() {
  if ( browser->isVisible() ) {
    return browser;
  } else {
    return editor;
  }
}

void KJotsWidget::updateMenu()
{
  QModelIndexList selection = treeview->selectionModel()->selectedRows();
  int selectionSize = selection.size();

  if ( !selectionSize ) {
    // no (meaningful?) selection
    foreach ( QAction* action, multiselectionActions )
      action->setEnabled(false);
    foreach ( QAction* action, entryActions )
      action->setEnabled(false);
    foreach ( QAction* action, bookActions )
      action->setEnabled(false);
    foreach ( QAction* action, pageActions )
      action->setEnabled(false);
    editor->setActionsEnabled( false );
  } else if ( selectionSize > 1 ) {
    foreach ( QAction* action, entryActions )
      action->setEnabled(false);
    foreach ( QAction* action, bookActions )
      action->setEnabled(false);
    foreach ( QAction* action, pageActions )
      action->setEnabled(false);
    foreach ( QAction* action, multiselectionActions )
      action->setEnabled(true);

    editor->setActionsEnabled( false );
  } else {

    foreach ( QAction* action, multiselectionActions )
      action->setEnabled(false);
    foreach ( QAction* action, entryActions )
      action->setEnabled(true);

    QModelIndex idx = selection.at( 0 );

    Collection col = idx.data( KJotsModel::CollectionRole ).value<Collection>();
    if ( col.isValid() ) {
      foreach ( QAction* action, pageActions )
        action->setEnabled(false);
      const bool colIsRootCollection = ( col.parentCollection() == Collection::root() );
      foreach ( QAction* action, bookActions ) {
        if (action->objectName() == QLatin1String("del_folder") && colIsRootCollection ) {
          action->setEnabled( false );
        } else {
          action->setEnabled( true );
        }

      }

      editor->setActionsEnabled( false );
    } else {
      foreach ( QAction* action, pageActions ) {
        if (action->objectName() == QLatin1String(name( KStandardAction::Cut )) ) {
          action->setEnabled( activeEditor()->textCursor().hasSelection() );
        } else {
          action->setEnabled( true );
        }
      }
      foreach ( QAction* action, bookActions )
        action->setEnabled( false );
      editor->setActionsEnabled( true );
    }
  }
}

void KJotsWidget::copy() {
  activeEditor()->copy();
}

void KJotsWidget::configure()
{
  // create a new preferences dialog...
  KJotsConfigDlg *dialog = new KJotsConfigDlg( i18n( "Settings" ), this );
  connect( dialog, SIGNAL(configCommitted()), SLOT(updateConfiguration()) );
  dialog->show();
}

void KJotsWidget::updateConfiguration()
{
    if (KJotsSettings::autoSave())
    {
        m_autosaveTimer->setInterval(KJotsSettings::autoSaveInterval()*1000*60);
        m_autosaveTimer->start();
    } else
        m_autosaveTimer->stop();
}

void KJotsWidget::copySelectionToTitle()
{
  QString newTitle( editor->textCursor().selectedText() );

  if ( !newTitle.isEmpty() ) {

    QModelIndexList rows = treeview->selectionModel()->selectedRows();

    if ( rows.size() != 1 )
      return;

    QModelIndex idx = rows.at( 0 );

    treeview->model()->setData( idx, newTitle );
  }
}

void KJotsWidget::deleteMultiple()
{
  QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

  if ( KMessageBox::questionYesNo( this,
        i18n( "Do you really want to delete all selected books and pages?" ),
        i18n("Delete?"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
        QString(), KMessageBox::Dangerous ) != KMessageBox::Yes )
    return;

  foreach ( const QModelIndex &index, selectedRows ) {
    bool ok;
    qlonglong id = index.data( EntityTreeModel::ItemIdRole ).toLongLong(&ok);
    Q_ASSERT(ok);
    if ( id >= 0 )
    {
      new ItemDeleteJob( Item( id ), this );
    }
    else {
      id = index.data( EntityTreeModel::CollectionIdRole ).toLongLong(&ok);
      Q_ASSERT(ok);
      if ( id >= 0 )
        new CollectionDeleteJob( Collection( id ), this );
    }
  }
}

void KJotsWidget::deletePage()
{
  QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

  if ( selectedRows.size() != 1 )
    return;

  const QModelIndex idx = selectedRows.at( 0 );
  Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return;

  if( item.hasAttribute<NoteShared::NoteLockAttribute>() ) {

        KMessageBox::information(topLevelWidget(),
            i18n("This page is locked. You can only delete it when you first unlock it."),
            i18n("Item is locked"));
        return;
  }

  if ( KMessageBox::warningContinueCancel(topLevelWidget(),
          i18nc("remove the page, by title", "<qt>Are you sure you want to delete the page <strong>%1</strong>?</qt>", idx.data().toString()),
          i18n("Delete"), KStandardGuiItem::del(), KStandardGuiItem::cancel(), QLatin1String("DeletePageWarning")) == KMessageBox::Cancel) {
      return;
  }

  (void) new Akonadi::ItemDeleteJob( item, this );
}

void KJotsWidget::deleteBook()
{
  QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

  if ( selectedRows.size() != 1 )
    return;

  const QModelIndex idx = selectedRows.at( 0 );
  Collection col = idx.data( EntityTreeModel::CollectionRole ).value<Collection>();

  if ( !col.isValid() )
    return;

  if ( col.parentCollection() == Collection::root() )
    return;

  if( col.hasAttribute<NoteShared::NoteLockAttribute>() ) {

      KMessageBox::information(topLevelWidget(),
          i18n("This book is locked. You can only delete it when you first unlock it."),
          i18n("Item is locked"));
      return;
  }
  if ( KMessageBox::warningContinueCancel(topLevelWidget(),
      i18nc("remove the book, by title", "<qt>Are you sure you want to delete the book <strong>%1</strong>?</qt>", idx.data().toString()),
      i18n("Delete"), KStandardGuiItem::del(), KStandardGuiItem::cancel(), QLatin1String("DeleteBookWarning")) == KMessageBox::Cancel) {
        return;
  }

  (void) new Akonadi::CollectionDeleteJob( col, this );
}

void KJotsWidget::newBook()
{
  QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

  if ( selectedRows.size() != 1 )
    return;

  Collection col = selectedRows.at( 0 ).data( EntityTreeModel::CollectionRole ).value<Collection>();

  if ( !col.isValid() )
    return;

  Collection newCollection;
  newCollection.setParentCollection( col );

  QString title = i18nc( "The default name for new books.", "New Book" );
  newCollection.setName( KRandom::randomString( 10 ) );
  newCollection.setContentMimeTypes( QStringList() << Akonadi::Collection::mimeType() << Akonotes::Note::mimeType() );

  Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
  eda->setIconName( QLatin1String("x-office-address-book") );
  eda->setDisplayName( title );
  newCollection.addAttribute( eda );

  Akonadi::CollectionCreateJob *job = new Akonadi::CollectionCreateJob( newCollection );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(newBookResult(KJob*)) );
}

void KJotsWidget::newPage()
{
  QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

  if ( selectedRows.size() != 1 )
    return;

  Item item = selectedRows.at( 0 ).data( EntityTreeModel::ItemRole ).value<Item>();

  Collection col;
  if ( item.isValid() )
  {
    col = selectedRows.at( 0 ).data( EntityTreeModel::ParentCollectionRole ).value<Collection>();
  } else {
    col = selectedRows.at( 0 ).data( EntityTreeModel::CollectionRole ).value<Collection>();
  }

  if ( !col.isValid() )
    return;
  doCreateNewPage(col);
}

void KJotsWidget::doCreateNewPage(const Collection &collection)
{
  Akonotes::NoteCreatorAndSelector *creatorAndSelector = new Akonotes::NoteCreatorAndSelector(treeview->selectionModel());
  creatorAndSelector->createNote(collection);
}

void KJotsWidget::newPageResult( KJob* job )
{
  if ( job->error() )
    kDebug() << job->errorString();
}

void KJotsWidget::newBookResult( KJob* job )
{
  if ( job->error() ) {
    kDebug() << job->errorString();
    return;
  }
  Akonadi::CollectionCreateJob *createJob = qobject_cast<Akonadi::CollectionCreateJob*>(job);
  if ( !createJob )
    return;
  const Collection collection = createJob->collection();
  if ( !collection.isValid() )
    return;

  doCreateNewPage(collection);
}

QString KJotsWidget::renderSelectionToHtml()
{
  QHash<QString, QVariant> hash;

  QList<QVariant> objectList;

  const int rows = selProxy->rowCount();
  const int column = 0;
  for ( int row = 0; row < rows; ++row )
  {
    QModelIndex idx = selProxy->index( row, column, QModelIndex() );

    QObject *obj = idx.data(KJotsModel::GrantleeObjectRole).value<QObject*>();
    KJotsEntity *kjotsEntity = qobject_cast<KJotsEntity*>(obj);
    kjotsEntity->setIndex(idx);
    objectList << QVariant::fromValue(static_cast<QObject *>(kjotsEntity));
  }

  hash.insert( QLatin1String( "entities" ), objectList);
  hash.insert( QLatin1String( "i18n_TABLE_OF_CONTENTS" ),
              i18nc("Header for 'Table of contents' section of rendered output", "Table of contents") );
  Context c(hash);

  Template t = m_templateEngine->loadByName( QLatin1String( "template.html" ) );

  QString result = t->render(&c);

  // TODO: handle errors.
  return result;
}

QString KJotsWidget::renderSelectionToPlainText()
{
  QHash<QString, QVariant> hash;

  QList<QVariant> objectList;

  const int rows = selProxy->rowCount();
  const int column = 0;
  for ( int row = 0; row < rows; ++row )
  {
    QModelIndex idx = selProxy->index( row, column, QModelIndex() );

    QObject *obj = idx.data(KJotsModel::GrantleeObjectRole).value<QObject*>();
    KJotsEntity *kjotsEntity = qobject_cast<KJotsEntity*>(obj);
    kjotsEntity->setIndex(idx);
    objectList << QVariant::fromValue(static_cast<QObject *>(kjotsEntity));
  }

  hash.insert( QLatin1String( "entities" ), objectList);
  hash.insert( QLatin1String( "i18n_TABLE_OF_CONTENTS" ),
              i18nc("Header for 'Table of contents' section of rendered output", "Table of contents") );
  Context c(hash);

  Template t = m_templateEngine->loadByName( QLatin1String( "template.txt" ) );

  QString result = t->render(&c);

  // TODO: handle errors.
  return result;
}


QString KJotsWidget::renderSelectionToXml()
{
  QHash<QString, QVariant> hash;

  QList<QVariant> objectList;

  const int rows = selProxy->rowCount();
  const int column = 0;
  for ( int row = 0; row < rows; ++row )
  {
    QModelIndex idx = selProxy->index( row, column, QModelIndex() );

    QObject *obj = idx.data(KJotsModel::GrantleeObjectRole).value<QObject*>();
    KJotsEntity *kjotsEntity = qobject_cast<KJotsEntity*>(obj);
    kjotsEntity->setIndex(idx);
    objectList << QVariant::fromValue(static_cast<QObject *>(kjotsEntity));
  }

  hash.insert( QLatin1String( "entities" ), objectList);
  Context c(hash);

  QString currentTheme = m_loader->themeName();
  m_loader->setTheme( QLatin1String("xml_output") );
  Template t = m_templateEngine->loadByName( QLatin1String( "template.xml" ) );

  QString result = t->render(&c);

  m_loader->setTheme(currentTheme);
  return result;
}

void KJotsWidget::renderSelection()
{
  const int rows = selProxy->rowCount();

  // If the selection is a single page, present it for editing...
  if (rows == 1)
  {
    QModelIndex idx = selProxy->index( 0, 0, QModelIndex());

    QTextDocument *document = idx.data( KJotsModel::DocumentRole ).value<QTextDocument*>();

    if ( document )
    {
      editor->setDocument( document );
      QTextCursor textCursor = document->property( "textCursor" ).value<QTextCursor>();
      if ( !textCursor.isNull() )
        editor->setTextCursor( textCursor );
      stackedWidget->setCurrentWidget( editor );
      editor->setFocus();
      return;
    } // else fallthrough
  }

  // ... Otherwise, render the selection read-only.

  QTextDocument doc;
  QTextCursor cursor(&doc);

  browser->setHtml( renderSelectionToHtml() );
  stackedWidget->setCurrentWidget( browser );
}

QString KJotsWidget::getThemeFromUser()
{
  return QString();
#if 0
  bool ok;
  QString text = QInputDialog::getText(this, i18n("Change Theme"),
                                      tr("Theme name:"), QLineEdit::Normal,
                                      m_loader->themeName(), &ok);
  if (!ok || text.isEmpty())
  {
    return QLatin1String("default");
  }

  return text;
#endif
}


void KJotsWidget::changeTheme()
{
#if 0
  m_loader->setTheme(getThemeFromUser());
  renderSelection();
#endif
}

void KJotsWidget::exportSelectionToHtml()
{
  QString currentTheme = m_loader->themeName();
  QString themeName = getThemeFromUser();
  if (themeName.isEmpty())
  {
    themeName = QLatin1String( "default" );
  }
  m_loader->setTheme(themeName);

  QString filename = KFileDialog::getSaveFileName();
  if (!filename.isEmpty())
  {
    QFile exportFile ( filename );
    if ( !exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
        m_loader->setTheme(currentTheme);
        KMessageBox::error(0, i18n("<qt>Error opening internal file.</qt>"));
        return;
    }
    exportFile.write(renderSelectionToHtml().toUtf8());

    exportFile.close();
  }
  m_loader->setTheme(currentTheme);
}

void KJotsWidget::exportSelectionToPlainText()
{
  QString currentTheme = m_loader->themeName();

  m_loader->setTheme( QLatin1String("plain_text") );

  QString filename = KFileDialog::getSaveFileName();
  if (!filename.isEmpty())
  {
    QFile exportFile ( filename );
    if ( !exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
        m_loader->setTheme(currentTheme);
        KMessageBox::error(0, i18n("<qt>Error opening internal file.</qt>"));
        return;
    }
    exportFile.write(renderSelectionToPlainText().toUtf8());

    exportFile.close();
  }
  m_loader->setTheme(currentTheme);
}

void KJotsWidget::exportSelectionToXml()
{
  QString currentTheme = m_loader->themeName();

  m_loader->setTheme( QLatin1String("xml_output") );

  QString filename = KFileDialog::getSaveFileName();
  if (!filename.isEmpty())
  {
    QFile exportFile ( filename );
    if ( !exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
        m_loader->setTheme(currentTheme);
        KMessageBox::error(0, i18n("<qt>Error opening internal file.</qt>"));
        return;
    }
    exportFile.write(renderSelectionToXml().toUtf8());

    exportFile.close();
  }
  m_loader->setTheme(currentTheme);
}

void KJotsWidget::printPreviewSelection()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(QLatin1String("KJots_Print"));
    printer.setFullPage(false);
    printer.setCreator(QLatin1String("KJots"));
    KPrintPreview previewdlg( &printer, 0 );
    print(printer);
    previewdlg.exec();
}

void KJotsWidget::printSelection()
{

  QPrinter printer(QPrinter::HighResolution);
  printer.setDocName(QLatin1String("KJots_Print"));
  printer.setFullPage(false);
  printer.setCreator(QLatin1String("KJots"));
  //Not supported in Qt?
  //printer->setPageSelection(QPrinter::ApplicationSide);

  //KPrinter::pageList() only works with ApplicationSide. ApplicationSide
  //requires min/max pages. How am I supposed to tell how many pages there
  //are before I setup the printer?


  QPointer<QPrintDialog> printDialog = new QPrintDialog(&printer, this);

  QAbstractPrintDialog::PrintDialogOptions options = printDialog->enabledOptions();
  options &= ~QAbstractPrintDialog::PrintPageRange;
  if (activeEditor()->textCursor().hasSelection())
    options |= QAbstractPrintDialog::PrintSelection;
  printDialog->setEnabledOptions(options);

  printDialog->setWindowTitle(i18n("Send To Printer"));
  if (printDialog->exec() == QDialog::Accepted) {
      print(printer);
  }
  delete printDialog;
}

void KJotsWidget::print(QPrinter &printer)
{
    QTextDocument printDocument;
    if ( printer.printRange() == QPrinter::Selection )
    {
      printDocument.setHtml( activeEditor()->textCursor().selection().toHtml() );
    } else {
      //QTextCursor printCursor ( &printDocument );
      QString currentTheme = m_loader->themeName();
      m_loader->setTheme( QLatin1String("default") );
      printDocument.setHtml( renderSelectionToHtml() );
      m_loader->setTheme( currentTheme );
    }

    QPainter p(&printer);

    // Check that there is a valid device to print to.
    if (p.isActive()) {
      QTextDocument *doc = &printDocument;

      QRectF body = QRectF(QPointF(0, 0), doc->pageSize());
      QPointF pageNumberPos;

      QAbstractTextDocumentLayout *layout = doc->documentLayout();
      layout->setPaintDevice(p.device());

      const int dpiy = p.device()->logicalDpiY();

      const int margin = (int) ((2/2.54)*dpiy); // 2 cm margins
      QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
      fmt.setMargin(margin);
      doc->rootFrame()->setFrameFormat(fmt);

      body = QRectF(0, 0, p.device()->width(), p.device()->height());
      pageNumberPos = QPointF(body.width() - margin,
                      body.height() - margin
                      + QFontMetrics(doc->defaultFont(), p.device()).ascent()
                      + 5 * p.device()->logicalDpiY() / 72);

      doc->setPageSize(body.size());

      int docCopies = printer.numCopies();
      for (int copy = 0; copy < docCopies; ++copy) {

        int lastPage = layout->pageCount();
        for ( int page = 1; page <= lastPage ; ++page ) {
          p.save();
          p.translate(body.left(), body.top() - (page - 1) * body.height());
          QRectF view(0, (page - 1) * body.height(), body.width(), body.height());

          QAbstractTextDocumentLayout *layout = doc->documentLayout();
          QAbstractTextDocumentLayout::PaintContext ctx;

          p.setClipRect(view);
          ctx.clip = view;

          // don't use the system palette text as default text color, on HP/UX
          // for example that's white, and white text on white paper doesn't
          // look that nice
          ctx.palette.setColor(QPalette::Text, Qt::black);

          layout->draw(&p, ctx);

          if (!pageNumberPos.isNull()) {
            p.setClipping(false);
            p.setFont(QFont(doc->defaultFont()));
            const QString pageString = QString::number(page);

            p.drawText(qRound(pageNumberPos.x() - p.fontMetrics().width(pageString)),
              qRound(pageNumberPos.y() + view.top()),
              pageString);
          }

          p.restore();

          if ( (page+1) <= lastPage ) {
            printer.newPage();
          }
        }
      }
    }

}

void KJotsWidget::selectNext( int role, int step )
{
  QModelIndexList list = treeview->selectionModel()->selectedRows();
  Q_ASSERT( list.size() == 1 );

  QModelIndex idx = list.at( 0 );

  const int column = idx.column();

  QModelIndex sibling = idx.sibling( idx.row() + step, column );
  while ( sibling.isValid() )
  {
    if ( sibling.data( role ).toInt() >= 0 )
    {
      treeview->selectionModel()->select( sibling, QItemSelectionModel::SelectCurrent );
      return;
    }
    sibling = sibling.sibling( sibling.row() + step, column );
  }
  kWarning() << "No valid selection";
}

void KJotsWidget::nextBook()
{
  return selectNext( EntityTreeModel::CollectionIdRole, 1 );
}

void KJotsWidget::nextPage()
{
  return selectNext( EntityTreeModel::ItemIdRole, 1 );
}

void KJotsWidget::prevBook()
{
  return selectNext( EntityTreeModel::CollectionIdRole, -1 );
}

void KJotsWidget::prevPage()
{
  return selectNext( EntityTreeModel::ItemIdRole, -1 );
}

bool KJotsWidget::canGo( int role, int step ) const
{
  QModelIndexList list = treeview->selectionModel()->selectedRows();
  if ( list.size() != 1 )
  {
    return false;
  }

  QModelIndex currentIdx = list.at( 0 );

  const int column = currentIdx.column();

  Q_ASSERT( currentIdx.isValid() );

  QModelIndex sibling = currentIdx.sibling( currentIdx.row() + step, column );

  while ( sibling.isValid() && sibling != currentIdx )
  {
    if ( sibling.data( role ).toInt() >= 0 )
      return true;

    sibling = sibling.sibling( sibling.row() + step, column );
  }

  return false;
}

bool KJotsWidget::canGoNextPage() const
{
  return canGo( EntityTreeModel::ItemIdRole, 1 );
}

bool KJotsWidget::canGoPreviousPage() const
{
  return canGo( EntityTreeModel::ItemIdRole, -1 );
}

bool KJotsWidget::canGoNextBook() const
{
  return canGo( EntityTreeModel::CollectionIdRole, 1 );
}

bool KJotsWidget::canGoPreviousBook() const
{
  return canGo( EntityTreeModel::CollectionIdRole, -1 );
}

void KJotsWidget::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected );

  emit canGoNextBookChanged( canGoPreviousBook() );
  emit canGoNextPageChanged( canGoNextPage() );
  emit canGoPreviousBookChanged( canGoPreviousBook() );
  emit canGoPreviousPageChanged( canGoPreviousPage() );

  if ( deselected.size() == 1 )
  {
    editor->document()->setProperty( "textCursor", QVariant::fromValue( editor->textCursor() ) );
    if ( editor->document()->isModified() )
    {
      treeview->model()->setData( deselected.indexes().first(), QVariant::fromValue( editor->document() ), KJotsModel::DocumentRole );
    }
  }
}

/*!
  Shows the search dialog when "Find" is selected.
*/
void KJotsWidget::onShowSearch()
{
  onUpdateSearch();

  QTextEdit *browserOrEditor = activeEditor();

  if ( browserOrEditor->textCursor().hasSelection() ) {
    searchDialog->setHasSelection(true);
    long dialogOptions = searchDialog->options();
    dialogOptions |= KFind::SelectedText;
    searchDialog->setOptions(dialogOptions);
  } else {
    searchDialog->setHasSelection(false);
  }

  searchDialog->setFindHistory(searchHistory);
  searchDialog->show();
  onUpdateSearch();
}


/*!
    Updates the search dialog if the user is switching selections while it is open.
*/
void KJotsWidget::onUpdateSearch()
{
  if ( searchDialog->isVisible() ) {
    long searchOptions = searchDialog->options();
    if ( searchOptions & KFind::SelectedText ) {
      searchAllPages->setCheckState( Qt::Unchecked );
      searchAllPages->setEnabled( false );
    } else {
      searchAllPages->setEnabled( true );
    }

    if ( searchAllPages->checkState() == Qt::Checked ) {
      searchOptions &= ~KFind::SelectedText;
      searchDialog->setOptions( searchOptions );
      searchDialog->setHasSelection( false );
    } else {
      if ( activeEditor()->textCursor().hasSelection() ) {
        searchDialog->setHasSelection( true );
      }
    }

    if ( activeEditor()->textCursor().hasSelection() ) {
      if ( searchAllPages->checkState() == Qt::Unchecked ) {
        searchDialog->setHasSelection( true );
      }
    } else {
      searchOptions &= ~KFind::SelectedText;
      searchDialog->setOptions( searchOptions );
      searchDialog->setHasSelection( false );
    }
  }
}

/*!
    Called when the user presses OK in the search dialog.
*/
void KJotsWidget::onStartSearch()
{
  QString searchPattern = searchDialog->pattern();
  if ( !searchHistory.contains ( searchPattern ) ) {
    searchHistory.prepend( searchPattern );
  }

  QTextEdit *browserOrEditor = activeEditor();
  QTextCursor cursor = browserOrEditor->textCursor();

  long searchOptions = searchDialog->options();
  if ( searchOptions & KFind::FromCursor ) {
    searchPos = cursor.position();
    searchBeginPos = 0;
    cursor.movePosition( QTextCursor::End );
    searchEndPos = cursor.position();
  } else {
    if ( searchOptions & KFind::SelectedText ) {
      searchBeginPos = cursor.selectionStart();
      searchEndPos = cursor.selectionEnd();
    } else {
      searchBeginPos = 0;
      cursor.movePosition( QTextCursor::End );
      searchEndPos = cursor.position();
    }

    if ( searchOptions & KFind::FindBackwards ) {
      searchPos = searchEndPos;
    } else {
      searchPos = searchBeginPos;
    }
  }

  m_xmlGuiClient->actionCollection()->action( QLatin1String(KStandardAction::name( KStandardAction::FindNext )) )->setEnabled( true );

  onRepeatSearch();
}

/*!
    Called when user chooses "Find Next"
*/
void KJotsWidget::onRepeatSearch()
{
  if ( search( false ) == 0 ) {
    KMessageBox::sorry( 0, i18n( "<qt>No matches found.</qt>" ) );
    m_xmlGuiClient->actionCollection()->action( QLatin1String(KStandardAction::name( KStandardAction::FindNext )) )->setEnabled( false );
  }
}

/*!
    Called when user presses Cancel in find dialog.
*/
void KJotsWidget::onEndSearch()
{
  m_xmlGuiClient->actionCollection()->action( QLatin1String(KStandardAction::name( KStandardAction::FindNext )) )->setEnabled( false );
}

/*!
    Shows the replace dialog when "Replace" is selected.
*/
void KJotsWidget::onShowReplace()
{
  Q_ASSERT( editor->isVisible() );

  if ( editor->textCursor().hasSelection() ) {
    replaceDialog->setHasSelection( true );
    long dialogOptions = replaceDialog->options();
    dialogOptions |= KFind::SelectedText;
    replaceDialog->setOptions( dialogOptions );
  } else {
    replaceDialog->setHasSelection( false );
  }

  replaceDialog->setFindHistory( searchHistory );
  replaceDialog->setReplacementHistory( replaceHistory );
  replaceDialog->show();
  onUpdateReplace();
}

/*!
    Updates the replace dialog if the user is switching selections while it is open.
*/
void KJotsWidget::onUpdateReplace()
{
  if ( replaceDialog->isVisible() ) {
    long replaceOptions = replaceDialog->options();
    if ( replaceOptions & KFind::SelectedText ) {
      replaceAllPages->setCheckState( Qt::Unchecked );
      replaceAllPages->setEnabled( false );
    } else {
      replaceAllPages->setEnabled( true );
    }

    if ( replaceAllPages->checkState() == Qt::Checked ) {
      replaceOptions &= ~KFind::SelectedText;
      replaceDialog->setOptions( replaceOptions );
      replaceDialog->setHasSelection( false );
    } else {
      if ( activeEditor()->textCursor().hasSelection() ) {
        replaceDialog->setHasSelection( true );
      }
    }
  }
}

/*!
    Called when the user presses OK in the replace dialog.
*/
void KJotsWidget::onStartReplace()
{
  QString searchPattern = replaceDialog->pattern();
  if ( !searchHistory.contains ( searchPattern ) ) {
    searchHistory.prepend( searchPattern );
  }

  QString replacePattern = replaceDialog->replacement();
  if ( !replaceHistory.contains ( replacePattern ) ) {
    replaceHistory.prepend( replacePattern );
  }

  QTextCursor cursor = editor->textCursor();

  long replaceOptions = replaceDialog->options();
  if ( replaceOptions & KFind::FromCursor ) {
    replacePos = cursor.position();
    replaceBeginPos = 0;
    cursor.movePosition( QTextCursor::End );
    replaceEndPos = cursor.position();
  } else {
    if ( replaceOptions & KFind::SelectedText ) {
      replaceBeginPos = cursor.selectionStart();
      replaceEndPos = cursor.selectionEnd();
    } else {
      replaceBeginPos = 0;
      cursor.movePosition( QTextCursor::End );
      replaceEndPos = cursor.position();
    }

    if ( replaceOptions & KFind::FindBackwards ) {
      replacePos = replaceEndPos;
    } else {
      replacePos = replaceBeginPos;
    }
  }

  replaceStartPage = treeview->selectionModel()->selectedRows().first();

  //allow KReplaceDialog to exit so the user can see.
  QTimer::singleShot( 0, this, SLOT(onRepeatReplace()) );
}

/*!
    Only called after onStartReplace. Kept the name scheme for consistancy.
*/
void KJotsWidget::onRepeatReplace()
{
  KJotsReplaceNextDialog *dlg = 0;

  QString searchPattern = replaceDialog->pattern();
  QString replacePattern = replaceDialog->replacement();
  int found = 0;
  int replaced = 0;

  long replaceOptions = replaceDialog->options();
  if ( replaceOptions & KReplaceDialog::PromptOnReplace ) {
    dlg = new KJotsReplaceNextDialog( this );
  }

  forever {
    if ( !search( true ) ) {
      break;
    }

    QTextCursor cursor = editor->textCursor();
    if ( !cursor.hasSelection() ) {
      break;
    } else {
      ++found;
    }

    QString replacementText = replacePattern;
    if ( replaceOptions & KReplaceDialog::BackReference ) {
      QRegExp regExp ( searchPattern, ( replaceOptions & Qt::CaseSensitive ) ?
                                        Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2 );
      regExp.indexIn(cursor.selectedText());
      int capCount = regExp.numCaptures();
      for ( int i=0; i <= capCount; ++i ) {
        QString c = QString::fromLatin1( "\\%1" ).arg( i );
        replacementText.replace( c, regExp.cap( i ) );
      }
    }

    if ( replaceOptions & KReplaceDialog::PromptOnReplace ) {
      dlg->setLabel( cursor.selectedText(), replacementText );

      if ( !dlg->exec() ) {
        break;
      }

      if ( dlg->answer() != KDialog::User2 ) {
        cursor.insertText( replacementText );
        editor->setTextCursor( cursor );
        ++replaced;
      }

      if ( dlg->answer() == KDialog::User1 ) {
        replaceOptions |= ~KReplaceDialog::PromptOnReplace;
      }
    } else {
      cursor.insertText( replacementText );
      editor->setTextCursor( cursor );
      ++replaced;
    }
  }

  if ( replaced == found )
  {
    KMessageBox::information( 0, i18np( "<qt>Replaced 1 occurrence.</qt>", "<qt>Replaced %1 occurrences.</qt>", replaced ) );
  }
  else if ( replaced < found )
  {
    KMessageBox::information( 0,
                             i18np( "<qt>Replaced %2 of 1 occurrence.</qt>", "<qt>Replaced %2 of %1 occurrences.</qt>", found, replaced ) );
  }

  if ( dlg ) {
    delete dlg;
  }
}

/*!
    Called when user presses Cancel in replace dialog. Just a placeholder for now.
*/
void KJotsWidget::onEndReplace()
{
}

/*!
    Searches for the given pattern, with the given options. This is huge and
    unwieldly function, but the operation we're performing is huge and unwieldly.
*/
int KJotsWidget::search( bool replacing )
{
  int rc = 0;
  int *beginPos = replacing ? &replaceBeginPos : &searchBeginPos;
  int *endPos = replacing ? &replaceEndPos : &searchEndPos;
  long options = replacing ? replaceDialog->options() : searchDialog->options();
  QString pattern = replacing ? replaceDialog->pattern() : searchDialog->pattern();
  int *curPos = replacing ? &replacePos : &searchPos;

  QModelIndex startPage = replacing ? replaceStartPage : treeview->selectionModel()->selectedRows().first();

  bool allPages = false;
  QCheckBox *box = replacing ? replaceAllPages : searchAllPages;
  if ( box->isEnabled() && box->checkState() == Qt::Checked ) {
    allPages = true;
  }

  QTextDocument::FindFlags findFlags = 0;
  if ( options & Qt::CaseSensitive ) {
    findFlags |= QTextDocument::FindCaseSensitively;
  }

  if ( options & KFind::WholeWordsOnly ) {
    findFlags |= QTextDocument::FindWholeWords;
  }

  if ( options & KFind::FindBackwards ) {
    findFlags |= QTextDocument::FindBackward;
  }

  // We will find a match or return 0
  int attempts = 0;
  forever {
    ++attempts;

    QTextEdit *browserOrEditor = activeEditor();
    QTextDocument *theDoc = browserOrEditor->document();

    QTextCursor cursor;
    if ( options & KFind::RegularExpression ) {
      QRegExp regExp ( pattern, ( options & Qt::CaseSensitive ) ?
                                  Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2 );
      cursor = theDoc->find( regExp, *curPos, findFlags );
    } else {
      cursor = theDoc->find( pattern, *curPos, findFlags );
    }

    if ( cursor.hasSelection() ) {
      if ( cursor.selectionStart() >= *beginPos && cursor.selectionEnd() <= *endPos ) {
        browserOrEditor->setTextCursor( cursor );
        browserOrEditor->ensureCursorVisible();
        *curPos =  ( options & KFind::FindBackwards ) ?
                    cursor.selectionStart() : cursor.selectionEnd();
        rc = 1;
        break;
      }
    }

    //No match. Determine what to do next.

    if ( replacing && !( options & KFind::FromCursor ) && !allPages ) {
      break;
    }

    if ( ( options & KFind::FromCursor ) && !allPages ) {
      if ( KMessageBox::questionYesNo( this,
              i18n("<qt>End of search area reached. Do you want to wrap around and continue?</qt>")) ==
              KMessageBox::No ) {
          rc = 3;
          break;
        }
    }

    if ( allPages ) {
      if ( options & KFind::FindBackwards ) {
        if ( canGoPreviousPage() )
          prevPage();
      } else {
        if ( canGoNextPage() )
          nextPage();
      }

      if ( startPage == treeview->selectionModel()->selectedRows().first() ) {
        rc = 0;
        break;
      }

      *beginPos = 0;
      cursor = editor->textCursor();
      cursor.movePosition( QTextCursor::End );
      *endPos = cursor.position();
      *curPos = ( options & KFind::FindBackwards ) ? *endPos : *beginPos;
      continue;
    }

    // By now, we should have figured out what to do. In all remaining cases we
    // will automatically loop and try to "find next" from the top/bottom, because
    // I like this behavior the best.
    if ( attempts <= 1 ) {
      *curPos = ( options & KFind::FindBackwards ) ? *endPos : *beginPos;
    } else {
      // We've already tried the loop and failed to find anything. Bail.
      rc = 0;
      break;
    }
  }

  return rc;
}


void KJotsWidget::updateCaption()
{
  emit captionChanged( treeview->captionForSelection( QLatin1String(" / ") ) );
}

void KJotsWidget::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
  QModelIndexList rows = treeview->selectionModel()->selectedRows();

  if ( rows.size() != 1 )
    return;

  QItemSelection changed( topLeft, bottomRight );
  if ( changed.contains( rows.first() ) )
  {
    emit captionChanged( treeview->captionForSelection( QLatin1String(" / ") ) );
  }
}

bool KJotsWidget::queryClose()
{
  KJotsSettings::setSplitterSizes(m_splitter->sizes());

  KJotsSettings::self()->writeConfig();
  m_orderProxy->saveOrder();

  return true;
}

void KJotsWidget::actionLock()
{
  QModelIndexList selection = treeview->selectionModel()->selectedRows();

  if ( selection.isEmpty() )
    return;

  Collection::List collections;
  Item::List items;
  foreach ( const QModelIndex &idx, selection )
  {
    Collection col = idx.data( EntityTreeModel::CollectionRole ).value<Collection>();
    if ( col.isValid() )
    {
      collections << col;
    } else {
      Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();
      if ( item.isValid() )
        items << item;
    }
  }
  if ( collections.isEmpty() && items.isEmpty() )
    return;

  KJotsLockJob *job = new KJotsLockJob(collections, items, this);
}

void KJotsWidget::actionUnlock()
{
  QModelIndexList selection = treeview->selectionModel()->selectedRows();

  if ( selection.isEmpty() )
    return;

  Collection::List collections;
  Item::List items;
  foreach ( const QModelIndex &idx, selection )
  {
    Collection col = idx.data( EntityTreeModel::CollectionRole ).value<Collection>();
    if ( col.isValid() )
    {
      collections << col;
    } else {
      Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();
      if ( item.isValid() )
        items << item;
    }
  }
  if ( collections.isEmpty() && items.isEmpty() )
    return;

  KJotsLockJob *job = new KJotsLockJob(collections, items, KJotsLockJob::UnlockJob, this);
}

void KJotsWidget::actionSortChildrenAlpha()
{
  QModelIndexList selection = treeview->selectionModel()->selectedRows();

  foreach( const QModelIndex &index, selection )
  {
    const QPersistentModelIndex persistent( index );
    m_sortProxyModel->sortChildrenAlphabetically( m_orderProxy->mapToSource( index ) );
    m_orderProxy->clearOrder( persistent );
  }
}

void KJotsWidget::actionSortChildrenByDate()
{
  QModelIndexList selection = treeview->selectionModel()->selectedRows();

  foreach( const QModelIndex &index, selection )
  {
    const QPersistentModelIndex persistent( index );
    m_sortProxyModel->sortChildrenByCreationTime( m_orderProxy->mapToSource( index ) );
    m_orderProxy->clearOrder( persistent );
  }
}

