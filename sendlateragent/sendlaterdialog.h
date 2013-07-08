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
class QLabel;

namespace Ui {
class SendLaterWidget;
}

namespace SendLater {
class SendLaterInfo;
class SendLaterTimeDateWidget;
class SENDLATER_EXPORT SendLaterDialog : public KDialog
{
    Q_OBJECT
public:
    enum SendLaterAction {
        Unknown = 0,
        SendDeliveryAtTime = 1,
        Canceled = 2
    };

    explicit SendLaterDialog(SendLater::SendLaterInfo *info, QWidget *parent = 0);
    ~SendLaterDialog();

    SendLater::SendLaterInfo *info();

    SendLaterAction action() const;

private Q_SLOTS:
    void slotRecurrenceClicked(bool);
    void slotOkClicked();

private:
    void load(SendLater::SendLaterInfo *info);

private:
    QDateTime mSendDateTime;
    SendLaterAction mAction;
    Ui::SendLaterWidget *mSendLaterWidget;
    SendLater::SendLaterInfo *mInfo;
};
}
#endif // SENDLATERDIALOG_H
