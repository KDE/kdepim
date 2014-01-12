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


#include "notesagentnotedialog.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KLocalizedString>
#include <KSharedConfig>

#include <KMime/KMimeMessage>

#include <QLineEdit>
#include <QVBoxLayout>
#include <QTextEdit>


NotesAgentNoteDialog::NotesAgentNoteDialog(QWidget *parent)
    : KDialog(parent)
{
    setButtons(Close);
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);

    mSubject = new QLineEdit;
    mSubject->setReadOnly(true);
    vbox->addWidget(mSubject);

    mNote = new QTextEdit;
    mNote->setReadOnly(true);
    vbox->addWidget(mNote);
    setMainWidget(w);
    readConfig();
}

NotesAgentNoteDialog::~NotesAgentNoteDialog()
{
    writeConfig();
}

void NotesAgentNoteDialog::setNoteId(Akonadi::Item::Id id)
{
    Akonadi::Item item(id);
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
    job->fetchScope().fetchFullPayload( true );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotFetchItem(KJob*)) );
}

void NotesAgentNoteDialog::slotFetchItem(KJob* job)
{
    if ( job->error() ) {
        qDebug()<<"fetch item failed "<<job->errorString();
        return;
    }
    Akonadi::ItemFetchJob *itemFetchJob = static_cast<Akonadi::ItemFetchJob *>(job);
    const Akonadi::Item::List lstItem = itemFetchJob->items();
    if (!lstItem.isEmpty()) {
        KMime::Message::Ptr noteMessage = lstItem.first().payload<KMime::Message::Ptr>();
        if (noteMessage) {
            mSubject->setText(noteMessage->subject(false)->asUnicodeString());
            if ( noteMessage->contentType()->isHTMLText() ) {
                mNote->setAcceptRichText(true);
                mNote->setHtml(noteMessage->mainBodyPart()->decodedText());
            } else {
                mNote->setAcceptRichText(false);
                mNote->setPlainText(noteMessage->mainBodyPart()->decodedText());
            }
        }
    }
}

void NotesAgentNoteDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "NotesAgentNoteDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void NotesAgentNoteDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "NotesAgentNoteDialog" );
    grp.writeEntry( "Size", size() );
    grp.sync();
}
