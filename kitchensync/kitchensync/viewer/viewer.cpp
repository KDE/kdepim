/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "viewer.h"

#include <konnector.h>
#include <core.h>
#include <engine.h>

#include <kaboutdata.h>
#include <kiconloader.h>
#include <kparts/genericfactory.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>

using namespace KSync;

typedef KParts::GenericFactory<Viewer> ViewerFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_viewer, ViewerFactory )

Viewer::Viewer( QWidget *parent, const char *name,
                    QObject *, const char *,const QStringList & )
  : ActionPart( parent, name ), mTopWidget( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon("xmag", KIcon::Desktop, 48 );
}

KAboutData *Viewer::createAboutData()
{
  return new KAboutData("KSyncViewer", I18N_NOOP("Sync Viewer Part"), "0.0" );
}

Viewer::~Viewer()
{
  delete mTopWidget;
}

QString Viewer::type() const
{
  return QString::fromLatin1("viewer");
}

QString Viewer::title() const
{
  return i18n("Data Viewer");
}

QString Viewer::description() const
{
  return i18n("Viewer for data handled by KitchenSync.");
}

QPixmap *Viewer::pixmap()
{
  return &m_pixmap;
}

QString Viewer::iconName() const
{
  return QString::fromLatin1("xmag");
}

bool Viewer::hasGui() const
{
  return true;
}

QWidget *Viewer::widget()
{
  if( !mTopWidget ) {
    mTopWidget = new QWidget;
    QBoxLayout *topLayout = new QVBoxLayout( mTopWidget );
    topLayout->setSpacing( KDialog::spacingHint() );

    mListView = new QListView( mTopWidget );
    mListView->setRootIsDecorated( true );
    mListView->addColumn( i18n("Konnector Data" ) );
    topLayout->addWidget( mListView );

    QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );
    QPushButton *button = new QPushButton( i18n("Expand all"), mTopWidget );
    connect( button, SIGNAL( clicked() ), SLOT( expandAll() ) );
    buttonLayout->addWidget( button );
    
    button = new QPushButton( i18n("Collapse all"), mTopWidget );
    connect( button, SIGNAL( clicked() ), SLOT( collapseAll() ) );
    buttonLayout->addWidget( button );
    
    buttonLayout->addStretch( 1 );
  }
  return mTopWidget;
}

void Viewer::expandAll()
{
  QListViewItemIterator it( mListView );
  while ( it.current() ) {
    if ( it.current()->childCount() > 0 ) it.current()->setOpen( true );
    ++it;
  }  
}

void Viewer::collapseAll()
{
  QListViewItemIterator it( mListView );
  while ( it.current() ) {
    if ( it.current()->childCount() > 0 ) it.current()->setOpen( false );
    ++it;
  }  
}

void Viewer::executeAction()
{
  kdDebug() << "Viewer::executeAction()" << endl;

  mListView->clear();

  Konnector::List konnectors = core()->engine()->konnectors();
  Konnector *k;
  for( k = konnectors.first(); k; k = konnectors.next() ) {
    QListViewItem *topItem = new QListViewItem( mListView, k->resourceName() );
//    kdDebug() << "Konnector: " << k->resourceName() << endl;
    SynceeList syncees = k->syncees();
//    kdDebug() << syncees.count() << " Syncees found." << endl;
    SynceeList::ConstIterator it2;
    for( it2 = syncees.begin(); it2 != syncees.end(); ++it2 ) {
      Syncee *s = *it2;
      if ( !s->isValid() ) continue;

//      kdDebug() << "Syncee " << s->identifier() << endl;
      QListViewItem *synceeItem = new QListViewItem( topItem,
                                                     s->identifier() );
      SyncEntry *entry;
      for( entry = s->firstEntry(); entry; entry = s->nextEntry() ) {
        kdDebug() << "  SyncEntry: " << entry->name() << endl;
        new QListViewItem( synceeItem, entry->name() );
      }
    }
  }

  expandAll();
}

#include "viewer.moc"
