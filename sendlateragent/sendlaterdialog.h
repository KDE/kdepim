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

class KComboBox;
class QCheckBox;
class QSpinBox;
class QDateTimeEdit;

class SendLaterInfo;

class SendLaterDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SendLaterDialog(SendLaterInfo *info, QWidget *parent = 0);
    ~SendLaterDialog();

    SendLaterInfo *info();

private Q_SLOTS:
    void slotSendLater();
    void slotSendNow();
    void slotRecursiveClicked(bool);
    void slotSendIn30Minutes();
    void slotSendIn1Hour();
    void slotSendIn2Hours();

private:
    void load(SendLaterInfo *info);
    void readConfig();
    void writeConfig();

private:
    QDateTime mSendDateTime;
    QDateTimeEdit *mDateTime;
    SendLaterInfo *mInfo;
    KComboBox *mRecursiveComboBox;
    QCheckBox *mRecursive;
    QSpinBox *mRecursiveValue;
    KPushButton *mSendIn30Minutes;
    KPushButton *mSendIn1Hour;
    KPushButton *mSendIn2Hours;
};

#endif // SENDLATERDIALOG_H
