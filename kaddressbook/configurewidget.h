/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef CONFIGUREWIDGET_H
#define CONFIGUREWIDGET_H

#include <qwidget.h>

#include <kconfig.h>

class ViewManager;

class ConfigureWidget : public QWidget
{
  public:
    ConfigureWidget( ViewManager *vm, QWidget *parent, const char *name = 0 );
    ~ConfigureWidget();

    /**
      This method is called before the configure dialog is shown.
      The widget should reimplement it and fill the GUI with the
      values from the config file.
      Important: Don't change the group of cfg!
     */
    virtual void restoreSettings( KConfig *cfg );

    /**
      This method is called after the user clicked the 'Ok' button.
      The widget should reimplement it and save all values from
      the GUI to the config file.
      Important: Don't change the group of cfg!
     */
    virtual void saveSettings( KConfig *cfg );


    /**
      Returns a pointer to the view manager of this widget.
     */
    ViewManager *viewManager();

  private:
    ViewManager *mViewManager;
};

#endif
