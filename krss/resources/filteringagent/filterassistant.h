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

#ifndef RSSFILTERINGAGENT_FILTERASSISTANT_H
#define RSSFILTERINGAGENT_FILTERASSISTANT_H

#include "filterdata.h"
#include "ui_filterpropertieswidget.h"

#include <KDE/KAssistantDialog>
#include <akonadi/filter/componentfactory.h>
#include <akonadi/filter/ui/editorfactory.h>
#include <akonadi/filter/ui/programeditor.h>

class CheckableFeedListModel;

class FilterAssistant : public KAssistantDialog
{
public:
    FilterAssistant( Akonadi::Filter::ComponentFactory* componentFactory,
                     Akonadi::Filter::UI::EditorFactory* editorFactory,
                     QWidget* parent = 0, Qt::WFlags flags = 0 );

    FilterData filterData();
    void setFilterData( const FilterData& filterData );

private:
    QString m_filterId;
    Ui::FilterPropertiesWidget m_filterPropertiesUi;
    CheckableFeedListModel* m_feedsModel;
    Akonadi::Filter::UI::ProgramEditor* m_programEditor;
};

#endif // RSSFILTERINGAGENT_FILTERASSISTANT_H
