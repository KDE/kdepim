/*  -*- c++ -*-
    attachmentcollector.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2004 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "attachmentcollector.h"

#include "helpers/nodehelper.h"

#include <qdebug.h>
#include <kmime/kmime_content.h>

static bool isInSkipMimeContentList(KMime::Content *)
{
    return false;
}

static bool isInExclusionFileTypeList(KMime::Content *node)
{
    if (!node) {
        return true;
    }

    if (node->contentType()->mediaType() == "application") {
        const QByteArray subType = node->contentType()->subType();
        if (subType == "pkcs7-mime"   ||
                subType == "pkcs7-signature" ||
                subType == "pgp-signature" ||
                subType == "pgp-encrypted") {
            return true;
        }
    }

    if (node->contentType()->isMultipart()) {
        return true;
    }

    return false;
}

class MessageCore::AttachmentCollector::Private
{
public:
    std::vector<KMime::Content *> mAttachments;
};

MessageCore::AttachmentCollector::AttachmentCollector()
    : d(new Private)
{
}

MessageCore::AttachmentCollector::~AttachmentCollector()
{
    delete d;
}

void MessageCore::AttachmentCollector::collectAttachmentsFrom(KMime::Content *node)
{
    KMime::Content *parent;

    while (node) {
        parent = node->parent();

        if (node->topLevel()->textContent() == node) {
            node = MessageCore::NodeHelper::next(node);
            continue;
        }

        if (isInSkipMimeContentList(node)) {
            node = MessageCore::NodeHelper::next(node, false);   // skip even the children
            continue;
        }

        if (isInExclusionFileTypeList(node)) {
            node = MessageCore::NodeHelper::next(node);
            continue;
        }

        if (parent && parent->contentType()->isMultipart() &&
                parent->contentType()->subType() == "related") {
            node = MessageCore::NodeHelper::next(node);    // skip embedded images
            continue;
        }

        if (MessageCore::NodeHelper::isHeuristicalAttachment(node)) {
            d->mAttachments.push_back(node);
            node = MessageCore::NodeHelper::next(node, false);   // just make double sure
            continue;
        }

        node = MessageCore::NodeHelper::next(node);
    }
}

const std::vector<KMime::Content *> &MessageCore::AttachmentCollector::attachments() const
{
    return d->mAttachments;
}
