/*******************************************************************
 KNotesIface.h  --  This file defines the DCOP interface for KNotes.

 Copyright (C) 2000 by Adriaan de Groot
               2001 by Michael Brade

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
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*******************************************************************/

#ifndef __KNotesIface_h__
#define __KNotesIface_h__

#include <qstring.h>
#include <qmap.h>

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
     * @return the new notes' id or -1 if an error occured
     */
    virtual int newNote( QString name = QString::null,
                         const QString& text = QString::null ) = 0;

    /**
     * Show a note as if it had been selected from the "notes" menu.
     * @param noteId the id of the note to show
     */
    virtual ASYNC showNote( int noteId ) = 0;

    /**
     * Deletes a note forever.
     * @param noteId the id of the note to kill
     */
    virtual ASYNC killNote( int noteId ) = 0;

    /**
     * Get all the notes including their ids.
     * @return a QMap that maps the id of a note to its name
     */
    virtual QMap<int,QString> notes() = 0;

    /**
     * Changes the title of a note.
     * @param noteId the id of the note to be modified
     * @param newName the new title
     */
    virtual ASYNC setName( int noteId, const QString& newName ) = 0;

    /**
     * Sets the text of a note. This will delete the old text!
     * @param noteId the id of the note
     * @param newText the new text for the note
     */
    virtual ASYNC setText( int noteId, const QString& newText ) = 0;

    /**
     * Returns the text of a note.
     * @param noteId the title of the note in question
     * @return the body as a QString
     */
    virtual QString text( int noteId ) = 0;

    /**
     * This tells KNotes that a specific app has synchronized with all the notes.
     * @param app the app that has synced with KNotes
     */
    virtual ASYNC sync( const QString& app ) = 0;

    /**
     * Test if a note was created new after the last sync.
     * @param app the app that wants to get the status since the last sync
     * @param noteId the id of the note
     * @return true if the note is new, false if not or if the note does
     *         not exist
     */
    virtual bool isNew( const QString& app, int noteId ) = 0;

    /**
     * Test if a note was modified since the last sync.
     * @param app the app that wants to get the status since the last sync
     * @param noteId the id of the note
     * @return true if modified (note that this will return true if the note is
     *         new as well!) and false if the note is not modified or doesn't exist
     */
    virtual bool isModified( const QString& app, int noteId ) = 0;
};

#endif
