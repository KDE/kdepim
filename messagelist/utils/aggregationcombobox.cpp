/* Copyright 2009 James Bendig <james@imptalk.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "aggregationcombobox.h"
#include "aggregationcombobox_p.h"

#include "messagelist/core/aggregation.h"
#include "messagelist/core/manager.h"
#include "messagelist/core/settings.h"
#include "messagelist/storagemodel.h"
#include <KGlobal>

using namespace MessageList::Core;
using namespace MessageList::Utils;

AggregationComboBox::AggregationComboBox( QWidget * parent )
    : KComboBox( parent ), d( new AggregationComboBoxPrivate( this ) )
{
    if( Manager::instance() )
        d->slotLoadAggregations();
    else
        setEnabled(false);
}

AggregationComboBox::~AggregationComboBox()
{
    delete d;
}

QString AggregationComboBox::currentAggregation() const
{
    return itemData( currentIndex() ).toString();
}

void AggregationComboBox::writeDefaultConfig() const
{
    KConfigGroup group( Settings::self()->config(), "MessageListView::StorageModelAggregations" );

    const QString aggregationID = currentAggregation();
    group.writeEntry( QLatin1String( "DefaultSet" ), aggregationID );

    if (Manager::instance())
        Manager::instance()->aggregationsConfigurationCompleted();
}

void AggregationComboBox::writeStorageModelConfig( MessageList::Core::StorageModel *storageModel, bool isPrivateSetting ) const
{
    writeStorageModelConfig( storageModel->id(), isPrivateSetting );
}

void AggregationComboBox::writeStorageModelConfig( const QString &id, bool isPrivateSetting ) const
{
    if (Manager::instance()) {
        // message list aggregation
        QString aggregationID;
        if ( isPrivateSetting ) {
            aggregationID = currentAggregation();
        } else { // explicitly use default aggregation id when using default aggregation.
            aggregationID = Manager::instance()->defaultAggregation()->id();
        }
        Manager::instance()->saveAggregationForStorageModel( id, aggregationID, isPrivateSetting );
        Manager::instance()->aggregationsConfigurationCompleted();
    }
}

void AggregationComboBox::writeStorageModelConfig( const Akonadi::Collection&col, bool isPrivateSetting ) const
{
    writeStorageModelConfig( QString::number( col.id() ), isPrivateSetting );
}

void AggregationComboBox::readStorageModelConfig( const QString & id, bool &isPrivateSetting )
{
    if (Manager::instance()) {
        const Aggregation *aggregation = Manager::instance()->aggregationForStorageModel( id, &isPrivateSetting );
        d->setCurrentAggregation( aggregation );
    }
}

void AggregationComboBox::readStorageModelConfig( MessageList::Core::StorageModel *storageModel, bool &isPrivateSetting )
{
    readStorageModelConfig( storageModel->id(), isPrivateSetting );
}

void AggregationComboBox::readStorageModelConfig( const Akonadi::Collection &col, bool &isPrivateSetting )
{
    if ( col.isValid() )
        readStorageModelConfig( QString::number( col.id() ), isPrivateSetting );
}

void AggregationComboBox::selectDefault()
{
    if (Manager::instance()) {
        const Aggregation *defaultAggregation = Manager::instance()->defaultAggregation();
        d->setCurrentAggregation( defaultAggregation );
    }
}

void AggregationComboBoxPrivate::slotLoadAggregations()
{
    if (!Manager::instance())
        return;
    q->clear();

    // Get all message list aggregations and sort them into alphabetical order.
    QList< Aggregation * > aggregations = Manager::instance()->aggregations().values();
    qSort( aggregations.begin(), aggregations.end(), MessageList::Core::Aggregation::compareName );

    foreach( const Aggregation * aggregation, aggregations )
    {
        q->addItem( aggregation->name(), QVariant( aggregation->id() ) );
    }
}

void AggregationComboBoxPrivate::setCurrentAggregation( const Aggregation *aggregation )
{
    Q_ASSERT( aggregation != 0 );

    const QString aggregationID = aggregation->id();
    const int aggregationIndex = q->findData( QVariant( aggregationID ) );
    q->setCurrentIndex( aggregationIndex );
}

#include "moc_aggregationcombobox.cpp"
