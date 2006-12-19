/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#ifndef KPIM_TRANSPORTMANAGEMENTWIDGET_H
#define KPIM_TRANSPORTMANAGEMENTWIDGET_H

#include <mailtransport/mailtransport_export.h>
#include <QtGui/QWidget>

namespace KPIM {

/**
  A widget to manage mail transports.
*/
class MAILTRANSPORT_EXPORT TransportManagementWidget : public QWidget
{
  Q_OBJECT

  public:
    /**
      Creates a new TransportManagementWidget.
      @param parent The parent widget.
    */
    TransportManagementWidget( QWidget *parent = 0 );

    /**
      Destroys the widget.
    */
    virtual ~TransportManagementWidget();

  private slots:
    void fillTransportList();
    void updateButtonState();
    void addClicked();
    void editClicked();
    void removeClicked();
    void defaultClicked();

  private:
    class Private;
    Private * const d;
};

}

#endif
