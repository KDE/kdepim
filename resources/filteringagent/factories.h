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

#ifndef RSSFILTERINGAGENT_FACTORIES_H
#define RSSFILTERINGAGENT_FACTORIES_H

#include <akonadi/filter/componentfactory.h>
#include <akonadi/filter/commanddescriptor.h>
#include <akonadi/filter/datamemberdescriptor.h>
#include <akonadi/filter/ui/editorfactory.h>

namespace RssIdentifiers {
    enum RssDataMemberIdentifiers {
        Status = Akonadi::Filter::DataMemberCustomFirst,
        Title,
        Description,
        Content
    };

    enum RssCommandIdentifiers {
        LinkCommand = Akonadi::Filter::CommandCustomFirst
    };
} // namespace RssIdentifiers

class RssComponentFactory : public Akonadi::Filter::ComponentFactory
{
public:
    RssComponentFactory();
};

class RssEditorFactory : public Akonadi::Filter::UI::EditorFactory
{
public:
    Akonadi::Filter::UI::CommandEditor* createCommandEditor( QWidget* parent,
                            const Akonadi::Filter::CommandDescriptor* command,
                            Akonadi::Filter::ComponentFactory* componentFactory );
};

#endif // RSSFILTERINGAGENT_FACTORIES_H
