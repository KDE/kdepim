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

#ifndef VACATIONEDITWIDGET_H
#define VACATIONEDITWIDGET_H

#include <QWidget>
#include "vacationutils.h"
class KDateComboBox;
class KTimeComboBox;

class QComboBox;
class QDate;
class QTime;

class QLabel;
class QSpinBox;
class QLineEdit;
class KDateComboBox;

class QDate;

namespace KPIMTextEdit
{
class PlainTextEditorWidget;
}

class QCheckBox;

namespace KMime
{
namespace Types
{
struct AddrSpec;
typedef QVector<AddrSpec> AddrSpecList;
}
}

namespace KSieveUi
{
class VacationEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VacationEditWidget(QWidget *parent = Q_NULLPTR);
    ~VacationEditWidget();

    void enableDomainAndSendForSpam(bool enable = true);
    void enableDates(bool enable = true);

    bool activateVacation() const;
    void setActivateVacation(bool activate);

    bool domainCheck() const;
    void setDomainCheck(bool check);

    QString messageText() const;
    void setMessageText(const QString &text);

    int notificationInterval() const;
    void setNotificationInterval(int days);

    KMime::Types::AddrSpecList mailAliases() const;
    void setMailAliases(const KMime::Types::AddrSpecList &aliases);
    void setMailAliases(const QString &aliases);

    QString domainName() const;
    void setDomainName(const QString &domain);

    QString subject() const;
    void setSubject(const QString &subject);

    bool sendForSpam() const;
    void setSendForSpam(bool enable);

    QDate startDate() const;
    void setStartDate(const QDate &startDate);

    QTime startTime() const;
    void setStartTime(const QTime &startTime);

    QDate endDate() const;
    void setEndDate(const QDate &endDate);

    QTime endTime() const;
    void setEndTime(const QTime &endTime);

    VacationUtils::MailAction mailAction() const;
    QString mailActionRecipient() const;
    void setMailAction(VacationUtils::MailAction action, const QString &recipient);

    void setDefault();

private Q_SLOTS:
    void slotIntervalSpinChanged(int value);
    void mailActionChanged(int index);

protected:
    QCheckBox *mActiveCheck;
    QSpinBox *mIntervalSpin;
    QLineEdit *mMailAliasesEdit;
    KPIMTextEdit::PlainTextEditorWidget *mTextEdit;
    QCheckBox *mSpamCheck;
    QCheckBox *mDomainCheck;
    QLineEdit *mDomainEdit;
    QLineEdit *mSubject;
    QComboBox *mMailAction;
    QLineEdit *mMailActionRecipient;
    KDateComboBox *mStartDate;
    KTimeComboBox *mStartTime;
    QCheckBox *mStartTimeActive;
    QLabel *mStartDateLabel;
    KDateComboBox *mEndDate;
    KTimeComboBox *mEndTime;
    QCheckBox *mEndTimeActive;
    QLabel *mEndDateLabel;
};
}

#endif // VACATIONEDITWIDGET_H
