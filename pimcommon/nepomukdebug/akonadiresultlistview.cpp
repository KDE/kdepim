/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "akonadiresultlistview.h"
#include "pimcommon/nepomukdebug/searchdebugnepomukshowdialog.h"

#include <KLocalizedString>

#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>
#include <QPointer>

using namespace PimCommon;
AkonadiResultListView::AkonadiResultListView(QWidget *parent)
    : QListView(parent)
{
    setItemDelegate(new AkonadiResultListDelegate(this));
}

AkonadiResultListView::~AkonadiResultListView()
{

}

void AkonadiResultListView::contextMenuEvent( QContextMenuEvent * event )
{
    const QModelIndex index = indexAt( event->pos() );
    if (!index.isValid())
        return;

    QMenu *popup = new QMenu(this);
    QAction *searchNepomukShow = new QAction(i18n("Search with nepomukshow..."), popup);
    popup->addAction(searchNepomukShow);
    QAction *act = popup->exec( event->globalPos() );
    delete popup;
    if (act == searchNepomukShow) {
        const QString uid = QLatin1String("akonadi:?item=")  + index.data( Qt::DisplayRole ).toString();
        QPointer<PimCommon::SearchDebugNepomukShowDialog> dlg = new PimCommon::SearchDebugNepomukShowDialog(uid, this);
        dlg->exec();
        delete dlg;
    }
}

AkonadiResultListDelegate::AkonadiResultListDelegate( QObject *parent )
    : QStyledItemDelegate ( parent )
{
}

AkonadiResultListDelegate::~AkonadiResultListDelegate()
{
}

QWidget *AkonadiResultListDelegate::createEditor( QWidget *, const QStyleOptionViewItem  &, const QModelIndex & ) const
{
    return 0;
}
