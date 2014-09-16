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

#ifndef FOLLOWUPREMINDERINFOWIDGET_H
#define FOLLOWUPREMINDERINFOWIDGET_H

#include <QWidget>
#include <KConfigGroup>
#include <QTreeWidgetItem>
#include <Akonadi/Item>
class QTreeWidget;
namespace FollowUpReminder
{
class FollowUpReminderInfo;
}

class FollowUpReminderInfoItem : public QTreeWidgetItem
{
public:
    explicit FollowUpReminderInfoItem(QTreeWidget *parent = 0);
    ~FollowUpReminderInfoItem();

    void setInfo(FollowUpReminder::FollowUpReminderInfo *info);
    FollowUpReminder::FollowUpReminderInfo *info() const;

private:
    FollowUpReminder::FollowUpReminderInfo *mInfo;
};

class FollowUpReminderInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FollowUpReminderInfoWidget(QWidget *parent = 0);
    ~FollowUpReminderInfoWidget();

    void restoreTreeWidgetHeader(const QByteArray &data);
    void saveTreeWidgetHeader(KConfigGroup &group);

    void setInfo(const QList<FollowUpReminder::FollowUpReminderInfo *> &infoList);

    void save();

private slots:
    void customContextMenuRequested(const QPoint &pos);

private:
    void removeItem(FollowUpReminderInfoItem *mailItem);
    void openShowMessage(Akonadi::Item::Id id);
    enum ItemData {
        AnswerItemId = Qt::UserRole + 1,
        AnswerItemFound = Qt::UserRole + 2
    };

    enum FollowUpReminderColumn {
        To = 0,
        Subject,
        DeadLine,
        AnswerWasReceived,
        MessageId
    };
    QTreeWidget *mTreeWidget;
    bool mChanged;
};

#endif // FOLLOWUPREMINDERINFOWIDGET_H
