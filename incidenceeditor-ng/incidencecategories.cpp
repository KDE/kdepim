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

#include "incidencecategories.h"

#include "editorconfig.h"

#include "ui_dialogdesktop.h"

#include <libkdepim/widgets/tagwidgets.h>
#include <KConfigSkeleton>
#include <KDebug>

#include <Akonadi/TagCreateJob>

using namespace IncidenceEditorNG;

IncidenceCategories::IncidenceCategories( Ui::EventOrTodoDesktop *ui )
  : mUi( ui )
{
  setObjectName( "IncidenceCategories" );

  connect( mUi->mTagWidget, SIGNAL(selectionChanged(Akonadi::Tag::List)),
          SLOT(onSelectionChanged(Akonadi::Tag::List)) );
}

void IncidenceCategories::onSelectionChanged(const Akonadi::Tag::List &list)
{
  mSelectedTags = list;
  mDirty = true;
  checkDirtyStatus();
}

void IncidenceCategories::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;
  mDirty = false;
  if ( mLoadedIncidence ) {
    checkForUnknownCategories( mLoadedIncidence->categories() );
  } else {
    mSelectedTags.clear();
  }

  mWasDirty = false;
}

void IncidenceCategories::load( const Akonadi::Item &item )
{
  mSelectedTags = item.tags();
  mUi->mTagWidget->setSelection(item.tags());
}

void IncidenceCategories::save( const KCalCore::Incidence::Ptr &incidence )
{
  Q_ASSERT( incidence );
  incidence->setCategories( categories() );
}

void IncidenceCategories::save( Akonadi::Item &item )
{
  item.setTags(mSelectedTags);
}

QStringList IncidenceCategories::categories() const
{
  QStringList list;
  Q_FOREACH (const Akonadi::Tag &tag, mSelectedTags) {
    list << tag.name();
  }
  return list;
}

bool IncidenceCategories::isDirty() const
{
  return mDirty;
}

void IncidenceCategories::printDebugInfo() const
{
  kDebug() << "mSelectedCategories = " << categories();
  kDebug() << "mLoadedIncidence->categories() = " << mLoadedIncidence->categories();
}

void IncidenceCategories::checkForUnknownCategories( const QStringList &categoriesToCheck )
{
  foreach ( const QString &category, categoriesToCheck ) {
    Akonadi::TagCreateJob *tagCreateJob = new Akonadi::TagCreateJob(Akonadi::Tag(category), this);
    tagCreateJob->setMergeIfExisting(true);
    //TODO add the missing tags to the item and add them to the list of selected tags in the widget
  }
}
