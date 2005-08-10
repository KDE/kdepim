/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCALRESOURCESLOXCONFIG_H
#define KCALRESOURCESLOXCONFIG_H

#include <kurlrequester.h>
#include <kdepimmacros.h>

#include <kresources/resource.h>
#include <kresources/configwidget.h>

class QCheckBox;
class KLineEdit;
class KPushButton;

namespace KCal {
class ResourceCachedReloadConfig;
class ResourceCachedSaveConfig;
}

class SloxBase;

/**
  Configuration widget for SLOX resource.

  @see KCalResourceSlox
*/
class KDE_EXPORT KCalResourceSloxConfig : public KRES::ConfigWidget
{
    Q_OBJECT
  public:
    KCalResourceSloxConfig( QWidget *parent = 0, const char *name = 0 );

  public slots:
    virtual void loadSettings( KRES::Resource *resource );
    virtual void saveSettings( KRES::Resource *resource );

  private slots:
    void selectCalendarFolder();
    void selectTaskFolder();

  private:
    KURLRequester *mDownloadUrl;
    KLineEdit *mUserEdit;
    KLineEdit *mPasswordEdit;
    QCheckBox *mLastSyncCheck;
    KPushButton *mCalButton;
    KPushButton *mTaskButton;
    QString mCalendarFolderId;
    QString mTaskFolderId;

    KCal::ResourceCachedReloadConfig *mReloadConfig;
    KCal::ResourceCachedSaveConfig *mSaveConfig;

    SloxBase *mRes;
};

#endif
