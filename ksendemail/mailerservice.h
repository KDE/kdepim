/*
    This file is part of KSendEmail. Some of the code has been taken from KMail (kmkernel.cpp)
    and akonadi (control.h)
    Copyright (c) 2008 Pradeepto Bhattacharya <pradeepto@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef MAILERSERVICE_H
#define MAILERSERVICE_H

class QEventLoop;

#include <QObject>

#include <QCommandLineParser>

class MailerService : public QObject
{
  Q_OBJECT

  public:
    MailerService();
    virtual ~MailerService();

    bool start();
    void processArgs( const QCommandLineParser &args );

  private slots :
    void serviceOwnerChanged( const QString&, const QString&, const QString& );

  private :
    bool mSuccess;
    QString mDBusService;
    QString mError;
    QEventLoop *mEventLoop;
};
#endif
