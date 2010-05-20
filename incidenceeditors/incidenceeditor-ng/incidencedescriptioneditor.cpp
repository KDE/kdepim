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

#include "incidencedescriptioneditor.h"

#include <QtCore/QDebug>

#include <KDE/KActionCollection>
#include <KDE/KToolBar>

#include "ui_incidencedescription.h"

using namespace IncidenceEditorsNG;

IncidenceDescriptionEditor::IncidenceDescriptionEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mUi( new Ui::IncidenceDescriptionEditor )
{
  mUi->setupUi( this );
  mUi->mDescriptionEdit->setRichTextSupport( KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportItalic |
                                             KRichTextWidget::SupportUnderline |
                                             KRichTextWidget::SupportStrikeOut |
                                             KRichTextWidget::SupportChangeListStyle |
                                             KRichTextWidget::SupportAlignment |
                                             KRichTextWidget::SupportFormatPainting );

  setupToolBar();
  
  connect( mUi->mRichTextCheck, SIGNAL(toggled(bool)),
           this, SLOT(enableRichTextDescription(bool)) );
  connect( mUi->mDescriptionEdit, SIGNAL(textChanged()),
           this, SLOT(checkDirtyStatus()) );
}

void IncidenceDescriptionEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( incidence ) {
    enableRichTextDescription( incidence->descriptionIsRich() );
    mUi->mRichTextCheck->setChecked( incidence->descriptionIsRich() );
    if ( incidence->descriptionIsRich() )
      mUi->mDescriptionEdit->setHtml( incidence->richDescription() );
    else
      mUi->mDescriptionEdit->setText( incidence->description() );
  } else {
    enableRichTextDescription( false );
    mUi->mRichTextCheck->setChecked( false );
    mUi->mDescriptionEdit->clear();
  }

  mWasDirty = false;
}

void IncidenceDescriptionEditor::save( KCal::Incidence::Ptr incidence )
{
  if ( mUi->mRichTextCheck->isChecked() ) {
    incidence->setDescription( mUi->mDescriptionEdit->toHtml(), true );
  } else {
    incidence->setDescription( mUi->mDescriptionEdit->toPlainText(), false );
  }
}

bool IncidenceDescriptionEditor::isDirty() const
{  
  if ( mUi->mRichTextCheck->isChecked() ) {
    return !mLoadedIncidence->descriptionIsRich() ||
      mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->toHtml();
  } else {
    return mLoadedIncidence->descriptionIsRich() ||
      mLoadedIncidence->description() != mUi->mDescriptionEdit->toPlainText();
  }
}

void IncidenceDescriptionEditor::enableRichTextDescription( bool enable )
{
  mUi->mDescriptionEdit->setActionsEnabled( enable );
  if ( !enable ) {
    mUi->mDescriptionEdit->switchToPlainText();
  } else {
    mUi->mDescriptionEdit->enableRichTextMode();
  }

  checkDirtyStatus();
}

void IncidenceDescriptionEditor::setupToolBar()
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
  mUi->mDescriptionEdit->setActionsEnabled( mUi->mRichTextCheck->isChecked() );

  QGridLayout *layout = new QGridLayout( mUi->mEditToolBarPlaceHolder );
  layout->addWidget( mEditToolBar );
}

#include "moc_incidencedescriptioneditor.cpp"
