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

#include "syncitiphandler.h"
#include <calendarsupport/calendarsingleton.h>
#include <QDebug>

using namespace Akonadi;

SyncItipHandler::SyncItipHandler(const QString &receiver, const QString &iCal,
                                 const QString &type, QObject *parent) : QObject(parent)
    , m_result(Akonadi::ITIPHandler::ResultSuccess)
{
    Akonadi::ITIPHandler *handler = new Akonadi::ITIPHandler(this);
    QObject::connect(handler, &Akonadi::ITIPHandler::iTipMessageProcessed, this, &SyncItipHandler::onITipMessageProcessed);

    m_counterProposalEditorDelegate = new IncidenceEditorNG::GroupwareUiDelegate();
    handler->setGroupwareUiDelegate(m_counterProposalEditorDelegate);

    Akonadi::ETMCalendar::Ptr etmCalendar = CalendarSupport::calendarSingleton(/*createIfNull=*/false);
    if (etmCalendar && etmCalendar->isLoaded()) {
        qDebug() << "Reusing exising ETM";
        handler->setCalendar(etmCalendar);
    } else {
        qDebug() << "Not reusing any ETM";
    }

    handler->processiTIPMessage(receiver, iCal, type);

    m_eventLoop.exec();
}

void SyncItipHandler::onITipMessageProcessed(Akonadi::ITIPHandler::Result result, const QString &errorMessage)
{
    m_result = result;
    m_errorMessage = errorMessage;
    m_eventLoop.exit();
    deleteLater();
    delete m_counterProposalEditorDelegate;
    m_counterProposalEditorDelegate = 0;
}

QString SyncItipHandler::errorMessage() const
{
    return m_errorMessage;
}

Akonadi::ITIPHandler::Result SyncItipHandler::result() const
{
    return m_result;
}
