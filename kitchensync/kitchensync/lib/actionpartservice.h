/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_ACTIONPARTSERVICE_H
#define KSYNC_ACTIONPARTSERVICE_H

#include <qvaluelist.h>
#include <qstring.h>

#include <kservice.h>

namespace KSync {

/**
  A ActionPartService saves a converted KService::Ptr of a ActionPart

  @see KService::Ptr
*/
class ActionPartService
{
  public:
    typedef QValueList<ActionPartService> List;

    /**
      Creates an empty service.
    */
    ActionPartService();
    ActionPartService( const KService::Ptr &service );
    ~ActionPartService();

    bool operator==( const ActionPartService & );
    bool operator==( const ActionPartService & ) const;

    QString id() const;
    QString name() const;
    QString comment() const;
    QString libraryName() const;
    QString iconName() const;

    void setId( const QString & );
    void setName( const QString & );
    void setComment( const QString & );
    void setLibraryName( const QString & );
    void setIconName( const QString & );

    ActionPartService &operator=( const ActionPartService & );

    static const ActionPartService::List &availableParts();
    static ActionPartService partForId( const QString & );

  private:
    QString m_id;
    QString m_name;
    QString m_comment;
    QString m_iconName;
    QString m_libName;

    static bool mAvailablePartsRead;
    static ActionPartService::List mAvailableParts;
};

}


#endif
