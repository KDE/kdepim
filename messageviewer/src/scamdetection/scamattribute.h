/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SCAMATTRIBUTE_H
#define SCAMATTRIBUTE_H

#include <AkonadiCore/attribute.h>
namespace MessageViewer
{
class ScamAttributePrivate;

class ScamAttribute : public Akonadi::Attribute
{
public:
    explicit ScamAttribute();
    ~ScamAttribute();

    ScamAttribute *clone() const Q_DECL_OVERRIDE;
    QByteArray type() const Q_DECL_OVERRIDE;
    QByteArray serialized() const Q_DECL_OVERRIDE;
    void deserialize(const QByteArray &data) Q_DECL_OVERRIDE;

    bool isAScam() const;
    void setIsAScam(bool b);

    bool operator==(const ScamAttribute &other) const;

private:
    friend class ScamAttributePrivate;
    ScamAttributePrivate *const d;
};
}

#endif // SCAMATTRIBUTE_H
