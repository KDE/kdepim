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

#include <QApplication>
#include <QDateTime>
#include <q3ptrlist.h>
#include <QStringList>
#include <QTimer>

#include <kabc/locknull.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <libkdepim/kpimprefs.h>

using namespace KCal;


ResourceFeaturePlan::ResourceFeaturePlan()
  : ResourceCached(), mLock( true )
{
  mPrefs = new Prefs;
}

ResourceFeaturePlan::ResourceFeaturePlan( const KConfigGroup &group )
  : ResourceCached( group ), mLock( true )
{
  mPrefs = new Prefs;

  readConfig( group );
}

ResourceFeaturePlan::~ResourceFeaturePlan()
{
}

Prefs *ResourceFeaturePlan::prefs()
{
  return mPrefs;
}

void ResourceFeaturePlan::readConfig( const KConfigGroup & )
{
  mPrefs->readConfig();
}

void ResourceFeaturePlan::writeConfig( KConfigGroup &group )
{
  ResourceCalendar::writeConfig( group );

  mPrefs->writeConfig();
}

bool ResourceFeaturePlan::doLoad( bool )
{
  kDebug() << "ResourceFeaturePlan::load()" << endl;

  calendar()->close();

  FeaturesParser parser;

  bool ok;
  Features features = parser.parseFile( mPrefs->filename(), &ok );

  if ( ok ) {
    Category::List categories = features.categoryList();

    if ( !categories.isEmpty() ) {
      KCal::Todo *masterTodo = new KCal::Todo;
      masterTodo->setSummary( i18n("Feature Plan") );
      calendar()->addTodo( masterTodo );

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

      calendar()->addTodo( todo );
    }
  }
}

bool ResourceFeaturePlan::doSave( bool )
{
  return true;
}

KABC::Lock *ResourceFeaturePlan::lock()
{
  return &mLock;
}


#include "kcal_resourcefeatureplan.moc"
