/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNCUI_H
#define KSYNCUI_H

class KSyncEntry;

#include <kdemacros.h>

/**
  @short Syncing conflict resolution user interface.
  @author Cornelius Schumacher
  @see KSyncer
  
  This class provides the abstract interface to a conflict resolution user
  interface. It is needed for cases, when a syncing process cannot resolve
  conflicts automatically. This is the case, when the same data entry has been
  changed in different data sets in an incompatible way.
  
  This class has to be implemented by a concrete subclass, which provides the
  actual user interface. While a GUI implementation, which provides interactive
  conflict resolution, is the most common implementation, there might also be
  use for a non-GUI or even non-interactive user interface.
*/
class KDE_EXPORT KSyncUi
{
  public:
    KSyncUi();
    virtual ~KSyncUi();
    
    /**
      Deconflict two conflicting @ref KSyncEntry objects. Returns the entry,
      which has been chosen by the user to take precedence over the other.
      
      The default implementation always returns 0, which should be interpreted
      to not sync the entries at all. Reimplement this function in a subclass to
      provide a more useful implementation to @ref KSyncer.
    */
    virtual KSyncEntry *deconflict(KSyncEntry *syncEntry,KSyncEntry *target);
};

#endif
