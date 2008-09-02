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

#ifndef CONFIGRESOURCEWIDGET_H
#define CONFIGRESOURCEWIDGET_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QWidget>

#include <libqopensync/pluginresource.h>

class QRadioButton;
class QStackedLayout;
class KComboBox;
class KLineEdit;
class KUrlRequester;

class ResourceWidget : public QWidget
{
  public:
    typedef QList<ResourceWidget*> List;

    ResourceWidget( const QSync::PluginResource &resource, QWidget *parent = 0 );
    ~ResourceWidget();

    void load();
    void save();

    QSync::PluginResource resource() const;

  private:
    QSync::PluginResource mResource;

    KLineEdit *mName;
    KUrlRequester *mPath;
    KUrlRequester *mUrl;
};

class TypeWidget : public QWidget
{
  public:
    typedef QList<TypeWidget*> List;
    typedef QMap<QString, TypeWidget*> Map;
    typedef QMapIterator<QString, TypeWidget*> MapIterator;

    TypeWidget( const QSync::PluginResource::List &resources, QWidget *parent = 0 );
    ~TypeWidget();

    void load();
    void save();

  private:
    ResourceWidget::List mResourceWidgets;
    QMap<QRadioButton*, ResourceWidget*> mMap;
};

class ConfigResourceWidget : public QWidget
{
  Q_OBJECT

  public:
    ConfigResourceWidget( const QSync::PluginResource::List &resources, QWidget *parent = 0 );
    ~ConfigResourceWidget();

    void load();
    void save();

  private Q_SLOTS:
    void typeChanged( int );

  private:
    KComboBox *mType;
    QStackedLayout *mStack;

    TypeWidget::List mTypeWidgetList;
    TypeWidget::Map mTypeWidgetMap;
};

#endif
