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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_RESOURCEFEATUREPLAN_H
#define KCAL_RESOURCEFEATUREPLAN_H

#include "prefs.h"

#include "kde-features.h"

#include <libkcal/resourcecached.h>

#include <kabc/locknull.h>

#include <kdepimmacros.h>
#include <kconfig.h>

namespace KCal {

/**
  This class represents a featureplan (in KDE XML format)
*/
class KDE_EXPORT ResourceFeaturePlan : public ResourceCached
{
    Q_OBJECT
  public:
    ResourceFeaturePlan( const KConfig * );
    virtual ~ResourceFeaturePlan();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    Prefs *prefs();

    bool doLoad();
    bool doSave();

    KABC::Lock *lock();

  protected:
    void insertCategories( const Category::List &categories, Todo *parent );

  private:
    Prefs *mPrefs;
    KABC::LockNull mLock;
};

}

#endif
