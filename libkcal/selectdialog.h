/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef LIBKCAL_KRESOURCES_SELECTDIALOG_H
#define LIBKCAL_KRESOURCES_SELECTDIALOG_H

#include <qobject.h>
#include <qptrlist.h>
#include <qmap.h>

#include <kdialog.h>

class KListBox;

namespace KCal {

class ResourceCalendar;
class Incidence;

/**
 * Dialog for selecting a resource.
 *
 * Example:
 *
 * \code
 *
 * QPtrList<Resource> list = ... // can be retrived from KRES::Manager (e.g. KABC::AddressBook)
 *
 * KABC::Resource *res = KABC::SelectDialog::getResource( list, parentWdg );
 * if ( !res ) {
 *   // no resource selected
 * } else {
 *   // do something with resource
 * }
 * \endcode
 */
class SelectDialog : KDialog
{
  public:
    /**
     * Constructor.
     * @param list   The list of available resources
     * @param parent The parent widget
     * @param name   The name of the dialog
     */
    SelectDialog( QPtrList<ResourceCalendar> list, Incidence* incidence,
                  QWidget *parent = 0, const char *name = 0);

    /**
     * Returns selected resource.
     */
    ResourceCalendar *resource();

    /**
     * Opens a dialog showing the available resources and returns the resource the
     * user has selected. Returns 0, if the dialog was canceled.
     */
    static ResourceCalendar *getResource( QPtrList<ResourceCalendar> list,
                                          Incidence* incidence,
                                          QWidget *parent = 0 );

  private:
    KListBox *mResourceId;

    QMap<int, ResourceCalendar*> mResourceMap;
};

}

#endif
