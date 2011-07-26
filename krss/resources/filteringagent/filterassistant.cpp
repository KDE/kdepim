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

#include "filterassistant.h"
#include "checkablefeedlistmodel.h"

#include <krss/feedlist.h>
#include <krss/resourcemanager.h>
#include <krss/ui/feedlistview.h>
#include <KLocale>
#include <QVBoxLayout>
#include <QLabel>

using namespace KRss;

FilterAssistant::FilterAssistant( Akonadi::Filter::ComponentFactory* componentFactory,
                                  Akonadi::Filter::UI::EditorFactory* editorFactory,
                                  QWidget* parent, Qt::WFlags flags )
    : KAssistantDialog( parent, flags ), m_feedsModel( 0 ), m_programEditor( 0 )
{
    // first page: name of the filter and source feeds
    QWidget* const propsWidget = new QWidget();
    m_filterPropertiesUi.setupUi( propsWidget );
    addPage( propsWidget, i18n( "Filter properties" ) );

    // second page: source feeds
    RetrieveFeedListJob* const rjob = new RetrieveFeedListJob( this );
    rjob->setResources( ResourceManager::self()->resources() );
    if ( !rjob->exec() ) {
        kWarning() << rjob->errorString();
        return;
    }

    TagProviderRetrieveJob* const tjob = new TagProviderRetrieveJob();
    if ( !tjob->exec() ) {
        kWarning() << tjob->errorString();
        return;
    }

    m_feedsModel = new CheckableFeedListModel( rjob->feedList(), tjob->tagProvider(), this );
    m_filterPropertiesUi.sourceFeedsView->setModel( m_feedsModel );

    // second page: filtering program
    m_programEditor = new Akonadi::Filter::UI::ProgramEditor( this, componentFactory, editorFactory );
    addPage( new KPageWidgetItem( m_programEditor, i18n( "Filtering program" ) ) );
}

FilterData FilterAssistant::filterData()
{
    FilterData data;
    data.setId( m_filterId );
    data.setName( m_filterPropertiesUi.nameEdit->text() );
    data.setSourceFeeds( m_feedsModel->selectedFeeds() );
    data.setProgram( m_programEditor->commit() );
    return data;
}

void FilterAssistant::setFilterData( const FilterData& filterData )
{
    m_filterId = filterData.id();
    m_filterPropertiesUi.nameEdit->setText( filterData.name() );
    m_feedsModel->setSelectedFeeds( filterData.sourceFeeds() );
    if ( filterData.program() )
        m_programEditor->fillFromProgram( filterData.program() );
}
