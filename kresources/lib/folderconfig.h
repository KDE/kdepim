/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KPIM_FOLDERCONFIG_H
#define KPIM_FOLDERCONFIG_H

#include <kurl.h>
#include <kdepimmacros.h>
#include <qwidget.h>

class KListView;
class QComboBox;

namespace KPIM {

class FolderLister;

class KDE_EXPORT FolderConfig : public QWidget
{
    Q_OBJECT
  public:
    FolderConfig( QWidget *parent );
    ~FolderConfig();

    void setFolderLister( FolderLister * );

    void saveSettings();

  public slots:
    void updateFolderList();
    void retrieveFolderList( const KURL & );

  signals:
    void updateFoldersClicked();

  private:
    KListView *mFolderList;
    QComboBox *mWriteCombo;

    FolderLister *mFolderLister;
};

}

#endif
