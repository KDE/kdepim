/* Copyright 2010 Thomas McGuire <mcguire@kde.org>
   Copyright 2012 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "tag.h"

#include <Akonadi/Tag>
#include <Akonadi/TagAttribute>

#include <QDebug>

using namespace MailCommon;

Tag::Ptr Tag::createDefaultTag(const QString& name)
{
  Tag::Ptr tag( new Tag() );
  tag->tagName = name;
  tag->iconName = QLatin1String("mail-tagged");

  tag->priority = -1;
  tag->inToolbar = false;
  tag->isImmutable = false;
  return tag;
}

Tag::Ptr Tag::fromAkonadi(const Akonadi::Tag& akonadiTag)
{
  Tag::Ptr tag( new Tag() );
  tag->tagName = akonadiTag.name();
  tag->mTag = akonadiTag;
  tag->priority = -1;
  tag->iconName = QLatin1String("mail-tagged");
  tag->inToolbar = false;
  tag->isImmutable = akonadiTag.isImmutable();
  Akonadi::TagAttribute *attr = akonadiTag.attribute<Akonadi::TagAttribute>();
  if (attr) {
    if (!attr->iconName().isEmpty()) {
      tag->iconName = attr->iconName();
    }
    tag->inToolbar = attr->inToolbar();
    tag->shortcut = KShortcut(attr->shortcut());
    tag->textColor = attr->textColor();
    tag->backgroundColor = attr->backgroundColor();
    if (!attr->font().isEmpty()) {
      tag->textFont.fromString( attr->font() );
    }
    tag->priority = attr->priority();
  }
  return tag;
}

Akonadi::Tag Tag::saveToAkonadi(Tag::SaveFlags saveFlags) const
{
  Akonadi::Tag tag( tagName );
  Akonadi::TagAttribute *attr = tag.attribute<Akonadi::TagAttribute>(Akonadi::AttributeEntity::AddIfMissing);
  attr->setDisplayName( tagName );
  attr->setIconName( iconName );
  attr->setInToolbar( inToolbar );
  attr->setShortcut( shortcut.toString() );
  attr->setPriority( priority );

  if ( textColor.isValid() && saveFlags & TextColor )
    attr->setTextColor( textColor );
  else
    attr->setTextColor( QColor() );

  if ( backgroundColor.isValid() && saveFlags & BackgroundColor )
    attr->setBackgroundColor( backgroundColor );
  else
    attr->setBackgroundColor( QColor() );

  if ( textFont != QFont() && saveFlags & Font )
    attr->setFont( textFont.toString() );
  else
    attr->setFont( QString() );

  tag.addAttribute(attr);
  return tag;
}

bool Tag::compare( Tag::Ptr &tag1, Tag::Ptr &tag2 )
{
  if ( tag1->priority < tag2->priority )
    return true;
  else if (tag1->priority == tag2->priority)
    return ( tag1->tagName < tag2->tagName );
  else
    return false;
}

bool Tag::compareName( Tag::Ptr &tag1, Tag::Ptr &tag2 )
{
  return ( tag1->tagName < tag2->tagName );
}

bool Tag::operator==( const Tag &other ) const
{
  if (mTag.isValid()) {
    return id() == other.id();
  }
  return tagName == other.tagName &&
         textColor == other.textColor &&
         backgroundColor == other.backgroundColor &&
         textFont == other.textFont &&
         iconName == other.iconName &&
         inToolbar == other.inToolbar &&
         shortcut.toString() == other.shortcut.toString() &&
         priority == other.priority;
}

bool Tag::operator!=( const Tag &other ) const
{
  return !( *this == other );
}

qint64 Tag::id() const
{
  return mTag.id();
}

QString Tag::name() const
{
  return mTag.name();
}

Akonadi::Tag Tag::tag() const
{
  return mTag;
}

