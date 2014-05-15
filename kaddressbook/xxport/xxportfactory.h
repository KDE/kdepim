/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef XXPORTFACTORY_H
#define XXPORTFACTORY_H

#include "xxport.h"

/**
 * @short A factory for import/export modules.
 *
 * This class is a factory that creates import/export modules
 * for a given identifier.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class XXPortFactory
{
public:
    /**
     * Returns a new import/export module that matches the given
     * @p identifier or a null pointer if the identifier is invalid.
     *
     * The caller has to take ownership of the created module.
     */
    XXPort *createXXPort( const QString &identifier, QWidget *parentWidget ) const;
};

#endif
