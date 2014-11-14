/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include <QDebug>
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
    const QString delvePath = QStandardPaths::findExecutable(QLatin1String("delve"));
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
        arguments << QLatin1String("-r") << mAkonadiId;
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
    mProcess = 0;
    deleteLater();
}

void BalooDebugSearchJob::slotReadError()
{
    const QByteArray errorStrg = mProcess->readAllStandardOutput();
    Q_EMIT error(QString::fromUtf8(errorStrg));
    mProcess->close();
    mProcess->deleteLater();
    mProcess = 0;
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

