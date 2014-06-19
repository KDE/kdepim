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

#include "resourcemanagement.h"
#include "ui_resourcemanagement.h"
#include "resourcemodel.h"
#include "freebusyitem.h"

//#include "korganizer/korganizer_part.h"
#include "visualfreebusywidget.h"

#include <akonadi/calendar/freebusymanager.h>
#include <kldap/ldapobject.h>

#include <QStringList>
#include <QLabel>

#include <KDebug>


using namespace IncidenceEditorNG;

ResourceManagement::ResourceManagement()
{

    mUi = new Ui_resourceManagement;

    QWidget *w = new QWidget( this );
    mUi->setupUi( w );
    setMainWidget( w );

    QVariantList list;
    //QGridLayout *layout = new QGridLayout( resourceCalender );
    //layout->setSpacing( 0 );
    //KOrganizerPart *view = new KOrganizerPart(resourceCalender,  this,  list);
    //resourceCalender = view->topLevelWidget();
    mModel = new FreeBusyItemModel;
#ifndef KDEPIM_MOBILE_UI
    VisualFreeBusyWidget *mVisualWidget = new VisualFreeBusyWidget( mModel, 8, this );
    mUi->resourceCalender->addWidget( mVisualWidget );
#endif

    QStringList attrs;
    attrs << QLatin1String("cn") << QLatin1String("mail") << QLatin1String("givenname") << QLatin1String("sn");

    ResourceModel *model = new ResourceModel(attrs);
    mUi->treeResults->setModel(model);

    // This doesn't work till now :( -> that's why i use the clieck signal
    mUi->treeResults->setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModel = mUi->treeResults->selectionModel();

    connect(mUi->resourceSearch, SIGNAL(textChanged(const QString&)),
            SLOT(slotStartSearch(const QString&)));

    connect(mUi->treeResults, SIGNAL(clicked(const QModelIndex &)),
            SLOT(slotShowDetails(const QModelIndex &)));

    Akonadi::FreeBusyManager *m = Akonadi::FreeBusyManager::self();
    connect( m, SIGNAL(freeBusyRetrieved(KCalCore::FreeBusy::Ptr,QString)),
        SLOT(slotInsertFreeBusy(KCalCore::FreeBusy::Ptr,QString)) );
}

void ResourceManagement::slotStartSearch(const QString &text)
{
    ((ResourceModel*)mUi->treeResults->model())->startSearch(text);
}

void ResourceManagement::slotShowDetails(const QModelIndex & current)
{
    ResourceItem::Ptr item = current.model()->data(current, ResourceModel::Resource).value<ResourceItem::Ptr>();
    showDetails(item->ldapObject());
}


void ResourceManagement::showDetails(const KLDAP::LdapObject &obj)
{
    // Clean up formDetails
    QLayoutItem *child;
    while ((child = mUi->formDetails->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }

    // Fill formDetails with data
    foreach(const QString & key, obj.attributes().keys()) {
        QStringList list;
        foreach(const QByteArray & value, obj.attributes().value(key)) {
            list << QString::fromUtf8(value);
        }
        kDebug() << key <<  list;
        mUi->formDetails->addRow(key, new QLabel(list.join("\n")));
    }

    QString name = QString::fromUtf8(obj.attributes().value("cn")[0]);
    QString email = QString::fromUtf8(obj.attributes().value("mail")[0]);
    kDebug() <<  name <<  email;
    KCalCore::Attendee::Ptr attendee(new KCalCore::Attendee(name,  email));
    FreeBusyItem::Ptr freebusy( new FreeBusyItem( attendee, this ));
    mModel->clear();
    mModel->addItem(freebusy);
}

void ResourceManagement::slotInsertFreeBusy(const KCalCore::FreeBusy::Ptr &fb, const QString &email)
{
    kDebug() <<  fb <<  email;

}


#include "resourcemanagement.moc"
