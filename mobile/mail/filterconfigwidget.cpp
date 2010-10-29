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
#include <KInputDialog>

using namespace MailCommon;

FilterConfigWidget::FilterConfigWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f), mFilter( 0 )
{
  mUi = new Ui_FilterConfigWidget;
  mUi->setupUi( this );
  mPatternEdit = new SearchPatternEdit( this );
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

void FilterConfigWidget::newFilter()
{
  bool ok = false;
  QString filterName = KInputDialog::getText( i18n( "Create filter" ), i18n( "Filter name" ), "", &ok,  this );
  if ( ok ) {
    MailFilter *newFilter = new MailFilter();
    newFilter->pattern()->setName( filterName );
    FilterIf->filterManager()->appendFilters( QList< MailFilter* > () << newFilter );
    loadFilter( newFilter );
  }
}

void FilterConfigWidget::deleteFilter( int filterIndex )
{
  QList< MailFilter* > filters = FilterIf->filterManager()->filters();
  if ( filterIndex < 0 || filterIndex >= filters.size() )
    return;

  mPatternEdit->reset();
  mActionLister->reset();
  
  MailFilter * filter = filters.at( filterIndex );
  FilterIf->filterManager()->removeFilter( filter );
  delete filter;
}

void FilterConfigWidget::renameFilter( int filterIndex )
{
  FilterManager* manager = FilterIf->filterManager();
  MailFilter* filter = manager->filters().at( filterIndex );
  bool ok = false;
  QString filterName = KInputDialog::getText( i18n( "Rename filter" ), i18n( "Edit name" ), filter->name(), &ok,  this );
  if ( ok ) {
    manager->beginUpdate();
    filter->pattern()->setName( filterName );
    manager->endUpdate();
  }
}

void FilterConfigWidget::moveUpFilter( int filterIndex )
{
  if ( filterIndex <= 0 )
    return; //first or invalid

  QList<MailFilter*> filters;

  FilterManager* manager = FilterIf->filterManager();
  QList<MailFilter*>::const_iterator it;
  for ( it = manager->filters().constBegin() ;
        it != manager->filters().constEnd();
        ++it ) {
    filters.append( new MailFilter( **it ) ); // deep copy
  }

  MailFilter *untouched = filters.at( filterIndex - 1 );
  MailFilter *moved = filters.takeAt( filterIndex );
  filters.insert( filters.indexOf( untouched ), moved );

  mPatternEdit->reset();
  mActionLister->reset();

  manager->setFilters( filters );
}

void FilterConfigWidget::moveDownFilter(int filterIndex)
{
  FilterManager* manager = FilterIf->filterManager();

  if ( filterIndex >= manager->filters().size() -1 || filterIndex < 0)
    return; //last or invalid

  QList<MailFilter*> filters;

  QList<MailFilter*>::const_iterator it;
  for ( it = manager->filters().constBegin() ;
        it != manager->filters().constEnd();
        ++it ) {
    filters.append( new MailFilter( **it ) ); // deep copy
  }

  MailFilter *moved = filters.takeAt( filterIndex );
  filters.insert( filterIndex + 2, moved );

  mPatternEdit->reset();
  mActionLister->reset();

  manager->setFilters( filters );
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

void DeclarativeFilterConfigWidget::newFilter()
{
  save();
  mFilterConfigWidget->newFilter();
}


void DeclarativeFilterConfigWidget::deleteFilter( int filterIndex )
{
  mFilterConfigWidget->deleteFilter( filterIndex );
}

void DeclarativeFilterConfigWidget::renameFilter( int filterIndex )
{
  mFilterConfigWidget->renameFilter( filterIndex );
}

void DeclarativeFilterConfigWidget::moveUpFilter( int filterIndex )
{
  mFilterConfigWidget->moveUpFilter( filterIndex );
}

void DeclarativeFilterConfigWidget::moveDownFilter(int filterIndex)
{
  mFilterConfigWidget->moveDownFilter( filterIndex );
}


#include "filterconfigwidget.moc"