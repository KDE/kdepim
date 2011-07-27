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

#ifndef RSSFILTERINGAGENT_RSSFILTERINGAGENT_H
#define RSSFILTERINGAGENT_RSSFILTERINGAGENT_H

#include "factories.h"
#include "filterdata.h"

#include <krss/feed.h>
#include <Akonadi/Collection>
#include <Akonadi/PreprocessorBase>

namespace Akonadi {
    namespace Filter {
        class Program;
    }
}

class RssFilteringAgent : public Akonadi::PreprocessorBase
{
public:
    explicit RssFilteringAgent( const QString& id );

    ProcessingResult processItem( const Akonadi::Item &item );
    void configure( WId windowId );

private:
    RssComponentFactory* const m_componentFactory;
    RssEditorFactory* const m_editorFactory;
    QHash<KRss::Feed::Id, QList<Akonadi::Filter::Program*> > m_programs;
    QHash<QString, FilterData> m_filters;
};

#endif // RSSFILTERINGAGENT_RSSFILTERINGAGENT_H
