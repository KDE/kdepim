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

#include "aclutils_p.h"

#include <KLocalizedString>

using namespace PimCommon;

static const struct {
    KIMAP::Acl::Rights permissions;
    const char *userString;
} standardPermissions[] = {
    {
        KIMAP::Acl::None,
        I18N_NOOP2("Permissions", "None")
    },

    {
        KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen,
        I18N_NOOP2("Permissions", "Read")
    },

    {
        KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen |
        KIMAP::Acl::Insert | KIMAP::Acl::Post,
        I18N_NOOP2("Permissions", "Append")
    },

    {
        KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen |
        KIMAP::Acl::Insert | KIMAP::Acl::Post | KIMAP::Acl::Write |
        KIMAP::Acl::CreateMailbox | KIMAP::Acl::DeleteMailbox |
        KIMAP::Acl::DeleteMessage | KIMAP::Acl::Expunge,
        I18N_NOOP2("Permissions", "Write")
    },

    {
        KIMAP::Acl::Lookup | KIMAP::Acl::Read | KIMAP::Acl::KeepSeen |
        KIMAP::Acl::Insert | KIMAP::Acl::Post | KIMAP::Acl::Write |
        KIMAP::Acl::CreateMailbox | KIMAP::Acl::DeleteMailbox |
        KIMAP::Acl::DeleteMessage | KIMAP::Acl::Expunge | KIMAP::Acl::Admin,
        I18N_NOOP2("Permissions", "All")
    }
};

uint AclUtils::standardPermissionsCount()
{
    return (sizeof(standardPermissions) / sizeof(*standardPermissions));
}

KIMAP::Acl::Rights AclUtils::permissionsForIndex(uint index)
{
    Q_ASSERT(index < standardPermissionsCount());

    return standardPermissions[ index ].permissions;
}

int AclUtils::indexForPermissions(KIMAP::Acl::Rights permissions)
{
    const uint maxSize(sizeof(standardPermissions) / sizeof(*standardPermissions));
    for (uint i = 0; i < maxSize; ++i) {
        if (KIMAP::Acl::normalizedRights(permissions) == standardPermissions[i].permissions) {
            return i;
        }
    }

    return -1;
}

QString AclUtils::permissionsToUserString(KIMAP::Acl::Rights permissions)
{
    const uint maxSize(sizeof(standardPermissions) / sizeof(*standardPermissions));
    for (uint i = 0; i < maxSize; ++i) {
        if (KIMAP::Acl::normalizedRights(permissions) == standardPermissions[i].permissions) {
            return i18nc("Permissions", standardPermissions[ i ].userString);
        }
    }

    return i18n("Custom Permissions (%1)",
                QString::fromLatin1(KIMAP::Acl::rightsToString(permissions)));
}
