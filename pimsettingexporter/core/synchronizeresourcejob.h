/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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

#ifndef SYNCHRONIZERESOURCEJOB_H
#define SYNCHRONIZERESOURCEJOB_H

#include <QObject>
#include <QStringList>
#include "pimsettingexporter_export.h"
class KJob;
class PIMSETTINGEXPORTER_EXPORT SynchronizeResourceJob : public QObject
{
    Q_OBJECT
public:
    explicit SynchronizeResourceJob(QObject *parent = Q_NULLPTR);
    ~SynchronizeResourceJob();

    void start();
    void setListResources(const QStringList &resources);
    void setSynchronizeOnlyCollection(bool onlyCollection);

Q_SIGNALS:
    void synchronizationFinished();
    void synchronizationInstanceDone(const QString &name, const QString &identifier);
    void synchronizationInstanceFailed(const QString &);

private Q_SLOTS:
    void slotSynchronizationFinished(KJob *);
    void slotNextSync();

private:
    QStringList mListResources;
    int mIndex;
    bool mOnlyCollection;
};

#endif // SYNCHRONIZERESOURCEJOB_H
