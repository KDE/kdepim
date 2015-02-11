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

#include "baloodebugsearchjob.h"

#include <QProcess>
#include "pimcommon_debug.h"
#include <QStandardPaths>

using namespace PimCommon;
BalooDebugSearchJob::BalooDebugSearchJob(QObject *parent)
    : QObject(parent)
{

}

BalooDebugSearchJob::~BalooDebugSearchJob()
{

}

void BalooDebugSearchJob::start()
{
    const QString delvePath = QStandardPaths::findExecutable(QStringLiteral("delve"));
    if (delvePath.isEmpty()) {
        //Don't translate it. Just debug
        Q_EMIT error(QStringLiteral("\"delve\" not installed on computer."));
        deleteLater();
        return;
    } else {
        mProcess = new QProcess(this);
        connect(mProcess, &QProcess::readyReadStandardOutput, this, &BalooDebugSearchJob::slotReadStandard);
        connect(mProcess, &QProcess::readyReadStandardError, this, &BalooDebugSearchJob::slotReadError);
        mProcess->setWorkingDirectory(mPath);
        QStringList arguments;
        arguments << QStringLiteral("-r") << mAkonadiId;
        arguments << mPath;
        mProcess->start(delvePath, QStringList() << arguments);
    }
}

void BalooDebugSearchJob::slotReadStandard()
{
    const QByteArray stdStrg = mProcess->readAllStandardOutput();
    Q_EMIT result(QString::fromUtf8(stdStrg));
    mProcess->close();
    mProcess->deleteLater();
    mProcess = Q_NULLPTR;
    deleteLater();
}

void BalooDebugSearchJob::slotReadError()
{
    const QByteArray errorStrg = mProcess->readAllStandardOutput();
    Q_EMIT error(QString::fromUtf8(errorStrg));
    mProcess->close();
    mProcess->deleteLater();
    mProcess = Q_NULLPTR;
    deleteLater();
}

void BalooDebugSearchJob::setAkonadiId(const QString &id)
{
    mAkonadiId = id;
}

void BalooDebugSearchJob::setArguments(const QStringList &args)
{
    mArguments = args;
}

void BalooDebugSearchJob::setSearchPath(const QString &path)
{
    mPath = path;
}

