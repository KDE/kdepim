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


// Local includes
#include "EmpathUtilities.h"
#include "rmm/MessageID.h"
#include "rmm/DateTime.h"
#include "rmm/Envelope.h"
#include "rmm/ContentType.h"
#include "rmm/Address.h"
#include "rmm/Mailbox.h"
#include "rmm/Enum.h"

EmpathIndexRecord indexRecordFromMessage(const QString & id, RMM::Message & m)
{
    return EmpathIndexRecord(
        id,
        QString::fromUtf8(m.envelope().subject().asString()),
        QString::fromUtf8(m.envelope().firstSender().phrase()),
        QString::fromUtf8(m.envelope().firstSender().route()),
        m.envelope().date().qdt(),
        0,
        EmpathIndexRecord::Status(m.status()),
        m.size(),
        QString::fromUtf8(m.envelope().messageID().asString()),
        QString::fromUtf8(m.envelope().parentMessageId().asString()),
        (0 != stricmp(m.envelope().contentType().type(), "multipart"))
    );
}


// vim:ts=4:sw=4:tw=78
