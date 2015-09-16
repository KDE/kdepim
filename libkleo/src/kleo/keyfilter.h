/*
    keyfilter.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#ifndef __KLEO_KEYFILTER_H__
#define __KLEO_KEYFILTER_H__

#include <QtCore/QFlags>

#include <algorithm>

namespace GpgME
{
class Key;
}

class QFont;
class QColor;
class QString;

namespace Kleo
{

/**
   @short An abstract base class key filters

*/
class KeyFilter
{
public:
    virtual ~KeyFilter() {}

    enum MatchContext {
        NoMatchContext = 0x0,
        Appearance = 0x1,
        Filtering = 0x2,

        AnyMatchContext = Appearance | Filtering
    };
    Q_DECLARE_FLAGS(MatchContexts, MatchContext)

    virtual bool matches(const GpgME::Key &key, MatchContexts ctx) const = 0;

    virtual unsigned int specificity() const = 0;
    virtual QString id() const = 0;
    virtual MatchContexts availableMatchContexts() const = 0;

    // not sure if we want these here, but for the time being, it's
    // the easiest way:
    virtual QColor fgColor() const = 0;
    virtual QColor bgColor() const = 0;
    virtual QString name() const = 0;
    virtual QString icon() const = 0;

    class FontDescription
    {
    public:
        FontDescription();
        FontDescription(const FontDescription &other);
        FontDescription &operator=(const FontDescription &other)
        {
            FontDescription copy(other);
            swap(copy);
            return *this;
        }
        ~FontDescription();

        static FontDescription create(bool bold, bool italic, bool strikeOut);
        static FontDescription create(const QFont &font, bool bold, bool italic, bool strikeOut);

        QFont font(const QFont &base) const;

        FontDescription resolve(const FontDescription &other) const;

        void swap(FontDescription &other)
        {
            std::swap(this->d, other.d);
        }
        struct Private;
    private:
        Private *d;
    };

    virtual FontDescription fontDesription() const = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KeyFilter::MatchContexts)

}

#endif // __KLEO_KEYFILTER_H__
