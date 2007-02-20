/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#ifndef CONFIGURETABLEVIEWDIALOG_H
#define CONFIGURETABLEVIEWDIALOG_H

#include "viewconfigurewidget.h"

class QString;
class QWidget;
class QRadioButton;
class QCheckBox;
class KUrlRequester;
class KConfig;

namespace KABC { class AddressBook; }

class LookAndFeelPage;

/**
  Configure dialog for the table view. This dialog inherits from the
  standard view dialog in order to add a custom page for the table
  view.
 */
class ConfigureTableViewWidget : public ViewConfigureWidget
{
  public:
    ConfigureTableViewWidget( KABC::AddressBook *ab, QWidget *parent );
    virtual ~ConfigureTableViewWidget();

    virtual void restoreSettings( const KConfigGroup & );
    virtual void saveSettings( KConfigGroup& );

  private:
    void initGUI();

    LookAndFeelPage *mPage;
};

/**
  Internal class. It is only defined here for moc
*/
class LookAndFeelPage : public QWidget
{
  Q_OBJECT

  public:
    LookAndFeelPage( QWidget *parent );
    ~LookAndFeelPage() {}

    void restoreSettings( const KConfigGroup&);
    void saveSettings( KConfigGroup& );

  protected slots:
    void enableBackgroundToggled( bool );

  private:
    void initGUI();

    QRadioButton *mAlternateButton;
    QRadioButton *mLineButton;
    QRadioButton *mNoneButton;
    QCheckBox *mToolTipBox;
    KUrlRequester *mBackgroundName;
    QCheckBox *mBackgroundBox;
    QCheckBox *mIMPresenceBox;
};

#endif
