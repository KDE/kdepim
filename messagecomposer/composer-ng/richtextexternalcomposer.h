/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef RICHTEXTEXTERNALCOMPOSER_H
#define RICHTEXTEXTERNALCOMPOSER_H

#include "messagecomposer_export.h"
#include <QObject>
#include <QProcess>

namespace MessageComposer
{
class MESSAGECOMPOSER_EXPORT RichTextExternalComposer : public QObject
{
public:
    explicit RichTextExternalComposer(QObject *parent = Q_NULLPTR);
    ~RichTextExternalComposer();

    bool useExtEditor() const;
    void setUseExtEditor(bool value);

    void startExternalEditor();

    void setExternalEditorPath(const QString &path);
    QString externalEditorPath() const;
    bool checkExternalEditorFinished();
    void killExternalEditor();
private:
    void slotEditorFinished(int codeError, QProcess::ExitStatus exitStatus);
    void cannotStartProcess(const QString &commandLine);
    class RichTextExternalComposerPrivate;
    RichTextExternalComposerPrivate *const d;
};
}

#endif // RICHTEXTEXTERNALCOMPOSER_H
