/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mimeparttreeview.h"
#include "mimetreemodel.h"
#include "settings/globalsettings.h"

#include <KMime/Content>

#include <KConfigGroup>
#include <QHeaderView>

using namespace MessageViewer;

MimePartTreeView::MimePartTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setObjectName( QLatin1String("mMimePartTree") );

    mMimePartModel = new MimeTreeModel( this );
    setModel( mMimePartModel );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );
    connect(this, SIGNAL(destroyed(QObject*)), this, SLOT(slotMimePartDestroyed()) );
    setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setResizeMode( QHeaderView::ResizeToContents );
    connect(mMimePartModel,SIGNAL(modelReset()),this,SLOT(expandAll()));
    restoreMimePartTreeConfig();
}

MimePartTreeView::~MimePartTreeView()
{
    saveMimePartTreeConfig();
}

MimeTreeModel *MimePartTreeView::mimePartModel() const
{
    return mMimePartModel;
}

void MimePartTreeView::restoreMimePartTreeConfig()
{
    KConfigGroup grp( GlobalSettings::self()->config(), "MimePartTree" );
    header()->restoreState( grp.readEntry( "State", QByteArray() ) );
}

void MimePartTreeView::saveMimePartTreeConfig()
{
    KConfigGroup grp( GlobalSettings::self()->config(), "MimePartTree" );
    grp.writeEntry( "State", header()->saveState() );
}

void MimePartTreeView::slotMimePartDestroyed()
{
    //root is either null or a modified tree that we need to clean up
    clearModel();
}


void MimePartTreeView::clearModel()
{
    delete mMimePartModel->root();
    mMimePartModel->setRoot(0);
}

void MimePartTreeView::setRoot(KMime::Content *root)
{
    delete mMimePartModel->root();
    mMimePartModel->setRoot( root );
}

KMime::Content::List MimePartTreeView::selectedContents()
{
    KMime::Content::List contents;
#ifndef QT_NO_TREEVIEW
    QItemSelectionModel *selectModel = selectionModel();
    QModelIndexList selectedRows = selectModel->selectedRows();

    Q_FOREACH( const QModelIndex &index, selectedRows )
    {
        KMime::Content *content = static_cast<KMime::Content*>( index.internalPointer() );
        if ( content )
            contents.append( content );
    }
#endif

    return contents;
}
