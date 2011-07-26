/*
    Copyright (C) 2009    Frank Osterfeld

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

#ifndef KRSS_FEEDVISITOR_H

#include "krss_export.h"

#include <boost/shared_ptr.hpp>

namespace KRss {

    class NetFeed;
    class SearchFeed;

    class KRSS_EXPORT FeedVisitor {
    public:
        virtual ~FeedVisitor();

        virtual void visitNetFeed( const boost::shared_ptr<NetFeed>& feed );
        virtual void visitSearchFeed( const boost::shared_ptr<SearchFeed>& feed );
    };

    class KRSS_EXPORT ConstFeedVisitor {
    public:
        virtual ~ConstFeedVisitor();

        virtual void visitNetFeed( const boost::shared_ptr<const NetFeed>& feed );
        virtual void visitSearchFeed( const boost::shared_ptr<const SearchFeed>& feed );
    };
}

#endif // KRSS_FEEDVISITOR_H
