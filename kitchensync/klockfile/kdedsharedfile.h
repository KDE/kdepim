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

class KShareFileModule  : public KDEDModule
{
Q_OBJECT
K_DCOP
 public:
  KShareFileModule(const QCString &obj );
  virtual ~KShareFileModule();
 k_dcop:
  ASYNC interestedIn(const QString &fileName );
  ASYNC removeInterestIn(const QString &fileName );
  bool readShareFile(const QString &fileName );
  bool readUnshareFile(const QString &fileName );
  bool writeLockFile(const QString &fileName);
  bool writeUnlockFile(const QString &fileName );

 private:
  class KShareFileModulePrivate;
  KShareFileModulePrivate *d;
};

#endif










