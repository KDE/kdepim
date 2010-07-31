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

#include "tag.h"
#include "tagset.h"

#include <tqdom.h>
#include <tqmap.h>
#include <tqstring.h>
#include <tqstringlist.h>

namespace Akregator {

class TagSet::TagSetPrivate 
{
    public:
    TQMap<TQString,Tag> map;
};

TagSet::TagSet(TQObject* parent) : TQObject(parent), d(new TagSetPrivate)
{
}

TagSet::~TagSet()
{
    TQValueList<Tag> tags = d->map.values();
    for (TQValueList<Tag>::Iterator it = tags.begin(); it != tags.end(); ++it)
        (*it).removedFromTagSet(this);
    
    delete d;
    d = 0;
}

void TagSet::insert(const Tag& tag)
{
    if (!d->map.contains(tag.id()))
    {
        d->map.insert(tag.id(), tag);
        tag.addedToTagSet(this);
        emit signalTagAdded(tag);
    }
}

void TagSet::remove(const Tag& tag)
{
    if (d->map.contains(tag.id()))
    {
        d->map.remove(tag.id());
        tag.removedFromTagSet(this);
        emit signalTagRemoved(tag);
    }
}

bool TagSet::containsID(const TQString& id) const
{
    return d->map.contains(id);
}

bool TagSet::contains(const Tag& tag) const
{
    return d->map.contains(tag.id());
}

Tag TagSet::findByID(const TQString& id) const
{
    return d->map.contains(id) ? d->map[id] : Tag();
}

TQMap<TQString,Tag> TagSet::toMap() const
{
    return d->map;
}

void TagSet::readFromXML(const TQDomDocument& doc)
{
    TQDomElement root = doc.documentElement();

    if (root.isNull())
        return;

    TQDomNodeList list = root.elementsByTagName(TQString::fromLatin1("tag"));

    for (uint i = 0; i < list.length(); ++i)
    {
        TQDomElement e = list.item(i).toElement();
        if (!e.isNull())
        {
            if (e.hasAttribute(TQString::fromLatin1("id")))
            {
                TQString id = e.attribute(TQString::fromLatin1("id"));
                TQString name = e.text();
                TQString scheme = e.attribute(TQString::fromLatin1("scheme"));
                Tag tag(id, name, scheme);
                
                TQString icon = e.attribute(TQString::fromLatin1("icon"));
                if (!icon.isEmpty())
                    tag.setIcon(icon);

                insert(tag);
                
            }
        }
    }

}
void TagSet::tagUpdated(const Tag& tag)
{
    emit signalTagUpdated(tag);
}
        
TQDomDocument TagSet::toXML() const
{
    TQDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

    TQDomElement root = doc.createElement("tagSet");
    root.setAttribute( "version", "0.1" );
    doc.appendChild(root);

    TQValueList<Tag> list = d->map.values();
    for (TQValueList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {    
    
        TQDomElement tn = doc.createElement("tag");
        
        TQDomText text = doc.createTextNode((*it).name());
        tn.setAttribute(TQString::fromLatin1("id"),(*it).id());
        if (!(*it).scheme().isEmpty())
            tn.setAttribute(TQString::fromLatin1("scheme"),(*it).scheme());
        if (!(*it).icon().isEmpty())
            tn.setAttribute(TQString::fromLatin1("icon"),(*it).icon());
        tn.appendChild(text);
        root.appendChild(tn);
    }
    return doc;
}

} // namespace Akregator

#include "tagset.moc"
