/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef RMM_ENVELOPE_H
#define RMM_ENVELOPE_H

#include <qcstring.h>

#include <rmm/MessageID.h>
#include <rmm/MimeType.h>
#include <rmm/Mechanism.h>
#include <rmm/ContentDisposition.h>
#include <rmm/DateTime.h>
#include <rmm/Address.h>
#include <rmm/AddressList.h>
#include <rmm/Enum.h>
#include <rmm/Header.h>
#include <rmm/Defines.h>
#include <rmm/MessageComponent.h>
#include <rmm/ContentType.h>
#include <rmm/Cte.h>
#include <rmm/HeaderBody.h>
#include <rmm/Text.h>

namespace RMM {

/**
 * @short An Envelope encapsulates the envelope of an RFC822 message.
 * An Envelope encapsulates the envelope of an RFC822 message.
 * The envelope consists of one or more Header(s).
 * An Envelope provides many convenience methods for referencing various
 * common headers.
 */
class Envelope : public MessageComponent
{

#include "rmm/Envelope_generated.h"
        
    public:

        /**
         * Find out if this header exists in the envelope.
         */
        bool has(RMM::HeaderType t);
        /**
         * Find out if this header exists in the envelope.
         */
        bool has(const QCString & headerName);

        /**
         * Set the specified header to the string value.
         */
        void set(RMM::HeaderType t, const QCString & s);
        void set(const QCString & headerName, const QCString & s);
        void addHeader(Header);
        void addHeader(const QCString &);
        void _createDefault(RMM::HeaderType t);
       
        HeaderList headerList() { return headerList_; }
           
        /**
         * @short Provides the 'default' sender.
         * Provides the 'default' sender. That is, if there's a 'From' header,
         * then you get the first Address in that header body. If there is no
         * 'From' header, then you get what's in 'Sender'.
         */
        Address firstSender();
        
        /**
         * @short The ID of the 'parent' message.
         * Looks at the 'In-Reply-To' and the 'References' headers.
         * If there's a 'References' header, the last reference in that header
         * is used, i.e. the last message that is referred to.
         * If there's no 'References' header, then the 'In-Reply-To' header is
         * used instead to get the id.
         */
        MessageID parentMessageId();

        /**
         * Gets the specified header.
         */
        Header        get(const QCString &);

        HeaderBody    * get(RMM::HeaderType h);
        
        /**
         * This applies to all similar methods:
         * Returns an object of the given return type.
         * If there is no object available, one will be created using sensible
         * defaults.
         * Note that you can accidentally create a header you didn't want by
         * calling one of these. Use has() instead before you try.
         */
        Text               approved();
        AddressList        bcc();
        AddressList        cc();
        Text               comments();
        Text               contentDescription();
        ContentDisposition contentDisposition();
        MessageID          contentID();
        Text               contentMD5();
        ContentType        contentType();
        Text               control();
        Cte                contentTransferEncoding();
        DateTime           date();
        Text               distribution();
        Text               encrypted();
        DateTime           expires();
        Text               followupTo();
        AddressList        from();
        Text               inReplyTo();
        Text               keywords();
        Text               lines();
        MessageID          messageID();
        Text               mimeVersion();
        Text               newsgroups();
        Text               organization();
        Text               path();
        Text               received();
        Text               references();
        AddressList        replyTo();
        AddressList        resentBcc();
        AddressList        resentCc();
        DateTime           resentDate();
        AddressList        resentFrom();
        MessageID          resentMessageID();
        AddressList        resentReplyTo();
        Address            resentSender();
        AddressList        resentTo();
        Text               returnPath();
        Address            sender();
        Text               subject();
        Text               summary();
        AddressList        to();
        Text               xref();
        
    private:

        void _init();
        void _replaceHeaderList(const HeaderList &);
        HeaderList headerList_;
};

}

#endif // rmm/ENVELOPE_H
// vim:ts=4:sw=4:tw=78
