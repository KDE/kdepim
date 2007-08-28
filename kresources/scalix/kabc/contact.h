/*
 *   This file is part of the scalix resource.
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

#ifndef SCALIX_CONTACT_H
#define SCALIX_CONTACT_H

#include <kabc/addressee.h>

namespace Scalix {

class Contact
{
  public:
    static QString toXml( const KABC::Addressee &addr );
    static KABC::Addressee fromXml( const QString &xml );
};

}

#endif
