/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "incidenceeditorgeneralpage.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

#include "incidencegeneraleditor.h"
#include "incidencedatetimeeditor.h"
#include "incidencedescriptioneditor.h"
#include "incidenceattachmenteditor.h"

#include <KLocale>

using namespace IncidenceEditorsNG;

IncidenceEditorGeneralPage::IncidenceEditorGeneralPage( QWidget *parent )
  : CombinedIncidenceEditor( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( 0 );

  IncidenceGeneralEditor *ieGeneral = new IncidenceGeneralEditor( this );
  layout->addWidget( ieGeneral );

  QGroupBox *timeGroupBox = new QGroupBox( i18nc( "@title:group", "Date && Time" ), this );
  timeGroupBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Sets options related to the date and time of the event or to-do." ) );
  QGridLayout *timeLayout = new QGridLayout( timeGroupBox );
           
  IncidenceDateTimeEditor *ieDateTime = new IncidenceDateTimeEditor( timeGroupBox );
  timeLayout->addWidget( ieDateTime );
  layout->addWidget( timeGroupBox );

  IncidenceDescriptionEditor *ieDescription = new IncidenceDescriptionEditor( this );
  layout->addWidget( ieDescription, 4 );

  IncidenceAttachmentEditor *ieAttachment = new IncidenceAttachmentEditor( this );
  layout->addWidget( ieAttachment, 1 );
  
//   QSpacerItem *verticalSpacer =
//     new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
//   layout->addItem( verticalSpacer );

  mDirtyLabel = new QLabel( i18n( "Clean" ), this );
  layout->addWidget( mDirtyLabel );

  connect( this, SIGNAL(dirtyStatusChanged(bool)),
           SLOT(updateDirtyLabel(bool)));
  
  // Combine the various editors with this page.
  combine( ieGeneral );
  combine( ieDateTime );
  combine( ieDescription );
  combine( ieAttachment );
}

void IncidenceEditorGeneralPage::updateDirtyLabel( bool isDirty )
{
  if ( isDirty )
    mDirtyLabel->setText( i18n( "Dirty" ) );
  else
    mDirtyLabel->setText( i18n( "Clean" ) );
}

#include "moc_incidenceeditorgeneralpage.cpp"
