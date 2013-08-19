/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef STRIGI_KMPSEARCHER_H
#define STRIGI_KMPSEARCHER_H

#include <string>
#include <stdlib.h>

#include <strigi/strigiconfig.h>

namespace Strigi {
/**
 * Class for string search that uses the Knuth-Morris-Pratt algorithm.
 * Code based on the example on
 * http://en.wikipedia.org/wiki/Knuth-Morris-Pratt_algorithm
 **/
class STREAMS_EXPORT KmpSearcher {
private:
    std::string m_query;
    qint32* table;
    qint32 len;
    qint32 maxlen;
public:
    KmpSearcher() :table(0) { }
    explicit KmpSearcher(const std::string& query);
    ~KmpSearcher() {
        if (table) {
            free(table);
        }
    }
    void setQuery(const std::string& query);
    qint32 queryLength() const { return len; }
    std::string query() const { return m_query; }
    /**
     * @brief Find the needle in @p haystack.
     * @param haystack the text to search in.
     * @param haylen   the length of the text to search in.
     * @return         a pointer to the start of the match if a match is found
     *                 Otherwise @c 0.
     **/
    const char* search(const char* haystack, qint32 haylen) const;
};

} // end namespace Strigi

#endif
