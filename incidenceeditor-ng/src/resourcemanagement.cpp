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

#include "resourcemanagement.h"
#include "ui_resourcemanagement.h"
#include "resourcemodel.h"
#include "CalendarSupport/FreeBusyItem"
#include "CalendarSupport/FreeBusyCalendar"
#include "ldaputils.h"

#include "freebusyganttproxymodel.h"

#include <EventViews/AgendaView>
#include <EventViews/ViewCalendar>

#include <KCalCore/Event>
#include <KCalCore/MemoryCalendar>

#include <KDGantt2/KDGanttGraphicsView>
#include <KDGantt2/KDGanttView>
#include <KDGantt2/KDGanttDateTimeGrid>
#include <KDGantt2/KDGanttAbstractRowController>

#include <akonadi/calendar/freebusymanager.h>
#include <kldap/ldapobject.h>

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QPointer>
#include <QPushButton>
#include <QSplitter>
#include <QStringList>
#include <QLabel>
#include <QColor>

#include <QDebug>

using namespace IncidenceEditorNG;

class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    virtual ~FreebusyViewCalendar() {}
    bool isValid(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE
    {
        return isValid(incidence->uid());
    }

    bool isValid(const QString &incidenceIdentifier) const Q_DECL_OVERRIDE
    {
        return incidenceIdentifier.startsWith(QStringLiteral("fb-"));
    }

    QString displayName(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE
    {
        Q_UNUSED(incidence);
        return QStringLiteral("Freebusy");
    }

    QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE
    {
        bool ok = false;
        int status = incidence->customProperty(QStringLiteral("FREEBUSY").toLatin1(), QStringLiteral("STATUS").toLatin1()).toInt(&ok);

        if (!ok) {
            return QColor(QStringLiteral("#555"));
        }

        switch (status) {
        case KCalCore::FreeBusyPeriod::Busy:
            return QColor(QStringLiteral("#f00"));
        case KCalCore::FreeBusyPeriod::BusyTentative:
        case KCalCore::FreeBusyPeriod::BusyUnavailable:
            return QColor(QStringLiteral("#f70"));
        case KCalCore::FreeBusyPeriod::Free:
            return QColor(QStringLiteral("#0f0"));
        default:
            return QColor(QStringLiteral("#555"));
        }
    }

    QString iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const Q_DECL_OVERRIDE
    {
        return QString();
    }

    KCalCore::Calendar::Ptr getCalendar() const Q_DECL_OVERRIDE
    {
        return mCalendar;
    }

    KCalCore::Calendar::Ptr mCalendar;
};

ResourceManagement::ResourceManagement(QWidget *parent)
    : QDialog(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18nc("@action:button add resource to attendeelist", "Book resource"));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ResourceManagement::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ResourceManagement::reject);

    mUi = new Ui_resourceManagement;

    QWidget *w = new QWidget(this);
    mUi->setupUi(w);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(w);

    mainLayout->addWidget(buttonBox);

    QVariantList list;
    mModel = new KPIM::FreeBusyItemModel(this);
    mFreebusyCalendar.setModel(mModel);

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false,  false);

    FreebusyViewCalendar *fbCalendar = new FreebusyViewCalendar();
    fbCalendar->mCalendar = mFreebusyCalendar.calendar();
    mFbCalendar = EventViews::ViewCalendar::Ptr(fbCalendar);
    mAgendaView->addCalendar(mFbCalendar);

    mUi->resourceCalender->addWidget(mAgendaView);

    QStringList attrs;
    attrs << QStringLiteral("cn") << QStringLiteral("mail")
          << QStringLiteral("owner") << QStringLiteral("givenname") << QStringLiteral("sn")
          << QStringLiteral("kolabDescAttribute") << QStringLiteral("description");
    ResourceModel *resourcemodel = new ResourceModel(attrs);
    mUi->treeResults->setModel(resourcemodel);

    // This doesn't work till now :(-> that's why i use the click signal
    mUi->treeResults->setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModel = mUi->treeResults->selectionModel();

    connect(mUi->resourceSearch, &KLineEdit::textChanged, this, &ResourceManagement::slotStartSearch);

    connect(mUi->treeResults, &QTreeView::clicked, this, &ResourceManagement::slotShowDetails);

    connect(resourcemodel, &ResourceModel::layoutChanged, this, &ResourceManagement::slotLayoutChanged);
}

ResourceManagement::~ResourceManagement()
{
    delete mModel;
    delete mUi;
}

ResourceItem::Ptr ResourceManagement::selectedItem() const
{
    return mSelectedItem;
}

void ResourceManagement::slotStartSearch(const QString &text)
{
    ((ResourceModel *)mUi->treeResults->model())->startSearch(text);
}

void ResourceManagement::slotShowDetails(const QModelIndex &current)
{
    ResourceItem::Ptr item = current.model()->data(current, ResourceModel::Resource).value<ResourceItem::Ptr>();
    mSelectedItem = item;
    showDetails(item->ldapObject(), item->ldapClient());
}

void ResourceManagement::showDetails(const KLDAP::LdapObject &obj, const KLDAP::LdapClient &client)
{
    // Clean up formDetails
    QLayoutItem *child;
    while ((child = mUi->formDetails->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }
    mUi->groupOwner->setHidden(true);

    // Fill formDetails with data
    for (auto it = obj.attributes().cbegin(), end = obj.attributes().cbegin(); it != end; ++it) {
        const QString &key = it.key();
        if (key ==  QStringLiteral("objectClass") || key == QLatin1String("email")) {
            continue;
        } else if (key ==  QStringLiteral("owner")) {
            QStringList attrs;
            attrs << QStringLiteral("cn") << QStringLiteral("mail")
                  << QStringLiteral("mobile") <<  QStringLiteral("telephoneNumber")
                  << QStringLiteral("kolabDescAttribute") << QStringLiteral("description");
            mOwnerItem = ResourceItem::Ptr(new ResourceItem(KLDAP::LdapDN(QString::fromUtf8(it.value().at(0))),
                                           attrs,  client));
            connect(mOwnerItem.data(),  &ResourceItem::searchFinished, this, &ResourceManagement::slotOwnerSearchFinished);
            mOwnerItem->startSearch();
            continue;
        }
        QStringList list;
        const QList<QByteArray> values = it.value();
        list.reserve(values.count());
        foreach (const QByteArray &value, values) {
            list << QString::fromUtf8(value);
        }
        mUi->formDetails->addRow(translateLDAPAttributeForDisplay(key), new QLabel(list.join(QStringLiteral("\n"))));
    }

    QString name = QString::fromUtf8(obj.attributes().value(QStringLiteral("cn"))[0]);
    QString email = QString::fromUtf8(obj.attributes().value(QStringLiteral("mail"))[0]);
    KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(name,  email));
    KPIM::FreeBusyItem::Ptr freebusy(new KPIM::FreeBusyItem(attendee, this));
    mModel->clear();
    mModel->addItem(freebusy);
}

void ResourceManagement::slotLayoutChanged()
{
    const int columnCount = mUi->treeResults->model()->columnCount(QModelIndex());
    for (int i = 1; i < columnCount; ++i) {
        mUi->treeResults->setColumnHidden(i, true);
    }
}

void ResourceManagement::slotOwnerSearchFinished()
{
    // Clean up formDetails
    QLayoutItem *child;
    while ((child = mUi->formOwner->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }
    mUi->groupOwner->setHidden(false);

    const KLDAP::LdapObject &obj = mOwnerItem->ldapObject();
    const KLDAP::LdapAttrMap &ldapAttrMap = obj.attributes();
    for (auto it = ldapAttrMap.cbegin(), end = ldapAttrMap.cend(); it != end; ++it) {
        const QString &key = it.key();
        if (key ==  QStringLiteral("objectClass")
                || key ==  QStringLiteral("owner")
                || key ==  QStringLiteral("givenname")
                || key ==  QStringLiteral("sn")) {
            continue;
        }
        QStringList list;
        const QList<QByteArray> values = it.value();
        list.reserve(values.count());
        foreach (const QByteArray &value, values) {
            list << QString::fromUtf8(value);
        }
        mUi->formOwner->addRow(translateLDAPAttributeForDisplay(key), new QLabel(list.join(QStringLiteral("\n"))));
    }
}

void ResourceManagement::slotDateChanged(const QDate &start, const QDate &end)
{
    if (start.daysTo(end) < 7) {
        mAgendaView->showDates(start, start.addDays(7));
    }
    mAgendaView->showDates(start, end);
}
