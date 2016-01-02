/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#ifndef FULLSYNCHRONIZERESOURCESJOB_H
#define FULLSYNCHRONIZERESOURCESJOB_H

#include <QObject>
#include <QStringList>

class QWidget;
class QProgressDialog;
class FullSynchronizeResourcesJob : public QObject
{
    Q_OBJECT
public:
    explicit FullSynchronizeResourcesJob(QObject *parent = Q_NULLPTR);
    ~FullSynchronizeResourcesJob();

    void setResources(const QStringList &lst);

    void setWindowParent(QWidget *parent);

    void start();
Q_SIGNALS:
    void synchronizeFinished();
    void synchronizeInstanceDone(const QString &instance);
    void synchronizeInstanceFailed(const QString &instance);

private Q_SLOTS:
    void slotSynchronizeInstanceFailed(const QString &identifier);
    void slotSynchronizeInstanceDone(const QString &identifier);
    void slotSynchronizeFinished();
private:
    QStringList mResources;
    QWidget *mWindowParent;
    QProgressDialog *mProgressDialog;
};

#endif // FULLSYNCHRONIZERESOURCESJOB_H
