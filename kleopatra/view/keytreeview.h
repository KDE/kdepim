/* -*- mode: c++; c-basic-offset:4 -*-
    view/keytreeview.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_VIEW_KEYTREEVIEW_H__
#define __KLEOPATRA_VIEW_KEYTREEVIEW_H__

#include <QWidget>

#include <QString>

#include <gpgme++/key.h>

#include <boost/shared_ptr.hpp>
#include <vector>

class QTreeView;

namespace Kleo
{

class KeyFilter;
class AbstractKeyListModel;
class AbstractKeyListSortFilterProxyModel;
class KeyListSortFilterProxyModel;

class KeyTreeView : public QWidget
{
    Q_OBJECT
public:
    explicit KeyTreeView(QWidget *parent = 0);
    KeyTreeView(const QString &stringFilter, const boost::shared_ptr<KeyFilter> &keyFilter,
                AbstractKeyListSortFilterProxyModel *additionalProxy, QWidget *parent);
    ~KeyTreeView();

    QTreeView *view() const
    {
        return m_view;
    }

    AbstractKeyListModel *model() const
    {
        return m_isHierarchical ? hierarchicalModel() : flatModel() ;
    }

    AbstractKeyListModel *flatModel() const
    {
        return m_flatModel;
    }
    AbstractKeyListModel *hierarchicalModel() const
    {
        return m_hierarchicalModel;
    }

    void setFlatModel(AbstractKeyListModel *model);
    void setHierarchicalModel(AbstractKeyListModel *model);

    void setKeys(const std::vector<GpgME::Key> &keys);
    const std::vector<GpgME::Key> &keys() const
    {
        return m_keys;
    }

    void selectKeys(const std::vector<GpgME::Key> &keys);
    std::vector<GpgME::Key> selectedKeys() const;

    void addKeysUnselected(const std::vector<GpgME::Key> &keys);
    void addKeysSelected(const std::vector<GpgME::Key> &keys);
    void removeKeys(const std::vector<GpgME::Key> &keys);

#if 0
    void setToolTipOptions(int options);
    int toolTipOptions() const;
#endif

    QString stringFilter() const
    {
        return m_stringFilter;
    }
    const boost::shared_ptr<KeyFilter> &keyFilter() const
    {
        return m_keyFilter;
    }
    bool isHierarchicalView() const
    {
        return m_isHierarchical;
    }

    void setColumnSizes(const std::vector<int> &sizes);
    std::vector<int> columnSizes() const;

    void setSortColumn(int sortColumn, Qt::SortOrder sortOrder);
    int sortColumn() const;
    Qt::SortOrder sortOrder() const;

    virtual KeyTreeView *clone() const
    {
        return new KeyTreeView(*this);
    }

    void disconnectSearchBar(const QObject *bar);
    bool connectSearchBar(const QObject *bar);

public Q_SLOTS:
    virtual void setStringFilter(const QString &text);
    virtual void setKeyFilter(const boost::shared_ptr<Kleo::KeyFilter> &filter);
    virtual void setHierarchicalView(bool on);

Q_SIGNALS:
    void stringFilterChanged(const QString &filter);
    void keyFilterChanged(const boost::shared_ptr<Kleo::KeyFilter> &filter);
    void hierarchicalChanged(bool on);

protected:
    KeyTreeView(const KeyTreeView &);

private:
    void init();
    void addKeysImpl(const std::vector<GpgME::Key> &, bool);

private:
    std::vector<GpgME::Key> m_keys;

    KeyListSortFilterProxyModel *m_proxy;
    AbstractKeyListSortFilterProxyModel *m_additionalProxy;

    QTreeView *m_view;

    AbstractKeyListModel *m_flatModel;
    AbstractKeyListModel *m_hierarchicalModel;

    QString m_stringFilter;
    boost::shared_ptr<KeyFilter> m_keyFilter;

    bool m_isHierarchical : 1;
};

}

#endif // __KLEOPATRA_VIEW_KEYTREEVIEW_H__
