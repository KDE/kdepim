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


#ifndef CALENDARINTERFACE_H
#define CALENDARINTERFACE_H

#include <QObject>
#include <KCalCore/Incidence>

class QDate;

/** Implement the www.kde.korganizer/Calendar interface for korganizer-mobile */
class CalendarInterface : public QObject
{
  Q_OBJECT
public:
    explicit CalendarInterface(QObject * parent = 0) :QObject(parent){}
public slots:
    void showDate(const QDate& date);
    void openEventEditor( const QString &);
    void openEventEditor( const QString &summary,
                          const QString &description,
                          const QStringList &attachments );
    void openEventEditor( const QString &summary,
                          const QString &description,
                          const QStringList &attachments,
                          const QStringList &attendees );
    void openEventEditor( const QString &summary,
                          const QString &description,
                          const QString &uri,
                          const QString &file,
                          const QStringList &attendees,
                          const QString &attachmentMimetype );

    void openEventEditor( const QString &summary,
                          const QString &description,
                          const QStringList &attachmentUris,
                          const QStringList &attendees,
                          const QStringList &attachmentMimetypes,
                          bool attachmentIsInline );

    void openTodoEditor( const QString &);
    void openTodoEditor( const QString &summary,
                         const QString &description,
                         const QStringList &attachments );
    void openTodoEditor( const QString &summary,
                         const QString &description,
                         const QStringList &attachments,
                         const QStringList &attendees );
    void openTodoEditor( const QString &summary,
                         const QString &description,
                         const QString &uri,
                         const QString &file,
                         const QStringList &attendees,
                         const QString &attachmentMimetype );

    void openTodoEditor( const QString &summary,
                         const QString &description,
                         const QStringList &attachmentUris,
                         const QStringList &attendees,
                         const QStringList &attachmentMimetypes,
                         bool attachmentIsInline );

    void openJournalEditor( const QDate &date );
    void openJournalEditor( const QString &text, const QDate &date );
    void openJournalEditor( const QString &text );

    void showJournalView();
    void showTodoView();
    void showEventView();

    void goDate( const QDate &);
    void goDate( const QString &);

signals:
    void showDateSignal(const QVariant& date);
    void showEventViewSignal();
    void openIncidenceEditorSignal( const QString &summary,
                                    const QString &description,
                                    const QStringList &attachmentUris,
                                    const QStringList &attendees,
                                    const QStringList &attachmentMimetypes,
                                    bool attachmentIsInline,
                                    KCalCore::Incidence::IncidenceType type );
};

#endif // CALENDARINTERFACE_H
