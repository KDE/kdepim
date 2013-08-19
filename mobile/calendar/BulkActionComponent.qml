/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

import QtQuick 1.1
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.BulkActionScreen {
  id : bulkActionScreen
  anchors.fill: parent
  backgroundImage : backgroundImage.source

  actionListWidth : 1/3 * parent.width
  multipleText : KDE.i18np("1 calendar", "%1 calendars", collectionView.numSelected)
  selectedItemModel : _breadcrumbNavigationFactory.qmlSelectedItemModel();
  headerList : EventListView {
    showCheckBox : true
    id: bulkActionHeaderList
    model: itemModel
    checkModel : _itemActionModel
    anchors.fill : parent
    itemHeight: bulkActionScreen.itemHeight
  }
}
