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

#include "ksieveui_export.h"

#include <QUrl>

#include <QtCore/QObject>

class QString;
template <typename T> class QList;

namespace KManageSieve
{
class SieveJob;
}

namespace KSieveUi
{

class VacationDialog;

class KSIEVEUI_EXPORT Vacation : public QObject
{
    Q_OBJECT

public:
    explicit Vacation(QObject *parent = 0, bool checkonly = false, const QUrl &url = QUrl());
    virtual ~Vacation();

    bool isUsable() const
    {
        return !mUrl.isEmpty();
    }
    QString serverName() const
    {
        return mServerName;
    }

    void showVacationDialog();

protected:
    QUrl findURL(QString &serverName) const;
    void handlePutResult(KManageSieve::SieveJob *job, bool success, bool);

signals:
    void result(bool success);
    // indicates if the vacation script is active or not
    void scriptActive(bool active, const QString &serverName);
    void requestEditVacation();

protected slots:
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
