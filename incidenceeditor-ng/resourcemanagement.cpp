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
#include "freebusymodel/freebusyitem.h"
#include "freebusymodel/freebusycalendar.h"
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

using namespace IncidenceEditorNG;

class FreebusyViewCalendar : public EventViews::ViewCalendar
{
public:
    virtual ~FreebusyViewCalendar() {};
    virtual bool isValid(const KCalCore::Incidence::Ptr &incidence) const
    {
        return isValid(incidence->uid());
    }

    virtual bool isValid(const QString &incidenceIdentifier) const
    {
        return incidenceIdentifier.startsWith("fb-");
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
        case KCalCore::FreeBusyPeriod::Busy:
            return QColor("#f00");
        case KCalCore::FreeBusyPeriod::BusyTentative:
        case KCalCore::FreeBusyPeriod::BusyUnavailable:
            return QColor("#f70");
        case KCalCore::FreeBusyPeriod::Free:
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
    mFreebusyCalendar.setModel(mModel);

    mAgendaView = new EventViews::AgendaView(QDate(), QDate(), false,  false);

    FreebusyViewCalendar *fbCalendar = new FreebusyViewCalendar();
    fbCalendar->mCalendar = mFreebusyCalendar.calendar();
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

#include "resourcemanagement.moc"