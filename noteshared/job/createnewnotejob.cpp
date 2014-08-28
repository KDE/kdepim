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

#include "createnewnotejob.h"
#include "notesharedglobalconfig.h"
#include "noteshared/attributes/showfoldernotesattribute.h"
#include "dialog/selectednotefolderdialog.h"

#include "akonadi_next/note.h"

#include <AkonadiCore/Collection>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/Item>
#include <AkonadiCore/CollectionModifyJob>
#include <AkonadiCore/CollectionFetchJob>

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
    connect(this, &CreateNewNoteJob::selectNewCollection, this, &CreateNewNoteJob::slotSelectNewCollection);
}

CreateNewNoteJob::~CreateNewNoteJob()
{
}

void CreateNewNoteJob::slotSelectNewCollection()
{
    createFetchCollectionJob(false);
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
    createFetchCollectionJob(true);
}

void CreateNewNoteJob::createFetchCollectionJob(bool useSettings)
{
    Akonadi::Collection col;
    Akonadi::Collection::Id id = -1;
    if (useSettings) {
        id = NoteShared::NoteSharedGlobalConfig::self()->defaultFolder();
    } else {
        NoteShared::NoteSharedGlobalConfig::self()->setDefaultFolder(id);
        NoteShared::NoteSharedGlobalConfig::self()->save();
    }
    if (id == -1) {
        QPointer<SelectedNotefolderDialog> dlg = new SelectedNotefolderDialog(mWidget);
        if (dlg->exec()) {
            col = dlg->selectedCollection();
        } else {
            deleteLater();
            return;
        }
        if (dlg->useFolderByDefault()) {
            NoteShared::NoteSharedGlobalConfig::self()->setDefaultFolder(col.id());
            NoteShared::NoteSharedGlobalConfig::self()->save();
        }
        delete dlg;
    } else {
        col = Akonadi::Collection(id);
    }
    Akonadi::CollectionFetchJob *fetchCollection = new Akonadi::CollectionFetchJob(col, Akonadi::CollectionFetchJob::Base);
    connect(fetchCollection, &Akonadi::CollectionFetchJob::result, this, &CreateNewNoteJob::slotFetchCollection);
}

void CreateNewNoteJob::slotFetchCollection(KJob *job)
{
    if (job->error()) {
        qDebug() << " Error during fetch: " << job->errorString();
        if (KMessageBox::Yes == KMessageBox::warningYesNo(0, i18n("An error occurred during fetching. Do you want select a new default collection?"))) {
            Q_EMIT selectNewCollection();
        } else {
            deleteLater();
        }
        return;
    }
    Akonadi::CollectionFetchJob *fetchCollection = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    if (fetchCollection->collections().isEmpty()) {
        qDebug() << "No collection fetched";
        if (KMessageBox::Yes == KMessageBox::warningYesNo(0, i18n("An error occurred during fetching. Do you want select a new default collection?"))) {
            Q_EMIT selectNewCollection();
        } else {
            deleteLater();
        }
        return;
    }
    Akonadi::Collection col = fetchCollection->collections().at(0);
    if (col.isValid()) {
        if (!col.hasAttribute<NoteShared::ShowFolderNotesAttribute>()) {
            if (KMessageBox::Yes == KMessageBox::warningYesNo(0, i18n("Collection is hidden. New note will stored but not displaying. Do you want to show collection?"))) {
                col.addAttribute(new NoteShared::ShowFolderNotesAttribute());
                Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob(col);
                connect(job, &Akonadi::CollectionModifyJob::result, this, &CreateNewNoteJob::slotCollectionModifyFinished);
            }
        }
        Akonadi::Item newItem;
        newItem.setMimeType(Akonotes::Note::mimeType());

        KMime::Message::Ptr newPage = KMime::Message::Ptr(new KMime::Message());

        QString title;
        if (mTitle.isEmpty()) {
            const QDateTime currentDateTime = QDateTime::currentDateTime();
            title = NoteShared::NoteSharedGlobalConfig::self()->defaultTitle();
            title.replace(QLatin1String("%t"), KLocale::global()->formatTime(currentDateTime.time()));
            title.replace(QLatin1String("%d"), KLocale::global()->formatDate(currentDateTime.date(), KLocale::ShortDate));
            title.replace(QLatin1String("%l"), KLocale::global()->formatDate(currentDateTime.date(), KLocale::LongDate));
        } else {
            title = mTitle;
        }
        QByteArray encoding("utf-8");

        newPage->subject(true)->fromUnicodeString(title, encoding);
        newPage->contentType(true)->setMimeType(mRichText ? "text/html" : "text/plain");
        newPage->contentType()->setCharset("utf-8");
        newPage->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEquPr);
        newPage->date(true)->setDateTime(QDateTime::currentDateTime());
        newPage->from(true)->fromUnicodeString(QString::fromLatin1("knotes@kde4"), encoding);
        // Need a non-empty body part so that the serializer regards this as a valid message.
        newPage->mainBodyPart()->fromUnicodeString(mText.isEmpty() ? QString::fromLatin1(" ") : mText);

        newPage->assemble();

        newItem.setPayload(newPage);

        Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();

        eda->setIconName(QString::fromLatin1("text-plain"));
        newItem.addAttribute(eda);

        Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(newItem, col, this);
        connect(job, &Akonadi::ItemCreateJob::result, this, &CreateNewNoteJob::slotNoteCreationFinished);
    } else {
        deleteLater();
    }
}

void CreateNewNoteJob::slotNoteCreationFinished(KJob *job)
{
    if (job->error()) {
        qWarning() << job->errorString();
        NoteShared::NoteSharedGlobalConfig::self()->setDefaultFolder(-1);
        NoteShared::NoteSharedGlobalConfig::self()->save();
        KMessageBox::error(mWidget, i18n("Note was not created."), i18n("Create new note"));
    }
    deleteLater();
}

void CreateNewNoteJob::slotCollectionModifyFinished(KJob *job)
{
    if (job->error()) {
        qWarning() << job->errorString();
    }
}
