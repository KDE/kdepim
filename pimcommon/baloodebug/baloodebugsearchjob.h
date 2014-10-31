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


#ifndef BALOODEBUGSEARCHJOB_H
#define BALOODEBUGSEARCHJOB_H

#include <QObject>
#include <QStringList>
namespace PimCommon {
class BalooDebugSearchJob : public QObject
{
    Q_OBJECT
public:
    explicit BalooDebugSearchJob(QObject *parent=0);
    ~BalooDebugSearchJob();

    void start();

    void setAkonadiId(const QString &id);
    void setArguments(const QStringList &args);
    void searchPath(const QString &path);

Q_SIGNALS:
    void error(const QString &errorString);
    void result(const QString &text);

private:
    QStringList mArguments;
    QString mAkonadiId;
    QString mPath;
};
}


#endif // BALOODEBUGSEARCHJOB_H

