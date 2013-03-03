/*
    Copyright (c) 2011 Tobias Koenig <tokoe@kde.org>

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

#ifndef GROUPWAREUIDELEGATE_H
#define GROUPWAREUIDELEGATE_H

#include "mobileuicalendar_export.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <Akonadi/Calendar/ITIPHandler>
#include <QtCore/QObject>

class MOBILEUICALENDAR_EXPORT GroupwareUiDelegate : public QObject, public Akonadi::GroupwareUiDelegate
{
  public:
    GroupwareUiDelegate();

    void setCalendar( const Akonadi::ETMCalendar::Ptr &calendar );
    void createCalendar();

    void requestIncidenceEditor( const Akonadi::Item &item );

  private:
    Akonadi::ETMCalendar::Ptr mCalendar;
};

#endif
