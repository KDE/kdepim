/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "annotationdialog.h"

#include <Nepomuk/Resource>

#include <KMessageBox>
#include <KLocale>

#include <QLabel>
#include <QGridLayout>

using namespace KPIM;

AnnotationEditDialog::AnnotationEditDialog( const QUrl &nepomukResourceUri, QWidget *parent )
  : KDialog( parent ),
  mNepomukResourceUri( nepomukResourceUri )
{
  Nepomuk::Resource resource( mNepomukResourceUri );
  const bool hasAnnotation = resource.hasProperty( QUrl( Nepomuk::Resource::descriptionUri() ) );
  if ( hasAnnotation ) {
    setCaption( i18n( "Edit Note" ) );
    setButtons( Ok | Cancel | User1 );
    setButtonText( User1, i18n( "Delete Note" ) );
    setButtonIcon( User1, KIcon( "edit-delete" ) );
  } else {
    setCaption( i18n( "Add Note" ) );
    setButtons( Ok | Cancel );
  }

  QLabel *label = new QLabel( i18n( "Enter the text that should be stored as a note to the mail:" ) );
  QGridLayout *grid = new QGridLayout( mainWidget() );
  mTextEdit = new KTextEdit( this );
  grid->addWidget( label );
  grid->addWidget( mTextEdit );

  if ( hasAnnotation ) {
    mTextEdit->setPlainText( resource.description() );
  }
}

AnnotationEditDialog::~AnnotationEditDialog()
{
}

void AnnotationEditDialog::slotButtonClicked ( int button )
{
  if ( button == KDialog::Ok ) {
    Nepomuk::Resource resource( mNepomukResourceUri );
    resource.setDescription( mTextEdit->toPlainText() );
    accept();
  } else if ( button == KDialog::Cancel ) {
    reject();
  } else if ( button == KDialog::User1 ) {
    const int answer = KMessageBox::warningContinueCancel( this,
                              i18n( "Do you really want to delete this note?" ),
                              i18n( "Delete Note?" ), KGuiItem( i18n( "Delete" ), "edit-delete" ) );
    if ( answer == KMessageBox::Continue ) {
      Nepomuk::Resource resource( mNepomukResourceUri );
      resource.removeProperty( QUrl( Nepomuk::Resource::descriptionUri() ) );
      accept();
    }
  }
}
