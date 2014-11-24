/*
 * This file is part of KMail.
 * Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KMAIL_ATTACHMENTMODEL_H
#define KMAIL_ATTACHMENTMODEL_H

#include "messagecomposer_export.h"

#include <QtCore/QAbstractItemModel>

#include <QUrl>

#include <messagecore/attachment/attachmentpart.h>
#include <AkonadiCore/item.h>

namespace MessageComposer
{

/**
  Columns:
  name
  size
  encoding
  mime type
  compress
  encrypt
  sign
*/
class MESSAGECOMPOSER_EXPORT AttachmentModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        AttachmentPartRole = Qt::UserRole,
        NameRole,
        SizeRole,
        EncodingRole,
        MimeTypeRole,
        CompressRole,
        EncryptRole,
        SignRole,
        AutoDisplayRole
    };

    /**
     * @todo: get rid of columns and use the roles instead.
     */
    enum Column {
        NameColumn,
        SizeColumn,
        EncodingColumn,
        MimeTypeColumn,
        CompressColumn,
        EncryptColumn,
        SignColumn,
        AutoDisplayColumn,
        LastColumn ///< @internal
    };

    explicit AttachmentModel(QObject *parent);
    ~AttachmentModel();

    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent);
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

    /// for the save/discard warning
    bool isModified() const;
    void setModified(bool modified);

    bool isEncryptEnabled() const;
    void setEncryptEnabled(bool enabled);
    bool isSignEnabled() const;
    void setSignEnabled(bool enabled);
    bool isEncryptSelected() const;
    /// sets for all
    void setEncryptSelected(bool selected);
    bool isSignSelected() const;
    /// sets for all
    void setSignSelected(bool selected);

    bool isAutoDisplayEnabled() const;
    void setAutoDisplayEnabled(bool enabled);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    bool addAttachment(MessageCore::AttachmentPart::Ptr part);
    bool updateAttachment(MessageCore::AttachmentPart::Ptr part);
    bool replaceAttachment(MessageCore::AttachmentPart::Ptr oldPart, MessageCore::AttachmentPart::Ptr newPart);
    bool removeAttachment(MessageCore::AttachmentPart::Ptr part);
    MessageCore::AttachmentPart::List attachments() const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

signals:
    void encryptEnabled(bool enabled);
    void signEnabled(bool enabled);
    void autoDisplayEnabled(bool enabled);
    void attachUrlsRequested(const QList<QUrl> &urls);
    void attachItemsRequester(const Akonadi::Item::List &);
    void attachmentRemoved(MessageCore::AttachmentPart::Ptr part);
    void attachmentCompressRequested(MessageCore::AttachmentPart::Ptr part, bool compress);

private:
    class Private;
    friend class Private;
    Private *const d;
};

} //

#endif // KMAIL_ATTACHMENTMODEL_H
