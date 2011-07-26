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

#include "feedvisitor.h"

using namespace KRss;
using namespace boost;

FeedVisitor::~FeedVisitor() {}
void FeedVisitor::visitNetFeed( const shared_ptr<NetFeed>& f ) {}
void FeedVisitor::visitSearchFeed( const shared_ptr<SearchFeed>& f ) {}

ConstFeedVisitor::~ConstFeedVisitor() {}
void ConstFeedVisitor::visitNetFeed( const shared_ptr<const NetFeed>& f ) {}
void ConstFeedVisitor::visitSearchFeed( const shared_ptr<const SearchFeed>& f ) {}
