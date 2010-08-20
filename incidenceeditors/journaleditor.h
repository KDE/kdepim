/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef JOURNALEDITOR_H
#define JOURNALEDITOR_H

#include "incidenceeditors_export.h"
#include "incidenceeditor.h"

#include <kcalcore/journal.h>

class EditorGeneralJournal;

namespace Akonadi {
  class Item;
}

namespace IncidenceEditors
{

/**
  This class provides a dialog for editing a Journal.
*/
class INCIDENCEEDITORS_EXPORT JournalEditor : public IncidenceEditor
{
  Q_OBJECT
  public:
    /**
      Constructs a new Journal editor.
    */
    JournalEditor( QWidget *parent );
    virtual ~JournalEditor();

    void init();

    /**
      Clear editor for new Journal
    */
    void newJournal();

    /**
      Sets the given summary and description. If description is empty and the
      summary contains multiple lines, the summary will be used as description
      and only the first line of summary will be used as the summary.
      @param summary The summary of the new journal. If description is empty
      and summary contains newlines, the summary will only be the first line
      of the string.
      @param description The extensive contents of the new journal. If empty
      and summary contains line breaks, the summary will be used as description
      and only the first line will be the summary.
    */
    void setTexts( const QString &summary,
                   const QString &description = QString(),
                   bool richDescription = false );

    /** Set date widget to default values */
    void setDate( const QDate &date );

    /** Set time widget to default values */
    void setTime( const QTime &time );

    /** Write Journal settings to journal object */
    void fillJournal( KCalCore::Journal::Ptr &  );

    /** Check if the input is valid. */
    bool validateInput();

    /**
      Process user input and create or update event.
      Returns false if input is not valid.
    */
    bool processInput();

    /** This Journal has been modified externally */
    void modified();

  public slots:
    void show();

  protected slots:
    void loadDefaults();
    void deleteJournal();

  protected:
    KCalCore::Incidence::IncidenceType type() const { return KCalCore::Incidence::TypeJournal; }
    QByteArray typeStr() const { return "Journal"; }

    /**
      Read journal object and setup widgets accordingly. If tmpl is true, the
      journal is read as template, i.e. the time and date information isn't set.

      @param journal the journal from which the data should be used
      @param tmpl If true, the journal is treated as a template, so the
      currently set time is preserved in the editor dialog.
    */
    bool read( const Akonadi::Item &journal, const QDate &date, bool tmpl = false );

    void setupGeneral();
    bool incidenceModified();

  private:
    // Journal which represents the initial dialog setup when creating a new journal.
    // If cancel is pressed and the dialog has different information than
    // this journal then the user will be asked if he really wants to cancel
    KCalCore::Journal::Ptr mInitialJournal;

    EditorGeneralJournal *mGeneral;
};

} // namespace IncidenceEditors

#endif
