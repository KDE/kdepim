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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef CONFIGURECARDVIEWDIALOG_H
#define CONFIGURECARDVIEWDIALOG_H

#include "viewconfigurewidget.h"

#include <qvbox.h>
#include <qwidget.h>
#include <qfont.h>

class QString;
class QWidget;
class QCheckBox;
class QLabel;
class KConfig;

namespace KABC { class AddressBook; }

class CardViewLookAndFeelPage;

/**
  Configure dialog for the card view. This dialog inherits from the
  standard view dialog in order to add a custom page for the card
  view.
 */
class ConfigureCardViewWidget : public ViewConfigureWidget
{
  public:
    ConfigureCardViewWidget( KABC::AddressBook *ab, QWidget *parent, const char *name );
    virtual ~ConfigureCardViewWidget();
    
    virtual void restoreSettings( KConfig* );
    virtual void saveSettings( KConfig* );
    
  private:
    class CardViewLookNFeelPage *mAdvancedPage;
};

/**
    Card View Advanced LookNFeel settings widget:
    this is a tabbed widget with 3 tabs:
    Fonts
    * text font
    * header font
    
    Colors
    * background color
    * text color
    * highlight color
    * title/sep text color
    * title/sep bg color
    
    Layout
    * item margin
    * item spacing
*/

class CardViewLookNFeelPage : public QVBox {

  Q_OBJECT
  
  public:
    CardViewLookNFeelPage( QWidget *parent=0, const char *name=0 );
    ~CardViewLookNFeelPage();
  
    void restoreSettings( KConfig* );
    void saveSettings( KConfig* );
  
  private slots:
    void setTextFont();
    void setHeaderFont();
    void enableFonts();
    void enableColors();
  
  private:
    void initGUI();
    void updateFontLabel( QFont, QLabel * );
    
    QCheckBox *cbEnableCustomFonts, 
              *cbEnableCustomColors,
              *cbDrawSeps, *cbDrawBorders,
              *cbShowFieldLabels, *cbShowEmptyFields;
    class ColorListBox *lbColors;
    QLabel *lTextFont, *lHeaderFont;
    class KPushButton *btnFont, *btnHeaderFont;
    class QSpinBox *sbMargin, *sbSpacing, *sbSepWidth;
    
    class QWidget *vbFonts;
};

#endif
