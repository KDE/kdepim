/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "filterlisteditor.h"
#include "filterassistant.h"

#include <KRandom>

FilterListEditor::FilterListEditor( QWidget* parent, Qt::WFlags flags )
    : KDialog( parent, flags )
{
    QWidget* const filterListWidget = new QWidget();
    m_filterListUi.setupUi( filterListWidget );
    setMainWidget( filterListWidget );
    setCaption( i18n( "Filter editor" ) );

    connect( m_filterListUi.newButton, SIGNAL( clicked() ), this, SLOT( slotNewFilter() ) );
    connect( m_filterListUi.editButton, SIGNAL( clicked() ), this, SLOT( slotEditCurrentFilter() ) );
    connect( m_filterListUi.deleteButton, SIGNAL( clicked() ), this, SLOT( slotDeleteCurrentFilter() ) );
}

void FilterListEditor::setComponentFactory( Akonadi::Filter::ComponentFactory* componentFactory )
{
    m_componentFactory = componentFactory;
}

void FilterListEditor::setEditorFactory( Akonadi::Filter::UI::EditorFactory* editorFactory )
{
    m_editorFactory = editorFactory;
}

void FilterListEditor::setFilters( const QHash<QString, FilterData>& filters )
{
    m_filters = filters;

    Q_FOREACH( const FilterData& filterData, m_filters ) {
        QListWidgetItem* const item = new QListWidgetItem( filterData.name() );
        item->setData( Qt::UserRole, filterData.id() );
        m_filterListUi.filterList->addItem( item );
    }
}

QHash<QString, FilterData> FilterListEditor::filters() const
{
    return m_filters;
}

void FilterListEditor::slotNewFilter()
{
    Q_ASSERT( m_componentFactory );
    Q_ASSERT( m_editorFactory );

    FilterAssistant* const assistant = new FilterAssistant( m_componentFactory, m_editorFactory, this );
    assistant->setCaption( i18n( "Creating new filter" ) );

    FilterData filter;
    filter.setId( QLatin1String( "rss_filter_" ) + QString::number( KRandom::random() ) );
    assistant->setFilterData( filter );

    if ( assistant->exec() ) {
        const FilterData filterData = assistant->filterData();
        m_filters.insert( filterData.id(), filterData );

        QListWidgetItem* const item = new QListWidgetItem( filterData.name() );
        item->setData( Qt::UserRole, filterData.id() );
        m_filterListUi.filterList->addItem( item );
    }

    delete assistant;
}

void FilterListEditor::slotEditCurrentFilter()
{
    Q_ASSERT( m_componentFactory );
    Q_ASSERT( m_editorFactory );

    const QString filterId = m_filterListUi.filterList->currentItem()->data( Qt::UserRole ).toString();
    const FilterData filterData = m_filters.value( filterId );

    FilterAssistant* const assistant = new FilterAssistant( m_componentFactory, m_editorFactory, this );
    assistant->setCaption( i18n( "Editing filter" ) );
    assistant->setFilterData( filterData );

    if ( assistant->exec() ) {
        const FilterData filterData = assistant->filterData();
        m_filters.insert( filterData.id(), filterData );

        QListWidgetItem* const item = m_filterListUi.filterList->currentItem();
        item->setText( filterData.name() );
    }

    delete assistant;
}

void FilterListEditor::slotDeleteCurrentFilter()
{
    Q_ASSERT( m_componentFactory );
    Q_ASSERT( m_editorFactory );

    const QString filterId = m_filterListUi.filterList->currentItem()->data( Qt::UserRole ).toString();
    m_filters.remove( filterId );
    delete m_filterListUi.filterList->takeItem( m_filterListUi.filterList->currentRow() );
}
