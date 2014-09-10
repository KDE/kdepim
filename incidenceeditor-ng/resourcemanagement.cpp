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

//http://stackoverflow.com/questions/18831242/qt-start-editing-of-cell-after-one-click

#include "resourcemanagement.h"
#include "ui_resourcemanagement.h"
#include "resourcemodel.h"
#include "freebusyitem.h"
#include "ldaputils.h"

#include "freebusyganttproxymodel.h"

#include <calendarviews/agenda/agendaview.h>
#include <calendarviews/agenda/viewcalendar.h>

#include <KCalCore/Event>
#include <KCalCore/MemoryCalendar>

#include <kdgantt2/kdganttgraphicsview.h>
#include <kdgantt2/kdganttview.h>
#include <kdgantt2/kdganttdatetimegrid.h>
#include <kdgantt2/kdganttabstractrowcontroller.h>

#include <akonadi/calendar/freebusymanager.h>
#include <kldap/ldapobject.h>

#include <qjson/parser.h>

#include <QPointer>
#include <QSplitter>
#include <QStringList>
#include <QLabel>
#include <QColor>

#include <KDebug>
#include <KSystemTimeZones>

using namespace IncidenceEditorNG;

enum FbStatus {
    Unkown,
    Free,
    Busy,
    Tentative
};

class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    virtual ~FreebusyViewCalendar() {};
    virtual bool isValid(const KCalCore::Incidence::Ptr &incidence) const
    {
        return incidence->uid().startsWith("fb-");
    }

    virtual QString displayName(const KCalCore::Incidence::Ptr &incidence) const
    {
        Q_UNUSED(incidence);
        return QLatin1String("Freebusy");
    }

    virtual QColor resourceColor(const KCalCore::Incidence::Ptr &incidence) const
    {
        bool ok = false;
        int status = incidence->customProperty("FREEBUSY", "STATUS").toInt(&ok);

        if (!ok) {
            return QColor("#555");
        }

        switch (status) {
        case Busy:
            return QColor("#f00");
        case Tentative:
            return QColor("#f70");
        case Free:
            return QColor("#0f0");
        default:
            return QColor("#555");
        }
    }

    virtual QString iconForIncidence(const KCalCore::Incidence::Ptr &incidence) const
    {
        return QString();
    }

    virtual KCalCore::Calendar::Ptr getCalendar() const
    {
        return mCalendar;
    }

    KCalCore::Calendar::Ptr mCalendar;
};

ResourceManagement::ResourceManagement()
{
    setButtonText(KDialog::Ok, i18nc("@action:button add resource to attendeelist", "Book resource"));

    mUi = new Ui_resourceManagement;

    QWidget *w = new QWidget( this );
    mUi->setupUi( w );
    setMainWidget( w );

    QVariantList list;
    mModel = new FreeBusyItemModel(this);

    connect(mModel, SIGNAL(layoutChanged()), SLOT(slotFbModelLayoutChanged()));
    connect(mModel, SIGNAL(modelReset()), SLOT(slotFbModelLayoutChanged()));
    connect(mModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                    SLOT(slotFbModelRowsRemoved(QModelIndex,int,int)));
    connect(mModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                    SLOT(slotFbModelRowsAdded(QModelIndex,int,int)));
    connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    SLOT(slotFbModelRowsChanged(QModelIndex,QModelIndex)));

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false,  false);

    KCalCore::Calendar::Ptr cal = KCalCore::Calendar::Ptr(new KCalCore::MemoryCalendar(KSystemTimeZones::local()));
    FreebusyViewCalendar *fbCalendar = new FreebusyViewCalendar();
    fbCalendar->mCalendar = cal;
    mFbCalendar = EventViews::ViewCalendar::Ptr(fbCalendar);
    mAgendaView->addCalendar(mFbCalendar);

    mUi->resourceCalender->addWidget( mAgendaView );

    QStringList attrs;
    attrs << QLatin1String("cn") << QLatin1String("mail")
          << QLatin1String("owner") << QLatin1String("givenname") << QLatin1String("sn")
          << QLatin1String("kolabDescAttribute") << QLatin1String("description");
    ResourceModel *resourcemodel = new ResourceModel(attrs);
    mUi->treeResults->setModel(resourcemodel);

    // This doesn't work till now :( -> that's why i use the click signal
    mUi->treeResults->setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModel = mUi->treeResults->selectionModel();

    connect(mUi->resourceSearch, SIGNAL(textChanged(const QString&)),
            SLOT(slotStartSearch(const QString&)));

    connect(mUi->treeResults, SIGNAL(clicked(const QModelIndex &)),
            SLOT(slotShowDetails(const QModelIndex &)));

    connect(resourcemodel,SIGNAL(layoutChanged()),SLOT(slotLayoutChanged()));
}

ResourceManagement::~ResourceManagement()
{
    delete mModel;
}


ResourceItem::Ptr ResourceManagement::selectedItem() const
{
    return mSelectedItem;
}

void ResourceManagement::slotStartSearch(const QString &text)
{
    ((ResourceModel*)mUi->treeResults->model())->startSearch(text);
}

void ResourceManagement::slotShowDetails(const QModelIndex & current)
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
    foreach(const QString & key, obj.attributes().keys()) {
        if (key ==  QLatin1String("objectClass")
            || key == QLatin1String("email")) {
            continue;
        } else if (key ==  QLatin1String("owner")) {
            QStringList attrs;
            attrs << QLatin1String("cn") << QLatin1String("mail")
                  << QLatin1String("mobile") <<  QLatin1String("telephoneNumber")
                  << QLatin1String("kolabDescAttribute") << QLatin1String("description");
            mOwnerItem = ResourceItem::Ptr(new ResourceItem(KLDAP::LdapDN(QString::fromUtf8(obj.attributes().value(key).at(0))),
                                                    attrs,  client));
            connect(mOwnerItem.data(),  SIGNAL(searchFinished()), SLOT(slotOwnerSearchFinished()));
            mOwnerItem->startSearch();
            continue;
        }
        QStringList list;
        foreach(const QByteArray & value, obj.attributes().value(key)) {
            list << QString::fromUtf8(value);
        }
        if (key ==  QLatin1String("kolabDescAttribute")) {
            QJson::Parser parser;
            foreach(const QString &attr,  list) {
                bool ok;
                QMap <QString, QVariant > map = parser.parse(attr.toUtf8(), &ok).toMap();
                foreach(const QString &pKey, map.keys()) {
                    QString value;
                    if (map.value(pKey).type() ==  QVariant::Bool) {
                        value = map.value(pKey).toBool()  ?  i18n("yes") : i18n("no");
                    } else {
                        value = map.value(pKey).toString();
                    }
                    mUi->formDetails->addRow(translateKolabLDAPAttributeForDisplay(pKey), new QLabel(value));
                 }
            }
        } else {
            mUi->formDetails->addRow(translateLDAPAttributeForDisplay(key), new QLabel(list.join("\n")));
        }
    }

    QString name = QString::fromUtf8(obj.attributes().value("cn")[0]);
    QString email = QString::fromUtf8(obj.attributes().value("mail")[0]);
    KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(name,  email));
    FreeBusyItem::Ptr freebusy( new FreeBusyItem( attendee, this ));
    mModel->clear();
    mModel->addItem(freebusy);


}

void ResourceManagement::slotLayoutChanged()
{
    for(int i = 1; i < mUi->treeResults->model()->columnCount(QModelIndex());i++) {
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
    foreach(const QString & key, obj.attributes().keys()) {
        if (key ==  QLatin1String("objectClass")
            || key ==  QLatin1String("owner")
            || key ==  QLatin1String("givenname")
            || key ==  QLatin1String("sn")) {
            continue;
        }
        QStringList list;
        foreach(const QByteArray & value, obj.attributes().value(key)) {
            list << QString::fromUtf8(value);
          }
        if (key ==  QLatin1String("kolabDescAttribute")) {
            QJson::Parser parser;
            foreach(const QString &attr,  list) {
                bool ok;
                QMap <QString, QVariant > map = parser.parse(attr.toUtf8(), &ok).toMap();
                foreach(const QString &pKey, map.keys()) {
                    QString value;
                    if (map.value(pKey).type() ==  QVariant::Bool) {
                        value = map.value(pKey).toBool()  ?  i18n("yes") : i18n("no");
                      } else {
                          value = map.value(pKey).toString();
                        }
                    mUi->formOwner->addRow(translateKolabLDAPAttributeForDisplay(pKey), new QLabel(value));
                  }
              }
          } else {
              mUi->formOwner->addRow(translateLDAPAttributeForDisplay(key), new QLabel(list.join("\n")));
          }
      }

}

void ResourceManagement::slotDateChanged(QDate start, QDate end)
{
    int days = start.daysTo(end);
    if (days < 7) {
        end = start.addDays(7);
    }
    mAgendaView->showDates(start, end);
}

void ResourceManagement::slotFbModelLayoutChanged()
{
    if (mFbEvent.count() > 0) {
        mFbCalendar->getCalendar()->deleteAllEvents();
        mFbEvent.clear();
        for (int i = mModel->rowCount()-1; i>=0; i--) {
            QModelIndex parent = mModel->index(i, 0);
            slotFbModelRowsAdded(parent, 0, mModel->rowCount(parent)-1);
        }
    }
}

void ResourceManagement::slotFbModelRowsAdded(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        return;
    }
    for(int i=first; i<=last; i++) {
        QModelIndex index = mModel->index(i, 0, parent);

        const KCalCore::FreeBusyPeriod &period = mModel->data(index, FreeBusyItemModel::FreeBusyPeriodRole).value<KCalCore::FreeBusyPeriod>();
        const KCalCore::Attendee::Ptr &attendee = mModel->data(parent, FreeBusyItemModel::AttendeeRole).value<KCalCore::Attendee::Ptr>();
        const KCalCore::FreeBusy::Ptr &fb = mModel->data(parent, FreeBusyItemModel::FreeBusyRole).value<KCalCore::FreeBusy::Ptr>();

        KCalCore::Event::Ptr inc = KCalCore::Event::Ptr(new KCalCore::Event());
        inc->setDtStart(period.start());
        inc->setDtEnd(period.end());
        inc->setUid(QLatin1String("fb-") + fb->uid());
        //TODO: set to correct status if it is added to KCalCore
        inc->setCustomProperty("FREEBUSY", "STATUS", QString::number(Busy));
        inc->setSummary(period.summary().isEmpty()? i18n("Busy") : period.summary());

        mFbEvent.insert(index, inc);
        mFbCalendar->getCalendar()->addEvent(inc);

    }
}

void ResourceManagement::slotFbModelRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        for (int i = first; i<=last; i--) {
            QModelIndex index = mModel->index(i, 0);
            slotFbModelRowsRemoved(index, 0, mModel->rowCount(index)-1);
        }
    } else {
        for(int i=first; i<=last; i++) {
            QModelIndex index = mModel->index(i, 0, parent);
            KCalCore::Event::Ptr inc = mFbEvent.take(index);
            mFbCalendar->getCalendar()->deleteEvent(inc);
        }
    }
}

void ResourceManagement::slotFbModelRowsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!topLeft.parent().isValid()) {
        return;
    }
    for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = mModel->index(i, 0, topLeft.parent());
        KCalCore::Event::Ptr inc = mFbEvent.value(index);
        mFbCalendar->getCalendar()->beginChange(inc);
        mFbCalendar->getCalendar()->endChange(inc);
    }
}




#include "resourcemanagement.moc"
