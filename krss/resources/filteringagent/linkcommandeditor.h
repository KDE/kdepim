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

#ifndef RSSFILTERINGAGENT_LINKCOMMANDEDITOR_H
#define RSSFILTERINGAGENT_LINKCOMMANDEDITOR_H

#include "ui_linkcommandeditor.h"
#include "virtualfeedlistmodel.h"

#include <akonadi/collection.h>
#include <akonadi/filter/ui/editorfactory.h>
#include <akonadi/filter/ui/actioneditor.h>
#include <akonadi/filter/ui/commandeditor.h>

namespace Akonadi {
    namespace Filter {
        class ComponentFactory;
        class Component;
        namespace UI {
            class EditorFactory;
        }
        namespace Action {
            class Base;
        }
    }
}

class LinkCommandEditor : public Akonadi::Filter::UI::CommandEditor
{
    Q_OBJECT
public:
    LinkCommandEditor( QWidget* parent, const Akonadi::Filter::CommandDescriptor * commandDescriptor,
                       Akonadi::Filter::ComponentFactory* componentFactory,
                       Akonadi::Filter::UI::EditorFactory* editorComponentFactory );

    void setVirtualFeedCollections( const QList<Akonadi::Collection>& collections );
    QList<Akonadi::Collection> virtualFeedCollections() const;

    void fillFromAction( Akonadi::Filter::Action::Base * action ); // reimpl
    Akonadi::Filter::Action::Base* commitState( Akonadi::Filter::Component * parent ); // reimpl

private Q_SLOTS:
    void slotNewVirtualFeed();

private:
    Ui::LinkCommandEditor m_ui;
    VirtualFeedListModel* m_virtualFeedListModel;
};

#endif // RSSFILTERINGAGENT_LINKCOMMANDEDITOR_H
