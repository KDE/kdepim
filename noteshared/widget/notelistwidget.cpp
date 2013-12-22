/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "notelistwidget.h"
#include <KMime/KMimeMessage>

using namespace NoteShared;
NoteListWidget::NoteListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

NoteListWidget::~NoteListWidget()
{

}

void NoteListWidget::setNotes(const Akonadi::Item::List &notes)
{
    mNotes = notes;
    Q_FOREACH (const Akonadi::Item &note, mNotes) {
        QListWidgetItem *item =new QListWidgetItem(this);
        KMime::Message::Ptr noteMessage = note.payload<KMime::Message::Ptr>();
        if (!noteMessage)
            continue;
        item->setText(noteMessage->subject(false)->asUnicodeString());
        //TODO
        /*
        if ( noteMessage->contentType()->isHTMLText() ) {
            m_editor->setAcceptRichText(true);
            m_editor->setHtml(noteMessage->mainBodyPart()->decodedText());
        } else {
            m_editor->setAcceptRichText(false);
            m_editor->setPlainText(noteMessage->mainBodyPart()->decodedText());
        }
        */
        //item->setToolTip(i.value()->text());
        item->setData(AkonadiId, note.id());
    }
}

QStringList NoteListWidget::selectedNotes() const
{
    QStringList lst;
    Q_FOREACH(QListWidgetItem *item, selectedItems()) {
        Akonadi::Item::Id akonadiId = item->data(AkonadiId).toLongLong();
        if (akonadiId != -1) {
            lst.append(QString::number(akonadiId));
        }
    }
    return lst;
}
