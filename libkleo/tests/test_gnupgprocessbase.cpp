/*
    gnupgviewer.cpp

    This file is part of libkleopatra's test suite.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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

#include "gnupgviewer.h"

#include "libkleo/backends/qgpgme/gnupgprocessbase.h"

#include <KAboutData>

#include <kmessagebox.h>
#include <QDebug>

#include <QStringList>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

GnuPGViewer::GnuPGViewer(QWidget *parent)
    : QTextEdit(parent), mProcess(0)
{
    setAcceptRichText(false);
    document()->setMaximumBlockCount(10000);
}

GnuPGViewer::~GnuPGViewer()
{
    if (mProcess) {
        mProcess->kill();
    }
}

void GnuPGViewer::setProcess(Kleo::GnuPGProcessBase *process)
{
    if (!process) {
        return;
    }
    mProcess = process;
    connect(mProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(slotProcessExited(int,QProcess::ExitStatus)));
    connect(mProcess, SIGNAL(readyReadStandardOutput()),
            SLOT(slotStdout()));
    connect(mProcess, SIGNAL(readyReadStandardError()),
            SLOT(slotStderr()));
    connect(mProcess, SIGNAL(status(Kleo::GnuPGProcessBase*,QString,QStringList)),
            SLOT(slotStatus(Kleo::GnuPGProcessBase*,QString,QStringList)));
}

static QStringList split(const QString &newLine, QString &old)
{
    // when done right, this would need to use QTextCodec...
    const QString str = old + newLine;
    QStringList l = str.split('\n');
    if (l.empty()) {
        return l;
    }
    if (str.endsWith('\n')) {
        old.clear();
    } else {
        old = l.back();
        l.pop_back();
    }
    return l;
}

static QString escape(QString str)
{
    return str.replace('&', "&amp").replace('<', "&lt;").replace('>', "&gt;");
}

void GnuPGViewer::slotStdout()
{
    QString line = mProcess-> readAllStandardOutput();
    const QStringList l = split(line, mLastStdout);
    for (QStringList::const_iterator it = l.begin() ; it != l.end() ; ++it) {
        append("stdout: " + escape(*it));
    }
}

void GnuPGViewer::slotStderr()
{
    QString line = mProcess->readAllStandardError();
    const QStringList l = split(line, mLastStderr);
    for (QStringList::const_iterator it = l.begin() ; it != l.end() ; ++it) {
        append("<b>stderr: " + escape(*it) + "</b>");
    }
}
void GnuPGViewer::slotStatus(Kleo::GnuPGProcessBase *, const QString &type, const QStringList &args)
{
    append("<b><font color=\"red\">status: " + escape(type + ' ' + args.join(" ")) + "</font></b>");
}
void GnuPGViewer::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        append(QString("<b>Process exit: return code %1</b>").arg(exitCode));
    } else {
        append("<b>Process exit: killed</b>");
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        qDebug() << "Need at least two arguments";
        return 1;
    }
    KAboutData aboutData(QLatin1String("test_gnupgprocessbase"), i18n("GnuPGProcessBase Test"), QLatin1String("0.1"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    Kleo::GnuPGProcessBase gpg;
    for (int i = 1 ; i < argc ; ++i) {
        gpg << argv[i];
    }

    gpg.setUseStatusFD(true);

    GnuPGViewer *gv = new GnuPGViewer();
    gv->setProcess(&gpg);

    gv->show();

    gpg.setOutputChannelMode(KProcess::SeparateChannels);
    gpg.start();

    return app.exec();
}

#include "moc_gnupgviewer.cpp"
