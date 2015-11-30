/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#ifndef CREATENOTEJOB_H
#define CREATENOTEJOB_H

#include "messageviewer_export.h"
#include <KJob>
#include <AkonadiCore/Item>
#include <AkonadiCore/Collection>
#include <Akonadi/Notes/NoteUtils>

#include <kmime/kmime_message.h>

#include <QObject>

namespace MessageViewer
{
class MESSAGEVIEWER_EXPORT CreateNoteJob : public KJob
{
    Q_OBJECT
public:
    explicit CreateNoteJob(const KMime::Message::Ptr &notePtr, const Akonadi::Collection &collection, const Akonadi::Item &item, QObject *parent = Q_NULLPTR);
    ~CreateNoteJob();

    void start() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void noteCreated(KJob *job);
    void noteUpdated(KJob *job);
    void relationCreated(KJob *job);

private:
    void createNote();
    Akonadi::Item mItem;
    Akonadi::Collection mCollection;
    Akonadi::NoteUtils::NoteMessageWrapper mNote;
};
}

#endif // CREATETODOJOB_H
