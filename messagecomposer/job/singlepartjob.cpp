/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "singlepartjob.h"

#include "composer/composer.h"
#include "contentjobbase_p.h"
#include "part/globalpart.h"
#include "util.h"

#include <QDebug>
#include <KLocalizedString>

#include <kmime/kmime_content.h>
#include <kmime/kmime_headers.h>

using namespace MessageComposer;

class MessageComposer::SinglepartJobPrivate : public ContentJobBasePrivate
{
public:
    SinglepartJobPrivate(SinglepartJob *qq)
        : ContentJobBasePrivate(qq)
        , contentDescription(0)
        , contentDisposition(0)
        , contentID(0)
        , contentTransferEncoding(0)
        , contentType(0)
    {
    }

    bool chooseCTE();

    QByteArray data;
    KMime::Headers::ContentDescription *contentDescription;
    KMime::Headers::ContentDisposition *contentDisposition;
    KMime::Headers::ContentID *contentID;
    KMime::Headers::ContentTransferEncoding *contentTransferEncoding;
    KMime::Headers::ContentType *contentType;

    Q_DECLARE_PUBLIC(SinglepartJob)
};

bool SinglepartJobPrivate::chooseCTE()
{
    Q_Q(SinglepartJob);

    QList<KMime::Headers::contentEncoding> allowed = KMime::encodingsForData(data);

    if (!q->globalPart()->is8BitAllowed()) {
        allowed.removeAll(KMime::Headers::CE8Bit);
    }

#if 0 //TODO signing
    // In the following cases only QP and Base64 are allowed:
    // - the buffer will be OpenPGP/MIME signed and it contains trailing
    //   whitespace (cf. RFC 3156)
    // - a line starts with "From "
    if ((willBeSigned && cf.hasTrailingWhitespace()) ||
            cf.hasLeadingFrom()) {
        ret.removeAll(DwMime::kCte8bit);
        ret.removeAll(DwMime::kCte7bit);
    }
#endif

    if (contentTransferEncoding) {
        // Specific CTE set.  Check that our data fits in it.
        if (!allowed.contains(contentTransferEncoding->encoding())) {
            q->setError(JobBase::BugError);
            q->setErrorText(i18n("%1 Content-Transfer-Encoding cannot correctly encode this message.",
                                 KMime::nameForEncoding(contentTransferEncoding->encoding())));
            return false;
            // TODO improve error message in case 8bit is requested but not allowed.
        }
    } else {
        // No specific CTE set.  Choose the best one.
        Q_ASSERT(!allowed.isEmpty());
        contentTransferEncoding = new KMime::Headers::ContentTransferEncoding;
        contentTransferEncoding->setEncoding(allowed.first());
    }
    qDebug() << "Settled on encoding" << KMime::nameForEncoding(contentTransferEncoding->encoding());
    return true;
}

SinglepartJob::SinglepartJob(QObject *parent)
    : ContentJobBase(*new SinglepartJobPrivate(this), parent)
{
}

SinglepartJob::~SinglepartJob()
{
}

QByteArray SinglepartJob::data() const
{
    Q_D(const SinglepartJob);
    return d->data;
}

void SinglepartJob::setData(const QByteArray &data)
{
    Q_D(SinglepartJob);
    d->data = data;
}

KMime::Headers::ContentDescription *SinglepartJob::contentDescription()
{
    Q_D(SinglepartJob);
    if (!d->contentDescription) {
        d->contentDescription = new KMime::Headers::ContentDescription;
    }
    return d->contentDescription;
}

KMime::Headers::ContentDisposition *SinglepartJob::contentDisposition()
{
    Q_D(SinglepartJob);
    if (!d->contentDisposition) {
        d->contentDisposition = new KMime::Headers::ContentDisposition;
    }
    return d->contentDisposition;
}

KMime::Headers::ContentID *SinglepartJob::contentID()
{
    Q_D(SinglepartJob);
    if (!d->contentID) {
        d->contentID = new KMime::Headers::ContentID;
    }
    return d->contentID;
}

KMime::Headers::ContentTransferEncoding *SinglepartJob::contentTransferEncoding()
{
    Q_D(SinglepartJob);
    if (!d->contentTransferEncoding) {
        d->contentTransferEncoding = new KMime::Headers::ContentTransferEncoding;
    }
    return d->contentTransferEncoding;
}

KMime::Headers::ContentType *SinglepartJob::contentType()
{
    Q_D(SinglepartJob);
    if (!d->contentType) {
        d->contentType = new KMime::Headers::ContentType;
    }
    return d->contentType;
}

void SinglepartJob::process()
{
    Q_D(SinglepartJob);
    Q_ASSERT(d->resultContent == 0);   // Not processed before.
    d->resultContent = new KMime::Content;

    if (!d->chooseCTE()) {
        Q_ASSERT(error());
        emitResult();
        return;
    }

    // Set headers.
    if (d->contentDescription) {
        d->resultContent->setHeader(d->contentDescription);
        d->contentDescription->setParent(d->resultContent);
    }
    if (d->contentDisposition) {
        d->resultContent->setHeader(d->contentDisposition);
        d->contentDisposition->setParent(d->resultContent);
    }
    if (d->contentID) {
        d->resultContent->setHeader(d->contentID);
        d->contentID->setParent(d->resultContent);
    }
    Q_ASSERT(d->contentTransferEncoding);   // chooseCTE() created it if it didn't exist.
    {
        d->resultContent->setHeader(d->contentTransferEncoding);
        d->contentTransferEncoding->setParent(d->resultContent);

    }
    if (d->contentType) {
        d->resultContent->setHeader(d->contentType);
        d->contentType->setParent(d->resultContent);
    }

    // Set data.
    d->resultContent->setBody(d->data);

    emitResult();
}

