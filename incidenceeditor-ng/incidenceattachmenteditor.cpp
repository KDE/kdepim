/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "incidenceattachmenteditor.h"

#include <QtCore/QPointer>

#include "attachmenticonview.h"
#include "attachmenteditdialog.h"
#include "ui_incidenceattachmenteditor.h"

using namespace IncidenceEditorsNG;

IncidenceAttachmentEditor::IncidenceAttachmentEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mUi( new Ui::IncidenceAttachmentEditor )
{
  mUi->setupUi( this );
  mUi->mAddButton->setIcon( KIcon( "list-add" ) );
  mUi->mRemoveButton->setIcon( KIcon( "list-remove" ) );

  setupAttachmentIconView();
  
  connect( mUi->mAddButton, SIGNAL(clicked()), SLOT(slotAddAttachment()) );
  connect( mUi->mRemoveButton, SIGNAL(clicked()), SLOT(slotRemoveAttachment()) );
}

void IncidenceAttachmentEditor::load( KCal::Incidence::ConstPtr incidence )
{

}

void IncidenceAttachmentEditor::save( KCal::Incidence::Ptr incidence )
{

}

bool IncidenceAttachmentEditor::isDirty() const
{
  return false;
}

void IncidenceAttachmentEditor::slotAddAttachment()
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachmentView );

  QPointer<AttachmentEditDialog> dlg = new AttachmentEditDialog( item, mAttachmentView );
  dlg->setCaption( i18nc( "@title", "Add Attachment" ) );
  if ( dlg->exec() == KDialog::Rejected )
    delete item;

  delete dlg;
}

void IncidenceAttachmentEditor::slotRemoveAttachment()
{

}

void IncidenceAttachmentEditor::setupAttachmentIconView()
{
  mAttachmentView = new AttachmentIconView( this );
  mAttachmentView->setWhatsThis( i18nc( "@info:whatsthis",
                                     "Displays items (files, mail, etc.) that "
                                     "have been associated with this event or to-do." ) );

  QGridLayout *layout = new QGridLayout( mUi->mAttachmentViewPlaceHolder );
  layout->addWidget( mAttachmentView );
}
