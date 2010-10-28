/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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


#include "filterconfigwidget.h"
#include "ui_filterconfigwidget.h"

#include "mailcommon/mailfilter.h"
#include "mailcommon/mailkernel.h"

#include <stylesheetloader.h>
#include <mailcommon/filtermanager.h>
#include <mailcommon/searchpatternedit.h>
#include <mailcommon/filteractionwidget.h>

using namespace MailCommon;

FilterConfigWidget::FilterConfigWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f), mFilter( 0 )
{
  mUi = new Ui_FilterConfigWidget;
  mUi->setupUi( this );
  mPatternEdit = new SearchPatternEdit( i18n("Criteria"), this );
  mUi->criteriaLayout->addWidget( mPatternEdit, 0, Qt::AlignTop );
  mActionLister = new FilterActionWidgetLister( this );
  mUi->actionsLayout->addWidget( mActionLister, 0, Qt::AlignTop );
}

FilterConfigWidget::~FilterConfigWidget()
{
  delete mUi;
}

void FilterConfigWidget::loadFilter(MailCommon::MailFilter* filter)
{
  mFilter = filter;

  if ( !mFilter )
    return;

  mPatternEdit->setSearchPattern( mFilter->pattern() );

  mActionLister->setActionList( mFilter->actions() );

  mUi->applyToIncomingCB->setChecked( mFilter->applyOnInbound() );
  mUi->applyToSentCB->setChecked( mFilter->applyOnOutbound() );
  mUi->applyBeforeSendCB->setChecked( mFilter->applyBeforeOutbound() );
  mUi->applyManuallyCB->setChecked( mFilter->applyOnExplicit() );
  mUi->stopIfMatchesCB->setChecked( mFilter->stopProcessingHere() );
}

void FilterConfigWidget::save()
{
  if ( !mFilter )
    return;
  
  mFilter->setApplyOnInbound( mUi->applyToIncomingCB->isChecked() );
  mFilter->setApplyOnOutbound( mUi->applyToSentCB->isChecked() );
  mFilter->setApplyBeforeOutbound( mUi->applyBeforeSendCB->isChecked() );
  mFilter->setApplyOnExplicit( mUi->applyManuallyCB->isChecked() );
  mFilter->setStopProcessingHere( mUi->stopIfMatchesCB->isChecked() );
}



DeclarativeFilterConfigWidget::DeclarativeFilterConfigWidget( QGraphicsItem *parent )
  : QGraphicsProxyWidget( parent ), mFilterConfigWidget( new FilterConfigWidget )
{
  QPalette palette = mFilterConfigWidget->palette();
  palette.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );
  mFilterConfigWidget->setPalette( palette );
  StyleSheetLoader::applyStyle( mFilterConfigWidget );

  setWidget( mFilterConfigWidget );
  setFocusPolicy( Qt::StrongFocus );
}

DeclarativeFilterConfigWidget::~DeclarativeFilterConfigWidget()
{
}

void DeclarativeFilterConfigWidget::loadFilter( int filterIndex )
{
  save();
  if ( filterIndex >= FilterIf->filterManager()->filters().size() || filterIndex < 0 )
    return;
  
  mFilterConfigWidget->loadFilter( FilterIf->filterManager()->filters().at( filterIndex ) );
}

void DeclarativeFilterConfigWidget::save()
{
  mFilterConfigWidget->save();
  FilterIf->filterManager()->writeConfig();
}


#include "filterconfigwidget.moc"