/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "notesagentnotedialog.h"
#include "notesagent_debug.h"
#include <ItemFetchJob>
#include <ItemFetchScope>
#include "attributes/notedisplayattribute.h"
#include "kpimtextedit/richtexteditorwidget.h"
#include "kpimtextedit/richtexteditor.h"

#include <KSharedConfig>

#include <KMime/KMimeMessage>

#include <QIcon>

#include <QLineEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>

NotesAgentNoteDialog::NotesAgentNoteDialog(QWidget *parent)
    : QDialog(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &NotesAgentNoteDialog::reject);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("knotes")));
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);

    mSubject = new QLineEdit;
    mSubject->setReadOnly(true);
    vbox->addWidget(mSubject);

    mNote = new KPIMTextEdit::RichTextEditorWidget;
    mNote->setReadOnly(true);
    vbox->addWidget(mNote);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

NotesAgentNoteDialog::~NotesAgentNoteDialog()
{
    writeConfig();
}

void NotesAgentNoteDialog::setNoteId(Akonadi::Item::Id id)
{
    Akonadi::Item item(id);
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item, this);
    job->fetchScope().fetchFullPayload(true);
    job->fetchScope().fetchAttribute< NoteShared::NoteDisplayAttribute >();
    connect(job, &Akonadi::ItemFetchJob::result, this, &NotesAgentNoteDialog::slotFetchItem);
}

void NotesAgentNoteDialog::slotFetchItem(KJob *job)
{
    if (job->error()) {
        qCDebug(NOTESAGENT_LOG) << "fetch item failed " << job->errorString();
        return;
    }
    Akonadi::ItemFetchJob *itemFetchJob = static_cast<Akonadi::ItemFetchJob *>(job);
    const Akonadi::Item::List lstItem = itemFetchJob->items();
    if (!lstItem.isEmpty()) {
        const Akonadi::Item item = lstItem.first();
        KMime::Message::Ptr noteMessage = item.payload<KMime::Message::Ptr>();
        if (noteMessage) {
            const KMime::Headers::Subject *const subject = noteMessage->subject(false);
            if (subject) {
                mSubject->setText(subject->asUnicodeString());
            }
            if (noteMessage->contentType()->isHTMLText()) {
                mNote->setAcceptRichText(true);
                mNote->setHtml(noteMessage->mainBodyPart()->decodedText());
            } else {
                mNote->setAcceptRichText(false);
                mNote->setPlainText(noteMessage->mainBodyPart()->decodedText());
            }
        }
        if (item.hasAttribute<NoteShared::NoteDisplayAttribute>()) {
            NoteShared::NoteDisplayAttribute *attr = item.attribute<NoteShared::NoteDisplayAttribute>();
            if (attr) {
                mNote->editor()->setTextColor(attr->backgroundColor());
                //TODO add background color.
            }
        }
    }
}

void NotesAgentNoteDialog::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "NotesAgentNoteDialog");
    const QSize size = grp.readEntry("Size", QSize(300, 200));
    if (size.isValid()) {
        resize(size);
    }
}

void NotesAgentNoteDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "NotesAgentNoteDialog");
    grp.writeEntry("Size", size());
    grp.sync();
}
