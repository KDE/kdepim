/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/filter.h"
#include "core/messageitem.h"

#include <baloo/pim/emailquery.h>
#include <baloo/pim/resultiterator.h>

using namespace MessageList::Core;

Filter::Filter()
{
}

bool Filter::containString(const QString &searchInString) const
{
    bool found = false;
    Q_FOREACH(const QString &str, mSearchList) {
        if(searchInString.contains(str,Qt::CaseInsensitive)) {
            found = true;
        } else {
            found = false;
            break;
        }
    }
    return found;
}

bool Filter::match( const MessageItem * item ) const
{
    if ( !mStatus.isEmpty() ) {
        Q_FOREACH(Akonadi::MessageStatus status, mStatus) {
            if (!(status & item->status())) {
                return false;
            }
        }
    }

    if ( !mSearchString.isEmpty() ) {
        if ( mMatchingItemIds.contains( item->itemId() ) )
            return true;
        bool searchMatches = false;
        if ( containString(item->subject()) ) {
            searchMatches = true;
        } else if ( containString(item->sender()) ) {
            searchMatches = true;
        } else if ( containString(item->receiver()) ) {
            searchMatches = true;
        }
        if ( !searchMatches )
            return false;
    }

    if ( !mTagId.isEmpty() ) {
        //mTagId is a Akonadi::Tag::url
        const bool tagMatches = item->findTag( mTagId ) != 0;
        if ( !tagMatches )
            return false;
    }

    return true;
}

bool Filter::isEmpty() const
{
    if ( !mStatus.isEmpty() )
        return false;

    if ( !mSearchString.isEmpty() )
        return false;

    if ( !mTagId.isEmpty() )
        return false;

    return true;
}

void Filter::clear()
{
    mStatus.clear();
    mSearchString.clear();
    mTagId.clear();
    mMatchingItemIds.clear();
    mSearchList.clear();
}

void Filter::setCurrentFolder( const Akonadi::Collection &folder )
{
    mCurrentFolder = folder;
}

void Filter::setSearchString( const QString &search, QuickSearchLine::SearchOptions options )
{
    const QString trimStr = search.trimmed();
    if ((mSearchString == trimStr) && (mOptions == options)) {
        return;
    }
    mSearchString = trimStr;
    mMatchingItemIds.clear();

    if (mSearchString.isEmpty()) {
        return;
    }
    bool needToSplitString = false;
    QString newStr = mSearchString;
    if (mSearchString.startsWith(QLatin1Char('"')) && mSearchString.startsWith(QLatin1Char('"'))) {
        newStr = newStr.remove(0,1);
        newStr = newStr.remove(newStr.length()-1,1);
        mSearchList = QStringList()<<newStr;
    } else {
        mSearchList = mSearchString.split(QLatin1Char(' '), QString::SkipEmptyParts);
        needToSplitString = true;
    }

    Baloo::PIM::EmailQuery query;
    if (options & QuickSearchLine::SearchEveryWhere) {
        query.matches(newStr);
        query.setSplitSearchMatchString(needToSplitString);
    } else if (options & QuickSearchLine::SearchAgainstSubject) {
        query.subjectMatches(newStr);
    } else if (options & QuickSearchLine::SearchAgainstBody) {
        query.bodyMatches(newStr);
    } else if (options & QuickSearchLine::SearchAgainstFrom) {
        query.setFrom(newStr);
    } else if (options & QuickSearchLine::SearchAgainstBcc) {
        query.setBcc(QStringList() << newStr);
    } else if (options & QuickSearchLine::SearchAgainstTo) {
        query.setTo(QStringList() << newStr);
    }


    //If the collection is virtual we're probably trying to filter the search collection, so we just search globally
    if (mCurrentFolder.isValid() && !mCurrentFolder.isVirtual()) {
        query.addCollection(mCurrentFolder.id());
    }

    Baloo::PIM::ResultIterator it = query.exec();
    while (it.next()) {
        mMatchingItemIds << it.id();
    }
    emit finished();
}

