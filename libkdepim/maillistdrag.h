/*
    This file is part of libkdepim.

    Copyright (c) 2003 Don Sanders <sanders@kde.org>
    Copyright (c) 2005 George Staikos <staikos@kde.org.

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef maillistdrag_h
#define maillistdrag_h

#include "tqdragobject.h"
#include "tqvaluelist.h"
#include "tqglobal.h"
#include "time.h"

#include <kdepimmacros.h>

/**
 * KDEPIM classes for drag and drop of mails
 * 
 * // Code example for drag and drop enabled widget
 *
 * void SomeWidget::contentsDropEvent(TQDropEvent *e)
 * {
 *    if (e->provides(MailListDrag::format())) {
 *	MailList mailList;
 *	MailListDrag::decode( e, mailList );
 *      ...
 **/

namespace KPIM {

class KDE_EXPORT MailSummary 
{
public:
    MailSummary( Q_UINT32 serialNumber, TQString messageId, TQString subject, 
		 TQString from, TQString to, time_t date );
    MailSummary() {}
    ~MailSummary() {}

    /*** Set fields for this mail summary  ***/
    void set( Q_UINT32, TQString, TQString, TQString, TQString, time_t );

    /*** KMail unique identification number ***/
    Q_UINT32 serialNumber() const;

    /*** MD5 checksum of message identification string ***/
    TQString messageId();

    /*** Subject of the message including prefixes ***/
    TQString subject();

    /*** Simplified from address ***/
    TQString from();

    /** Simplified to address ***/
    TQString to();

    /*** Date the message was sent ***/
    time_t date();

private:
    Q_UINT32 mSerialNumber;
    TQString mMessageId, mSubject, mFrom, mTo;
    time_t mDate;
};

// List of mail summaries
typedef TQValueList<MailSummary> MailList;

// Object for the drag object to call-back for message fulltext
class KDE_EXPORT MailTextSource {
public:
    MailTextSource() {}
    virtual ~MailTextSource() {}

    virtual TQCString text(Q_UINT32 serialNumber) const = 0;
};

// Drag and drop object for mails
class KDE_EXPORT MailListDrag : public QStoredDrag
{
public:
    // Takes ownership of "src" and deletes it when done
    MailListDrag( MailList, TQWidget * parent = 0, MailTextSource *src = 0 );
    ~MailListDrag();

    const char *format(int i) const;

    bool provides(const char *mimeType) const;

    TQByteArray encodedData(const char *) const;

    /* Reset the list of mail summaries */
    void setMailList( MailList );

    /* The format for this drag - "x-kmail-drag/message-list" */
    static const char* format();
    
    /* Returns TRUE if the information in e can be decoded into a TQString;
       otherwsie returns FALSE */
    static bool canDecode( TQMimeSource* e );

    /* Attempts to decode the dropped information;
       Returns TRUE if successful; otherwise return false */
    static bool decode( TQDropEvent* e, MailList& s );

    /* Attempts to decode the serialNumbers of the dropped information;
       Returns TRUE if successful; otherwise return false */
    static bool decode( TQDropEvent* e, TQByteArray& a );

    /* Attempts to decode the encoded MailList;
       Returns TRUE if successful; otherwise return false */
    static bool decode( TQByteArray& a, MailList& s );

private:
    MailTextSource *_src;
};

}
#endif /*maillistdrag_h*/
