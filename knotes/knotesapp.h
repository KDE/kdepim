/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2004, The KNotes Developers

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

#ifndef KNOTESAPP_H
#define KNOTESAPP_H

#include <qstring.h>
#include <qptrlist.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>

#include "KNotesIface.h"

class KNote;
class KPopupMenu;
class KAction;
class KActionMenu;
class KGlobalAccel;
class KXMLGUIFactory;
class ResourceManager;


class KNotesApp : public QLabel, virtual public KNotesIface, public KSessionManaged,
    public KXMLGUIBuilder, virtual public KXMLGUIClient
{
    friend class ResourceManager;

    Q_OBJECT
public:
    KNotesApp();
    ~KNotesApp();

    QString newNote( const QString& name = QString::null,
                            const QString& text = QString::null );
    QString newNoteFromClipboard( const QString& name = QString::null );

    void showNote( const QString& id ) const;
    void hideNote( const QString& id ) const;
    void killNote( const QString& id, bool force );
    void killNote( const QString& id );

    QString name( const QString& id ) const;
    QString text( const QString& id ) const;

    void setName( const QString& id, const QString& newName );
    void setText( const QString& id, const QString& newText );

    QMap<QString,QString> notes() const;

    void sync( const QString& app );
    bool isNew( const QString& app, const QString& id ) const;
    bool isModified( const QString& app, const QString& id ) const;

    bool commitData( QSessionManager& );

protected:
    void mousePressEvent( QMouseEvent* );
    bool eventFilter( QObject*, QEvent* );

protected slots:
    void slotNewNote();
    void slotNewNoteFromClipboard();
    void slotShowNote();

    void slotPreferences() const;
    void slotConfigureAccels();

    void slotQuit();

private slots:
    void updateNoteActions();
    void updateGlobalAccels();

    void saveNotes();

private:
    void showNote( KNote *note ) const;
    void saveConfig();

private:
    class KNoteActionList : public QPtrList<KAction>
    {
    public:
        virtual int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 );
    };

    ResourceManager* m_manager;

    KNoteActionList m_noteActions;

    KPopupMenu *m_note_menu;
    KPopupMenu *m_context_menu;

    KXMLGUIFactory *factory;

    KGlobalAccel *globalAccel;
};

#endif
