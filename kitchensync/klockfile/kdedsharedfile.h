/* This file is part of the KDE Project
   Copyright (c) 2002 Holger Freyther <freyther@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kdedsharedfile_h
#define kdedsharedfile_h

#include <kdedmodule.h>
#include <qstring.h>

/**
 * A small KDE Daemon to control
 * shared access to files. It will be started
 * on demand inside the KDED Server.
 * @short Controls shared access to resources
 * @author Holger 'zecke' Freyther <freyther@kde.org>
 * @version: 0.1
 */
class KShareFileModule  : public KDEDModule
{
Q_OBJECT
K_DCOP
 public:
 
 /**
  * This is the constructor it takes a QCString as parameter
  * @param obj the object
  */
  KShareFileModule(const QCString &obj );
  
  /**
   * This is the destructor.
   */
  virtual ~KShareFileModule();
 k_dcop:
 
  /**
   * This function will be called if a module
   * got interest on a resource
   * @param resource The resource interested in
   */
  ASYNC interestedIn(const QString &resource );
  
  /**
   * This function will remove the interest
   * in the resource.
   * @param resource The resource name
   */
  ASYNC removeInterestIn(const QString &resource );
  
  /**
   * This will try to share this file for reading
   * multiple resource can have a read share at the
   * same time. If there was no interest in the
   * resource it will be added.
   * see @ref interestedIn
   * @param resource The resource name
   * @return the success or failure of locking
   */
  bool readShareFile(const QString &resource );
  
  /**
   * This will remove a read share 
   * @param resource The resource name
   * @return success or failure of unlocking
   */
  bool readUnshareFile(const QString &resource );
  
  /**
   * This will try to lock a resource for
   * writing only. It will success if there
   * are no locks
   * @param resource The resource to lock
   * @return failure or success
   */
  bool writeLockFile(const QString &resource);
  
  /**
   * This will unlock the resource
   * @param resource The resource name
   * @return success or failure
   */
  bool writeUnlockFile(const QString &resource );

 private:
  class KShareFileModulePrivate;
  KShareFileModulePrivate *d;
};

#endif










