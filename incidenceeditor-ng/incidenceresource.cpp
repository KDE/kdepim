/*
  Copyright (C) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#include "incidenceresource.h"
#include "resourcemanagement.h"
#include "resourcemodel.h"
#include "attendeecomboboxdelegate.h"

#ifdef KDEPIM_MOBILE_UI
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <KDebug>
#include <KDescendantsProxyModel>
#include <KMessageBox>

#include <KCalUtils/Stringify>
#include <KPIMUtils/Email>


#include <QCompleter>
using namespace IncidenceEditorNG;

#ifdef KDEPIM_MOBILE_UI
IncidenceResource::IncidenceResource(Ui::EventOrTodoMore *ui)
#else
IncidenceResource::IncidenceResource(Ui::EventOrTodoDesktop *ui)
#endif
    : IncidenceEditor(0)
    , mUi(ui)
{
    setObjectName("IncidenceResource");

    AttendeeComboBoxDelegate* roleDelegate(new AttendeeComboBoxDelegate(this));
#ifdef KDEPIM_MOBILE_UI
    roleDelegate->addItem(DesktopIcon("meeting-participant", 48),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::ReqParticipant));
    roleDelegate->addItem(DesktopIcon("meeting-participant-optional", 48),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::OptParticipant));
    roleDelegate->addItem(DesktopIcon("meeting-observer", 48),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::NonParticipant));
    roleDelegate->addItem(DesktopIcon("meeting-chair", 48),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::Chair));

#else
    roleDelegate->addItem(SmallIcon("meeting-participant"),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::ReqParticipant));
    roleDelegate->addItem(SmallIcon("meeting-participant-optional"),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::OptParticipant));
    roleDelegate->addItem(SmallIcon("meeting-observer"),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::NonParticipant));
    roleDelegate->addItem(SmallIcon("meeting-chair"),
                          KCalUtils::Stringify::attendeeRole(KCalCore::Attendee::Chair));
#endif

    AttendeeComboBoxDelegate *stateDelegate(new AttendeeComboBoxDelegate(this));

#ifdef KDEPIM_MOBILE_UI
    stateDelegate->addItem(DesktopIcon("task-attention", 48),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::NeedsAction));
    stateDelegate->addItem(DesktopIcon("task-accepted", 48),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Accepted));
    stateDelegate->addItem(DesktopIcon("task-reject", 48),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Declined));
    stateDelegate->addItem(DesktopIcon("task-attempt", 48),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Tentative));
    stateDelegate->addItem(DesktopIcon("task-delegate", 48),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Delegated));
#else
    stateDelegate->addItem(SmallIcon("task-attention"),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::NeedsAction));
    stateDelegate->addItem(SmallIcon("task-accepted"),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Accepted));
    stateDelegate->addItem(SmallIcon("task-reject"),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Declined));
    stateDelegate->addItem(SmallIcon("task-attempt"),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Tentative));
    stateDelegate->addItem(SmallIcon("task-delegate"),
                           KCalUtils::Stringify::attendeeStatus(KCalCore::Attendee::Delegated));
#endif

#ifndef KDEPIM_MOBILE_UI
    QStringList attrs;
    attrs << QLatin1String("cn");

    completer = new QCompleter(this);
    ResourceModel *model = new ResourceModel(attrs, this);

    KDescendantsProxyModel *proxyModel = new KDescendantsProxyModel( this );
    proxyModel->setSourceModel( model );

    completer->setModel(proxyModel);
    completer->setWrapAround(false);
    mUi->mNewResource->setCompleter(completer);

    KCalCore::Attendee::List resources;
    AttendeeLineEditDelegate *attendeeDelegate = new AttendeeLineEditDelegate(this);

    dataModel = new AttendeeTableModel(resources, this);
    ResourceFilterProxyModel *filterProxyModel = new ResourceFilterProxyModel(this);
    filterProxyModel->setDynamicSortFilter(true);
    filterProxyModel->setSourceModel(dataModel);

    mUi->mResourcesTable->setModel(filterProxyModel);
    mUi->mResourcesTable->setColumnHidden(4, true);
    mUi->mResourcesTable->setItemDelegateForColumn(0, roleDelegate);
    mUi->mResourcesTable->setItemDelegateForColumn(1, attendeeDelegate);
    mUi->mResourcesTable->setItemDelegateForColumn(3, stateDelegate);


    AttendeeFilterProxyModel *attendeeProxyModel = new AttendeeFilterProxyModel(this);
    attendeeProxyModel->setDynamicSortFilter(true);
    attendeeProxyModel->setSourceModel(dataModel);
    mUi->mAttendeeTable->setModel(attendeeProxyModel);
    mUi->mAttendeeTable->setItemDelegateForColumn(0, roleDelegate);
    mUi->mAttendeeTable->setItemDelegateForColumn(1, attendeeDelegate);
    mUi->mAttendeeTable->setItemDelegateForColumn(3, stateDelegate);

    connect(mUi->mFindResourcesButton, SIGNAL(clicked()), SLOT(findResources()));
    connect(mUi->mBookResourceButton, SIGNAL(clicked()), SLOT(bookResource()));
    connect(dataModel, SIGNAL(layoutChanged()), SLOT(layoutChanged()));
    connect(dataModel, SIGNAL(rowsInserted(const QModelIndex&, int , int)), SLOT(layoutChanged()));
    connect(dataModel, SIGNAL(rowsRemoved(const QModelIndex&, int , int)), SLOT(layoutChanged()));
#endif
}

void IncidenceResource::load(const KCalCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    dataModel->setAttendees(incidence->attendees());
}

void IncidenceResource::save(const KCalCore::Incidence::Ptr &incidence)
{
    incidence->clearAttendees();
    KCalCore::Attendee::List attendees = dataModel->attendees();

    foreach(KCalCore::Attendee::Ptr attendee, attendees) {
        Q_ASSERT(attendee);

        bool skip = false;
        if (KPIMUtils::isValidAddress(attendee->email())) {
            if (KMessageBox::warningYesNo(
                        0,
                        i18nc("@info",
                              "%1 does not look like a valid email address. "
                              "Are you sure you want to invite this participant?",
                              attendee->email()),
                        i18nc("@title:window", "Invalid Email Address")) != KMessageBox::Yes) {
                skip = true;
            }
        }
        if (!skip) {
            incidence->addAttendee(attendee);
        }
    }

    // Must not have an organizer for items without attendees
    if (!incidence->attendeeCount()) {
        return;
    }
}

bool IncidenceResource::isDirty() const
{
    const KCalCore::Attendee::List originalList = mLoadedIncidence->attendees();
    KCalCore::Attendee::List newList = dataModel->attendees();

    // The lists sizes *must* be the same. When the organizer is attending the
    // event as well, he should be in the attendees list as well.
    if (originalList.size() != newList.size()) {
        return true;
    }

    // Okay, again not the most efficient algorithm, but I'm assuming that in the
    // bulk of the use cases, the number of attendees is not much higher than 10 or so.
    foreach(const KCalCore::Attendee::Ptr & attendee, originalList) {
        bool found = false;
        for (int i = 0; i < newList.count(); ++i) {
            if (newList[i] == attendee) {
                newList.remove(i);
                found = true;
                break;
            }
        }

        if (!found) {
            // One of the attendees in the original list was not found in the new list.
            return true;
        }
    }

    return false;
}

void IncidenceResource::bookResource()
{
#ifndef KDEPIM_MOBILE_UI
    QString name, email;

    KPIMUtils::extractEmailAddressAndName(mUi->mNewResource->text(), email, name);
    KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(name, email));
    attendee->setCuType(KCalCore::Attendee::Resource);
    dataModel->insertAttendee(dataModel->rowCount(), attendee);
#endif
}

void IncidenceResource::findResources()
{
    ResourceManagement* dialog = new ResourceManagement();
    dialog->show();
}

void IncidenceResource::layoutChanged()
{
    emit resourceCountChanged(resourcesCount());
}

int IncidenceResource::resourcesCount() const
{
#ifndef KDEPIM_MOBILE_UI
    return mUi->mResourcesTable->model()->rowCount(QModelIndex());
#endif
    return 0;
}

#include "incidenceresource.moc"
