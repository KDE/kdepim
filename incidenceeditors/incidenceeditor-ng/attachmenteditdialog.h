/*
  This file is part of KOrganizer.

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

#ifndef ATTACHMENTEDITDIALOG_H
#define ATTACHMENTEDITDIALOG_H

#include <KDE/KDialog>
#include <KDE/KMimeType>

namespace KCal {
class Attachment;
}

namespace Ui {
class AttachmentEditDialog;
}

namespace IncidenceEditorsNG {

class AttachmentIconItem;

class AttachmentEditDialog : public KDialog
{
  Q_OBJECT
  public:
    AttachmentEditDialog( AttachmentIconItem *item, QWidget *parent, bool modal = true );

    void accept();

  protected slots:
    void urlChanged( const KUrl &url );
    void urlChanged( const QString & url );
    virtual void slotApply();

  private:
    KCal::Attachment *mAttachment;
    AttachmentIconItem *mItem;
    KMimeType::Ptr mMimeType;
    Ui::AttachmentEditDialog *mUi;
};

} // IncidenceEditorsNG

#endif // ATTACHMENTEDITDIALOG_H
