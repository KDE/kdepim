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
#include "feed.h"
#include "image.h"
#include "item.h"
#include "person.h"

#include <QList>
#include <QString>

namespace Syndication {

Feed::~Feed()
{
}

QString Feed::debugInfo() const
{
    QString info;
    
    info += "# Feed begin ######################\n";

    QString dtitle = title();
    if (!dtitle.isNull())
        info += "title: #" + dtitle + "#\n";
    
    QString dlink = link();
    if (!dlink.isNull())
        info += "link: #" + dlink + "#\n";
    
    QString ddescription = description();
    if (!ddescription.isNull())
        info += "description: #" + ddescription + "#\n";
    
    QString dcopyright = copyright();
    if (!dcopyright.isNull())
        info += "copyright: #" + dcopyright + "#\n";

    QString dlanguage = language();
    if (!dlanguage.isNull())
        info += "language: #" + dlanguage + "#\n";
    
    QList<PersonPtr> dauthors = authors();
    QList<PersonPtr>::ConstIterator itp = dauthors.begin();
    QList<PersonPtr>::ConstIterator endp = dauthors.end();
    
    for ( ; itp != endp; ++itp)
        info += (*itp)->debugInfo();
                
    QList<CategoryPtr> dcategories = categories();
    QList<CategoryPtr>::ConstIterator itc = dcategories.begin();
    QList<CategoryPtr>::ConstIterator endc = dcategories.end();
    
    for ( ; itc != endc; ++itc)
        info += (*itc)->debugInfo();

    ImagePtr dimage = image();
     
    if (!dimage->isNull())
        info += dimage->debugInfo();
    
    QList<ItemPtr> ditems = items();
    QList<ItemPtr>::ConstIterator it = ditems.begin();
    QList<ItemPtr>::ConstIterator end = ditems.end();
    
    for ( ; it != end; ++it)
        info += (*it)->debugInfo();

    info += "# Feed end ########################\n";

    return info;
}

} // namespace Syndication
