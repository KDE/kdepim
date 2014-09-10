/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef RESOURCEMANAGEMENT_H
#define RESOURCEMANAGEMENT_H

#include "incidenceeditors-ng_export.h"

#include <ldap/ldapclient.h>
#include <ldap/ldapclientsearch.h>

#include "freebusyitemmodel.h"
#include "resourceitem.h"

#include <calendarviews/agenda/viewcalendar.h>

#include <KCalCore/FreeBusy>
#include <KDialog>

#include <QStringList>
#include <QStringListModel>

class  Ui_resourceManagement;

namespace EventViews
{
    class AgendaView;
}

namespace IncidenceEditorNG
{

class QTreeModel;

class INCIDENCEEDITORS_NG_EXPORT ResourceManagement : public KDialog
{
    Q_OBJECT
public:
    ResourceManagement();
    ~ResourceManagement();

    ResourceItem::Ptr selectedItem() const;

public slots:
    void slotDateChanged(QDate start, QDate end);

private:
    /* Shows the details of a resource
     *
     */
    void showDetails(const KLDAP::LdapObject&, const KLDAP::LdapClient &client);

    QItemSelectionModel *selectionModel;

private slots:
    /* A new searchString is entered
     *
     */
    void slotStartSearch(const QString&);

    /* A detail view is requested
     *
     */
    void slotShowDetails(const QModelIndex & current);

    /**
     * The Owner search is done
     */
    void slotOwnerSearchFinished();

    void slotLayoutChanged();

    void slotFbModelLayoutChanged();
    void slotFbModelRowsRemoved(const QModelIndex &parent, int first, int last);
    void slotFbModelRowsAdded(const QModelIndex &parent, int first, int last);
    void slotFbModelRowsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    FreeBusyItemModel *mModel;
    ResourceItem::Ptr mOwnerItem;
    ResourceItem::Ptr mSelectedItem;
    EventViews::ViewCalendar::Ptr mFbCalendar;
    Ui_resourceManagement *mUi;
    QMap<QModelIndex,KCalCore::Event::Ptr> mFbEvent;
    EventViews::AgendaView *mAgendaView;
};

}
#endif // RESOURCEMANAGEMENT_H
