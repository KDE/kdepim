/*
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef INCIDENCEEDITOR_ATTACHMENTEDITDIALOG_H
#define INCIDENCEEDITOR_ATTACHMENTEDITDIALOG_H

#include "incidenceeditors_ng_export.h"

#include <KCalCore/Attachment>
#include <QUrl>
#include <QDialog>
#include <KMimeType>
class QPushButton;

namespace Ui {
  class AttachmentEditDialog;
}

namespace IncidenceEditorNG {

class AttachmentIconItem;

class INCIDENCEEDITORS_NG_EXPORT AttachmentEditDialog : public QDialog
{
  Q_OBJECT
  public:
    AttachmentEditDialog( AttachmentIconItem *item, QWidget *parent, bool modal = true );
    virtual ~AttachmentEditDialog();
    void accept();

  protected slots:
    void inlineChanged( int state );
    void urlChanged( const QUrl &url );
    void urlChanged( const QString & url );
    virtual void slotApply();

  private:
    KCalCore::Attachment::Ptr mAttachment;
    AttachmentIconItem *mItem;
    KMimeType::Ptr mMimeType;
    Ui::AttachmentEditDialog *mUi;
    QPushButton *mOkButton;
};

}

#endif // INCIDENCEEDITOR_ATTACHMENTEDITDIALOG_H
