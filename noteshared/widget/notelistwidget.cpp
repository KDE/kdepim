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

void NoteListWidget::addNotes(const Akonadi::Item::List &notes)
{
    Q_FOREACH (const Akonadi::Item &note, notes) {
        if (mNotes.contains(note)) {
            continue;
        }
        createItem(note);
        mNotes.append(note);
    }
}

void NoteListWidget::removeNote(const Akonadi::Item &note)
{
    for (int i=0; i <count(); ++i) {
        if (item(i)->data(AkonadiId)==note.id()) {
            delete item(i);
            mNotes.removeAll(note);
            break;
        }
    }
}

void NoteListWidget::setNotes(const Akonadi::Item::List &notes)
{
    clear();
    mNotes = notes;
    Q_FOREACH (const Akonadi::Item &note, mNotes) {
        createItem(note);
    }
}

void NoteListWidget::createItem(const Akonadi::Item &note)
{
    KMime::Message::Ptr noteMessage = note.payload<KMime::Message::Ptr>();
    if (!noteMessage)
        return;
    QListWidgetItem *item =new QListWidgetItem(this);
    item->setText(noteMessage->subject(false)->asUnicodeString());

    QString text;
    if ( noteMessage->contentType()->isHTMLText() ) {
        text = noteMessage->mainBodyPart()->decodedText();
    } else {
        text = noteMessage->mainBodyPart()->decodedText().replace(QLatin1Char('\n'), QLatin1String("<br>"));
    }
    if (!text.trimmed().isEmpty()) {
        item->setToolTip(QLatin1String("<qt>") + text + QLatin1String("</qt>"));
    }
    item->setData(AkonadiId, note.id());
}

QStringList NoteListWidget::selectedNotes() const
{
    QStringList lst;
    Q_FOREACH (QListWidgetItem *item, selectedItems()) {
        Akonadi::Item::Id akonadiId = item->data(AkonadiId).toLongLong();
        if (akonadiId != -1) {
            lst.append(QString::number(akonadiId));
        }
    }
    return lst;
}

Akonadi::Entity::Id NoteListWidget::itemId(QListWidgetItem *item) const
{
    if (item) {
        return item->data(AkonadiId).toLongLong();
    }
    return -1;
}

Akonadi::Entity::Id NoteListWidget::currentItemId() const
{
    QListWidgetItem *item = currentItem();
    if (item) {
        return item->data(AkonadiId).toLongLong();
    }
    return -1;
}
