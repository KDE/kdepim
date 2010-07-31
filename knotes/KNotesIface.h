/*******************************************************************
 KNotesIface.h  --  This file defines the DCOP interface for KNotes.

 Copyright (C) 2000 by Adriaan de Groot
               2001-2004 by Michael Brade <brade@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#ifndef __KNotesIface_h__
#define __KNotesIface_h__

#include <tqstring.h>
#include <tqmap.h>

#include <dcopobject.h>


class KNotesIface : virtual public DCOPObject
{
    K_DCOP
k_dcop:
    /**
     * Create a new note.
     * @param name the name (title) of the new note, if it is empty,
     *        KNotes will choose an appropriate name
     * @param text the body of the new note
     * @return the new notes' id
     */
    virtual TQString newNote( const TQString& name = TQString::null,
                             const TQString& text = TQString::null ) = 0;

    /**
     * Create a new note and inserts the current text in the clipboard
     * as text.
     *
     * @param name the name (title) of the new note, if it is empty,
     *        KNotes will choose an appropriate name
     * @return the new notes' id
     */
    virtual TQString newNoteFromClipboard( const TQString& name = TQString::null ) = 0;

    /**
     * Deletes a note forever.
     * @param noteId the id of the note to kill
     */
    virtual ASYNC killNote( const TQString& noteId ) = 0;

    /**
     * Deletes a note forever.
     * @param noteId the id of the note to kill
     * @param force do not request confirmation
     */
    virtual ASYNC killNote( const TQString& noteId, bool force ) = 0;

    /**
     * Get all the notes including their ids.
     * @return a TQMap that maps the id of a note to its name
     */
    virtual TQMap<TQString,TQString> notes() const = 0;

    /**
     * Changes the title/name of a note.
     * @param noteId the id of the note to be modified
     * @param newName the new title
     */
    virtual ASYNC setName( const TQString& noteId, const TQString& newName ) = 0;

    /**
     * Sets the text of a note. This will delete the old text!
     * @param noteId the id of the note
     * @param newText the new text for the note
     */
    virtual ASYNC setText( const TQString& noteId, const TQString& newText ) = 0;

    /**
     * Returns the title/name of a note.
     * @param noteId the id of the note in question
     * @return the name as a QString
     */
    virtual TQString name( const TQString& noteId ) const = 0;

    /**
     * Returns the text of a note.
     * @param noteId the id of the note in question
     * @return the body as a QString
     */
    virtual TQString text( const TQString& noteId ) const = 0;
};

#endif
