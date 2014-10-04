/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  Based on KMail code by various authors (kmmsgpartdlg).

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

#ifndef MESSAGECORE_ATTACHMENTPROPERTIESDIALOG_H
#define MESSAGECORE_ATTACHMENTPROPERTIESDIALOG_H

#include "messagecore_export.h"

#include "attachmentpart.h"

#include <QDialog>

namespace MessageCore
{

/**
 * @short A dialog for editing attachment properties.
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 */
class MESSAGECORE_EXPORT AttachmentPropertiesDialog: public QDialog
{
    Q_OBJECT

public:
    /**
     * Creates a new attachment properties dialog.
     *
     * @param part The attachment part which properties to change.
     * @param readOnly Whether the dialog should be in read-only mode.
     * @param parent The parent object.
     */
    explicit AttachmentPropertiesDialog(const AttachmentPart::Ptr &part,
                                        bool readOnly = false, QWidget *parent = 0);

    /**
     * Creates a new attachment properties dialog.
     *
     * @param content The mime content that represents the attachment which properties to change.
     * @param parent The parent object.
     *
     * @note This converts the KMime::Content to an AttachmentPart internally.
     *       Therefore, saving the changes to the KMime::Content is not supported,
     *       and the dialog is in readOnly mode.
     */
    explicit AttachmentPropertiesDialog(const KMime::Content *content, QWidget *parent = 0);

    /**
     * Destroys the attachment properties dialog.
     */
    virtual ~AttachmentPropertiesDialog();

    /**
     * Returns the modified attachment.
     */
    AttachmentPart::Ptr attachmentPart() const;

    /**
     * Sets whether the encryption status of the attachment can be changed.
     */
    void setEncryptEnabled(bool enabled);

    /**
     * Returns whether the encryption status of the attachment can be changed.
     */
    bool isEncryptEnabled() const;

    /**
     * Sets whether the signature status of the attachment can be changed.
     */
    void setSignEnabled(bool enabled);

    /**
     * Returns whether the signature status of the attachment can be changed.
     */
    bool isSignEnabled() const;

public Q_SLOTS:
    /* reimpl */
    virtual void accept();
    void slotHelp();

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void mimeTypeChanged(const QString &))
    //@endcond
};

}

#endif
