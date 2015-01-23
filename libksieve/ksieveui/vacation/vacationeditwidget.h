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

#ifndef VACATIONEDITWIDGET_H
#define VACATIONEDITWIDGET_H

#include <QWidget>
#include "vacationutils.h"
class KIntSpinBox;
class KLineEdit;
class KDateComboBox;
class KTimeComboBox;

class QComboBox;
class QDate;
class QTime;

namespace PimCommon {
class RichTextEditorWidget;
}

class QCheckBox;
template <typename T> class QList;

namespace KMime {
namespace Types {
struct AddrSpec;
typedef QList<AddrSpec> AddrSpecList;
}
}

namespace KSieveUi {
class VacationEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VacationEditWidget(QWidget *parent=0);
    ~VacationEditWidget();

    void enableDates( bool enable = true );

    void enableDomainAndSendForSpam( bool enable = true );

    bool activateVacation() const;
    void setActivateVacation( bool activate );

    bool domainCheck() const;
    void setDomainCheck( bool check );

    QString messageText() const;
    void setMessageText( const QString &text );

    int notificationInterval() const;
    void setNotificationInterval( int days );

    KMime::Types::AddrSpecList mailAliases() const;
    void setMailAliases( const KMime::Types::AddrSpecList & aliases );
    void setMailAliases( const QString &aliases );

    QString domainName() const;
    void setDomainName( const QString &domain );

    QString subject() const;
    void setSubject(const QString &subject);

    bool sendForSpam() const;
    void setSendForSpam( bool enable );

    QDate startDate() const;
    void setStartDate( const QDate &startDate );

    QTime startTime() const;
    void setStartTime( const QTime &startTime );

    QDate endDate() const;
    void setEndDate( const QDate &endDate );

    QTime endTime() const;
    void setEndTime( const QTime &endTime );

    VacationUtils::MailAction mailAction() const;
    QString mailActionRecipient() const;
    void setMailAction(VacationUtils::MailAction action, const QString &recipient);

    void setDefault();

private Q_SLOTS:
    void slotIntervalSpinChanged( int value );
    void mailActionChanged(int index);

protected:
    QCheckBox *mActiveCheck;
    KIntSpinBox *mIntervalSpin;
    KLineEdit *mMailAliasesEdit;
    PimCommon::RichTextEditorWidget *mTextEdit;
    QCheckBox *mSpamCheck;
    QCheckBox *mDomainCheck;
    KLineEdit *mDomainEdit;
    KLineEdit *mSubject;
    QComboBox *mMailAction;
    KLineEdit *mMailActionRecipient;
    KDateComboBox *mStartDate;
    KTimeComboBox *mStartTime;
    QCheckBox *mStartTimeActive;
    KDateComboBox *mEndDate;
    KTimeComboBox *mEndTime;
    QCheckBox *mEndTimeActive;
};
}

#endif // VACATIONEDITWIDGET_H
