/*
 * Copyright 2010 Thomas McGuire <mcguire@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef INSERTTEXTFILEJOB_H
#define INSERTTEXTFILEJOB_H

#include "messagecomposer_export.h"

#include <KJob>
#include <QUrl>

#include <QPointer>

class QTextEdit;

namespace KIO
{
class Job;
}

namespace MessageComposer
{

/**
 * A job that downloads a given URL, interprets the result as a text file with the
 * given encoding and then inserts the text into the editor.
 */
class InsertTextFileJobPrivate;
class MESSAGECOMPOSER_EXPORT InsertTextFileJob : public KJob
{
    Q_OBJECT

public:
    InsertTextFileJob(QTextEdit *editor, const QUrl &url);
    ~InsertTextFileJob();

    void setEncoding(const QString &encoding);

    void start() Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotGetJobFinished(KJob *job);
    void slotFileData(KIO::Job *job, const QByteArray &data);

private:
    InsertTextFileJobPrivate *const d;
};

}

#endif
