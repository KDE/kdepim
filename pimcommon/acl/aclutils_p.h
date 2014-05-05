/*
 * Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
 * Copyright (c) 2010 Tobias Koenig <tokoe@kdab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MAILCOMMON_ACLUTILS_P_H
#define MAILCOMMON_ACLUTILS_P_H

#include <KImap/kimap/Acl>

namespace PimCommon {

namespace AclUtils {

/**
 * Returns the number of standard permissions available.
 */
uint standardPermissionsCount();

/**
 * Returns the standard permissions at the given @p index.
 */
KIMAP::Acl::Rights permissionsForIndex( uint index );

/**
 * Returns the index of the given standard @p permissions.
 *
 * If there are no matching permissions, @c -1 is returned.
 */
int indexForPermissions( KIMAP::Acl::Rights permissions );

/**
 * Returns the i18n'd representation of the given @p permissions.
 */
QString permissionsToUserString( KIMAP::Acl::Rights permissions );

}

}

#endif
