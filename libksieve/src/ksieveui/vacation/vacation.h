/*  -*- c++ -*-
    vacation.cpp

    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KSIEVE_KSIEVEUI_VACATION_H
#define KSIEVE_KSIEVEUI_VACATION_H

#include <QUrl>

#include <QObject>

class QString;

namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{

class VacationDialog;

class Vacation : public QObject
{
    Q_OBJECT

public:
    explicit Vacation(QObject *parent = Q_NULLPTR, bool checkonly = false, const QUrl &url = QUrl());
    virtual ~Vacation();

    bool isUsable() const;
    QString serverName() const;

    void showVacationDialog();

protected:
    QUrl findURL(QString &serverName) const;
    void handlePutResult(KManageSieve::SieveJob *job, bool success, bool);

Q_SIGNALS:
    void result(bool success);
    // indicates if the vacation script is active or not
    void scriptActive(bool active, const QString &serverName);
    void requestEditVacation();

protected Q_SLOTS:
    void slotGetResult(KManageSieve::SieveJob *job, bool success,
                       const QString &script, bool active);
    void slotDialogOk();
    void slotDialogCancel();
    void slotPutActiveResult(KManageSieve::SieveJob *, bool);
    void slotPutInactiveResult(KManageSieve::SieveJob *, bool);
protected:
    // IO:
    KManageSieve::SieveJob *mSieveJob;
    QUrl mUrl;
    QString mServerName;
    // GUI:
    VacationDialog *mDialog;
    bool mWasActive;
    bool mCheckOnly;
};

}

#endif
