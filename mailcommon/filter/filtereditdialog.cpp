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

#include "filtereditdialog_p.h"

#include "filteractionwidget.h"
#include "filtermanager.h"
#include "mailfilter.h"
#include "search/searchpatternedit.h"
#include "ui_filterconfigwidget.h"

using namespace MailCommon;

FilterEditDialog::FilterEditDialog( QWidget *parent )
    : KDialog( parent ), mFilter( 0 )
{
    mUi = new Ui_FilterConfigWidget;
    mUi->setupUi( mainWidget() );

    mPatternEdit = new SearchPatternEdit( this, MailCommon::SearchPatternEdit::MatchAllMessages );
    mUi->criteriaLayout->addWidget( mPatternEdit, 0, Qt::AlignTop );
    mActionLister = new FilterActionWidgetLister( this );
    mUi->actionsLayout->addWidget( mActionLister, 0, Qt::AlignTop );
}

FilterEditDialog::~FilterEditDialog()
{
    delete mUi;
}

void FilterEditDialog::load( int index )
{
    mFilter = FilterManager::instance()->filters().at( index );

    if ( !mFilter )
        return;

    mPatternEdit->setSearchPattern( mFilter->pattern() );

    mActionLister->setActionList( mFilter->actions() );

    mUi->filterName->setText( mFilter->pattern()->name() );
    mUi->applyToIncomingCB->setChecked( mFilter->applyOnInbound() );
    mUi->applyToSentCB->setChecked( mFilter->applyOnOutbound() );
    mUi->applyBeforeSendCB->setChecked( mFilter->applyBeforeOutbound() );
    mUi->applyManuallyCB->setChecked( mFilter->applyOnExplicit() );
    mUi->stopIfMatchesCB->setChecked( mFilter->stopProcessingHere() );
}

void FilterEditDialog::save()
{
    if ( !mFilter )
        return;

    mPatternEdit->updateSearchPattern();
    mActionLister->updateActionList();

    FilterManager::instance()->beginUpdate();

    mFilter->pattern()->setName( mUi->filterName->text() );
    mFilter->setApplyOnInbound( mUi->applyToIncomingCB->isChecked() );
    mFilter->setApplyOnOutbound( mUi->applyToSentCB->isChecked() );
    mFilter->setApplyBeforeOutbound( mUi->applyBeforeSendCB->isChecked() );
    mFilter->setApplyOnExplicit( mUi->applyManuallyCB->isChecked() );
    mFilter->setStopProcessingHere( mUi->stopIfMatchesCB->isChecked() );

    FilterManager::instance()->endUpdate();
}

#include "moc_filtereditdialog_p.cpp"
