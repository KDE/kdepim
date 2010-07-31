/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <kurl.h>
#include <kio/global.h>
#include <kio/slave.h>

class Settings
{
  public:
    ~Settings();

    static Settings* self();

    KIO::MetaData accountData() const;
    KURL accountUrl() const;
    TQString accountPassword() const;

    KIO::Slave *globalSlave() const;

    TQString rulesWizardUrl() const;

    TQString ldapHost() const;
    TQString ldapPort() const;
    TQString ldapBase() const;
    TQString ldapBindDn() const;
    TQString ldapPassword() const;

  private:
    Settings();
    KIO::Slave *mSlave;

    static Settings* mSelf;
};

#endif
