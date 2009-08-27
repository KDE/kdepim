/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Akonadi
#include "akonadi_next/entitytreeview.h"
#include <akonadi/entityfilterproxymodel.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/monitor.h>
#include <akonadi/selectionproxymodel.h>
#include <akonadi/session.h>
#include <akonadi/statisticstooltipproxymodel.h>

#include <messagelist/pane.h>

//Qt
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

//KDE
#include <KDebug>
#include <KColorScheme>
#include <KIcon>
#include <KIconLoader>
#include <KMimeType>
#include <KRun>

//plasma
#include <Plasma/Dialog>
#include <Plasma/Theme>


//own
#include "kpdialog.h"
#include "kpapplet.h"


using namespace KP;
using namespace Plasma;


KPDialog::KPDialog(KPApplet * kpapplet,QObject *parent)
    : QObject(parent),
      m_widget(0),
      m_applet(kpapplet)
{
    buildDialog();
}

KPDialog::~KPDialog()
{
}

QWidget * KPDialog::dialog()
{
    return m_widget;
}

void KPDialog::buildDialog()
{
    m_widget = new QWidget();

    QVBoxLayout *l_layout = new QVBoxLayout(m_widget);
    l_layout->setSpacing(0);
    l_layout->setMargin(0);

    m_button = new KPushButton(m_widget);
    m_button->setIcon(KIcon("kmail"));
    m_button->setText("kmail!");
    l_layout->addWidget(m_button);

    setupPane();
    l_layout->addWidget(m_collectionView);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    updateColors();
}

void KPDialog::updateColors()
{
    QPalette p = m_widget->palette();
    p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    m_widget->setPalette(p);
    m_button->setPalette(p);
}

void KPDialog::setupPane()
{
    // Setup the core model
    Akonadi::Session *session = new Akonadi::Session( "KPApplet", m_widget );

    Akonadi::Monitor *monitor = new Akonadi::Monitor( m_widget );
    monitor->setCollectionMonitored( Akonadi::Collection::root() );
    monitor->fetchCollection( true );
    monitor->setMimeTypeMonitored( "message/rfc822", true );
    monitor->itemFetchScope().fetchFullPayload(true);

    Akonadi::EntityTreeModel *entityModel = new Akonadi::EntityTreeModel( session, monitor, m_widget );
    entityModel->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );

    // Create the collection view
    m_collectionView = new Akonadi::EntityTreeView( 0, m_widget );
    m_collectionView->setSelectionMode( QAbstractItemView::ExtendedSelection );


    // Setup the message folders collection...
    Akonadi::EntityFilterProxyModel *collectionFilter = new Akonadi::EntityFilterProxyModel( m_widget );
    collectionFilter->setSourceModel( entityModel );
    collectionFilter->addMimeTypeInclusionFilter( "message/rfc822" );
    collectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
    collectionFilter->setHeaderSet( Akonadi::EntityTreeModel::CollectionTreeHeaders );

    // ... with statistics...
    Akonadi::StatisticsToolTipProxyModel *statisticsProxyModel = new Akonadi::StatisticsToolTipProxyModel( m_widget );
    statisticsProxyModel->setSourceModel( collectionFilter );

    // ... and sortable
    QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( m_widget );
    sortModel->setDynamicSortFilter( true );
    sortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
    sortModel->setSourceModel( statisticsProxyModel );
    // Use the model
    m_collectionView->setModel( sortModel );
    entityModel->setRootCollection(Akonadi::Collection(12));

    /*
    // Now make the message list multi-tab pane
    m_messagePane = new MessageList::Pane( entityModel, collectionView->selectionModel(), m_widget );
    connect( m_messagePane, SIGNAL(messageSelected(Akonadi::Item)),
           this, SLOT(slotMessageSelected(Akonadi::Item)) );
    */
}
#include "kpdialog.moc"
