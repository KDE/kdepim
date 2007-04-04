/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/
#ifndef PLUGINPICKER_H
#define PLUGINPICKER_H

#include <libqopensync/plugin.h>

#include <kdialogbase.h>
#include <kwidgetlist.h>

#include <qwidget.h>

class PluginItem : public KWidgetListItem
{
  public:
   PluginItem( KWidgetList *, const QSync::Plugin & );

   QSync::Plugin plugin() const { return mPlugin; }

  private:
    QSync::Plugin mPlugin;
};

class PluginPicker : public QWidget
{
  Q_OBJECT

  public:
    PluginPicker( QWidget *parent );

    QSync::Plugin selectedPlugin() const;

  signals:
    void selected();

  protected:
    void updatePluginList();

  private:
    KWidgetList *mPluginList;
};

class PluginPickerDialog : public KDialogBase
{
  Q_OBJECT

  public:
    PluginPickerDialog( QWidget *parent );

    QSync::Plugin selectedPlugin() const;

    static QSync::Plugin getPlugin( QWidget *parent );

  protected slots:
    void slotOk();
    void slotCancel();

  private:
    PluginPicker *mPicker;
};

#endif
