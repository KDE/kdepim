/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2010 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/



#include "calendarinterface.h"
#include "mainview.h"

#include <QDebug>
#include <QGraphicsItem>

void CalendarInterface::showDate( const QDate &date )
{
  emit showDateSignal(QVariant::fromValue<QDate>( date ) );
}

void CalendarInterface::goDate( const QDate& )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
}

void CalendarInterface::goDate( const QString& )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
}

void CalendarInterface::openEventEditor( const QString& )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
}

void CalendarInterface::openEventEditor( const QString &summary,
                                         const QString &description,
                                         const QStringList &attachments)
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( summary );
  Q_UNUSED( description );
  Q_UNUSED( attachments );
}

void CalendarInterface::openEventEditor( const QString &summary,
                                         const QString &description,
                                         const QStringList &attachments,
                                         const QStringList &attendees)
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( summary );
  Q_UNUSED( description );
  Q_UNUSED( attachments );
  Q_UNUSED( attendees );
}

void CalendarInterface::openEventEditor( const QString &summary,
                                         const QString &description,
                                         const QString &uri,
                                         const QString &file,
                                         const QStringList &attendees,
                                         const QString &attachmentMimetype)
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( summary );
  Q_UNUSED( description );
  Q_UNUSED( uri );
  Q_UNUSED( file );
  Q_UNUSED( attendees );
  Q_UNUSED( attachmentMimetype );
}

void CalendarInterface::openEventEditor( const QString &summary,
                                         const QString &description,
                                         const QStringList &attachmentUris,
                                         const QStringList &attendees,
                                         const QStringList &attachmentMimetypes,
                                         bool attachmentIsInline )
{
  emit openIncidenceEditorSignal( summary, description, attachmentUris, attendees,
                                  attachmentMimetypes, attachmentIsInline,
                                  KCalCore::Incidence::TypeEvent );
}

void CalendarInterface::openJournalEditor( const QDate & )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
}

void CalendarInterface::openJournalEditor( const QString &text, const QDate &date )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( text );
  Q_UNUSED( date );
}

void CalendarInterface::openJournalEditor( const QString &text )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( text );
}

void CalendarInterface::openTodoEditor( const QString& )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
}

void CalendarInterface::openTodoEditor( const QString &summary,
                                        const QString &description,
                                        const QStringList &attachments )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( summary );
  Q_UNUSED( description );
  Q_UNUSED( attachments );
}


void CalendarInterface::openTodoEditor( const QString &summary,
                                        const QString &description,
                                        const QStringList &attachments,
                                        const QStringList &attendees )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( summary );
  Q_UNUSED( description );
  Q_UNUSED( attachments );
  Q_UNUSED( attendees );
}

void CalendarInterface::openTodoEditor( const QString &summary,
                                        const QString &description,
                                        const QString &uri,
                                        const QString &file,
                                        const QStringList &attendees,
                                        const QString &attachmentMimetype )
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
  Q_UNUSED( summary );
  Q_UNUSED( description );
  Q_UNUSED( uri );
  Q_UNUSED( file );
  Q_UNUSED( attendees );
  Q_UNUSED( attachmentMimetype );
}

void CalendarInterface::openTodoEditor( const QString &summary,
                                        const QString &description,
                                        const QStringList &attachmentUris,
                                        const QStringList &attendees,
                                        const QStringList &attachmentMimetypes,
                                        bool attachmentIsInline )
{
  emit openIncidenceEditorSignal( summary, description, attachmentUris, attendees,
                                  attachmentMimetypes, attachmentIsInline,
                                  KCalCore::Incidence::TypeTodo );
}

void CalendarInterface::showEventView()
{
  emit showEventViewSignal();
}

void CalendarInterface::showJournalView()
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
}

void CalendarInterface::showTodoView()
{
  qWarning() << Q_FUNC_INFO << " is not yet implemented in korganzier-mobile";
}

