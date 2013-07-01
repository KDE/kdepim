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

#ifndef SENDLATERDIALOG_H
#define SENDLATERDIALOG_H

#include <KDialog>
#include "sendlater_export.h"

class KComboBox;
class QCheckBox;
class QSpinBox;
class QDateTimeEdit;

namespace SendLater {
class SendLaterInfo;
class SENDLATER_EXPORT SendLaterDialog : public KDialog
{
    Q_OBJECT
public:
    enum SendLaterAction {
        Unknown = 0,
        SendNow = 1,
        SendLater = 2,
        Canceled = 3,
        SendDeliveryAtTime = 4
    };

    explicit SendLaterDialog(SendLater::SendLaterInfo *info, QWidget *parent = 0);
    ~SendLaterDialog();

    SendLater::SendLaterInfo *info();

    SendLaterAction action() const;

private Q_SLOTS:
    void slotSendLater();
    void slotSendNow();
    void slotRecurrenceClicked(bool);
    void slotSendIn30Minutes();
    void slotSendIn1Hour();
    void slotSendIn2Hours();
    void slotDateTimeChanged(const QDateTime &);
    void slotSendAtTime();

private:
    void load(SendLater::SendLaterInfo *info);
    void readConfig();
    void writeConfig();

private:
    QDateTime mSendDateTime;
    SendLaterAction mAction;
    QDateTimeEdit *mDateTime;
    SendLater::SendLaterInfo *mInfo;
    KComboBox *mRecurrenceComboBox;
    QCheckBox *mRecurrence;
    QSpinBox *mRecurrenceValue;
    KPushButton *mSendIn30Minutes;
    KPushButton *mSendIn1Hour;
    KPushButton *mSendIn2Hours;
    KPushButton *mSendAtTime;
};
}
#endif // SENDLATERDIALOG_H
