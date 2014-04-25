/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "incidencedescription.h"
#ifdef KDEPIM_MOBILE_UI
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <KDebug>
#include <KActionCollection>
#include <KToolBar>
#include <KLocalizedString>

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG {

class IncidenceDescriptionPrivate
{
  public:
    IncidenceDescriptionPrivate() : mRichTextEnabled( false )
    {
    }

    QString mRealOriginalDescriptionEditContents;
    bool mRichTextEnabled;
};

}

#ifdef KDEPIM_MOBILE_UI
IncidenceDescription::IncidenceDescription( Ui::EventOrTodoMore *ui )
#else
IncidenceDescription::IncidenceDescription( Ui::EventOrTodoDesktop *ui )
#endif
  : IncidenceEditor( 0 ), mUi( ui ), d( new IncidenceDescriptionPrivate() )
{
  setObjectName( "IncidenceDescription" );
  mUi->mDescriptionEdit->setRichTextSupport( KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportItalic |
                                             KRichTextWidget::SupportUnderline |
                                             KRichTextWidget::SupportStrikeOut |
                                             KRichTextWidget::SupportChangeListStyle |
                                             KRichTextWidget::SupportAlignment |
                                             KRichTextWidget::SupportFormatPainting );
#ifdef KDEPIM_MOBILE_UI
  mUi->mRichTextLabel->hide();
#else
  setupToolBar();
  connect( mUi->mRichTextLabel, SIGNAL(linkActivated(QString)),
           this, SLOT(toggleRichTextDescription()) );
#endif
  connect( mUi->mDescriptionEdit, SIGNAL(textChanged()),
           this, SLOT(checkDirtyStatus()) );
}

IncidenceDescription::~IncidenceDescription()
{
  delete d;
}

void IncidenceDescription::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;

  d->mRealOriginalDescriptionEditContents.clear();

  if ( incidence ) {
    enableRichTextDescription( incidence->descriptionIsRich() );
    if ( incidence->descriptionIsRich() ) {
      mUi->mDescriptionEdit->setHtml( incidence->richDescription() );
      d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->toHtml();
    } else {
      QString original = incidence->description();
      mUi->mDescriptionEdit->setText( incidence->description() );
      d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->toPlainText();
    }
  } else {
    enableRichTextDescription( false );
    mUi->mDescriptionEdit->clear();
  }

  mWasDirty = false;
}

void IncidenceDescription::save( const KCalCore::Incidence::Ptr &incidence )
{
  if ( d->mRichTextEnabled ) {
    incidence->setDescription( mUi->mDescriptionEdit->toHtml(), true );
  } else {
    incidence->setDescription( mUi->mDescriptionEdit->toPlainText(), false );
  }
}

bool IncidenceDescription::isDirty() const
{

  /* Sometimes, what you put in a KRichTextWidget isn't the same as what you get out.
     Line terminators (cr,lf) for example can be converted.

     So, to see if the user changed something, we can't compare the original incidence
     with the new editor content.

     Instead we compare the new editor content, with the original editor content, this way
     any tranformation regarding non-printable chars will be irrelevant.
  */
  if ( d->mRichTextEnabled ) {
    return !mLoadedIncidence->descriptionIsRich() ||
      d->mRealOriginalDescriptionEditContents != mUi->mDescriptionEdit->toHtml();
  } else {
    return mLoadedIncidence->descriptionIsRich() ||
      d->mRealOriginalDescriptionEditContents != mUi->mDescriptionEdit->toPlainText();
  }
}

void IncidenceDescription::enableRichTextDescription( bool enable )
{
  d->mRichTextEnabled = enable;

  QString rt( i18nc( "@action Enable or disable rich text editting", "Enable rich text" ) );
  QString placeholder( "<a href=\"show\"><font color='blue'>%1 &gt;&gt;</font></a>" );

  if ( enable ) {
    rt = i18nc( "@action Enable or disable rich text editting", "Disable rich text" );
    placeholder = QString( "<a href=\"show\"><font color='blue'>&lt;&lt; %1</font></a>" );
    mUi->mDescriptionEdit->enableRichTextMode();
    d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->toHtml();
  } else {
    mUi->mDescriptionEdit->switchToPlainText();
    d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->toPlainText();
  }

  placeholder = placeholder.arg( rt );
  mUi->mRichTextLabel->setText( placeholder );
  mUi->mEditToolBarPlaceHolder->setVisible( enable );
  mUi->mDescriptionEdit->setActionsEnabled( enable );
  checkDirtyStatus();
}

void IncidenceDescription::toggleRichTextDescription()
{
  enableRichTextDescription( !d->mRichTextEnabled );
}

void IncidenceDescription::setupToolBar()
{
#ifndef QT_NO_TOOLBAR
  KActionCollection *collection = new KActionCollection( this );
//QT5
  //mUi->mDescriptionEdit->createActions( collection );

  KToolBar *mEditToolBar = new KToolBar( mUi->mEditToolBarPlaceHolder );
  mEditToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
  mEditToolBar->addAction( collection->action( "format_text_bold" ) );
  mEditToolBar->addAction( collection->action( "format_text_italic" ) );
  mEditToolBar->addAction( collection->action( "format_text_underline" ) );
  mEditToolBar->addAction( collection->action( "format_text_strikeout" ) );
  mEditToolBar->addSeparator();

  mEditToolBar->addAction( collection->action( "format_list_style" ) );
  mEditToolBar->addSeparator();

  mEditToolBar->addAction( collection->action( "format_align_left" ) );
  mEditToolBar->addAction( collection->action( "format_align_center" ) );
  mEditToolBar->addAction( collection->action( "format_align_right" ) );
  mEditToolBar->addAction( collection->action( "format_align_justify" ) );
  mEditToolBar->addSeparator();

  mEditToolBar->addAction( collection->action( "format_painter" ) );
  mUi->mDescriptionEdit->setActionsEnabled( false );

  QGridLayout *layout = new QGridLayout( mUi->mEditToolBarPlaceHolder );
  layout->addWidget( mEditToolBar );
#endif

  // By default we don't show the rich text toolbar.
  mUi->mEditToolBarPlaceHolder->setVisible( false );
  d->mRichTextEnabled = false;
}

void IncidenceDescription::printDebugInfo() const
{
  // We're going to crash
  kDebug() << "RichText enabled " << d->mRichTextEnabled;

  if ( mLoadedIncidence ) {
    kDebug() << "Incidence description is rich " << mLoadedIncidence->descriptionIsRich();

    if ( mLoadedIncidence->descriptionIsRich() ) {
      kDebug() << "desc is rich, and it is <desc>" <<  mLoadedIncidence->richDescription()
               << "</desc>; "
               << "widget has <desc>" << mUi->mDescriptionEdit->toHtml()
               << "</desc>; "
               << "expr mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->toHtml() is "
               << ( mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->toHtml() );
    } else {
      kDebug() << "desc is not rich, and it is <desc>" << mLoadedIncidence->description()
               << "</desc>; "
               << "widget has <desc>" << mUi->mDescriptionEdit->toPlainText()
               << "</desc>; "
               << "expr mLoadedIncidence->description() != mUi->mDescriptionEdit->toPlainText() is "
               <<  ( mLoadedIncidence->description() != mUi->mDescriptionEdit->toPlainText() );
    }

  } else {
    kDebug() << "Incidence is invalid";
  }
}

