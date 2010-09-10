/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include <QtCore/QDebug>

#include <KDE/KActionCollection>
#include <KDE/KToolBar>

#include <KDebug>

#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomoremobile.h"
#else
#include "ui_eventortododesktop.h"
#endif

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG {

class IncidenceDescriptionPrivate
{
  public:
    IncidenceDescriptionPrivate() : mRichTextEnabled( false )
    {
    }

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
  if ( incidence ) {
    enableRichTextDescription( incidence->descriptionIsRich() );
    if ( incidence->descriptionIsRich() ) {
      mUi->mDescriptionEdit->setHtml( incidence->richDescription() );
    } else {
      mUi->mDescriptionEdit->setText( incidence->description() );
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
  if ( d->mRichTextEnabled ) {
    return !mLoadedIncidence->descriptionIsRich() ||
      mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->toHtml();
  } else {
    return mLoadedIncidence->descriptionIsRich() ||
      mLoadedIncidence->description() != mUi->mDescriptionEdit->toPlainText();
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
  enableRichTextDescription( !d->mRichTextEnabled );
}

void IncidenceDescription::setupToolBar()
{
#ifndef QT_NO_TOOLBAR
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
#endif

  // By default we don't show the rich text toolbar.
  mUi->mEditToolBarPlaceHolder->setVisible( false );
  d->mRichTextEnabled = false;
}

#include "moc_incidencedescription.cpp"
