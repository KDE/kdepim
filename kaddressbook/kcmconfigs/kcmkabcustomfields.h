/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KCMKABCUSTOMFIELDS_H
#define KCMKABCUSTOMFIELDS_H

#include <kcmodule.h>

class KListView;

class QLabel;

class KCMKabCustomFields : public KCModule
{
  Q_OBJECT

  public:
    KCMKabCustomFields( QWidget *parent = 0, const char *name = 0 );

    virtual void load();
    virtual void save();
    virtual void defaults();

  protected:
    void        loadUiFiles();
    void        loadActivePages(const QStringList&);
    QStringList saveActivePages();
    QString     kabLocalDir();

  private slots:
    void updatePreview( QListViewItem* );
    void itemClicked( QListViewItem* );
    void startDesigner();
    void rebuildList();
    void deleteFile();
    void importFile();

    
  private:
    void initGUI();

    KListView *mPageView;
    QLabel *mPagePreview;
    QLabel *mPageDetails;
    QPushButton *mDeleteButton;    
    QPushButton *mImportButton;
    QPushButton *mDesignerButton;
};

#endif
