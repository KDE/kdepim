/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

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

#ifndef SLOXFOLDERDIALOG_H
#define SLOXFOLDERDIALOG_H

#include <qstring.h>
#include <kdialogbase.h>

#include "sloxfolder.h"

class KListView;
class SloxFolder;
class SloxFolderManager;

class SloxFolderDialog : public KDialogBase
{
  Q_OBJECT
  public:
    SloxFolderDialog( SloxFolderManager *manager, FolderType type, QWidget* parent = 0, const char *name = 0 );
    ~SloxFolderDialog();

    QString selectedFolder() const;
    void setSelectedFolder( const QString &id );

  protected slots:
    virtual void slotUser1();
    void updateFolderView();

  private:
    void createFolderViewItem( SloxFolder *folder );

  private:
    KListView *mListView;
    SloxFolderManager *mManager;
    QString mFolderId;
    FolderType mFolderType;
};

#endif
