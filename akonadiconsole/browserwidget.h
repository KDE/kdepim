/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <ui_browserwidget_itemview.h>
#include <ui_browserwidget_contentview.h>

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/etmviewstatesaver.h>

#include <QtGui/QWidget>

class QModelIndex;
class QItemSelectionModel;
class QStandardItemModel;

class KJob;
class KXmlGuiWindow;

class AkonadiBrowserModel;

template <typename T> class KViewStateMaintainer;

namespace Akonadi {
class EntityTreeView;
class Job;
class StandardActionManager;
class StatisticsProxyModel;
class Monitor;
}

class BrowserWidget: public QWidget
{
  Q_OBJECT

  public:
    explicit BrowserWidget( KXmlGuiWindow *xmlGuiWindow, QWidget *parent = 0 );
    ~BrowserWidget();

  public slots:
    void dumpToXml();

  private slots:
    void itemActivated( const QModelIndex &index );
    void itemFetchDone( KJob *job );
    void modelChanged();
    void save();
    void saveResult( KJob* job );
    void addAttribute();
    void delAttribute();
    void setItem( const Akonadi::Item &item );
    void dumpToXmlResult( KJob *job );
    void clear();

  private:
    AkonadiBrowserModel *mBrowserModel;
    Akonadi::EntityTreeView *mCollectionView;
    Akonadi::StatisticsProxyModel *statisticsProxyModel;
    Ui::ItemViewWidget itemUi;
    Ui::ContentViewWidget contentUi;
    Akonadi::Item mCurrentItem;
    QStandardItemModel *mAttrModel;
    QStandardItemModel *mNepomukModel;
    Akonadi::StandardActionManager *mStdActionManager;
    Akonadi::Monitor *mMonitor;
    KViewStateMaintainer<Akonadi::ETMViewStateSaver> *m_stateMaintainer;
};

#endif
