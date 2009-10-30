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
#include <QTextBrowser>
#include <QTimer>

// Akonadi
#include <akonadi/control.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/collectioncreatejob.h>
#include <akonadi/changerecorder.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/item.h>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/session.h>

// Grantlee
#include <grantlee/template.h>
#include <grantlee/engine.h>
#include <grantlee/context.h>

// KDE
#include <KAction>
#include <KActionCollection>
#include <KBookmarkMenu>
#include <kdescendantsproxymodel.h>
#include <KFileDialog>
#include <KFind>
#include <KFindDialog>
#include <KLocale>
#include <KMessageBox>
#include <KReplaceDialog>
#include <kselectionproxymodel.h>
#include <KStandardDirs>
#include <KTextEdit>
#include <KGlobalSettings>
#include <KXMLGUIClient>

// KMime
#include <KMime/KMimeMessage>

// KJots
#include "kjotsbookmarks.h"
#include "kjotsmodel.h"
#include "kjotsedit.h"
#include "kjotstreeview.h"
#include "kjotsconfigdlg.h"
#include "kjotsreplacenextdialog.h"

#include <kdebug.h>

#include <memory>

using namespace Akonadi;
using namespace Grantlee;

KJotsWidget::KJotsWidget( QWidget * parent, KXMLGUIClient *xmlGuiClient, Qt::WindowFlags f )
    : QWidget( parent, f ), m_xmlGuiClient( xmlGuiClient )
{

  Akonadi::Control::widgetNeedsAkonadi( this );
  Akonadi::Control::start( this );


  QSplitter *splitter = new QSplitter( this );

  splitter->setStretchFactor(1, 1);
  splitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  QHBoxLayout *layout = new QHBoxLayout( this );

  KStandardDirs KStd;
  Engine *engine = Engine::instance();
  engine->setPluginDirs( KStd.findDirs( "lib", QLatin1String( "grantlee" ) ) );

  m_loader = FileSystemTemplateLoader::Ptr( new FileSystemTemplateLoader() );
  m_loader->setTemplateDirs( KStd.findDirs( "data", QLatin1String( "kjots/themes" ) ) );
  m_loader->setTheme( QLatin1String( "default" ) );

  engine->addTemplateLoader( m_loader );

  treeview = new KJotsTreeView( xmlGuiClient, splitter );

  ItemFetchScope scope;
  scope.fetchFullPayload( true ); // Need to have full item when adding it to the internal data structure
  scope.fetchAttribute< EntityDisplayAttribute >();

  ChangeRecorder *monitor = new ChangeRecorder( this );
  monitor->fetchCollection( true );
  monitor->setItemFetchScope( scope );
  monitor->setCollectionMonitored( Collection::root() );
  monitor->setMimeTypeMonitored( QLatin1String( "text/x-vnd.akonadi.note" ) );

  Session *session = new Session( QByteArray( "EntityTreeModel-" ) + QByteArray::number( qrand() ), this );

  m_kjotsModel = new KJotsModel(session, monitor, this);

  treeview->setModel( m_kjotsModel );
  treeview->setSelectionMode( QAbstractItemView::ExtendedSelection );
  treeview->setEditTriggers( QAbstractItemView::DoubleClicked );

  selProxy = new KSelectionProxyModel( treeview->selectionModel(), this );
  selProxy->setSourceModel( m_kjotsModel );

  // TODO: Write a QAbstractItemView subclass to render kjots selection.
  connect( selProxy, SIGNAL( dataChanged(QModelIndex,QModelIndex)), SLOT(renderSelection()) );
  connect( selProxy, SIGNAL( rowsInserted(const QModelIndex &, int, int)), SLOT(renderSelection()) );
  connect( selProxy, SIGNAL( rowsRemoved(const QModelIndex &, int, int)), SLOT(renderSelection()) );

  connect( treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)) );

  stackedWidget = new QStackedWidget( splitter );

  KActionCollection *actionCollection = xmlGuiClient->actionCollection();

  editor = new KJotsEdit( treeview->selectionModel(), stackedWidget );
  editor->createActions( actionCollection );
  stackedWidget->addWidget( editor );

  layout->addWidget( splitter );

  browser = new QTextBrowser( stackedWidget );
  stackedWidget->addWidget( browser );
  stackedWidget->setCurrentWidget( browser );


  KAction *action;

  action = actionCollection->addAction( "go_next_book" );
  action->setText( i18n( "Next Book" ) );
  action->setIcon( KIcon( "go-down" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_D ) );
  connect( action, SIGNAL(triggered()), SLOT(nextBook()) );
  connect( this, SIGNAL(canGoNextBookChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( "go_prev_book" );
  action->setText( i18n( "Previous Book" ) );
  action->setIcon( KIcon( "go-up" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_D ) );
  connect( action, SIGNAL(triggered()), SLOT(prevBook()) );
  connect( this, SIGNAL(canGoPreviousBookChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( "go_next_page" );
  action->setText( i18n( "Next Page" ) );
  action->setIcon( KIcon( "go-next" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_PageDown ) );
  connect( action, SIGNAL(triggered()), SLOT(nextPage()));
  connect( this, SIGNAL(canGoNextPageChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( "go_prev_page" );
  action->setText( i18n( "Previous Page" ) );
  action->setIcon( KIcon( "go-previous" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_PageUp ) );
  connect( action, SIGNAL(triggered()), SLOT(prevPage()) );
  connect( this, SIGNAL(canGoPreviousPageChanged(bool)), action, SLOT(setEnabled(bool)) );

  action = actionCollection->addAction( "new_page" );
  action->setText( i18n( "&New Page" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  action->setIcon( KIcon( "document-new" ) );
  connect( action, SIGNAL(triggered()), SLOT(newPage()) );

  action = actionCollection->addAction("new_book");
  action->setText( i18n( "New &Book..." ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_N ) );
  action->setIcon( KIcon( "address-book-new" ) );
  connect( action, SIGNAL(triggered()), SLOT(newBook()) );

  action = actionCollection->addAction( "del_page" );
  action->setText( i18n( "&Delete Page" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Delete ) );
  action->setIcon( KIcon( "edit-delete-page" ) );
  connect( action, SIGNAL(triggered()), SLOT(deletePage()) );

  action = actionCollection->addAction( "del_folder" );
  action->setText( i18n( "Delete Boo&k" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Delete ) );
  action->setIcon( KIcon( "edit-delete" ) );
  connect( action, SIGNAL(triggered()), SLOT(deleteBook()) );

  action = actionCollection->addAction( "del_mult" );
  action->setText( i18n( "Delete Selected" ) );
  action->setIcon( KIcon( "edit-delete" ) );
  connect( action, SIGNAL(triggered()), SLOT(deleteMultiple()) );

  action = actionCollection->addAction( "manual_save" );
  action->setText( i18n( "Manual Save" ) );
  action->setIcon( KIcon( "document-save" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );

  action = actionCollection->addAction( "auto_bullet" );
  action->setText( i18n( "Auto Bullets" ) );
  action->setIcon( KIcon( "format-list-unordered" ) );
  action->setCheckable( true );

  action = actionCollection->addAction( "auto_decimal" );
  action->setText( i18n( "Auto Decimal List" ) );
  action->setIcon( KIcon( "format-list-ordered" ) );
  action->setCheckable( true );

  action = actionCollection->addAction( "manage_link" );
  action->setText( i18n( "Link" ) );
  action->setIcon( KIcon( "insert-link" ) );

  action = actionCollection->addAction( "insert_checkmark" );
  action->setText( i18n( "Insert Checkmark" ) );
  action->setIcon( KIcon( "checkmark" ) );
  action->setEnabled( false );

  action = actionCollection->addAction( "rename_entry" );
  action->setText( i18n( "Rename..." ) );
  action->setIcon( KIcon( "edit-rename" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ) );

  action = actionCollection->addAction( "insert_date" );
  action->setText( i18n( "Insert Date" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_I ) );
  action->setIcon( KIcon( "view-calendar-time-spent" ) );

  action = actionCollection->addAction( "change_color" );
  action->setIcon( KIcon( "format-fill-color" ) );
  action->setText( i18n( "Change Color..." ) );

  action = actionCollection->addAction( "copy_link_address" );
  action->setText( i18n( "Copy Link Address" ) );

  action = KStandardAction::cut( editor, SLOT(cut()), actionCollection );
  connect( editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  action->setEnabled( false );

  action = KStandardAction::copy( this, SLOT(copy()), actionCollection );
  connect( editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  connect( browser, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  action->setEnabled( false );

  KStandardAction::pasteText( editor, SLOT(paste()), actionCollection );

  action = actionCollection->addAction( "copyIntoTitle" );
  action->setText( i18n( "Copy &into Page Title" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ) );
  action->setIcon( KIcon( "edit-copy" ) );
  connect( action, SIGNAL(triggered()), SLOT(copySelectionToTitle()) );
  connect( editor, SIGNAL(copyAvailable(bool)), action, SLOT(setEnabled(bool)) );
  action->setEnabled( false );

  action = actionCollection->addAction( "paste_plain_text" );
  action->setText( i18nc( "@action Paste the text in the clipboard without rich text formatting.", "Paste Plain Text" ) );
  connect( action, SIGNAL(triggered()), editor, SLOT(pastePlainText()) );

  KStandardAction::preferences( this, SLOT(configure()), actionCollection );

  bookmarkMenu = actionCollection->add<KActionMenu>( "bookmarks" );
  bookmarkMenu->setText( i18n( "&Bookmarks" ) );
  KJotsBookmarks* bookmarks = new KJotsBookmarks( treeview );
  /*KBookmarkMenu *bmm =*/ new KBookmarkMenu(
      KBookmarkManager::managerForFile( KStandardDirs::locateLocal( "data","kjots/bookmarks.xml" ), "kjots" ),
      bookmarks, bookmarkMenu->menu(), actionCollection );

  KStandardAction::find( this, SLOT( onShowSearch() ), actionCollection );
  action = KStandardAction::findNext( this, SLOT( onRepeatSearch() ), actionCollection );
  action->setEnabled(false);
  KStandardAction::replace( this, SLOT( onShowReplace() ), actionCollection );

  QTimer::singleShot( 0, this, SLOT(delayedInitialization()) );

  connect( treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(updateMenu()) );
}

void KJotsWidget::delayedInitialization()
{

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
  entryActions.insert( actionCollection->action(KStandardAction::name(KStandardAction::Find)) );
//   entryActions.insert( actionCollection->action(KStandardAction::name(KStandardAction::Print)) );
  entryActions.insert( actionCollection->action("rename_entry") );
  entryActions.insert( actionCollection->action("change_color") );
//   entryActions.insert( actionCollection->action("save_to") );
  entryActions.insert( actionCollection->action("copy_link_address") );

  // Actions that are used only when a page is selected.
  pageActions.insert( actionCollection->action(KStandardAction::name(KStandardAction::Cut)) );
  pageActions.insert( actionCollection->action(KStandardAction::name(KStandardAction::Paste)) );
  pageActions.insert( actionCollection->action(KStandardAction::name(KStandardAction::Replace)) );
  pageActions.insert( actionCollection->action("del_page") );
  pageActions.insert( actionCollection->action("insert_date") );
  pageActions.insert( actionCollection->action("auto_bullet") );
  pageActions.insert( actionCollection->action("auto_decimal") );
  pageActions.insert( actionCollection->action("manage_link") );
  pageActions.insert( actionCollection->action("insert_checkmark") );

  // Actions that are used only when a book is selected.
//   bookActions.insert( actionCollection->action("save_to_book") );
  bookActions.insert( actionCollection->action("del_folder") );

  // Actions that are used when multiple items are selected.
  multiselectionActions.insert( actionCollection->action(KStandardAction::name(KStandardAction::Find)) );
//   multiselectionActions.insert( actionCollection->action(KStandardAction::name(KStandardAction::Print)));
  multiselectionActions.insert( actionCollection->action("del_mult") );
//   multiselectionActions.insert( actionCollection->action("save_to") );
  multiselectionActions.insert( actionCollection->action("change_color") );


  treeview->delayedInitialization();
  editor->delayedInitialization( m_xmlGuiClient->actionCollection() );
//   browser->delayedInitialization( m_xmlGuiClient->actionCollection() );


  connect( treeview->itemDelegate(), SIGNAL(closeEditor(QWidget *,QAbstractItemDelegate::EndEditHint) ),
      SLOT(bookshelfEditItemFinished(QWidget *,QAbstractItemDelegate::EndEditHint)) );

  connect( editor, SIGNAL(currentCharFormatChanged(const QTextCharFormat&)),
      SLOT(currentCharFormatChanged(const QTextCharFormat &)) );
  updateMenu();
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
      foreach ( QAction* action, bookActions )
        action->setEnabled(true);

      editor->setActionsEnabled( false );
    } else {
      foreach ( QAction* action, pageActions ) {
        if (action->objectName() == name( KStandardAction::Cut ) ) {
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

KJotsWidget::~KJotsWidget()
{

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

  Item item = selectedRows.at( 0 ).data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return;

  (void) new Akonadi::ItemDeleteJob( item, this );

}

void KJotsWidget::deleteBook()
{
  QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

  if ( selectedRows.size() != 1 )
    return;

  Collection col = selectedRows.at( 0 ).data( EntityTreeModel::CollectionRole ).value<Collection>();

  if ( !col.isValid() )
    return;

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
  newCollection.setName( title );
  newCollection.setContentMimeTypes( QStringList( "text/x-vnd.akonadi.note" ) );

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

  Item newItem;
  newItem.setMimeType( QLatin1String( "text/x-vnd.akonadi.note" ) );

  KMime::Message::Ptr newPage = KMime::Message::Ptr( new KMime::Message() );

  QString title = i18nc( "The default name for new pages.", "New Page" );
  QByteArray encoding( "utf-8" );

  newPage->subject( true )->fromUnicodeString( title, encoding );
  newPage->contentType( true )->setMimeType( "text/plain" );
  newPage->date( true )->setDateTime( KDateTime::currentLocalDateTime() );
  newPage->from( true )->fromUnicodeString( "Kjots@kde4", encoding );
  newPage->to( true )->fromUnicodeString( "user@kde4", encoding );

  newPage->assemble();

  newItem.setPayload( newPage );

  Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( newItem, col, this );
  connect( job, SIGNAL( result( KJob* ) ), SLOT(newPageResult( KJob* )) );

}

void KJotsWidget::newPageResult( KJob* job )
{
  if ( job->error() )
    kDebug() << job->errorString();
}

void KJotsWidget::newBookResult( KJob* job )
{
  if ( job->error() )
    kDebug() << job->errorString();
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
    objectList << QVariant::fromValue(obj);
  }

  hash.insert( QLatin1String( "entities" ), objectList);
  Context c(hash);

  Engine *engine = Engine::instance();
  Template t = engine->loadByName( QLatin1String( "template.html" ) );

  QString result = t->render(&c);
  // TODO: handle errors.
  return result;
}

void KJotsWidget::renderSelection()
{
  const int rows = selProxy->rowCount();

  // If the selection is a single page, present it for editing...
  if (rows == 1)
  {
    QModelIndex idx = selProxy->index( 0, 0, QModelIndex());

    Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();
    if (item.isValid())
    {
      if (!item.hasPayload<KMime::Message::Ptr>())
        return;

      KMime::Message::Ptr page = item.payload<KMime::Message::Ptr>();
      editor->setText( page->mainBodyPart()->decodedText() );
      stackedWidget->setCurrentWidget( editor );
      return;
    }
  }

  // ... Otherwise, render the selection read-only.

  QTextDocument doc;
  QTextCursor cursor(&doc);

  browser->setHtml( renderSelectionToHtml() );
  stackedWidget->setCurrentWidget( browser );
}

QString KJotsWidget::getThemeFromUser()
{
  bool ok;
  QString text = QInputDialog::getText(this, i18n("Change Theme"),
                                      tr("Theme name:"), QLineEdit::Normal,
                                      m_loader->themeName(), &ok);
  if (!ok || text.isEmpty())
  {
    return QLatin1String("default");
  }

  return text;
}


void KJotsWidget::changeTheme()
{
  m_loader->setTheme(getThemeFromUser());
  renderSelection();
}

void KJotsWidget::exportSelection()
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
  kWarning( "No valid selection" );
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
    kDebug() << sibling << sibling.data() << sibling.data( role ).toInt();
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
  Q_UNUSED( deselected );

  emit canGoNextBookChanged( canGoPreviousBook() );
  emit canGoNextPageChanged( canGoNextPage() );
  emit canGoPreviousBookChanged( canGoPreviousBook() );
  emit canGoPreviousPageChanged( canGoPreviousPage() );
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

  m_xmlGuiClient->actionCollection()->action( KStandardAction::name( KStandardAction::FindNext ) )->setEnabled( true );

  onRepeatSearch();
}

/*!
    Called when user chooses "Find Next"
*/
void KJotsWidget::onRepeatSearch()
{
  if ( search( false ) == 0 ) {
    KMessageBox::sorry( 0, i18n( "<qt>No matches found.</qt>" ) );
    m_xmlGuiClient->actionCollection()->action( KStandardAction::name( KStandardAction::FindNext ) )->setEnabled( false );
  }
}

/*!
    Called when user presses Cancel in find dialog.
*/
void KJotsWidget::onEndSearch()
{
  m_xmlGuiClient->actionCollection()->action( KStandardAction::name( KStandardAction::FindNext ) )->setEnabled( false );
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
  QTimer::singleShot( 0, this, SLOT( onRepeatReplace() ) );
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
      for ( int i=0; i <= capCount; i++ ) {
        QString c = QString( "\\%1" ).arg( i );
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


#include "kjotswidget.moc"
