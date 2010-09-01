/*
   This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2004-2006 Michael Brade <brade@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KNOTES_PART_H
#define KNOTES_PART_H

#include <tqdict.h>

#include <kiconview.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <libkcal/journal.h>
#include <kparts/part.h>

#include "knotes/KNotesIface.h"

class KIconView;
class TQIconViewItem;
class KNotesIconViewItem;
class KNoteTip;
class KNoteEditDlg;
class KNotesResourceManager;

namespace KCal {
class Journal;
}

class KNotesPart : public KParts::ReadOnlyPart, virtual public KNotesIface
{
  Q_OBJECT

  public:
    KNotesPart( TQObject *parent = 0, const char *name = 0 );
   ~KNotesPart();

    bool openFile();

  public slots:
    TQString newNote( const TQString& name = TQString::null,
                     const TQString& text = TQString::null );
    TQString newNoteFromClipboard( const TQString& name = TQString::null );

  public:
    void killNote( const TQString& id );
    void killNote( const TQString& id, bool force );

    TQString name( const TQString& id ) const;
    TQString text( const TQString& id ) const;

    void setName( const TQString& id, const TQString& newName );
    void setText( const TQString& id, const TQString& newText );

    TQMap<TQString, TQString> notes() const;

  private slots:
    void createNote( KCal::Journal *journal );
    void killNote( KCal::Journal *journal );

    void editNote( TQIconViewItem *item );

    void renameNote();
    void renamedNote( TQIconViewItem *item );

    void slotOnItem( TQIconViewItem *item );
    void slotOnViewport();
    void slotOnCurrentChanged( TQIconViewItem *item );

    void popupRMB( TQIconViewItem *item, const TQPoint& pos );
    void killSelectedNotes();

    void printSelectedNotes();

  private:
    KIconView *mNotesView;
    KNoteTip *mNoteTip;
    KNoteEditDlg *mNoteEditDlg;

    KNotesResourceManager *mManager;
    TQDict<KNotesIconViewItem> mNoteList;
  TQString mOldName;
};

#endif
