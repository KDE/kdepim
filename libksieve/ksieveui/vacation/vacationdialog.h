/*  -*- c++ -*-
    vacationdialog.h

    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KSIEVE_KSIEVEUI_VACATIONDIALOG_H
#define KSIEVE_KSIEVEUI_VACATIONDIALOG_H

#include <QDialog>

template <typename T> class QList;

namespace KMime
{
namespace Types
{
struct AddrSpec;
typedef QList<AddrSpec> AddrSpecList;
}
}

namespace KSieveUi
{
class VacationEditWidget;
class VacationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VacationDialog(const QString &caption, QWidget *parent = 0,
                            bool modal = true);
    ~VacationDialog();

    void enableDomainAndSendForSpam(bool enable = true);

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

    bool sendForSpam() const;
    void setSendForSpam(bool enable);

Q_SIGNALS:
    void okClicked();
    void cancelClicked();

private slots:
    void slotDialogDefaults();

    void slotAccepted();
    void slotRejected();
private:
    void writeConfig();
    void readConfig();

    VacationEditWidget *mVacationEditWidget;
};

}

#endif
