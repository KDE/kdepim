/* -*- mode: c++; c-basic-offset:4 -*-
    view/tabwidget.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEOPATRA_VIEW_TABWIDGET_H__
#define __KLEOPATRA_VIEW_TABWIDGET_H__

#include <QWidget>

#include <vector>

#include <utils/pimpl_ptr.h>

#include <boost/shared_ptr.hpp>

class QAbstractItemView;

class KConfigGroup;
class KActionCollection;
class KConfig;

namespace Kleo
{

class AbstractKeyListModel;
class AbstractKeyListSortFilterProxyModel;
class KeyFilter;

class TabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~TabWidget();

    void setFlatModel(AbstractKeyListModel *model);
    AbstractKeyListModel *flatModel() const;
    void setHierarchicalModel(AbstractKeyListModel *model);
    AbstractKeyListModel *hierarchicalModel() const;

    QAbstractItemView *addView(const QString &title = QString(), const QString &keyFilterID = QString(), const QString &searchString = QString());
    QAbstractItemView *addView(const KConfigGroup &group);
    QAbstractItemView *addTemporaryView(const QString &title = QString(), AbstractKeyListSortFilterProxyModel *proxy = 0, const QString &tabToolTip = QString());

    void loadViews(const KConfig *cfg);
    void saveViews(KConfig *cfg) const;

    std::vector<QAbstractItemView *> views() const;
    QAbstractItemView *currentView() const;

    unsigned int count() const;

    void createActions(KActionCollection *collection);
    void connectSearchBar(QObject *sb);

    void setMultiSelection(bool on);

public Q_SLOTS:
    void setKeyFilter(const boost::shared_ptr<Kleo::KeyFilter> &filter);
    void setStringFilter(const QString &filter);

Q_SIGNALS:
    void viewAdded(QAbstractItemView *view);
    void viewAboutToBeRemoved(QAbstractItemView *view);

    void currentViewChanged(QAbstractItemView *view);
    void stringFilterChanged(const QString &filter);
    void keyFilterChanged(const boost::shared_ptr<Kleo::KeyFilter> &filter);

    void enableChangeStringFilter(bool enable);
    void enableChangeKeyFilter(bool enable);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;

    Q_PRIVATE_SLOT(d, void slotContextMenu(QWidget *, const QPoint &))
    Q_PRIVATE_SLOT(d, void slotContextMenu(const QPoint &))
    Q_PRIVATE_SLOT(d, void currentIndexChanged(int))
    Q_PRIVATE_SLOT(d, void slotPageTitleChanged(const QString &))
    Q_PRIVATE_SLOT(d, void slotPageKeyFilterChanged(const boost::shared_ptr<Kleo::KeyFilter> &))
    Q_PRIVATE_SLOT(d, void slotPageStringFilterChanged(const QString &))
    Q_PRIVATE_SLOT(d, void slotPageHierarchyChanged(bool))
    Q_PRIVATE_SLOT(d, void slotRenameCurrentTab())
    Q_PRIVATE_SLOT(d, void slotNewTab())
    Q_PRIVATE_SLOT(d, void slotDuplicateCurrentTab())
    Q_PRIVATE_SLOT(d, void slotCloseCurrentTab())
    Q_PRIVATE_SLOT(d, void slotMoveCurrentTabLeft())
    Q_PRIVATE_SLOT(d, void slotMoveCurrentTabRight())
    Q_PRIVATE_SLOT(d, void slotToggleHierarchicalView(bool))
    Q_PRIVATE_SLOT(d, void slotExpandAll())
    Q_PRIVATE_SLOT(d, void slotCollapseAll())
};

}

#endif /* __KLEOPATRA_VIEW_TABWIDGET_H__ */
