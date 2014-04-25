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
#include <AkonadiCore/changerecorder.h>
#include <AkonadiWidgets/entitytreeview.h>
#include <AkonadiCore/entitymimetypefiltermodel.h>
#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/itemfetchscope.h>
#include <AkonadiCore/monitor.h>
#include <AkonadiCore/selectionproxymodel.h>
#include <AkonadiCore/session.h>
#include <libkdepim/misc/statisticsproxymodel.h>

#include <messagelist/pane.h>

//Qt
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

//KDE
#include <KDebug>

//plasma
#include <Plasma/Dialog>
#include <Plasma/TabBar>
#include <Plasma/Theme>


//own
#include "kpdialog.h"
#include "kpapplet.h"


using namespace KP;
using namespace Plasma;


KPDialog::KPDialog(KPApplet * kpapplet, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_tabs(0),
      m_folderListWidget(0),
      m_applet(kpapplet)
{
    (void)dialog();
    setMinimumSize(300, 400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

KPDialog::~KPDialog()
{
}

QGraphicsWidget * KPDialog::dialog()
{
    if (!m_tabs) {
        m_tabs = new Plasma::TabBar(this);
        //m_tabs->setPreferredSize(300, 400);
        //m_tabs->setMinimumSize(150, 200);
        //m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_folderListWidget = new QWidget();
        QVBoxLayout *f_layout = new QVBoxLayout(m_folderListWidget);
        m_folderListProxyWidget = new QGraphicsProxyWidget(m_tabs);

        m_messageListWidget = new QWidget();
        QVBoxLayout *m_layout = new QVBoxLayout(m_messageListWidget);
        m_messageListProxyWidget = new QGraphicsProxyWidget(m_tabs);

        f_layout->setSpacing(0);
        f_layout->setMargin(0);

        setupPane();
        f_layout->addWidget(m_folderListView);

        m_folderListProxyWidget->setWidget(m_folderListWidget);
        m_tabs->addTab(i18n("Folders"), m_folderListProxyWidget);

        m_messageListProxyWidget->setWidget(m_messageListWidget);
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->addWidget(m_messagePane);

        m_tabs->addTab(i18n("Messages"), m_messageListProxyWidget);

        connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
        updateColors();
    }
    return m_tabs;
}

void KPDialog::updateColors()
{
    QPalette p = m_folderListWidget->palette();
    p.setColor(QPalette::Window, Qt::transparent);
    p.setColor(QPalette::Base, Qt::transparent);
    //p.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    p.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    m_folderListWidget->setPalette(p);
    m_folderListView->setPalette(p);
    m_folderListView->setAttribute(Qt::WA_NoSystemBackground);
    m_folderListWidget->setAttribute(Qt::WA_NoSystemBackground);
    m_messageListWidget->setAttribute(Qt::WA_NoSystemBackground);
    m_messageListWidget->setPalette(p);
}

void KPDialog::setupPane()
{
    kDebug() << "Setting up";
    // Setup the core model
    Akonadi::Session *session = new Akonadi::Session( "KPApplet", m_folderListWidget );

      Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( m_folderListWidget );
      monitor->setCollectionMonitored( Akonadi::Collection::root() );
      monitor->fetchCollection( true );
      monitor->setMimeTypeMonitored( QLatin1String("message/rfc822"), true );
      monitor->itemFetchScope().fetchFullPayload(true);

      Akonadi::EntityTreeModel *entityModel = new Akonadi::EntityTreeModel( monitor, m_folderListWidget );
      entityModel->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );

      // Create the collection view
      m_folderListView = new Akonadi::EntityTreeView( 0, m_folderListWidget );
      m_folderListView->setSelectionMode( QAbstractItemView::ExtendedSelection );



      // Setup the message folders collection...
      Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel( m_folderListWidget );
      collectionFilter->setSourceModel( entityModel );
      //collectionFilter->addMimeTypeInclusionFilter( "message/rfc822" );
      collectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
      collectionFilter->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );

      // ... with statistics...
      KPIM::StatisticsProxyModel *statisticsProxyModel = new KPIM::StatisticsProxyModel( m_folderListWidget );
      statisticsProxyModel->setSourceModel( collectionFilter );

      // ... and sortable
      QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( m_folderListWidget );
      sortModel->setDynamicSortFilter( true );
      sortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
      sortModel->setSourceModel( statisticsProxyModel );
      // Use the model
      m_folderListView->setModel( sortModel );
      //entityModel->setRootCollection(Akonadi::Collection::root());
      
      // Now make the message list multi-tab pane
      m_messagePane = new MessageList::Pane( true, entityModel, m_folderListView->selectionModel(), m_messageListWidget );
      //connect( m_messagePane, SIGNAL(messageSelected(Akonadi::Item)),
      //       this, SLOT(slotMessageSelected(Akonadi::Item)) );

}
