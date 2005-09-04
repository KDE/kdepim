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

#ifndef EMAILEDITWIDGET_H
#define EMAILEDITWIDGET_H

#include <kabc/addressee.h>
#include <kdialogbase.h>

class Q3ButtonGroup;
class QCheckBox;
class Q3ListView;
class Q3TextEdit;
class QToolButton;

class KComboBox;
class KLineEdit;
class KListView;

/**
  This widget displays a list box of the email addresses as well as buttons
  to manipulate them (up, down, add, remove).
*/
class EmailEditWidget : public QWidget
{
  Q_OBJECT

  public:
    EmailEditWidget( QWidget *parent, const char *name = 0 );
    ~EmailEditWidget();

    void setEmails( const QStringList &list );
    QStringList emails();

    void setReadOnly( bool readOnly );

  signals:
    void modified();

  private slots:
    void edit();
    void textChanged( const QString& );

  private:
    KLineEdit *mEmailEdit;
    QPushButton *mEditButton;
    QStringList mEmailList;
};

class EmailEditDialog : public KDialogBase
{
  Q_OBJECT

  public:
    EmailEditDialog( const QStringList &list, QWidget *parent,
                     const char *name = 0 );
    ~EmailEditDialog();

    QStringList emails() const;
    bool changed() const;

  protected slots:
    void add();
    void edit();
    void remove();
    void standard();
    void selectionChanged( int );

  private:
    Q3ListBox *mEmailListBox;
    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QPushButton *mEditButton;
    QPushButton *mStandardButton;

    bool mChanged;
};

#endif
