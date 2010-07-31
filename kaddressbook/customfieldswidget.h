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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef CUSTOMFIELDSWIDGET_H
#define CUSTOMFIELDSWIDGET_H

#include <kabc/addressee.h>
#include <kdialogbase.h>
#include <klocale.h>

#include <tqmap.h>
#include <tqpair.h>
#include <tqstringlist.h>
#include <tqvaluevector.h>
#include <tqwidget.h>

#include "contacteditorwidget.h"

class QCheckBox;
class QGridLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;

class KComboBox;
class KLineEdit;

typedef struct {
  TQString mIdentifier;
  TQString mTitle;
  bool mGlobal;

  TQLabel *mLabel;
  TQWidget *mWidget;
  TQHBoxLayout *mLayout;
} FieldRecord;

typedef TQValueList<FieldRecord> FieldRecordList;

class AddFieldDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AddFieldDialog( TQWidget *parent, const char *name = 0 );

    TQString title() const;
    TQString identifier() const;
    TQString type() const;
    bool isGlobal() const;

  private slots:
    void nameChanged( const TQString& );

  private:
    KLineEdit *mTitle;
    KComboBox *mType;
    TQCheckBox *mGlobal;

    TQValueVector<TQString> mTypeList;
    TQValueVector<TQString> mTypeName;
};

class FieldWidget : public QWidget
{
  Q_OBJECT

  public:
    FieldWidget( TQWidget *parent, const char *name = 0 );

    void addField( const TQString &identifier, const TQString &title,
                   const TQString &type, bool isGlobal );

    void removeField( const TQString &identifier );

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );

    FieldRecordList fields() const { return mFieldList; }

    void removeLocalFields();
    void clearFields();

  signals:
    void changed();

  private:
    void recalculateLayout();

    TQVBoxLayout *mGlobalLayout;
    TQVBoxLayout *mLocalLayout;
    TQFrame *mSeparator;

    FieldRecordList mFieldList;
};

class CustomFieldsWidget : public KAB::ContactEditorWidget
{
  Q_OBJECT

  public:
    CustomFieldsWidget( KABC::AddressBook *ab, TQWidget *parent, const char *name = 0 );

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );

  private slots:
    void addField();
    void removeField();

  private:
    void initGUI();

    TQStringList marshallFields( bool ) const;

    TQPushButton *mAddButton;
    TQPushButton *mRemoveButton;
    TQGridLayout *mLayout;

    FieldWidget *mFieldWidget;

    KABC::Addressee mAddressee;
};

class CustomFieldsWidgetFactory : public KAB::ContactEditorWidgetFactory
{
  public:
    KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, TQWidget *parent, const char *name )
    {
      return new CustomFieldsWidget( ab, parent, name );
    }

    TQString pageTitle() const { return i18n( "Custom Fields" ); }
    TQString pageIdentifier() const { return "custom_fields"; }
};

void splitField( const TQString&, TQString&, TQString&, TQString& );

#endif
