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

#include "incidenceresource.h"
#include "resourcemanagement.h"
#include "resourcemodel.h"
#include "attendeecomboboxdelegate.h"
#include "attendeelineeditdelegate.h"
#include "incidencedatetime.h"

#include "ui_dialogdesktop.h"

#include <KDescendantsProxyModel>
#include <KEmailAddress>
#include <KMessageBox>
#include <KGlobalSettings>

#include <QCompleter>
#include <QDebug>

using namespace IncidenceEditorNG;

class SwitchRoleProxy: public QSortFilterProxyModel
{

public:
    explicit SwitchRoleProxy(QObject *parent = Q_NULLPTR)
        : QSortFilterProxyModel(parent)
    {

    }

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE
    {
        QVariant d;
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            d = QSortFilterProxyModel::data(index, ResourceModel::FullName);
            return d;
        }
        d = QSortFilterProxyModel::data(index, role);
        return d;
    }
};

IncidenceResource::IncidenceResource(IncidenceAttendee *ieAttendee, IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(0)
    , mUi(ui)
    , dataModel(ieAttendee->dataModel())
    , mDateTime(dateTime)
    , resourceDialog(new ResourceManagement())
{
    setObjectName(QStringLiteral("IncidenceResource"));
    connect(resourceDialog, &ResourceManagement::accepted, this, &IncidenceResource::dialogOkPressed);

    connect(mDateTime, &IncidenceDateTime::startDateChanged, this, &IncidenceResource::slotDateChanged);
    connect(mDateTime, &IncidenceDateTime::endDateChanged, this, &IncidenceResource::slotDateChanged);

    QStringList attrs;
    attrs << QStringLiteral("cn") <<  QStringLiteral("mail");

    completer = new QCompleter(this);
    ResourceModel *model = new ResourceModel(attrs, this);

    KDescendantsProxyModel *proxyModel = new KDescendantsProxyModel(this);
    proxyModel->setSourceModel(model);
    SwitchRoleProxy *proxyModel2 = new SwitchRoleProxy(this);
    proxyModel2->setSourceModel(proxyModel);

    completer->setModel(proxyModel2);
    completer->setCompletionRole(ResourceModel::FullName);
    completer->setWrapAround(false);
    mUi->mNewResource->setCompleter(completer);

    AttendeeLineEditDelegate *attendeeDelegate = new AttendeeLineEditDelegate(this);
    attendeeDelegate->setCompletionMode((KCompletion::CompletionMode)KGlobalSettings::self()->completionMode());

    ResourceFilterProxyModel *filterProxyModel = new ResourceFilterProxyModel(this);
    filterProxyModel->setDynamicSortFilter(true);
    filterProxyModel->setSourceModel(dataModel);

    mUi->mResourcesTable->setModel(filterProxyModel);
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Role, ieAttendee->roleDelegate());
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::FullName, attendeeDelegate);
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Status, ieAttendee->stateDelegate());
    mUi->mResourcesTable->setItemDelegateForColumn(AttendeeTableModel::Response, ieAttendee->responseDelegate());

    connect(mUi->mFindResourcesButton, &QPushButton::clicked, this, &IncidenceResource::findResources);
    connect(mUi->mBookResourceButton, &QPushButton::clicked, this, &IncidenceResource::bookResource);
    connect(filterProxyModel, &ResourceFilterProxyModel::layoutChanged, this, &IncidenceResource::layoutChanged);
    connect(filterProxyModel, &ResourceFilterProxyModel::layoutChanged, this, &IncidenceResource::updateCount);
    connect(filterProxyModel, &ResourceFilterProxyModel::rowsInserted, this, &IncidenceResource::updateCount);
    connect(filterProxyModel, &ResourceFilterProxyModel::rowsRemoved, this, &IncidenceResource::updateCount);
    // only update when FullName is changed
    connect(filterProxyModel, &ResourceFilterProxyModel::dataChanged, this, &IncidenceResource::updateCount);
}

void IncidenceResource::load(const KCalCore::Incidence::Ptr &incidence)
{
    slotDateChanged();
}

void IncidenceResource::slotDateChanged()
{
    resourceDialog->slotDateChanged(mDateTime->startDate(), mDateTime->endDate());
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
    if (mUi->mNewResource->text().trimmed().isEmpty()) {
        return;
    }
    QString name, email;
    KEmailAddress::extractEmailAddressAndName(mUi->mNewResource->text(), email, name);
    KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(name, email));
    attendee->setCuType(KCalCore::Attendee::Resource);
    dataModel->insertAttendee(dataModel->rowCount(), attendee);
}

void IncidenceResource::findResources()
{
    resourceDialog->show();
}

void IncidenceResource::dialogOkPressed()
{
    ResourceItem::Ptr item = resourceDialog->selectedItem();
    if (item) {
        const QString name = item->ldapObject().value(QStringLiteral("cn"));
        const QString email = item->ldapObject().value(QStringLiteral("mail"));
        KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(name, email));
        attendee->setCuType(KCalCore::Attendee::Resource);
        dataModel->insertAttendee(dataModel->rowCount(), attendee);
    }
}

void IncidenceResource::layoutChanged()
{
    QHeaderView *headerView = mUi->mResourcesTable->horizontalHeader();
    headerView->setSectionHidden(AttendeeTableModel::CuType, true);
    headerView->setSectionHidden(AttendeeTableModel::Name, true);
    headerView->setSectionHidden(AttendeeTableModel::Email, true);
    headerView->setResizeMode(AttendeeTableModel::Role,  QHeaderView::ResizeToContents);
    headerView->setResizeMode(AttendeeTableModel::FullName, QHeaderView::Stretch);
    headerView->setResizeMode(AttendeeTableModel::Available,  QHeaderView::ResizeToContents);
    headerView->setResizeMode(AttendeeTableModel::Status,  QHeaderView::ResizeToContents);
    headerView->setResizeMode(AttendeeTableModel::Response,  QHeaderView::ResizeToContents);
}

void IncidenceResource::updateCount()
{
    Q_EMIT resourceCountChanged(resourceCount());
}

int IncidenceResource::resourceCount() const
{
    int c = 0;
    QModelIndex index;
    QAbstractItemModel *model = mUi->mResourcesTable->model();
    if (!model) {
        return 0;
    }
    const int nbRow = model->rowCount(QModelIndex());
    for (int i = 0; i < nbRow; ++i) {
        index = model->index(i, AttendeeTableModel::FullName);
        if (!model->data(index).toString().isEmpty()) {
            ++c;
        }
    }
    return c;

}

