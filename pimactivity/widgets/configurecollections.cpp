/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "configurecollections.h"
#include "activitymanager.h"

#include <Akonadi/EntityTreeModel>
#include <Akonadi/ETMViewStateSaver>
#include <Akonadi/ChangeRecorder>

#include <KMime/KMimeMessage>

#include <KConfigGroup>
#include <KDialog>
#include <KCheckableProxyModel>

#include <QVBoxLayout>
#include <QTreeView>


namespace PimActivity {

ConfigureCollections::ConfigureCollections(QWidget *parent)
    : QWidget(parent), mModelState(0)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing( KDialog::spacingHint() );
    layout->setMargin( 0 );

    mFolderView = new QTreeView( this );
    mFolderView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    layout->addWidget( mFolderView );
    setLayout(layout);
    initCollections();
}

ConfigureCollections::~ConfigureCollections()
{

}

void ConfigureCollections::initCollections()
{
    // Create a new change recorder.
    mChangeRecorder = new Akonadi::ChangeRecorder( this );
    mChangeRecorder->setMimeTypeMonitored( KMime::Message::mimeType() );

    mModel = new Akonadi::EntityTreeModel( mChangeRecorder, this );

    // Set the model to show only collections, not items.
    mModel->setItemPopulationStrategy( Akonadi::EntityTreeModel::NoItemPopulation );

    // Create the Check proxy model.
    mSelectionModel = new QItemSelectionModel( mModel );
    mCheckProxy = new KCheckableProxyModel( this );
    mCheckProxy->setSelectionModel( mSelectionModel );
    mCheckProxy->setSourceModel( mModel );

    connect(mCheckProxy, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(slotDataChanged()));
    mFolderView->setModel( mCheckProxy );
}

void ConfigureCollections::slotDataChanged()
{
    Q_EMIT(changed(true));
}

void ConfigureCollections::readConfig(const QString &id)
{
    KSharedConfigPtr conf = ActivityManager::configFromActivity(id);
    if (!mModelState) {
        mModelState = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>( conf->group( "collections" ), this );
        mModelState->setSelectionModel( mSelectionModel );
    }
    mModelState->restoreState();
}

void ConfigureCollections::writeConfig(const QString &id)
{
    KSharedConfigPtr conf = ActivityManager::configFromActivity(id);
    KConfigGroup grp = conf->group(QLatin1String("collections"));
    if (mModelState) {
        delete mModelState;
        mModelState = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>( grp, this );
        mModelState->saveState();
    }
    Q_EMIT(changed(false));
}

void ConfigureCollections::setDefault()
{
    //TODO
}

}

