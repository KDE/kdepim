/*
 * This file is part of KJots
 *
 * Copyright 2008 Stephen Kelly <steveire@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef KNOWITIMPORTER_H
#define KNOWITIMPORTER_H

class QDomElement;

#include <QHash>
#include <QDomDocument>

class KUrl;

struct KnowItNote {
    QString title;
    int depth;
    QString content;
    int id; // Only used to determine parent /child relationships. This is not the KJots Page or Book id.
    int parent;

    QList< QPair< QString, QString > > links;

};

/**
Class for importing KNowIt notes. KNowIt is not longer maintained.
@since 4.2
*/
class KnowItImporter
{
public:
    KnowItImporter();

    /**
    Create a KJotsBook from the knowit file at @p url.
    */
    void importFromUrl(const KUrl &url);

private:
    /**
    Builds several trees with roots at m_notes.
    @param url The url of the knowit file.
    */
    void buildNoteTree(const KUrl &url);

    /**
    Add a representation of note @p n to m_domDoc. If @p n has child notes, it will create a book, otherwise a page.
    */
    QDomElement addNote(const KnowItNote &n);

    /**
    Build a domDocument from the notes rooted at m_notes.
    */
    void buildDomDocument();

    QList<KnowItNote> m_notes; // Top level notes.
    QList<KnowItNote> m_lastNoteAtLevel;
    QDomDocument m_domDoc;
    QHash <int, KnowItNote> m_noteHash;
    QHash <int, QList< int > > m_childNotes;
};

#endif
