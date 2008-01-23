/*
    This file is part of KitchenSync.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2007 Anirudh Ramesh <abattoir@abattoir.in>

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

#ifndef CONFIGGUISUNBIRD_H
#define CONFIGGUISUNBIRD_H

#include "configgui.h"

class QWidget;
class QSpinBox;
class QCheckBox;
class QVBoxLayout;
class QSpacerItem;
class QSignalMapper;

class KURLRequester;
class KPushButton;
class KLineEdit;

class LocalCalendar : public QWidget
{
  Q_OBJECT

  public:
    LocalCalendar( QWidget *parent = 0 );
    LocalCalendar( const QString &path,
                   const QString &defaultcal,
                   const QString &days, QWidget *parent = 0 );

    KURLRequester *mPathRequester;
    QCheckBox *mDaysCheckBox;
    QSpinBox *mDaysSpinBox;
    QCheckBox *mDefaultCheckBox;

  signals:
    void deleteRequest( LocalCalendar* );

  private slots:
    void deleteWidget();
    void toggleDays( bool days );

  private:
    void initGui();
};

class WebdavCalendar : public QWidget
{
  Q_OBJECT

  public:
    WebdavCalendar( QWidget *parent = 0 );
    WebdavCalendar( const QString &username,
                    const QString &password,
                    const QString &url,
                    const QString &defaultcal,
                    const QString &days, QWidget *parent = 0 );

    KLineEdit *mUrl;
    QCheckBox *mDaysCheckBox;
    QSpinBox *mDaysSpinBox;
    QCheckBox *mDefaultCheckBox;
    KLineEdit *mUsername;
    KLineEdit *mPassword;

  signals:
    void deleteRequest( WebdavCalendar* );

  private slots:
    void deleteWidget();
    void toggleDays( bool state );

  private:
    void initGui();
};

class ConfigGuiSunbird : public ConfigGui
{
  Q_OBJECT

  public:
    ConfigGuiSunbird( const QSync::Member &, QWidget *parent );

    void load( const QString &xml );

    QString save() const;

  public slots:
    void addLocalCalendar();
    void addWebdavCalendar();

    void delLocalCalendar( LocalCalendar* );
    void delWebdavCalendar( WebdavCalendar* );

  private:
    QValueList<LocalCalendar*> mLocalList;
    QValueList<WebdavCalendar*> mWebdavList;

    QWidget *mLocalWidget;
    QWidget *mWebdavWidget;

    QVBoxLayout *mLocalLayout;
    QVBoxLayout *mWebdavLayout;

    KPushButton *mLocalAddButton;
    KPushButton *mWebdavAddButton;

    QSpacerItem *mLocalSpacer;
    QSpacerItem *mWebdavSpacer;
};

#endif
