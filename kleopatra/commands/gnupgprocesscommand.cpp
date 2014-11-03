/* -*- mode: c++; c-basic-offset:4 -*-
    commands/gnupgprocesscommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "gnupgprocesscommand.h"

#include "command_p.h"

#include <utils/kdlogtextwidget.h>

#include <QDebug>
#include <KProcess>
#include <KLocalizedString>
#include <KWindowSystem>

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPointer>

static const int PROCESS_TERMINATE_TIMEOUT = 5000; // milliseconds

using namespace Kleo;
using namespace Kleo::Commands;

namespace
{

class OutputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OutputDialog(QWidget *parent = 0)
        : QDialog(parent),
          vlay(this),
          logTextWidget(this),
          buttonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Close, Qt::Horizontal, this)
    {
        KDAB_SET_OBJECT_NAME(vlay);
        KDAB_SET_OBJECT_NAME(logTextWidget);
        KDAB_SET_OBJECT_NAME(buttonBox);

        logTextWidget.setMinimumVisibleLines(20);
        logTextWidget.setMinimumVisibleColumns(80);

        vlay.addWidget(&logTextWidget, 1);
        vlay.addWidget(&buttonBox);

        connect(closeButton(), SIGNAL(clicked()), this, SLOT(close()));
        connect(cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
    }

Q_SIGNALS:
    void cancelRequested();

public Q_SLOTS:
    void message(const QString &s)
    {
        logTextWidget.message(s);
    }
    void setComplete(bool complete)
    {
        cancelButton()->setVisible(!complete);
    }

private Q_SLOTS:
    void slotCancelClicked()
    {
        cancelButton()->hide();
        emit cancelRequested();
    }

private:
    QAbstractButton *closeButton() const
    {
        return buttonBox.button(QDialogButtonBox::Close);
    }
    QAbstractButton *cancelButton() const
    {
        return buttonBox.button(QDialogButtonBox::Cancel);
    }

private:
    QVBoxLayout vlay;
    KDLogTextWidget logTextWidget;
    QDialogButtonBox buttonBox;
};

}

class GnuPGProcessCommand::Private : Command::Private
{
    friend class ::Kleo::Commands::GnuPGProcessCommand;
    GnuPGProcessCommand *q_func() const
    {
        return static_cast<GnuPGProcessCommand *>(q);
    }
public:
    explicit Private(GnuPGProcessCommand *qq, KeyListController *c);
    ~Private();

private:
    void init();
    void ensureDialogCreated()
    {
        if (!showsOutputWindow) {
            return;
        }
        if (!dialog) {
            dialog = new OutputDialog;
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            applyWindowID(dialog);
            connect(dialog, SIGNAL(cancelRequested()), q, SLOT(cancel()));
            dialog->setWindowTitle(i18n("Subprocess Diagnostics"));
        }
    }
    void ensureDialogVisible()
    {
        if (!showsOutputWindow) {
            return;
        }
        ensureDialogCreated();
        if (dialog->isVisible()) {
            dialog->raise();
        } else {
            dialog->show();
        }
#ifdef Q_OS_WIN
        KWindowSystem::forceActiveWindow(dialog->winId());
#endif
    }
    void message(const QString &msg)
    {
        if (dialog) {
            dialog->message(msg);
        } else {
            qDebug() << msg;
        }
    }

private:
    void slotProcessFinished(int, QProcess::ExitStatus);
    void slotProcessReadyReadStandardError();

private:
    KProcess process;
    QPointer<OutputDialog> dialog;
    QStringList arguments;
    QByteArray errorBuffer;
    bool ignoresSuccessOrFailure;
    bool showsOutputWindow;
    bool canceled;
};

GnuPGProcessCommand::Private *GnuPGProcessCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const GnuPGProcessCommand::Private *GnuPGProcessCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

GnuPGProcessCommand::Private::Private(GnuPGProcessCommand *qq, KeyListController *c)
    : Command::Private(qq, c),
      process(),
      dialog(),
      errorBuffer(),
      ignoresSuccessOrFailure(false),
      showsOutputWindow(false),
      canceled(false)
{
    process.setOutputChannelMode(KProcess::OnlyStderrChannel);
    process.setReadChannel(KProcess::StandardError);
}

GnuPGProcessCommand::Private::~Private() {}

GnuPGProcessCommand::GnuPGProcessCommand(KeyListController *c)
    : Command(new Private(this, c))
{
    d->init();
}

GnuPGProcessCommand::GnuPGProcessCommand(QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
}

GnuPGProcessCommand::GnuPGProcessCommand(const GpgME::Key &key)
    : Command(key, new Private(this, 0))
{
    d->init();
}

void GnuPGProcessCommand::Private::init()
{
    connect(&process, SIGNAL(finished(int,QProcess::ExitStatus)),
            q, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
    connect(&process, SIGNAL(readyReadStandardError()),
            q, SLOT(slotProcessReadyReadStandardError()));
}

GnuPGProcessCommand::~GnuPGProcessCommand() {}

QDialog *GnuPGProcessCommand::dialog() const
{
    return d->dialog;
}

bool GnuPGProcessCommand::preStartHook(QWidget *) const
{
    return true;
}

void GnuPGProcessCommand::postSuccessHook(QWidget *)
{

}

void GnuPGProcessCommand::doStart()
{

    if (!preStartHook(d->parentWidgetOrView())) {
        d->finished();
        return;
    }

    d->arguments = arguments();

    d->process << d->arguments;

    d->process.start();

    if (!d->process.waitForStarted()) {
        d->error(i18n("Unable to start process %1. "
                      "Please check your installation.", d->arguments[0]),
                 errorCaption());
        d->finished();
    } else {
        d->ensureDialogVisible();
        d->message(i18n("Starting %1...", d->arguments.join(QLatin1String(" "))));
    }
}

void GnuPGProcessCommand::doCancel()
{
    d->canceled = true;
    if (d->process.state() != QProcess::NotRunning) {
        d->process.terminate();
        QTimer::singleShot(PROCESS_TERMINATE_TIMEOUT, &d->process, SLOT(kill()));
    }
}

void GnuPGProcessCommand::Private::slotProcessFinished(int code, QProcess::ExitStatus status)
{
    if (!canceled)
        if (status == QProcess::CrashExit) {
            error(q->crashExitMessage(arguments), q->errorCaption());
        } else if (ignoresSuccessOrFailure) {
            if (dialog) {
                message(i18n("Process finished"));
            } else {
                ;
            }
        } else if (code) {
            error(q->errorExitMessage(arguments), q->errorCaption());
        } else {
            q->postSuccessHook(parentWidgetOrView());
            const QString successMessage = q->successMessage(arguments);
            if (dialog) {
                message(successMessage);
            } else {
                information(successMessage, q->successCaption());
            }
        }
    if (dialog) {
        dialog->setComplete(true);
    }
    finished();
}

void GnuPGProcessCommand::Private::slotProcessReadyReadStandardError()
{
    while (process.canReadLine()) {
        QByteArray ba = process.readLine();
        errorBuffer += ba;
        while (ba.endsWith('\n') || ba.endsWith('\r')) {
            ba.chop(1);
        }
        message(QString::fromLocal8Bit(ba.constData(), ba.size()));
    }
}

QString GnuPGProcessCommand::errorString() const
{
    return QString::fromLocal8Bit(d->errorBuffer);
}

void GnuPGProcessCommand::setIgnoresSuccessOrFailure(bool ignores)
{
    d->ignoresSuccessOrFailure = ignores;
}

bool GnuPGProcessCommand::ignoresSuccessOrFailure() const
{
    return d->ignoresSuccessOrFailure;
}

void GnuPGProcessCommand::setShowsOutputWindow(bool show)
{
    if (show == d->showsOutputWindow) {
        return;
    }
    d->showsOutputWindow = show;
    if (show) {
        d->ensureDialogCreated();
    } else {
        if (d->dialog) {
            d->dialog->deleteLater();
        }
        d->dialog = 0;
    }
}

bool GnuPGProcessCommand::showsOutputWindow() const
{
    return d->showsOutputWindow;
}

#undef d
#undef q

#include "moc_gnupgprocesscommand.cpp"
#include "gnupgprocesscommand.moc"
