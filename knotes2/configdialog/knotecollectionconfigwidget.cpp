/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "knotecollectionconfigwidget.h"
#include "pimcommon/folderdialog/checkedcollectionwidget.h"

#include "akonadi_next/note.h"

#include <Akonadi/ETMViewStateSaver>

#include <KLocale>
#include <KGlobal>

#include <QHBoxLayout>

KNoteCollectionConfigWidget::KNoteCollectionConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mCheckedCollection = new PimCommon::CheckedCollectionWidget(Akonotes::Note::mimeType());
    setLayout(lay);
}

KNoteCollectionConfigWidget::~KNoteCollectionConfigWidget()
{

}

void KNoteCollectionConfigWidget::load()
{
    mModelState = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>( KGlobal::config()->group( "CheckState" ), this );
    mModelState->setSelectionModel( mCheckedCollection->selectionModel() );
    mModelState->restoreState();
}

void KNoteCollectionConfigWidget::save()
{
    mModelState->saveState();
}

