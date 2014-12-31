/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef ATTACHMENTEDITJOB_H
#define ATTACHMENTEDITJOB_H

#include <QObject>
#include <AkonadiCore/Item>
#include <QHash>
#include <kmime/kmime_message.h>
namespace KMime
{
class Content;
}
namespace MessageViewer
{
class EditorWatcher;
class AttachmentEditJob : public QObject
{
    Q_OBJECT
public:
    explicit AttachmentEditJob(QObject *parent = Q_NULLPTR);
    ~AttachmentEditJob();

    void setMainWindow(QWidget *mainWindow);

    void setMessageItem(const Akonadi::Item &messageItem);

    bool addAttachment(KMime::Content *node, bool showWarning);

    void canDeleteJob();

    void setMessage(const KMime::Message::Ptr &message);

Q_SIGNALS:
    void refreshMessage(const Akonadi::Item &item);
private Q_SLOTS:
    void slotAttachmentEditDone(MessageViewer::EditorWatcher *editorWatcher);
    void slotItemModifiedResult(KJob *);
private:
    void removeEditorWatcher(MessageViewer::EditorWatcher *editorWatcher, const QString &name);
    QHash<EditorWatcher *, KMime::Content *> mEditorWatchers;
    Akonadi::Item mMessageItem;
    KMime::Message::Ptr mMessage;
    bool mShowWarning;
    QWidget *mMainWindow;
};
}

#endif // ATTACHMENTEDITJOB_H
