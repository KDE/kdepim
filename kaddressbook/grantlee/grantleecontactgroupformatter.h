/*
  This file is part of KAddressBook.

  Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef GRANTLEECONTACTGROUPFORMATTER_H
#define GRANTLEECONTACTGROUPFORMATTER_H

#include <Akonadi/Contact/AbstractContactGroupFormatter>
#include "kaddressbook_grantlee_export.h"

namespace GrantleeTheme {
class Theme;
}

namespace Akonadi {

/**
 * @short A class that formats a contact group as HTML code.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class KADDRESSBOOK_GRANTLEE_EXPORT GrantleeContactGroupFormatter : public AbstractContactGroupFormatter
{
  public:
    /**
     * Creates a new grantlee contact group formatter.
     */
    explicit GrantleeContactGroupFormatter();

    void setGrantleeTheme(const GrantleeTheme::Theme &theme);

    /**
     * Destroys the grantlee contact group formatter.
     */
    virtual ~GrantleeContactGroupFormatter();

    /**
     * Returns the contact group formatted as HTML
     */
    virtual QString toHtml( HtmlForm form = SelfcontainedForm ) const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
