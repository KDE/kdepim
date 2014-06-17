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

#include <KPIMUtils/Email>

#include <QCompleter>
using namespace IncidenceEditorNG;

#ifdef KDEPIM_MOBILE_UI
IncidenceResource::IncidenceResource(IncidenceAttendee* ieAttendee, Ui::EventOrTodoMore *ui)
#else
IncidenceResource::IncidenceResource(IncidenceAttendee* ieAttendee, Ui::EventOrTodoDesktop *ui)
#endif
    : IncidenceEditor(0)
    , mUi(ui)
    , dataModel(ieAttendee->dataModel())
{
    setObjectName("IncidenceResource");


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

    AttendeeLineEditDelegate *attendeeDelegate = new AttendeeLineEditDelegate(this);

    ResourceFilterProxyModel *filterProxyModel = new ResourceFilterProxyModel(this);
    filterProxyModel->setDynamicSortFilter(true);
    filterProxyModel->setSourceModel(dataModel);

    QHeaderView* headerView = mUi->mResourcesTable->horizontalHeader();
    headerView->setResizeMode(QHeaderView::ResizeToContents);

    mUi->mResourcesTable->setModel(filterProxyModel);
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Role, ieAttendee->roleDelegate());
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::FullName, attendeeDelegate);
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Status, ieAttendee->stateDelegate());
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Response, ieAttendee->responseDelegate());

    connect(mUi->mFindResourcesButton, SIGNAL(clicked()), SLOT(findResources()));
    connect(mUi->mBookResourceButton, SIGNAL(clicked()), SLOT(bookResource()));
    connect(dataModel, SIGNAL(layoutChanged()), SLOT(layoutChanged()));
    connect(dataModel, SIGNAL(rowsInserted(const QModelIndex&, int , int)), SLOT(updateCount()));
    connect(dataModel, SIGNAL(rowsRemoved(const QModelIndex&, int , int)), SLOT(updateCount()));
#endif
}

void IncidenceResource::load(const KCalCore::Incidence::Ptr &incidence)
{
  //all logic inside IncidenceAtendee (using same model)
}

void IncidenceResource::save(const KCalCore::Incidence::Ptr &incidence)
{
  //all logic inside IncidenceAtendee (using same model)
}

bool IncidenceResource::isDirty() const
{
  //all logic inside IncidenceAtendee (using same model)
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

#ifndef KDEPIM_MOBILE_UI
    mUi->mResourcesTable->setColumnHidden(AttendeeTableModel::CuType, true);
    mUi->mResourcesTable->setColumnHidden(AttendeeTableModel::Name, true);
    mUi->mResourcesTable->setColumnHidden(AttendeeTableModel::Email, true);
#endif

    updateCount();
}


void IncidenceResource::updateCount()
{
    emit resourceCountChanged(resourceCount());
}

int IncidenceResource::resourceCount() const
{
#ifndef KDEPIM_MOBILE_UI
    return mUi->mResourcesTable->model()->rowCount(QModelIndex());
#endif
    return 0;
}

#include "incidenceresource.moc"
