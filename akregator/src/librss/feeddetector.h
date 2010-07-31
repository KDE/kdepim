/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
 
#ifndef LIBRSS_FEEDDETECTOR_H
#define LIBRSS_FEEDDETECTOR_H

#include <tqstring.h>
#include <tqvaluelist.h>

class QStringList;
class KURL;

namespace RSS
{

    class FeedDetectorEntry
    {
        public:
            FeedDetectorEntry() {}
            FeedDetectorEntry(const TQString& url, const TQString& title) 
                : m_url(url), m_title(title) {}

            const TQString& url() const { return m_url; } 
            const TQString& title() const { return m_title; }

        private:	
            const TQString m_url;
            const TQString m_title;
    };	

    typedef TQValueList<FeedDetectorEntry> FeedDetectorEntryList; 

    /** a class providing functions to detect linked feeds in HTML sources */
    class FeedDetector
    {
        public:
            /** \brief searches an HTML page for feeds listed in @c <link> tags
            @c <link> tags with @c rel attribute values @c alternate or 
            @c service.feed are considered as feeds 
            @param s the html source to scan (the actual source, no URI)
            @return a list containing the detected feeds
            */
            static FeedDetectorEntryList extractFromLinkTags(const TQString& s);

            /** \brief searches an HTML page for slightly feed-like looking links and catches everything not running away quickly enough. 
            Extracts links from @c <a @c href> tags which end with @c xml, @c rss or @c rdf
            @param s the html source to scan (the actual source, no URI)
            @return a list containing the detected feeds
            */
            static TQStringList extractBruteForce(const TQString& s);

            static TQString fixRelativeURL(const TQString &s, const KURL &baseurl);
            
        private:
            FeedDetector() {}
    };
}

#endif //LIBRSS_FEEDDETECTOR_H
