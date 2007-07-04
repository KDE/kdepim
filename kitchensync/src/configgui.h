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

#include <qwidget.h>

class QBoxLayout;
class KLineEdit;
class QTextEdit;

class ConfigGui : public QWidget
{
  public:
    ConfigGui( const QSync::Member &, QWidget *parent );

    class Factory
    {
      public:
        static ConfigGui *create( const QSync::Member &, QWidget *parent );
    };

    void setInstanceName( const QString & );
    QString instanceName() const;

    virtual void load( const QString &xml ) = 0;
    virtual QString save() const = 0;

    QSync::Member member() const { return mMember; }

    QBoxLayout *topLayout() const { return mTopLayout; }

  private:
    QSync::Member mMember;

    QBoxLayout *mTopLayout;
    KLineEdit *mNameEdit;
};

class ConfigGuiXml : public ConfigGui
{
  public:
    ConfigGuiXml( const QSync::Member &, QWidget *parent );

    void load( const QString & );
    QString save() const;

  private:
    QTextEdit *mTextEdit;
};

#endif
