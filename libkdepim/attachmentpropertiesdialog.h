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

#ifndef KDEPIM_ATTACHMENTPROPERTIESDIALOG_H
#define KDEPIM_ATTACHMENTPROPERTIESDIALOG_H

#include "attachmentpart.h"
#include "kdepim_export.h"

#include <KDE/KDialog>

namespace KPIM {

class KDEPIM_EXPORT AttachmentPropertiesDialog: public KDialog
{
  Q_OBJECT

  public:
    explicit AttachmentPropertiesDialog( KPIM::AttachmentPart::Ptr part,
                                         QWidget *parent = 0, bool readOnly = false );

    /**
      This converts the KMime::Content to an AttachmentPart. Therefore, saving
      the changes to the KMime::Content is not supported, and readOnly must
      be true.
    */
    explicit AttachmentPropertiesDialog( const KMime::Content *content,
                                         QWidget *parent = 0, bool readOnly = true );
    virtual ~AttachmentPropertiesDialog();

    KPIM::AttachmentPart::Ptr attachmentPart() const;

    bool isEncryptEnabled() const;
    /** Sets whether or not this attachment can be encrypted */
    void setEncryptEnabled( bool enabled );
    bool isSignEnabled() const;
    /** Sets whether or not this attachment can be signed */
    void setSignEnabled( bool enabled );

  public slots:
    /* reimpl */
    virtual void accept();

  private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void mimeTypeChanged( const QString& ) )
};

} // namespace KMail

#endif
