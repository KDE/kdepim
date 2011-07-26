/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSS_ITEMVIEW_H
#define KRSS_ITEMVIEW_H

#include "krss/krss_export.h"

#include <QtGui/QTreeView>

namespace KRss {

class Item;

class KRSS_EXPORT ItemView : public QTreeView
{
    Q_OBJECT

public:

    explicit ItemView( QWidget *parent = 0 );
    ~ItemView();

Q_SIGNALS:

    void clicked( const KRss::Item &item );
    void activated( const KRss::Item &item );

private Q_SLOTS:

    void slotClicked( const QModelIndex &index );
    void slotActivated( const QModelIndex &index );
};

} // namespace KRss

#endif // KRSS_ITEMVIEW_H
