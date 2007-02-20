/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#ifndef VIEWCONFIGUREWIDGET_H
#define VIEWCONFIGUREWIDGET_H

#include <kdialog.h>
#include <kdemacros.h>
#include <QPixmap>
#include <kvbox.h>

#include "configurewidget.h"

class KPageWidget;

class QString;
class KVBox;

class ViewConfigureFieldsPage;
class ViewConfigureFilterPage;

/**
  This widget is the base class for all view configuration widgets. The
  author of a view may wish to inherit from this widget and add config pages
  that add custom config options. The default implementation of this widget
  is to show a page with the select fields widget. For simple views this may
  be sufficient.
*/
class KDE_EXPORT ViewConfigureWidget : public KAB::ConfigureWidget
{
  Q_OBJECT

  public:
    ViewConfigureWidget( KABC::AddressBook *ab, QWidget *parent );
    virtual ~ViewConfigureWidget();

    /**
      Reads the configuration from the config object and sets the values
      in the GUI. If this method is overloaded, be sure to call the base
      class's method.

     */
    virtual void restoreSettings( const KConfigGroup &config );

    /**
      Writes the configuration from the GUI to the config object. If this
      method is overloaded, be sure to call the base class's method.

     */
    virtual void saveSettings( KConfigGroup &config );


    /**
      Use this method to add new pages to the widget.
     */
	KVBox *addPage( const QString &item, const QString &header = QString(),
                    const QPixmap &pixmap = QPixmap() );

  private:
    KPageWidget *mMainWidget;

    ViewConfigureFieldsPage *mFieldsPage;
    ViewConfigureFilterPage *mFilterPage;
};

class ViewConfigureDialog : public KDialog
{
  Q_OBJECT

  public:
    ViewConfigureDialog( ViewConfigureWidget *wdg, const QString &viewName,
                         QWidget *parent, const char *name = 0 );
    virtual ~ViewConfigureDialog();

    void restoreSettings( const KConfigGroup& );
    void saveSettings( KConfigGroup& );

  protected slots:
    void slotHelp();

  private:
    ViewConfigureWidget *mConfigWidget;
};

#endif
