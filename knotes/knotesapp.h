/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2006, The KNotes Developers

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

#ifndef KNOTESAPP_H
#define KNOTESAPP_H

#include <tqstring.h>
#include <tqdict.h>
#include <tqptrlist.h>
#include <tqlabel.h>
#include <tqdom.h>

#include <kapplication.h>
#include <kxmlguiclient.h>

#include "KNotesAppIface.h"

class KFind;
class KNote;
class KPopupMenu;
class KAction;
class KActionMenu;
class KGlobalAccel;
class KXMLGUIFactory;
class KXMLGUIBuilder;
class KNotesAlarm;
class KNotesResourceManager;

namespace KCal {
    class Journal;
}

namespace KNetwork {
    class KServerSocket;
}


class KNotesApp : public TQLabel, public KSessionManaged, virtual public KXMLGUIClient,
    virtual public KNotesAppIface
{
    Q_OBJECT
public:
    KNotesApp();
    ~KNotesApp();

    void showNote( const TQString& id ) const;
    void hideNote( const TQString& id ) const;

    void killNote( const TQString& id );
    void killNote( const TQString& id, bool force );

    TQString name( const TQString& id ) const;
    TQString text( const TQString& id ) const;

    void setName( const TQString& id, const TQString& newName );
    void setText( const TQString& id, const TQString& newText );

    TQString fgColor( const TQString& id ) const;
    TQString bgColor( const TQString& id ) const;

    void setColor( const TQString& id, const TQString& fgColor,
                                      const TQString& bgColor );

    TQMap<TQString,TQString> notes() const;

    int width( const TQString& noteId ) const;
    int height( const TQString& noteId ) const;

    void move( const TQString& noteId, int x, int y ) const;
    void resize( const TQString& noteId, int width, int height ) const;

    void sync( const TQString& app );
    bool isNew( const TQString& app, const TQString& id ) const;
    bool isModified( const TQString& app, const TQString& id ) const;

    bool commitData( QSessionManager& );

public slots:
    TQString newNote( const TQString& name = TQString::null,
                     const TQString& text = TQString::null );
    TQString newNoteFromClipboard( const TQString& name = TQString::null );

    void hideAllNotes() const;
    void showAllNotes() const;

protected:
    void mousePressEvent( TQMouseEvent* );
    void resizeEvent ( TQResizeEvent * );

protected slots:
    void slotShowNote();
    void slotWalkThroughNotes();

    void slotOpenFindDialog();
    void slotFindNext();

    void slotPreferences();
    void slotConfigureAccels();

    void slotNoteKilled( KCal::Journal *journal );

    void slotQuit();

private:
    void showNote( KNote *note ) const;
    void saveConfigs();

private slots:
    void acceptConnection();
    void saveNotes();
    void saveNotes( const TQString & uid );
    void updateNoteActions();
    void updateGlobalAccels();
    void updateNetworkListener();
    void updateStyle();

    void createNote( KCal::Journal *journal );
    void killNote( KCal::Journal *journal );

private:
    class KNoteActionList : public TQPtrList<KAction>
    {
    public:
        virtual int compareItems( TQPtrCollection::Item s1, TQPtrCollection::Item s2 );
    };

    KNotesResourceManager *m_manager;

    KNotesAlarm     *m_alarm;
    KNetwork::KServerSocket   *m_listener;

    TQDict<KNote>    m_noteList;
    KNoteActionList m_noteActions;

    KFind           *m_find;
    TQDictIterator<KNote> *m_findPos;

    KPopupMenu      *m_note_menu;
    KPopupMenu      *m_context_menu;

    KGlobalAccel    *m_globalAccel;
    KXMLGUIFactory  *m_guiFactory;
    KXMLGUIBuilder  *m_guiBuilder;

    TQDomDocument    m_noteGUI;
    KAction         *m_findAction;
    TQString m_noteUidModify;
};

#endif
