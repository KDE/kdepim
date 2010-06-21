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

#include "incidencedescription.h"

#include <QtCore/QDebug>

#include <KDE/KActionCollection>
#include <KDE/KToolBar>

#include "ui_eventortododesktop.h"

using namespace IncidenceEditorsNG;

IncidenceDescription::IncidenceDescription( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor( 0 )
  , mUi( ui )
{
  mUi->mDescriptionEdit->setRichTextSupport( KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportItalic |
                                             KRichTextWidget::SupportUnderline |
                                             KRichTextWidget::SupportStrikeOut |
                                             KRichTextWidget::SupportChangeListStyle |
                                             KRichTextWidget::SupportAlignment |
                                             KRichTextWidget::SupportFormatPainting );

  setupToolBar();

  connect( mUi->mRichTextLabel, SIGNAL(linkActivated(QString)),
           this, SLOT(toggleRichTextDescription()) );
  connect( mUi->mDescriptionEdit, SIGNAL(textChanged()),
           this, SLOT(checkDirtyStatus()) );
}

void IncidenceDescription::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( incidence ) {
    enableRichTextDescription( incidence->descriptionIsRich() );
    if ( incidence->descriptionIsRich() )
      mUi->mDescriptionEdit->setHtml( incidence->richDescription() );
    else
      mUi->mDescriptionEdit->setText( incidence->description() );
  } else {
    enableRichTextDescription( false );
    mUi->mDescriptionEdit->clear();
  }

  mWasDirty = false;
}

void IncidenceDescription::save( KCal::Incidence::Ptr incidence )
{
  if ( mUi->mEditToolBarPlaceHolder->isVisible() ) {
    incidence->setDescription( mUi->mDescriptionEdit->toHtml(), true );
  } else {
    incidence->setDescription( mUi->mDescriptionEdit->toPlainText(), false );
  }
}

bool IncidenceDescription::isDirty() const
{
  if ( mUi->mEditToolBarPlaceHolder->isVisible() ) {
    return !mLoadedIncidence->descriptionIsRich() ||
      mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->toHtml();
  } else {
    return mLoadedIncidence->descriptionIsRich() ||
      mLoadedIncidence->description() != mUi->mDescriptionEdit->toPlainText();
  }
}

void IncidenceDescription::enableRichTextDescription( bool enable )
{
  QString rt( i18nc( "@info:label", "Enable rich text" ) );
  QString placeholder( "<a href=\"show\"><font color='blue'>%1 &gt;&gt;</font></a>" );
  
  if ( enable ) {
    rt = i18nc( "@info:label", "Disable rich text" );
    placeholder = QString( "<a href=\"show\"><font color='blue'>&lt;&lt; %1</font></a>" );
    mUi->mDescriptionEdit->enableRichTextMode();
  } else {
    mUi->mDescriptionEdit->switchToPlainText();
  }

  placeholder = placeholder.arg( rt );
  mUi->mRichTextLabel->setText( placeholder );
  mUi->mEditToolBarPlaceHolder->setVisible( enable );
  mUi->mDescriptionEdit->setActionsEnabled( enable );
  checkDirtyStatus();
}

void IncidenceDescription::toggleRichTextDescription()
{
  enableRichTextDescription( !mUi->mEditToolBarPlaceHolder->isVisible() );
}

void IncidenceDescription::setupToolBar()
{
  KActionCollection *collection = new KActionCollection( this ); //krazy:exclude=tipsandthis
  mUi->mDescriptionEdit->createActions( collection );

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

  // By default we don't show the rich text toolbar.
  mUi->mEditToolBarPlaceHolder->setVisible( false );
}

#include "moc_incidencedescription.cpp"
