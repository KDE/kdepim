/* This file is part of the KDE libraries
   Copyright (C) 2002 Carsten Burghardt <burghardt@kde.org>

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
#ifndef __KACCOUNT
#define __KACCOUNT

#include <qstring.h>

class KAccount
{
  public:
    /** Type information */
    enum Type {
      Imap,
      MBox,
      Maildir,
      News,
      Other
    };

    KAccount( const uint id = 0, const QString &name = QString::null,
       const Type type = Other );
    
    /**
     * Get/Set name
     */ 
    QString name() { return mName; }
    void setName( const QString& name ) { mName = name; }
    
    /**
     * Get/Set id
     */ 
    uint id() { return mId; }
    void setId( const uint id ) { mId = id; }

    /**
     * Get/Set type
     */ 
    Type type() { return mType; }
    void setType( const Type type ) { mType = type; }

  protected:
    uint mId;
    QString mName;
    Type mType; 
};

#endif
