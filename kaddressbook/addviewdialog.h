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

#ifndef ADDVIEWDIALOG_H
#define ADDVIEWDIALOG_H

#include <kdialogbase.h>
#include <q3dict.h>
#include <qstring.h>

class Q3ButtonGroup;
class QLineEdit;
class ViewFactory;


/**
  Modal dialog used for adding a new view. The dialog asks for the name of
  the view as well as the type. Someday it would be nice for this to be a
  wizard.
 */
class AddViewDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AddViewDialog( Q3Dict<ViewFactory> *viewFactoryDict, QWidget *parent,
                   const char *name = 0 );
    ~AddViewDialog();

    QString viewName()const;

    QString viewType()const ;

  protected slots:
    /**
      Called when the user selects a type radio button.
     */
    void clicked( int id );

    /**
      Called when the user changes the text in the name of the view.
     */
    void textChanged( const QString &text );

  private:
    Q3Dict<ViewFactory> *mViewFactoryDict;
    QLineEdit *mViewNameEdit;
    Q3ButtonGroup *mTypeGroup;

    int mTypeId;
};

#endif
