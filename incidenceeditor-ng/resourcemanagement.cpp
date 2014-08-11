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


#include <KDebug>


using namespace IncidenceEditorNG;

class RowController : public KDGantt::AbstractRowController
{
  private:
    static const int ROW_HEIGHT ;
    QPointer<QAbstractItemModel> m_model;

  public:
    RowController()
    {
      mRowHeight = 20;
    }

    void setModel( QAbstractItemModel *model )
    {
      m_model = model;
    }

    /*reimp*/
    int headerHeight() const
    {
      return mRowHeight + 10;
    }

    /*reimp*/
    bool isRowVisible( const QModelIndex & ) const
    {
      return true;
    }

    /*reimp*/
    bool isRowExpanded( const QModelIndex & ) const
    {
      return false;
    }

    /*reimp*/
    KDGantt::Span rowGeometry( const QModelIndex &idx ) const
    {
      return KDGantt::Span( idx.row() * mRowHeight, mRowHeight );
    }

    /*reimp*/
    int maximumItemHeight() const
    {
      return mRowHeight*6/8;
    }

    /*reimp*/
    int totalHeight() const
    {
      return m_model->rowCount() * mRowHeight;
    }

    /*reimp*/
    QModelIndex indexAt( int height ) const
    {
      return m_model->index( height / mRowHeight, 0 );
    }

    /*reimp*/
    QModelIndex indexBelow( const QModelIndex &idx ) const
    {
      if ( !idx.isValid() ) {
        return QModelIndex();
      }
      return idx.model()->index( idx.row() + 1, idx.column(), idx.parent() );
    }

    /*reimp*/
    QModelIndex indexAbove( const QModelIndex &idx ) const
    {
      if ( !idx.isValid() ) {
        return QModelIndex();
      }
      return idx.model()->index( idx.row() - 1, idx.column(), idx.parent() );
    }

    void setRowHeight( int height )
    {
      mRowHeight = height;
    }

  private:
    int mRowHeight;

};

class GanttHeaderView : public QHeaderView
{
public:
    explicit GanttHeaderView( QWidget *parent = 0 ) : QHeaderView( Qt::Horizontal, parent )
    {
    }

    QSize sizeHint() const
    {
        QSize s = QHeaderView::sizeHint();
        s.rheight() *= 2;
        return s;
    }
};


ResourceManagement::ResourceManagement()
{

    mUi = new Ui_resourceManagement;

    QWidget *w = new QWidget( this );
    mUi->setupUi( w );
    setMainWidget( w );

    QVariantList list;
    mModel = new FreeBusyItemModel;
#ifndef KDEPIM_MOBILE_UI

    KDGantt::GraphicsView *mGanttGraphicsView = new KDGantt::GraphicsView( this );
    mGanttGraphicsView->setObjectName( "mGanttGraphicsView" );
    mGanttGraphicsView->setToolTip(
        i18nc( "@info:tooltip",
            "Shows the Free/Busy status of a resource.") );
    mGanttGraphicsView->setWhatsThis(
        i18nc( "@info:whatsthis",
            "Shows the Free/Busy status of a resource.") );
    FreeBusyGanttProxyModel *model = new FreeBusyGanttProxyModel( this );
    model->setSourceModel( mModel );

    RowController *mRowController = new RowController;
    mRowController->setRowHeight( fontMetrics().height()*4 );   //TODO: detect

    mRowController->setModel( model );
    mGanttGraphicsView->setRowController( mRowController );

    KDGantt::DateTimeGrid *mGanttGrid = new KDGantt::DateTimeGrid;
    mGanttGrid->setScale( KDGantt::DateTimeGrid::ScaleDay );
    mGanttGrid->setDayWidth( 300 );
    mGanttGrid->setRowSeparators( true );
    mGanttGraphicsView->setGrid( mGanttGrid );
    mGanttGraphicsView->setModel( model );
    mGanttGraphicsView->viewport()->setFixedWidth( 300 * 30 );

    mUi->resourceCalender->addWidget( mGanttGraphicsView );
#endif

    QStringList attrs;
    attrs << QLatin1String("cn") << QLatin1String("mail")
          << QLatin1String("owner") << QLatin1String("givenname") << QLatin1String("sn")
          << QLatin1String("kolabDescAttribute") << QLatin1String("description");
    ResourceModel *resourcemodel = new ResourceModel(attrs);
    mUi->treeResults->setModel(resourcemodel);

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

    connect(resourcemodel,SIGNAL(layoutChanged()),SLOT(slotLayoutChanged()));
}

void ResourceManagement::slotStartSearch(const QString &text)
{
    ((ResourceModel*)mUi->treeResults->model())->startSearch(text);
}

void ResourceManagement::slotShowDetails(const QModelIndex & current)
{
    ResourceItem::Ptr item = current.model()->data(current, ResourceModel::Resource).value<ResourceItem::Ptr>();
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

void ResourceManagement::slotInsertFreeBusy(const KCalCore::FreeBusy::Ptr &fb, const QString &email)
{
    kDebug() <<  fb <<  email;

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


#include "resourcemanagement.moc"
