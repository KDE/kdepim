/*
    This file is part of libkdepim.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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

#ifndef RESOURCEABC_H
#define RESOURCEABC_H

#include <kabc/resource.h>
#include <tqmap.h>
#include <kdepimmacros.h>

// This is here because it can't go to kdelibs/kabc yet, but ultimately
// it should probably go there (maybe redesigned to have a real object
// for subresources).

namespace KPIM {

/**
 * This class is the implementation of subfolder resources for KABC.
 * More methods will be added to give KAddressBook the possibility to
 * handle subresources.
 */

class KDE_EXPORT ResourceABC : public KABC::Resource
{
  Q_OBJECT

public:
  ResourceABC( const KConfig* );
  virtual ~ResourceABC();

  /**
   * Get the UID to subresource map. This is necessary to implement
   * the search order.
   * The returned map has the UID as key and the resource it's in as
   * the data.
   */
  virtual TQMap<TQString, TQString> uidToResourceMap() const = 0;

  /**
   * If this resource has subresources, return a TQStringList of them.
   * In most cases, resources do not have subresources, so this is
   * by default just empty.
   */
  virtual TQStringList subresources() const { return TQStringList(); }

  /**
   * Is this subresource active or not?
   */
  virtual bool subresourceActive( const TQString& ) const { return true; }

  /**
   * Is the given subresource writable?
   */
  virtual bool subresourceWritable( const TQString& ) const = 0;

  /**
   * Completion weight for a given subresource
   */
  virtual int subresourceCompletionWeight( const TQString& ) const = 0;

  /**
   * Label for a given subresource
   */
  virtual TQString subresourceLabel( const TQString& ) const = 0;

public slots:
  /**
   * (De-)activate a subresource.
   */
  virtual void setSubresourceActive( const TQString &, bool active ) = 0;

  /**
   * Set completion weight for a given subresource
   */
  virtual void setSubresourceCompletionWeight( const TQString&, int weight ) = 0;

signals:
  /**
   * This signal is emitted when a subresource is added.
   */
  void signalSubresourceAdded( KPIM::ResourceABC *, const TQString &type,
                               const TQString &subResource );

  /**
   * This signal is emitted when a subresource is removed.
   */
  void signalSubresourceRemoved( KPIM::ResourceABC *, const TQString &type,
                                 const TQString &subResource );

};

}

#endif // RESOURCEABC_H
