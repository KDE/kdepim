/*
    This file is part of KJots.

    Copyright (c) 2008-2009 Stephen Kelly <steveire@gmail.com>

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
#include <kdescendantsproxymodel.h>
#include <KFileDialog>
#include <KLocale>
#include <KMessageBox>
#include <kselectionproxymodel.h>
#include <KStandardDirs>
#include <KTextEdit>
#include <KGlobalSettings>
#include <KAction>
#include <KActionCollection>
#include <KXMLGUIClient>

// KMime
#include <KMime/KMimeMessage>

// KJots
#include "kjotsmodel.h"
#include "kjotsedit.h"
#include "kjotstreeview.h"

#include <kdebug.h>

#include <memory>

using namespace Akonadi;
using namespace Grantlee;

KJotsWidget::KJotsWidget( QWidget * parent, KXMLGUIClient *xmlGuiclient, Qt::WindowFlags f )
    : QWidget( parent, f )
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

  treeview = new KJotsTreeView( xmlGuiclient, splitter );

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

  KActionCollection *actionCollection = xmlGuiclient->actionCollection();

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
  connect( treeview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), action, SLOT(selectionChanged()) );

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

}

KJotsWidget::~KJotsWidget()
{

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

void KJotsWidget::savePage(const QModelIndex &parent, int start, int end)
{
  // Disable this for now.
  return;

  if(parent.isValid() || start != 0 || end != 0)
    return;

  const int column = 0;
  QModelIndex idx = selProxy->index(start, column, parent);
  Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();
  if (!item.isValid())
    return;

  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  KMime::Message::Ptr page = item.payload<KMime::Message::Ptr>();



//   page.setContent(editor->toPlainText());
  item.setPayload(page);
  selProxy->setData(idx, QVariant::fromValue(item), EntityTreeModel::ItemRole );
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


#include "kjotswidget.moc"
