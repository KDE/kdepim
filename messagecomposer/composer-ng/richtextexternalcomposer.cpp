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

#include "richtextexternalcomposer.h"

#include <KProcess>
#include <KLocalizedString>
#include <QTemporaryFile>

using namespace MessageComposer;

class RichTextExternalComposer::RichTextExternalComposerPrivate
{
public:
    RichTextExternalComposerPrivate()
        : mExtEditorProcess(Q_NULLPTR),
          mExtEditorTempFile(Q_NULLPTR),
          useExtEditor(false)
    {

    }
    QString extEditorPath;
    KProcess *mExtEditorProcess;
    QTemporaryFile *mExtEditorTempFile;
    bool useExtEditor;
};

RichTextExternalComposer::RichTextExternalComposer(QObject *parent)
    : QObject(parent), d(new RichTextExternalComposerPrivate())
{

}

RichTextExternalComposer::~RichTextExternalComposer()
{
    delete d;
}

bool RichTextExternalComposer::useExtEditor() const
{
    return d->useExtEditor;
}

void RichTextExternalComposer::setUseExtEditor(bool value)
{
    d->useExtEditor = value;
}

void RichTextExternalComposer::setExternalEditorPath(const QString &path)
{
    d->extEditorPath = path;
}

QString RichTextExternalComposer::externalEditorPath() const
{
    return d->extEditorPath;
}

void RichTextExternalComposer::startExternalEditor()
{

#if 0
    if (d->useExtEditor && !d->mExtEditorProcess) {

        const QString commandLine = extEditorPath.trimmed();
        if (extEditorPath.isEmpty()) {
            q->setUseExternalEditor(false);
            KMessageBox::error(q, i18n("Command line is empty. Please verify settings."), i18n("Empty command line"));
            return;
        }

        mExtEditorTempFile = new QTemporaryFile();
        if (!mExtEditorTempFile->open()) {
            delete mExtEditorTempFile;
            mExtEditorTempFile = Q_NULLPTR;
            q->setUseExternalEditor(false);
            return;
        }

        mExtEditorTempFile->write(q->textOrHtml().toUtf8());
        mExtEditorTempFile->close();

        mExtEditorProcess = new KProcess();
        // construct command line...
        QHash<QChar, QString> map;
        map.insert(QLatin1Char('l'), QString::number(q->textCursor().blockNumber() + 1));
        map.insert(QLatin1Char('w'), QString::number((qulonglong)q->winId()));
        map.insert(QLatin1Char('f'), mExtEditorTempFile->fileName());
        const QString cmd = KMacroExpander::expandMacrosShellQuote(commandLine, map);
        const QStringList arg = KShell::splitArgs(cmd);
        bool filenameAdded = false;
        if (commandLine.contains(QLatin1String("%f"))) {
            filenameAdded = true;
        }
        QStringList command;
        if (!arg.isEmpty()) {
            command << arg;
        }
        if (command.isEmpty()) {
            cannotStartProcess(commandLine);
            return;
        }

        (*mExtEditorProcess) << command;
        if (!filenameAdded) {   // no %f in the editor command
            (*mExtEditorProcess) << mExtEditorTempFile->fileName();
        }

        QObject::connect(mExtEditorProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                         q, SLOT(slotEditorFinished(int,QProcess::ExitStatus)));
        mExtEditorProcess->start();
        if (!mExtEditorProcess->waitForStarted()) {
            cannotStartProcess(commandLine);
        } else {
            Q_EMIT q->externalEditorStarted();
        }
    }
#endif
}

void RichTextExternalComposer::cannotStartProcess(const QString &commandLine)
{
#if 0
    KMessageBox::error(q->topLevelWidget(), i18n("External editor cannot be started. Please verify command \"%1\"", commandLine));
    q->killExternalEditor();
    q->setUseExternalEditor(false);
#endif
}

void RichTextExternalComposer::slotEditorFinished(int codeError, QProcess::ExitStatus exitStatus)
{
#if 0
    if (exitStatus == QProcess::NormalExit) {
        // the external editor could have renamed the original file and recreated a new file
        // with the given filename, so we need to reopen the file after the editor exited
        QFile localFile(mExtEditorTempFile->fileName());
        if (localFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray f = localFile.readAll();
            q->setTextOrHtml(QString::fromUtf8(f.data(), f.size()));
            q->document()->setModified(true);
            localFile.close();
        }
        if (codeError > 0) {
            KMessageBox::error(q->topLevelWidget(), i18n("Error was found when we started external editor."), i18n("External Editor Closed"));
            q->setUseExternalEditor(false);
        }
        Q_EMIT q->externalEditorClosed();
    }

    q->killExternalEditor();   // cleanup...
#endif

}

bool RichTextExternalComposer::checkExternalEditorFinished()
{
#if 0
    if (!d->mExtEditorProcess) {
        return true;
    }

    int ret = KMessageBox::warningYesNoCancel(
                  topLevelWidget(),
                  xi18nc("@info",
                         "The external editor is still running.<nl/>"
                         "Do you want to stop the editor or keep it running?<nl/>"
                         "<warning>Stopping the editor will cause all your "
                         "unsaved changes to be lost.</warning>"),
                  i18nc("@title:window", "External Editor Running"),
                  KGuiItem(i18nc("@action:button", "Stop Editor")),
                  KGuiItem(i18nc("@action:button", "Keep Editor Running")));

    switch (ret) {
    case KMessageBox::Yes:
        killExternalEditor();
        return true;
    case KMessageBox::No:
        return true;
    default:
        return false;

    }
#else
    return false;
#endif
}

void RichTextExternalComposer::killExternalEditor()
{
#if 0
    if (d->mExtEditorProcess) {
        d->mExtEditorProcess->deleteLater();
    }
    d->mExtEditorProcess = Q_NULLPTR;
    delete d->mExtEditorTempFile;
    d->mExtEditorTempFile = Q_NULLPTR;
#endif
}
