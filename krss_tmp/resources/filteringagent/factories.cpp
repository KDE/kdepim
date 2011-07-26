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

#include "factories.h"
#include "linkcommandeditor.h"

#include <KLocale>

using Akonadi::Filter::DataMemberDescriptor;
using Akonadi::Filter::CommandDescriptor;
using Akonadi::Filter::UI::CommandEditor;

RssComponentFactory::RssComponentFactory()
{
    registerDataMember( new DataMemberDescriptor( RssIdentifiers::Status,
                                                  QString::fromAscii( "rss:status" ),
                                                  i18n( "RSS item status" ),
                                                  Akonadi::Filter::DataTypeInteger ) );
    registerDataMember( new DataMemberDescriptor( RssIdentifiers::Title,
                                                  QString::fromAscii( "rss:title" ),
                                                  i18n( "RSS item title" ),
                                                  Akonadi::Filter::DataTypeString ) );
    registerDataMember( new DataMemberDescriptor( RssIdentifiers::Description,
                                                  QString::fromAscii( "rss:description" ),
                                                  i18n( "RSS item description" ),
                                                  Akonadi::Filter::DataTypeString ) );
    registerDataMember( new DataMemberDescriptor( RssIdentifiers::Content,
                                                  QString::fromAscii( "rss:content" ),
                                                  i18n( "RSS item content" ),
                                                  Akonadi::Filter::DataTypeString ) );

    CommandDescriptor::ParameterDescriptor* const pd = new CommandDescriptor::ParameterDescriptor(
                                             Akonadi::Filter::DataTypeInteger, QLatin1String( "Feed::Id" ) );
    CommandDescriptor* const lcd = new CommandDescriptor( RssIdentifiers::LinkCommand,
                                                          QLatin1String( "link" ),
                                                          i18n( "link to filtering feed" ), true );
    lcd->addParameter( pd );
    registerCommand( lcd );
}


CommandEditor* RssEditorFactory::createCommandEditor( QWidget* parent, const CommandDescriptor* command,
                                                      Akonadi::Filter::ComponentFactory* componentFactory )
{
    switch( command->id() ) {
        case RssIdentifiers::LinkCommand:
            return new LinkCommandEditor( parent, command, componentFactory, this );
        default:
            return EditorFactory::createCommandEditor( parent, command, componentFactory );
    }

    return 0;
}
