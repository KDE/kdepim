/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVETREEWIDGETITEM_H
#define SIEVETREEWIDGETITEM_H

#include <KPixmapSequence>
#include <QTreeWidgetItem>

#include "ksieveui_export.h"

class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

namespace KSieveUi
{
class SieveTreeWidgetProgress;
class SieveTreeWidgetItemPrivate;
class KSIEVEUI_EXPORT SieveTreeWidgetItem : public QTreeWidgetItem
{
public:
    SieveTreeWidgetItem(QTreeWidget *treeWidget, QTreeWidgetItem *item);
    ~SieveTreeWidgetItem();

    void startAnimation();
    void stopAnimation();
    void setDefaultIcon();
    void setProgressAnimation(const QPixmap &pix);

private:
    SieveTreeWidgetItemPrivate *const d;
};

class SieveTreeWidgetProgress : public QObject
{
    Q_OBJECT
public:
    explicit SieveTreeWidgetProgress(SieveTreeWidgetItem *item, QObject *parent = Q_NULLPTR);
    ~SieveTreeWidgetProgress();

    void startAnimation();
    void stopAnimation();

private Q_SLOTS:
    void slotTimerDone();

private:
    int mProgressCount;
    KPixmapSequence mProgressPix;
    QTimer *mProgressTimer;
    SieveTreeWidgetItem *mItem;
};
}

#endif // SIEVETREEWIDGETITEM_H
