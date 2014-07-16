/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef NOTESAGENTNOTEDIALOG_H
#define NOTESAGENTNOTEDIALOG_H

#include <KDialog>
#include <Akonadi/Item>
class QTextEdit;
class QLineEdit;
class KJob;
namespace PimCommon {
class RichTextEditorWidget;
}
class NotesAgentNoteDialog : public KDialog
{
    Q_OBJECT
public:
    explicit NotesAgentNoteDialog(QWidget *parent = 0);
    ~NotesAgentNoteDialog();

    void setNoteId(Akonadi::Item::Id id);

private slots:
    void slotFetchItem(KJob *job);

private:
    void readConfig();
    void writeConfig();
    PimCommon::RichTextEditorWidget *mNote;
    QLineEdit *mSubject;
};

#endif // NOTESAGENTNOTEDIALOG_H
