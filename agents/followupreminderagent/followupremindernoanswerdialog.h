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

#ifndef FOLLOWUPREMINDERNOANSWERDIALOG_H
#define FOLLOWUPREMINDERNOANSWERDIALOG_H

#include <KDialog>

class FollowUpReminderInfo;
class QTreeWidget;
class FollowUpReminderNoAnswerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FollowUpReminderNoAnswerWidget(QWidget *parent=0);
    ~FollowUpReminderNoAnswerWidget();

    void restoreTreeWidgetHeader(const QByteArray &data);
    void saveTreeWidgetHeader(KConfigGroup &group);

    void setInfo(const QList<FollowUpReminderInfo *> &info);
private slots:
    void customContextMenuRequested(const QPoint &pos);
    void slotRemoveItem();

private:
    enum FollowUpReminderColumn {
        To = 0,
        Subject,
        MessageId,
        Date
    };
    QTreeWidget *mTreeWidget;
};

class FollowUpReminderNoAnswerDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FollowUpReminderNoAnswerDialog(QWidget *parent);
    ~FollowUpReminderNoAnswerDialog();

    void setInfo(const QList<FollowUpReminderInfo *> &info);

private:
    void readConfig();
    void writeConfig();
    FollowUpReminderNoAnswerWidget *mWidget;
};

#endif // FOLLOWUPREMINDERNOANSWERDIALOG_H
