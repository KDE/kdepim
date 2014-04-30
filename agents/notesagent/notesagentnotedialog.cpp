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

#include <ItemFetchJob>
#include <ItemFetchScope>
#include "noteshared/attributes/notedisplayattribute.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <KMime/KMimeMessage>

#include <KIcon>
#include <KGlobal>

#include <QLineEdit>
#include <QVBoxLayout>
#include <QTextEdit>


NotesAgentNoteDialog::NotesAgentNoteDialog(QWidget *parent)
    : KDialog(parent)
{
    setButtons(Close);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowIcon( KIcon( QLatin1String("knotes") ) );
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);

    mSubject = new QLineEdit;
    mSubject->setReadOnly(true);
    vbox->addWidget(mSubject);

    mNote = new PimCommon::RichTextEditorWidget;
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
    job->fetchScope().fetchAttribute< NoteShared::NoteDisplayAttribute >();
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
        const Akonadi::Item item = lstItem.first();
        KMime::Message::Ptr noteMessage = item.payload<KMime::Message::Ptr>();
        if (noteMessage) {
            const KMime::Headers::Subject * const subject = noteMessage->subject(false);
            if (subject)
                mSubject->setText(subject->asUnicodeString());
            if ( noteMessage->contentType()->isHTMLText() ) {
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
                mNote->editor()->setTextColor( attr->backgroundColor() );
                //TODO add background color.
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
