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

#ifdef __GNUG__
# pragma implementation "EmpathUtilities.h"
#endif

// Qt includes
#include <qstring.h>

// Local includes
#include "EmpathUtilities.h"
#include "RMM_MessageID.h"
#include "RMM_DateTime.h"
#include "RMM_Envelope.h"
#include "RMM_ContentType.h"
#include "RMM_Address.h"
#include "RMM_Mailbox.h"
#include "RMM_Enum.h"

QString baseName(const QString & filename)
{
    return filename.mid(filename.findRev('/') + 1);
}

bool stricmp(const QString & a, const QString & b)
{
    return a.lower() == b.lower();
}

EmpathIndexRecord indexRecordFromMessage(const QString & id, RMM::RMessage & m)
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
