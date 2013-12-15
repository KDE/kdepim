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

#include "createnewnotejob.h"
#include "notesharedglobalconfig.h"

#include "dialog/selectednotefolderdialog.h"

#include "akonadi_next/note.h"

#include <Akonadi/Collection>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Item>

#include <KMime/KMimeMessage>

#include <KMessageBox>
#include <KLocale>
#include <KLocalizedString>

#include <QPointer>

using namespace NoteShared;

CreateNewNoteJob::CreateNewNoteJob(QObject *parent, QWidget *widget)
    : QObject(parent),
      mRichText(false),
      mWidget(widget)
{
}

CreateNewNoteJob::~CreateNewNoteJob()
{
}

void CreateNewNoteJob::setNote(const QString &name, const QString &text)
{
    mTitle = name;
    mText = text;
}

void CreateNewNoteJob::setRichText(bool richText)
{
    mRichText = richText;
}

void CreateNewNoteJob::start()
{
    Akonadi::Collection col;
    Akonadi::Collection::Id id = NoteShared::NoteSharedGlobalConfig::self()->defaultFolder();
    if (id == -1) {
        QPointer<SelectedNotefolderDialog> dlg = new SelectedNotefolderDialog(mWidget);
        if (dlg->exec()) {
            col = dlg->selectedCollection();
        }
        if (dlg->useFolderByDefault()) {
            NoteShared::NoteSharedGlobalConfig::self()->setDefaultFolder(col.id());
            NoteShared::NoteSharedGlobalConfig::self()->writeConfig();
        }
        delete dlg;
    } else {
        col = Akonadi::Collection(id);
    }

    if (col.isValid()) {
        Akonadi::Item newItem;
        newItem.setMimeType( Akonotes::Note::mimeType() );

        KMime::Message::Ptr newPage = KMime::Message::Ptr( new KMime::Message() );

        QString title;
        if (mTitle.isEmpty()) {
            title = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() );
        } else {
            title = mTitle;
        }
        QByteArray encoding( "utf-8" );

        newPage->subject( true )->fromUnicodeString( title, encoding );
        newPage->contentType( true )->setMimeType( mRichText ? "text/html" : "text/plain" );
        newPage->contentType()->setCharset("utf-8");
        newPage->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEquPr);
        newPage->date( true )->setDateTime( KDateTime::currentLocalDateTime() );
        newPage->from( true )->fromUnicodeString( QString::fromLatin1( "knotes@kde4" ), encoding );
        // Need a non-empty body part so that the serializer regards this as a valid message.
        newPage->mainBodyPart()->fromUnicodeString( mText.isEmpty() ? QString::fromLatin1( " " ) : mText);

        newPage->assemble();

        newItem.setPayload( newPage );

        Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();


        eda->setIconName( QString::fromLatin1( "text-plain" ) );
        newItem.addAttribute(eda);

        Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( newItem, col, this );
        connect( job, SIGNAL(result(KJob*)), SLOT(slotNoteCreationFinished(KJob*)) );
    } else {
        deleteLater();
    }
}

void CreateNewNoteJob::slotNoteCreationFinished(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        NoteShared::NoteSharedGlobalConfig::self()->setDefaultFolder(-1);
        NoteShared::NoteSharedGlobalConfig::self()->writeConfig();
        KMessageBox::error(mWidget, i18n("Note was not created."), i18n("Create new note"));
    }
    deleteLater();
}
