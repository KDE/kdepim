/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef STORAGELISTWIDGETITEM_H
#define STORAGELISTWIDGETITEM_H
#include <QListWidgetItem>
#include <KPixmapSequence>
#include <QIcon>

class QTimer;
class QListWidget;
namespace PimCommon
{
class StorageListWidgetItemProgress;
class StorageListWidgetItem : public QListWidgetItem
{
public:
    explicit StorageListWidgetItem(QListWidget *parent = 0);
    ~StorageListWidgetItem();

    void startAnimation();
    void stopAnimation();
    void resetToDefaultIcon();
    void setProgressAnimation(const QPixmap &pix);
    void setDefaultIcon(const QString &defaultIconName);
    void setDefaultIcon(const QIcon &icon);

private:
    QIcon mDefaultIcon;
    StorageListWidgetItemProgress *mProgress;
};

class StorageListWidgetItemProgress : public QObject
{
    Q_OBJECT
public:
    explicit StorageListWidgetItemProgress(StorageListWidgetItem *item, QObject *parent = 0);
    ~StorageListWidgetItemProgress();

    void startAnimation();
    void stopAnimation();

private Q_SLOTS:
    void slotTimerDone();

private:
    int mProgressCount;
    KPixmapSequence mProgressPix;
    QTimer *mProgressTimer;
    StorageListWidgetItem *mItem;
};

}

#endif // STORAGELISTWIDGETITEM_H
