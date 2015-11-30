/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef RESOURCEMANAGEMENT_H
#define RESOURCEMANAGEMENT_H

#include "incidenceeditors_ng_export.h"

#include <Libkdepim/LdapClient>
#include <Libkdepim/LdapClientSearch>

#include "CalendarSupport/FreeBusyCalendar"
#include "resourceitem.h"

#include <EventViews/ViewCalendar>

#include <QDialog>
#include <QStringList>
#include <QStringListModel>

class  Ui_resourceManagement;

class QItemSelectionModel;

namespace EventViews
{
class AgendaView;
}

namespace IncidenceEditorNG
{

class INCIDENCEEDITORS_NG_EXPORT ResourceManagement : public QDialog
{
    Q_OBJECT
public:
    explicit ResourceManagement(QWidget *parent = Q_NULLPTR);
    ~ResourceManagement();

    ResourceItem::Ptr selectedItem() const;

public Q_SLOTS:
    void slotDateChanged(const QDate &start, const QDate &end);

private:
    /* Shows the details of a resource
     *
     */
    void showDetails(const KLDAP::LdapObject &, const KLDAP::LdapClient &client);

    QItemSelectionModel *selectionModel;

private Q_SLOTS:
    /* A new searchString is entered
     *
     */
    void slotStartSearch(const QString &);

    /* A detail view is requested
     *
     */
    void slotShowDetails(const QModelIndex &current);

    /**
     * The Owner search is done
     */
    void slotOwnerSearchFinished();

    void slotLayoutChanged();

private:
    KPIM::FreeBusyItemModel *mModel;
    KPIM::FreeBusyCalendar mFreebusyCalendar;
    ResourceItem::Ptr mOwnerItem;
    ResourceItem::Ptr mSelectedItem;
    EventViews::ViewCalendar::Ptr mFbCalendar;
    Ui_resourceManagement *mUi;
    QMap<QModelIndex, KCalCore::Event::Ptr> mFbEvent;
    EventViews::AgendaView *mAgendaView;
};

}
#endif // RESOURCEMANAGEMENT_H
