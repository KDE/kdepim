/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "shared.h"
#include "tag.h"
#include "tagset.h"

#include <tqstring.h>
#include <tqvaluelist.h>

namespace Akregator {

class Tag::TagPrivate : public Shared
{
    public:
    TQString id;
    TQString name;
    TQString scheme;
    TQString icon;

    TQValueList<TagSet*> tagSets;
    bool operator==(const TagPrivate& other) const
    {
        return id == other.id; // name is ignored!
    }
};

Tag::Tag() : d(new TagPrivate)
{}

Tag::Tag(const TQString& id, const TQString& name, const TQString& scheme) : d(new TagPrivate)
{
    d->id = id;
    d->name = name.isNull() ? id : name;
    d->scheme = scheme;
    d->icon = "rss_tag";
}

Tag Tag::fromCategory(const TQString& term, const TQString& scheme, const TQString& name)
{
    Tag tag(scheme + "/" + term, name, scheme);
    return tag;
}


Tag::Tag(const Tag& other) : d(0)
{
    *this = other;
}

Tag::~Tag()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Tag& Tag::operator=(const Tag& other)
{
    if (this != &other)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}


bool Tag::operator==(const Tag& other) const
{
    return *(other.d) == *d;
}

bool Tag::operator<(const Tag& other) const
{
    return (name() < other.name()) || (name() == other.name() && id() < other.id());
}

bool Tag::isNull() const
{
    return d->id.isNull();
}

TQString Tag::name() const
{
    return d->name;
}

TQString Tag::scheme() const
{
    return d->scheme;
}

TQString Tag::icon() const
{
    return d->icon;
}

void Tag::setIcon(const TQString& icon)
{
    if (icon != d->icon)
    {
        d->icon = icon;
        for (TQValueList<TagSet*>::ConstIterator it = d->tagSets.begin(); it != d->tagSets.end(); ++it)
            (*it)->tagUpdated(*this);
    }
}


void Tag::setName(const TQString& name)
{
    if (name != d->name)
    {
        d->name = name;
        for (TQValueList<TagSet*>::ConstIterator it = d->tagSets.begin(); it != d->tagSets.end(); ++it)
            (*it)->tagUpdated(*this);
    }
}

void Tag::addedToTagSet(TagSet* tagSet) const
{
    d->tagSets.append(tagSet);
}

void Tag::removedFromTagSet(TagSet* tagSet) const
{
    d->tagSets.remove(tagSet);
}

TQString Tag::id() const
{
    return d->id;
}

} // namespace Akregator
