/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma interface "RMM_Envelope.h"
#endif

#ifndef RMM_ENVELOPE_H
#define RMM_ENVELOPE_H

#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>

#include <RMM_Entity.h>
#include <RMM_MessageID.h>
#include <RMM_Mailbox.h>
#include <RMM_MailboxList.h>
#include <RMM_MimeType.h>
#include <RMM_Mechanism.h>
#include <RMM_DispositionType.h>
#include <RMM_DateTime.h>
#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_Text.h>
#include <RMM_Enum.h>
#include <RMM_Header.h>
#include <RMM_Defines.h>
#include <RMM_MessageComponent.h>
#include <RMM_ContentType.h>
#include <RMM_Cte.h>
#include <RMM_HeaderBody.h>

namespace RMM {

/**
 * @short An REnvelope encapsulates the envelope of an RFC822 message.
 * An REnvelope encapsulates the envelope of an RFC822 message.
 * The envelope consists of one or more RHeader(s).
 * An REnvelope provides many convenience methods for referencing various
 * common headers.
 */
class REnvelope : public RMessageComponent
{
    public:

#include "generated/REnvelope_generated.h"
        
        /**
         * Find out if this header exists in the envelope.
         */
        bool has(RMM::HeaderType t);
        /**
         * Find out if this header exists in the envelope.
         */
        bool has(const QCString & headerName);

        QCString asString();
        
        /**
         * Set the specified header to the string value.
         */
        void set(RMM::HeaderType t, const QCString & s);
        void set(const QCString headerName, const QCString & s);
        void addHeader(RHeader);
        void addHeader(const QCString &);
        void _createDefault(RMM::HeaderType t);
        
        /**
         * @short Provides the 'default' sender.
         * Provides the 'default' sender. That is, if there's a 'From' header,
         * then you get the first RAddress in that header body. If there is no
         * 'From' header, then you get what's in 'Sender'.
         */
        RMailbox firstSender();
        
        /**
         * @short The ID of the 'parent' message.
         * Looks at the 'In-Reply-To' and the 'References' headers.
         * If there's a 'References' header, the last reference in that header
         * is used, i.e. the last message that is referred to.
         * If there's no 'References' header, then the 'In-Reply-To' header is
         * used instead to get the id.
         */
        RMessageID parentMessageId();

        /**
         * Gets the specified header.
         */
        RHeader        * get(const QCString &);

        RHeaderBody    * get(RMM::HeaderType h);

        /**
         * This applies to all similar methods:
         * Returns an reference to an object of the given return type.
         * If there is no object available, one will be created using sensible
         * defaults, and returned, so you won't get a hanging reference.
         * Note that you can accidentally create a header you didn't want by
         * calling one of these. Use has() instead before you try.
         */
        RText                approved();
        RAddressList         bcc();
        RMailboxList         cc();
        RText                 comments();
        RText                 contentDescription();
        RDispositionType     contentDisposition();
        RMessageID             contentID();
        RText                 contentMD5();
        RContentType         contentType();
        RText                 control();
        RCte                 contentTransferEncoding();
        RDateTime             date();
        RText                 distribution();
        RText                 encrypted();
        RDateTime             expires();
        RText                 followupTo();
        RMailboxList         from();
        RText                 inReplyTo();
        RText                 keywords();
        RText                 lines();
        RMessageID             messageID();
        RText                 mimeVersion();
        RText                 newsgroups();
        RText                 organization();
        RText                 path();
        RText                 received();
        RText                 references();
        RAddressList         replyTo();
        RAddressList         resentBcc();
        RAddressList         resentCc();
        RDateTime             resentDate();
        RMailboxList         resentFrom();
        RMessageID             resentMessageID();
        RAddressList         resentReplyTo();
        RMailbox             resentSender();
        RAddressList         resentTo();
        RText                 returnPath();
        RMailbox             sender();
        RText                 subject();
        RText                 summary();
        RAddressList         to();
        RText                 xref();
        
    private:

        RHeaderList headerList_;
};

}

#endif // RMM_ENVELOPE_H
// vim:ts=4:sw=4:tw=78
