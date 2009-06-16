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

#ifndef PHONEEDITWIDGET_H
#define PHONEEDITWIDGET_H

#include <QtCore/QList>

#include <kcombobox.h>
#include <kdialog.h>

#include "addresseeconfig.h"

class QButtonGroup;
class QCheckBox;
class QScrollArea;
class QSignalMapper;
class QVBoxLayout;

class KLineEdit;
class KComboBox;

class PhoneTypeCombo : public KComboBox
{
  Q_OBJECT

  public:
    PhoneTypeCombo( QWidget *parent );
    ~PhoneTypeCombo();

    void setType( KABC::PhoneNumber::Type type );
    KABC::PhoneNumber::Type type() const;

  Q_SIGNALS:
    void modified();

  protected Q_SLOTS:
    void selected( int );
    void otherSelected();

  private:
    void update();

    KABC::PhoneNumber::Type mType;
    int mLastSelected;
    QList<int> mTypeList;
};

class PhoneNumberWidget : public QWidget
{
  Q_OBJECT

  public:
    PhoneNumberWidget( QWidget *parent );

    void setNumber( const KABC::PhoneNumber &number );
    KABC::PhoneNumber number() const;

    void setReadOnly( bool readOnly );

  Q_SIGNALS:
    void modified();

  private:
    PhoneTypeCombo *mTypeCombo;
    KLineEdit *mNumberEdit;
    KABC::PhoneNumber mNumber;
};

class PhoneNumberListWidget : public QWidget
{
  Q_OBJECT

  public:
    PhoneNumberListWidget( QWidget *parent = 0 );
    ~PhoneNumberListWidget();

    void setPhoneNumbers( const KABC::PhoneNumber::List &list );
    KABC::PhoneNumber::List phoneNumbers() const;

    void setReadOnly( bool readOnly );

    int phoneNumberCount() const;

  Q_SIGNALS:
    void modified();

  public Q_SLOTS:
    void add();
    void remove();

  protected Q_SLOTS:
    void changed();
    void changed( int pos );

  private:
    void recreateNumberWidgets();

    KABC::PhoneNumber::List mPhoneNumberList;
    QList<PhoneNumberWidget*> mWidgets;

    QVBoxLayout *mWidgetLayout;

    bool mReadOnly;
    QSignalMapper *mMapper;
};

/**
  Widget for editing phone numbers.
*/
class PhoneEditWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit PhoneEditWidget( QWidget *parent, const char *name = 0 );
    ~PhoneEditWidget();

    void setPhoneNumbers( const KABC::PhoneNumber::List &list );
    KABC::PhoneNumber::List phoneNumbers() const;

    void setReadOnly( bool readOnly );

  Q_SIGNALS:
    void modified();

  protected Q_SLOTS:
    void changed();

  private:
    QPushButton *mAddButton;
    QPushButton *mRemoveButton;

    bool mReadOnly;

    QScrollArea *mListScrollArea;
    PhoneNumberListWidget *mPhoneNumberListWidget;
};

/**
  Dialog for editing phone number types.
 */
class PhoneTypeDialog : public KDialog
{
  Q_OBJECT
  public:
    PhoneTypeDialog( KABC::PhoneNumber::Type type, QWidget *parent );

    KABC::PhoneNumber::Type type() const;

  private:
    KABC::PhoneNumber::Type mType;
    KABC::PhoneNumber::TypeList mTypeList;

    QButtonGroup *mGroup;
    QCheckBox *mPreferredBox;
};

#endif
