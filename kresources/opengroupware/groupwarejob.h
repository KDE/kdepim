/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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
#ifndef KPIM_GROUPWAREJOB_H
#define KPIM_GROUPWAREJOB_H

#include <qobject.h>

namespace KPIM {

class GroupwareDataAdaptor;

/**
  This class provides a resource for accessing a Groupware kioslave-based
  calendar.
*/
class GroupwareJob : public QObject
{
    Q_OBJECT
  public:
    GroupwareJob( GroupwareDataAdaptor *adaptor );

    bool error() const;
    QString errorString() const;

    virtual void kill() = 0;

  signals:
    void result( KPIM::GroupwareJob * );

  protected:
    void success();
    void error( const QString & );

  protected slots:
    virtual void run() = 0;

  protected:
    GroupwareDataAdaptor *mAdaptor;

  private:
    QString mErrorString;
};

}

#endif
