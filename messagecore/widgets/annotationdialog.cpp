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

#include <Nepomuk2/Resource>
#include <Soprano/Vocabulary/NAO>

#include <KMessageBox>
#include <KLocale>
#include <KTextEdit>

#include <QLabel>
#include <QGridLayout>

using namespace MessageCore;

class AnnotationEditDialog::Private
{
  public:
    Private( const QUrl &uri )
      : mNepomukResourceUri( uri ),
        mTextEdit( 0 ),
        mHasAnnotation( false )
    {
    }

    QUrl mNepomukResourceUri;
    KTextEdit *mTextEdit;
    bool mHasAnnotation;
};

AnnotationEditDialog::AnnotationEditDialog( const QUrl &uri, QWidget *parent )
  : KDialog( parent ), d( new Private( uri ) )
{
  Nepomuk2::Resource resource( d->mNepomukResourceUri );

  d->mHasAnnotation = resource.hasProperty( QUrl( Soprano::Vocabulary::NAO::description().toString() ) );
  if ( d->mHasAnnotation ) {
    setCaption( i18n( "Edit Note" ) );
    setButtons( Ok | Cancel | User1 );
    setButtonText( User1, i18n( "Delete Note" ) );
    setButtonIcon( User1, KIcon( "edit-delete" ) );
  } else {
    setCaption( i18n( "Add Note" ) );
    setButtons( Ok | Cancel );
  }

  setDefaultButton( KDialog::Ok );

  QLabel *label = new QLabel( i18n( "Enter the text that should be stored as a note to the mail:" ) );
  QGridLayout *grid = new QGridLayout( mainWidget() );
  d->mTextEdit = new KTextEdit( this );
  grid->addWidget( label );
  grid->addWidget( d->mTextEdit );
  d->mTextEdit->setFocus();

  if ( d->mHasAnnotation ) {
    d->mTextEdit->setPlainText( resource.description() );
  }
  readConfig();
}

AnnotationEditDialog::~AnnotationEditDialog()
{
  writeConfig();
  delete d;
}

void AnnotationEditDialog::slotButtonClicked( int button )
{
  if ( button == KDialog::Ok ) {
    bool textIsEmpty = d->mTextEdit->toPlainText().isEmpty();
    if ( !textIsEmpty ) {
      Nepomuk2::Resource resource( d->mNepomukResourceUri );
      resource.setDescription( d->mTextEdit->toPlainText() );
    } else if ( d->mHasAnnotation && textIsEmpty ) {
      Nepomuk2::Resource resource( d->mNepomukResourceUri );

      resource.removeProperty( QUrl( Soprano::Vocabulary::NAO::description().toString() ) );
    }

    accept();
  } else if ( button == KDialog::Cancel ) {
    reject();
  } else if ( button == KDialog::User1 ) {
    const int answer = KMessageBox::warningContinueCancel( this,
                              i18n( "Do you really want to delete this note?" ),
                              i18n( "Delete Note?" ), KGuiItem( i18n( "Delete" ), "edit-delete" ) );
    if ( answer == KMessageBox::Continue ) {
      Nepomuk2::Resource resource( d->mNepomukResourceUri );
      resource.removeProperty( QUrl( Soprano::Vocabulary::NAO::description().toString() ) );
      accept();
    }
  }
}

void AnnotationEditDialog::readConfig()
{
  KSharedConfig::Ptr cfg = KGlobal::config();
  KConfigGroup group( cfg, "AnnotationEditDialog" );
  QSize size = group.readEntry( "Size", QSize() );
  if ( !size.isEmpty() ) {
    resize( size );
  }
}

void AnnotationEditDialog::writeConfig()
{
  KSharedConfig::Ptr cfg = KGlobal::config();
  KConfigGroup group( cfg, "AnnotationEditDialog" );
  group.writeEntry( "Size", size() );
}



#include "annotationdialog.moc"
