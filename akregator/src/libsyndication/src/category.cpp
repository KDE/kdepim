/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "category.h"

#include <QString>

namespace LibSyndication {

QString Category::debugInfo() const
{
    QString info;
    info += "# Category begin ##############\n";
    
    QString dterm = term();
    
    if (!dterm.isNull())
    {
        info += "term: #" + dterm + "#\n";
    }
    
    QString dscheme = scheme();
    
    if (!dscheme.isNull())
    {
        info += "scheme: #" + dscheme + "#\n";
    }
    
    QString dlabel = label();
    
    if (!dlabel.isNull())
    {
        info += "label: #" + dlabel + "#\n";
    }
    
    info += "# Category end ################\n";
    
    return info;
}

} // namespace LibSyndication
