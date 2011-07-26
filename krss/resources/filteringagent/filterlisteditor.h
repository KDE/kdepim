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

#ifndef RSSFILTERINGAGENT_FILTERLISTEDITOR_H
#define RSSFILTERINGAGENT_FILTERLISTEDITOR_H

#include "filterdata.h"
#include "ui_filterlistwidget.h"

#include <akonadi/filter/componentfactory.h>
#include <akonadi/filter/ui/editorfactory.h>
#include <KDialog>

class FilterListEditor : public KDialog
{
    Q_OBJECT

public:
    explicit FilterListEditor( QWidget* parent = 0, Qt::WFlags flags = 0 );

    void setComponentFactory( Akonadi::Filter::ComponentFactory* componentFactory );
    void setEditorFactory( Akonadi::Filter::UI::EditorFactory* editorFactory );
    void setFilters( const QHash<QString, FilterData>& filters );
    QHash<QString, FilterData> filters() const;

private Q_SLOTS:
    void slotNewFilter();
    void slotEditCurrentFilter();
    void slotDeleteCurrentFilter();

private:
    Ui::FilterListWidget m_filterListUi;
    Akonadi::Filter::ComponentFactory* m_componentFactory;
    Akonadi::Filter::UI::EditorFactory* m_editorFactory;
    QHash<QString, FilterData> m_filters;
};

#endif // RSSFILTERINGAGENT_FILTERLISTEDITOR_H
