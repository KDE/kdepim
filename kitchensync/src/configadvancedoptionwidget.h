/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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
*/

#ifndef CONFIGADVANCEDOPTIONWIDGET_H
#define CONFIGADVANCEDOPTIONWIDGET_H

#include <QtCore/QList>
#include <QtGui/QWidget>

#include <libqopensync/pluginadvancedoption.h>

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class KComboBox;
class KLineEdit;

class OptionWidget : public QWidget
{
  public:
    typedef QList<OptionWidget*> List;

    OptionWidget( const QSync::PluginAdvancedOption &option, QWidget *parent = 0 );
    ~OptionWidget();

    void load();
    void save();

  private:
    QCheckBox *mBoolValue;
    QDoubleSpinBox *mDoubleValue;
    QSpinBox *mIntValue;
    KComboBox *mEnumValue;
    KLineEdit *mStringValue;

    QSync::PluginAdvancedOption mOption;
};

class ConfigAdvancedOptionWidget : public QWidget
{
  public:
    ConfigAdvancedOptionWidget( const QSync::PluginAdvancedOption::List &options, QWidget *parent = 0 );
    ~ConfigAdvancedOptionWidget();

    void load();
    void save();

  private:
    OptionWidget::List mOptionWidgets;
};

#endif
