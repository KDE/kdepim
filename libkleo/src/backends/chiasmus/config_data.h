/*
    config_data.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEO__CHIASMUS_CONFIG_DATA_H__
#define __KLEO__CHIASMUS_CONFIG_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

struct kleo_chiasmus_config_data {
    const char *name;
    const char *description;
    int level;
    int type;
    union {
        const char *path;  /* must be first, see config_data.c */
        const char *string;
        const char *url;
        struct {
            unsigned int value : 1;
            unsigned int numTimesSet : 31;
        } boolean;
        int integer;
        unsigned int unsigned_integer;
    } defaults;
    unsigned int is_optional : 1;
    unsigned int is_list : 1;
    unsigned int is_runtime : 1;
};

extern const struct kleo_chiasmus_config_data kleo_chiasmus_config_entries[];
extern const unsigned int kleo_chiasmus_config_entries_dim;

#ifdef __cplusplus
}
#endif

#endif /* __KLEO__CHIASMUS_CONFIG_DATA_H__ */

