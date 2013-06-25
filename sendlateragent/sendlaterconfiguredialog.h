/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SENDLATERCONFIGUREDIALOG_H
#define SENDLATERCONFIGUREDIALOG_H

#include <KDialog>
#include "ui_sendlaterwidget.h"


#include <Akonadi/Item>

#include <QTreeWidgetItem>

class KAboutData;
class SendLaterInfo;

class SendLaterItem : public QTreeWidgetItem
{
public:
    explicit SendLaterItem(QTreeWidget *parent = 0);
    ~SendLaterItem();

    void setInfo(SendLaterInfo *info);
    SendLaterInfo *info() const;

private:
    SendLaterInfo *mInfo;
};


class SendLaterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SendLaterWidget( QWidget *parent = 0 );
    ~SendLaterWidget();

    enum SendLaterColumn {
        Subject = 0,
        Date,
        Recursive,
        MessageId
    };

    void save();
    void saveTreeWidgetHeader(KConfigGroup &group);
    void restoreTreeWidgetHeader(const QByteArray &group);
    void needToReload();

private Q_SLOTS:
    void slotRemoveItem();
    void slotModifyItem();
    void updateButtons();
    void customContextMenuRequested(const QPoint &);
    void slotSendNow();

Q_SIGNALS:
    void sendNow(Akonadi::Item::Id);

private:
    void createOrUpdateItem(SendLaterInfo *info, SendLaterItem *item = 0);
    void load();
    bool mChanged;
    Ui::SendLaterWidget *mWidget;
};


class SendLaterConfigureDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SendLaterConfigureDialog(QWidget *parent = 0);
    ~SendLaterConfigureDialog();

public Q_SLOTS:
    void slotNeedToReloadConfig();

private Q_SLOTS:
    void slotSave();

Q_SIGNALS:
    void sendNow(Akonadi::Item::Id);

private:
    void readConfig();
    void writeConfig();
    KAboutData *mAboutData;
    SendLaterWidget *mWidget;
};

#endif // SENDLATERCONFIGUREDIALOG_H
