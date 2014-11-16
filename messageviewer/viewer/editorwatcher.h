/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MESSAGEVIEWER_EDITORWATCHER_H
#define MESSAGEVIEWER_EDITORWATCHER_H

#include "messageviewer_export.h"
#include <qurl.h>

#include <qobject.h>
#include <qtimer.h>
#include <QTime>
class KProcess;

namespace MessageViewer
{

/**
  Starts an editor for the given URL and emits an signal when
  editing has been finished. Both, the editor process as well
  as the edited file are watched to work with as many as possible
  editors.
*/
class MESSAGEVIEWER_EXPORT EditorWatcher : public QObject
{
    Q_OBJECT
public:
    enum OpenWithOption {
        OpenWithDialog,
        NoOpenWithDialog
    };
    /**
     * Constructs an EditorWatcher.
     * @param parent the parent object of this EditorWatcher, which will take care of deleting
     *               this EditorWatcher if the parent is deleted.
     * @param parentWidget the parent widget of this EditorWatcher, which will be used as the parent
     *                     widget for message dialogs.
     */
    EditorWatcher( const KUrl &url, const QString &mimeType, OpenWithOption option,
                   QObject *parent, QWidget *parentWidget );

    bool start();
    bool fileChanged() const;
    QUrl url() const;
signals:
    void editDone(MessageViewer::EditorWatcher *watcher);

private slots:
    void editorExited();
    void inotifyEvent();
    void checkEditDone();

private:
    QUrl mUrl;
    QString mMimeType;
    QTimer mTimer;
    QTime mEditTime;

    int mInotifyFd;
    int mInotifyWatch;

    KProcess *mEditor;
    QWidget *mParentWidget;

    OpenWithOption mOpenWithOption;
    bool mHaveInotify;
    bool mFileOpen;
    bool mEditorRunning;
    bool mFileModified;
    bool mDone;
};

}

#endif
