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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef KCALRESOURCESLOXCONFIG_H
#define KCALRESOURCESLOXCONFIG_H

#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <kurlrequester.h>

#include <kresources/resource.h>
#include <kresources/configwidget.h>

/**
  Configuration widget for remote resource.
  
  @see KCalResourceSlox
*/
class KCalResourceSloxConfig : public KRES::ConfigWidget
{ 
    Q_OBJECT
  public:
    KCalResourceSloxConfig( QWidget *parent = 0, const char *name = 0 );

  public slots:
    virtual void loadSettings( KRES::Resource *resource );
    virtual void saveSettings( KRES::Resource *resource );

  private:
    KURLRequester *mDownloadUrl;
    KURLRequester *mUploadUrl;

    QButtonGroup *mReloadGroup;

    class Private;
    Private *d;
};

#endif
