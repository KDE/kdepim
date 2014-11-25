/*
    This file is part of libkdepim.

    Copyright (c) 2003 Don Sanders <sanders@kde.org>
    Copyright (c) 2005 George Staikos <staikos@kde.org.
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
    Copyright (c) 2008 Thomas McGuire <mcguire@kde.org>

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
#ifndef KDEPIM_MAILLISTDRAG_H
#define KDEPIM_MAILLISTDRAG_H

#include "kdepim_export.h"

#include <QList>
#include <QMimeData>
#include <QString>

#include <time.h>

class QUrl;

namespace KPIM
{

/**
 * @defgroup maildnd Mail drag and drop
 *
 * KDEPIM classes for drag and drop of mails
 *
 * \code
 * // Code example for drag and drop enabled widget
 *
 * void SomeWidget::contentsDropEvent(QDropEvent *e)
 * {
 *    if ( KPIM::MailList::canDecode( e->mimeData() ) ) {
 *      MailList mailList = KPIM::MailList::fromMimeData( e->mimeData() );
 *      ...
 * \endcode
 * @{
 **/

/**
  Represents a single dragged mail.
*/
class KDEPIM_EXPORT MailSummary
{
public:
    MailSummary(quint32 serialNumber, const QString &messageId, const QString &subject,
                const QString &from, const QString &to, time_t date);
    MailSummary() {}
    ~MailSummary() {}

    /** Set fields for this mail summary  */
    void set(quint32, const QString &, const QString &, const QString &, const QString &, time_t);

    /** KMail unique identification number */
    quint32 serialNumber() const;

    /** MD5 checksum of message identification string */
    QString messageId() const;

    /** Subject of the message including prefixes */
    QString subject() const;

    /** Simplified from address */
    QString from() const;

    /** Simplified to address */
    QString to() const;

    /** Date the message was sent */
    time_t date() const;

private:
    quint32 mSerialNumber;
    QString mMessageId, mSubject, mFrom, mTo;
    time_t mDate;
};

/**
  Object for the drag object to call-back for message fulltext.
*/
class KDEPIM_EXPORT MailTextSource
{
public:
    MailTextSource() {}
    virtual ~MailTextSource() {}

    virtual QByteArray text(quint32 serialNumber) const = 0;
};

/**
   List of mail summaries.
*/
class KDEPIM_EXPORT MailList : public QList<MailSummary>
{
public:
    static QString mimeDataType();
    static bool canDecode(const QMimeData *md);
    static MailList fromMimeData(const QMimeData *md);
    static QByteArray serialsFromMimeData(const QMimeData *md);
    static MailList decode(const QByteArray &payload);
    void populateMimeData(QMimeData *md);
};

} // namespace KPIM

#endif /*maillistdrag_h*/
