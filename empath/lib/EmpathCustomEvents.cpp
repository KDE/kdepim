/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "EmpathCustomEvents.h"

EmpathIndexReadEvent::EmpathIndexReadEvent(
        EmpathIndex     * index,
        EmpathURL       folder,
        bool            success
)
    :   QCustomEvent(EmpathIndexReadEventT),
        index_(index),
        folder_(folder),
        success_(success)
{
}

EmpathIndexReadEvent::~EmpathIndexReadEvent()
{
}


EmpathMessageWrittenEvent::EmpathMessageWrittenEvent(
        QString         messageID,
        RMM::Message    message,
        EmpathURL       folder,
        bool            success
)
    :   QCustomEvent(EmpathMessageWrittenEventT),
        messageID_(messageID),
        message_(message),
        folder_(folder),
        success_(success)
{
}

EmpathMessageWrittenEvent::~EmpathMessageWrittenEvent()
{
}

EmpathMessageCopiedEvent::EmpathMessageCopiedEvent(
        EmpathURL   source,
        EmpathURL   destination,
        bool        success
)
    :   QCustomEvent(EmpathMessageCopiedEventT),
        source_(source),
        destination_(destination),
        success_(success)
{
}

EmpathMessageCopiedEvent::~EmpathMessageCopiedEvent()
{
}

EmpathMessageMovedEvent::EmpathMessageMovedEvent(
        EmpathURL   source,
        EmpathURL   destination,
        bool        success
)
    :   QCustomEvent(EmpathMessageMovedEventT),
        source_(source),
        destination_(destination),
        success_(success)
{
}

EmpathMessageMovedEvent::~EmpathMessageMovedEvent()
{
}

EmpathMessageRetrievedEvent::EmpathMessageRetrievedEvent(
        EmpathURL url,
        RMM::Message message,
        bool success
)
    :   QCustomEvent(EmpathMessageRetrievedEventT),
        url_(url),
        message_(message),
        success_(success)
{
}

EmpathMessageRetrievedEvent::~EmpathMessageRetrievedEvent()
{
}

EmpathMessageMarkedEvent::EmpathMessageMarkedEvent(bool success)
    :   QCustomEvent(EmpathMessageMarkedEventT),
        success_(success)
{
}

EmpathMessageMarkedEvent::~EmpathMessageMarkedEvent()
{
}

EmpathFolderCreatedEvent::EmpathFolderCreatedEvent(
        EmpathURL url,
        bool success
)
    :   QCustomEvent(EmpathFolderCreatedEventT),
        url_(url),
        success_(success)
{
}

EmpathFolderCreatedEvent::~EmpathFolderCreatedEvent()
{
}

EmpathFolderRemovedEvent::EmpathFolderRemovedEvent(
        EmpathURL url,
        bool success
)
    :   QCustomEvent(EmpathFolderRemovedEventT),
        url_(url),
        success_(success)
{
}

EmpathFolderRemovedEvent::~EmpathFolderRemovedEvent()
{
}

