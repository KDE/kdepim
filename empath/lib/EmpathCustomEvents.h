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

#ifndef EMPATH_CUSTOM_EVENTS_H
#define EMPATH_CUSTOM_EVENTS_H

#include <qevent.h>
#include <rmm/Message.h>
#include "EmpathURL.h"
#include "EmpathIndex.h"

enum EmpathCustomEventType {

    EmpathMessageWrittenEventT = QCustomEvent::User,
    EmpathMessageCopiedEventT,
    EmpathMessageMovedEventT,
    EmpathMessageRemovedEventT,
    EmpathMessageRetrievedEventT,
    EmpathMessageMarkedEventT,
    EmpathFolderCreatedEventT,
    EmpathFolderRemovedEventT,
    EmpathIndexReadEventT
};

class EmpathIndexReadEvent : public QCustomEvent
{
    public:

        EmpathIndexReadEvent(
                EmpathIndex     * index,
                EmpathURL       folder,
                bool            success
        );

        ~EmpathIndexReadEvent();

        EmpathIndex * index() { return index_; }
        EmpathURL folder() const { return folder_; }
        bool success() const { return success_; }

    private:

        EmpathIndex * index_;
        EmpathURL folder_;
        bool success_;
};

class EmpathMessageWrittenEvent : public QCustomEvent
{
    public:

        EmpathMessageWrittenEvent(
                QString         messageID,
                RMM::Message    message,
                EmpathURL       folder,
                bool            success
        );

        ~EmpathMessageWrittenEvent();

        QString messageID() const { return messageID_; }
        RMM::Message message();
        EmpathURL folder() const { return folder_; }
        bool success() const { return success_; }

    private:

        QString messageID_;
        RMM::Message message_;
        EmpathURL folder_;
        bool success_;
};

class EmpathMessageCopiedEvent : public QCustomEvent
{
    public:

        EmpathMessageCopiedEvent(
                EmpathURL   source,
                EmpathURL   destination,
                bool        success
        );

        ~EmpathMessageCopiedEvent();

        EmpathURL source() const { return source_; }
        EmpathURL destination() const { return destination_; }
        bool success() const { return success_; }

    private:

        EmpathURL source_;
        EmpathURL destination_;
        bool success_;
};


class EmpathMessageMovedEvent : public QCustomEvent
{
    public:

        EmpathMessageMovedEvent(
                EmpathURL   source,
                EmpathURL   destination,
                bool        success
        );

        ~EmpathMessageMovedEvent();

        EmpathURL source() const { return source_; }
        EmpathURL destination() const { return destination_; }
        bool success() const { return success_; }

    private:

        EmpathURL source_;
        EmpathURL destination_;
        bool success_;
};


class EmpathMessageRetrievedEvent : public QCustomEvent
{
    public:

        EmpathMessageRetrievedEvent(
                EmpathURL url,
                RMM::Message message,
                bool success
        );

        ~EmpathMessageRetrievedEvent();

        EmpathURL url() const { return url_; }
        RMM::Message message() const { return message_; }
        bool success() const { return success_; }

    private:

        EmpathURL url_;
        RMM::Message message_;
        bool success_;
};

class EmpathMessageMarkedEvent : public QCustomEvent
{
    public:

        EmpathMessageMarkedEvent(bool success);
        ~EmpathMessageMarkedEvent();

        bool success() const { return success_; }

    private:

        bool success_;
};


class EmpathFolderCreatedEvent : public QCustomEvent
{
    public:

        EmpathFolderCreatedEvent(
                EmpathURL url,
                bool success
        );

        ~EmpathFolderCreatedEvent();

        EmpathURL url() const { return url_; }
        bool success() const { return success_; }

    private:

        EmpathURL url_;
        bool success_;
};


class EmpathFolderRemovedEvent : public QCustomEvent
{
    public:

        EmpathFolderRemovedEvent(
                EmpathURL url,
                bool success
        );

        ~EmpathFolderRemovedEvent();

        EmpathURL url() const { return url_; }
        bool success() const { return success_; }

    private:

        EmpathURL url_;
        bool success_;
};

#endif

// vim:ts=4:sw=4:tw=78
