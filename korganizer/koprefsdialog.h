/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOPREFSDIALOG_H
#define KOPREFSDIALOG_H

#include <libkdepim/kprefsdialog.h>
#include <libkdepim/kcmdesignerfields.h>

#include <tqdict.h>

class QLineEdit;
class QLabel;
class QSpinBox;
class QComboBox;
class KColorButton;
class KPushButton;
class QColor;
class QListView;

class KDE_EXPORT KOPrefsDialogMain : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogMain( TQWidget *parent, const char *name );

  protected slots:
    void toggleEmailSettings( bool on );
  private:
    TQWidget *mUserEmailSettings;
};

class KDE_EXPORT KOPrefsDialogColors : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogColors( TQWidget *parent, const char *name );

  protected:
    void usrWriteConfig();
    void usrReadConfig();

  protected slots:
    void updateCategories();
    void setCategoryColor();
    void updateCategoryColor();

    void updateResources();
    void setResourceColor();
    void updateResourceColor();
  private:
    TQComboBox     *mCategoryCombo;
    KColorButton  *mCategoryButton;
    TQDict<TQColor> mCategoryDict;

    TQComboBox     *mResourceCombo;
    KColorButton  *mResourceButton;
    TQDict<TQColor> mResourceDict;
    //For translation Identifier <->idx in Combo
    TQStringList mResourceIdentifier;
};

class KDE_EXPORT KOPrefsDialogGroupScheduling : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogGroupScheduling( TQWidget *parent, const char *name );

  protected:
    void usrReadConfig();
    void usrWriteConfig();

  protected slots:
    void addItem();
    void removeItem();
    void updateItem();
    void updateInput();

  private:
    TQListView *mAMails;
    TQLineEdit *aEmailsEdit;
};

class KOGroupwarePrefsPage;

class KDE_EXPORT KOPrefsDialogGroupwareScheduling : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogGroupwareScheduling( TQWidget *parent, const char *name );

  protected:
    void usrReadConfig();
    void usrWriteConfig();

  private:
    KOGroupwarePrefsPage* mGroupwarePage;
};

class KDE_EXPORT KOPrefsDialogPlugins : public KPrefsModule
{
    Q_OBJECT
  public:
    KOPrefsDialogPlugins( TQWidget *parent, const char *name );

  protected slots:
    void usrReadConfig();
    void usrWriteConfig();
    void configure();
    void selectionChanged( TQListViewItem* );

  private:
    void buildList();
    TQListView *mListView;
    TQLabel *mDescription;
    KPushButton *mConfigureButton;
};

class KDE_EXPORT KOPrefsDesignerFields : public KPIM::KCMDesignerFields
{
  public:
    KOPrefsDesignerFields( TQWidget *parent = 0, const char *name = 0 );

  protected:
    TQString localUiDir();
    TQString uiPath();
    void writeActivePages( const TQStringList & );
    TQStringList readActivePages();
    TQString applicationName();
};

#endif
