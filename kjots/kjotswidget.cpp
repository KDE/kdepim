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
#include <akonadi/changerecorder.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/entityfilterproxymodel.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/item.h>
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

// KMime
#include <KMime/KMimeMessage>

// KJots
#include "kjotsmodel.h"

#include <kdebug.h>

#include <memory>

using namespace Akonadi;
using namespace Grantlee;

KJotsWidget::KJotsWidget( QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f )
{

  Akonadi::Control::widgetNeedsAkonadi( this );
  Akonadi::Control::start( this );


  QSplitter *splitter = new QSplitter( this );
  QHBoxLayout *layout = new QHBoxLayout( this );

  KStandardDirs KStd;
  Engine *engine = Engine::instance();
  engine->setPluginDirs( KStd.findDirs( "lib", QLatin1String( "grantlee" ) ) );

  m_loader = FileSystemTemplateLoader::Ptr( new FileSystemTemplateLoader() );
  m_loader->setTemplateDirs( KStd.findDirs( "data", QLatin1String( "kjotsrewrite/themes" ) ) );
  m_loader->setTheme( QLatin1String( "default" ) );

  engine->addTemplateLoader( m_loader );

  treeview = new EntityTreeView( splitter );

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

  selProxy = new KSelectionProxyModel( treeview->selectionModel(), this );
  selProxy->setSourceModel( m_kjotsModel );

  // TODO: Write a QAbstractItemView subclass to render kjots selection.
  connect( selProxy, SIGNAL( dataChanged(QModelIndex,QModelIndex)), SLOT(renderSelection()) );
  connect( selProxy, SIGNAL( rowsInserted(const QModelIndex &, int, int)), SLOT(renderSelection()) );
  connect( selProxy, SIGNAL( rowsRemoved(const QModelIndex &, int, int)), SLOT(renderSelection()) );

  stackedWidget = new QStackedWidget( splitter );

  editor = new KTextEdit( stackedWidget );
  stackedWidget->addWidget( editor );

  layout->addWidget( splitter );

  browser = new QTextBrowser( stackedWidget );
  stackedWidget->addWidget( browser );
  stackedWidget->setCurrentWidget( browser );
}

KJotsWidget::~KJotsWidget()
{

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
  for (int row = 0; row < rows; ++row)
  {
    QModelIndex idx = selProxy->index(row, column, QModelIndex());

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


#include "kjotswidget.moc"
