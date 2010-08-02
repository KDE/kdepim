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
#ifndef CONFIGGUI_H
#define CONFIGGUI_H

#include <libqopensync/member.h>

#include <tqwidget.h>

class TQBoxLayout;
class KLineEdit;
class TQTextEdit;

class ConfigGui : public QWidget
{
  public:
    ConfigGui( const QSync::Member &, TQWidget *parent );

    class Factory
    {
      public:
        static ConfigGui *create( const QSync::Member &, TQWidget *parent );
    };

    void setInstanceName( const TQString & );
    TQString instanceName() const;

    virtual void load( const TQString &xml ) = 0;
    virtual TQString save() const = 0;

    QSync::Member member() const { return mMember; }

    TQBoxLayout *topLayout() const { return mTopLayout; }

  private:
    QSync::Member mMember;

    TQBoxLayout *mTopLayout;
    KLineEdit *mNameEdit;
};

class ConfigGuiXml : public ConfigGui
{
  public:
    ConfigGuiXml( const QSync::Member &, TQWidget *parent );

    void load( const TQString & );
    TQString save() const;

  private:
    TQTextEdit *mTextEdit;
};

#endif
