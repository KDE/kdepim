/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

#ifndef KSYNC_CONFIGUREDIALOG_H
#define KSYNC_CONFIGUREDIALOG_H

#include <kdialogbase.h>

namespace KSync {

/**
 * The ConfigureDialog of the KitchenSync Framework
 * It'll contain all widgets of the ActionPart
 * @short The small ConfigurationDialog
 * @see ManipulatorPart
 * @author Zecke
 * @version 0.1
 */
class ConfigureDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /**
     * simple c'tor
     * @param parent The parent
     * @param name The name
     * @param modal if the dialog is modal
     */
    ConfigureDialog( QWidget *parent = 0, const char *name = 0, bool modal = true );
    ~ConfigureDialog();

    virtual void show();

    /**
     * add a widget to ConfigureDialog
     * @param widget The widget to be added. It'll be reparented
     * @param name The string shown as name
     * @param pixmap the QPixmap shown
     */
    void addWidget( QWidget *widget ,const QString &name, QPixmap *pixmap );

  signals:
    /**
     * Emitted whenever the Ok button is clicked.
     */
    void ok();

  protected slots:
     virtual void slotOk();
     virtual void slotCancel();

  protected:
     /**
      * Plugin sensitive.
      */
     void apply( bool );

  private:
     /**
      * load and registers the plugins
      */
     void setup();

     /**
      * unload the plugins
      */
     void unload();
};

}

#endif
