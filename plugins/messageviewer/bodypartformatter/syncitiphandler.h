/*
  This file is part of kdepim.

  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#ifndef SYNCITIPHANDLER_H
#define SYNCITIPHANDLER_H

#include <Akonadi/Calendar/ITIPHandler>

#include <QObject>
#include <QEventLoop>

class SyncItipHandler : public QObject
{
    Q_OBJECT
public:
    SyncItipHandler(const QString &receiver, const QString &iCal,
                    const QString &type, QObject *parent = 0);

public Q_SLOTS:
    void onITipMessageProcessed(Akonadi::ITIPHandler::Result, const QString &errorMessage);

public:
    QString errorMessage() const;
    Akonadi::ITIPHandler::Result result() const;

private:
    QString m_errorMessage;
    Akonadi::ITIPHandler::Result m_result;
    QEventLoop m_eventLoop;
};

#endif // SYNCITIPHANDLER_H
