/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_PROFILECONFIG_H
#define KSYNC_PROFILECONFIG_H

#include "profile.h"

class KConfig;

namespace KSync {

/**
  @internal
  It's responsible for loading and saving a list
  of profiles somewhere.... kconfig currently
*/
class ProfileConfig
{
  public:
    ProfileConfig();
    ~ProfileConfig();

    Profile::List load();
    void save( const Profile::List & );

  protected:
    Profile::List defaultProfiles();

  private:
    void addPart( const QString &id, ActionPartService::List & );

    void saveProfile( KConfig *conf, const Profile &prof );
    void saveActionPart( KConfig *conf,  const ActionPartService & );
    Profile readProfile( KConfig * );
    void clear( KConfig *conf );

    KConfig *mConfig;
};

}

#endif
