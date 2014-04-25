//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

//Own Header
#include "kjotsbrowser.h"

#include <QItemSelectionModel>

#include <krun.h>
#include <kdebug.h>
#include <AkonadiCore/EntityTreeModel>

KJotsBrowser::KJotsBrowser ( QItemSelectionModel *selectionModel, QWidget *parent )
  : QTextBrowser(parent), m_itemSelectionModel( selectionModel )
{
    setWordWrapMode(QTextOption::WordWrap);
}

KJotsBrowser::~KJotsBrowser()
{
}

void KJotsBrowser::delayedInitialization ()
{
    connect(this, SIGNAL(anchorClicked(QUrl)), SLOT(linkClicked(QUrl)));
}

/*!
    \brief Handle link clicks.
*/
void KJotsBrowser::linkClicked(const QUrl& link)
{
    //Stop QTextBrowser from being stupid by giving it an invalid url.
    QUrl url;
    setSource(url);

    QString anchor = link.fragment();

    if ( link.toString().startsWith(QLatin1String("#")) && (anchor.startsWith( QLatin1String( "book_" ) )
            || anchor.startsWith( QLatin1String( "page_" ) ) ) ) {
        scrollToAnchor(anchor);
        return;
    }

    if ( link.scheme() == QLatin1String("kjots") ) {
        const quint64 targetId = link.path().mid(1).toULongLong();
        if (link.host().endsWith(QLatin1String("book"))) {
          const QModelIndex colIndex = Akonadi::EntityTreeModel::modelIndexForCollection(m_itemSelectionModel->model(), Akonadi::Collection(targetId));
          if (!colIndex.isValid())
            return;
          m_itemSelectionModel->select(colIndex, QItemSelectionModel::ClearAndSelect);
        } else {
          Q_ASSERT(link.host().endsWith(QLatin1String("page")));
          const QModelIndexList itemIndexes = Akonadi::EntityTreeModel::modelIndexesForItem(m_itemSelectionModel->model(), Akonadi::Item(targetId));
          if (itemIndexes.size() != 1)
            return;
          m_itemSelectionModel->select(itemIndexes.first(), QItemSelectionModel::ClearAndSelect);
        }
    } else {
        new KRun ( link, this );
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
