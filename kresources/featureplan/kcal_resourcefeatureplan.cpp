/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kcal_resourcefeatureplan.h"

#include "kcal_resourcefeatureplanconfig.h"

#include "kde-features.h"
#include "kde-features_parser.h"

#include <qapplication.h>
#include <qdatetime.h>
#include <q3ptrlist.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <kabc/locknull.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <libkdepim/kpimprefs.h>

using namespace KCal;

ResourceFeaturePlan::ResourceFeaturePlan( const KConfig *config )
  : ResourceCached( config ), mLock( true )
{
  mPrefs = new Prefs;

  if ( config ) readConfig( config );
}

ResourceFeaturePlan::~ResourceFeaturePlan()
{
}

Prefs *ResourceFeaturePlan::prefs()
{
  return mPrefs;
}

void ResourceFeaturePlan::readConfig( const KConfig * )
{
  mPrefs->readConfig();
}

void ResourceFeaturePlan::writeConfig( KConfig *config )
{
  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();
}

bool ResourceFeaturePlan::doLoad()
{
  kDebug() << "ResourceFeaturePlan::load()" << endl;

  mCalendar.close();

  FeaturesParser parser;

  bool ok;
  Features features = parser.parseFile( mPrefs->filename(), &ok );

  if ( ok ) {
    Category::List categories = features.categoryList();

    if ( !categories.isEmpty() ) {
      KCal::Todo *masterTodo = new KCal::Todo;
      masterTodo->setSummary( i18n("Feature Plan") );
      mCalendar.addTodo( masterTodo );

      insertCategories( categories, masterTodo );
      emit resourceChanged( this );
    }
  }

  return ok;
}

void ResourceFeaturePlan::insertCategories( const Category::List &categories,
                                            Todo *parent )
{
  foreach ( Category c, categories ) {

    Todo *categoryTodo = new Todo;
    categoryTodo->setSummary( c.name() );
    categoryTodo->setRelatedTo( parent );

    insertCategories( c.categoryList(), categoryTodo );

    foreach ( Feature f,  c.featureList() ) {
      Todo *todo = new Todo;

      QString summary = f.summary();
      int pos = summary.indexOf( '\n' );
      if ( pos > 0 ) summary = summary.left( pos ) + "...";
      todo->setSummary( summary );

      todo->setDescription( f.summary() );

      todo->setRelatedTo( categoryTodo );

      int completed;
      if ( f.status() == "done" ) completed = 100;
      else if ( f.status() == "inprogress" ) completed = 50;
      else completed = 0;
      todo->setPercentComplete( completed );

      mCalendar.addTodo( todo );
    }
  }
}

bool ResourceFeaturePlan::doSave()
{
  return true;
}

KABC::Lock *ResourceFeaturePlan::lock()
{
  return &mLock;
}


#include "kcal_resourcefeatureplan.moc"
