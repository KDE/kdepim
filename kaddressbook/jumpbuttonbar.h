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

#ifndef JUMPBUTTONBAR_H
#define JUMPBUTTONBAR_H

#include <tqsizepolicy.h>
#include <tqwidget.h>
#include <tqptrlist.h>
#include <tqstringlist.h>

class TQButtonGroup;
class TQResizeEvent;
class TQPushButton;

namespace KAB {
class Core;
}

namespace KABC {
class Field;
}

/**
  Used to draw the jump button bar on the right of the view.
 */
class JumpButtonBar : public QWidget
{
  Q_OBJECT

  public:
    JumpButtonBar( KAB::Core *core, TQWidget *parent, const char *name = 0 );
    ~JumpButtonBar();

  public slots:
    void updateButtons();

  signals:
    /**
      Emitted whenever a letter is selected by the user.
     */
    void jumpToLetter( const TQString &character );

  protected slots:
    void letterClicked();

  protected:
    virtual void resizeEvent( TQResizeEvent* );

  private:
    void sortListLocaleAware( TQStringList &list );

    KAB::Core *mCore;

    TQButtonGroup *mGroupBox;
    TQPtrList<TQPushButton> mButtons;
};

#endif
