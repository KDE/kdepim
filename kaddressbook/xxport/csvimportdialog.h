/*
   This file is part of KAddressBook.
   Copyright (C) 2003 Tobias Koenig <tokoe@kde.org>
                 based on the code of KSpread's CSV Import Dialog

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CSV_IMPORT_DLG_H
#define CSV_IMPORT_DLG_H

#include <kabc/addressbook.h>
#include <kabc/addresseelist.h>
#include <kdialogbase.h>

#include <tqvaluelist.h>

class KURLRequester;

class QButtonGroup;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QTable;

class CSVImportDialog : public KDialogBase
{
  Q_OBJECT

  public:
    CSVImportDialog( KABC::AddressBook *ab, TQWidget *parent,
                     const char *name = 0 );
    ~CSVImportDialog();

    KABC::AddresseeList contacts() const;

  protected slots:
    virtual void slotOk();

  private slots:
    void returnPressed();
    void delimiterClicked( int id );
    void lineSelected( const TQString& line );
    void textquoteSelected( const TQString& mark );
    void textChanged ( const TQString & );
    void ignoreDuplicatesChanged( int );
    void setFile( const TQString& );
    void urlChanged( const TQString& );
    void codecChanged();

    void applyTemplate();
    void saveTemplate();

  private:
    enum { Undefined, FormattedName, FamilyName, GivenName, AdditionalName,
           Prefix, Suffix, NickName, Birthday,
           HomeAddressStreet, HomeAddressLocality, HomeAddressRegion,
           HomeAddressPostalCode, HomeAddressCountry, HomeAddressLabel,
           BusinessAddressStreet, BusinessAddressLocality, BusinessAddressRegion,
           BusinessAddressPostalCode, BusinessAddressCountry,
           BusinessAddressLabel,
           HomePhone, BusinessPhone, MobilePhone, HomeFax, BusinessFax, CarPhone,
           Isdn, Pager, Email, Mailer, Title, Role, Organization, Note, URL
         };

    TQTable* mTable;
    TQButtonGroup* mDelimiterBox;
    TQRadioButton* mRadioComma;
    TQRadioButton* mRadioSemicolon;
    TQRadioButton* mRadioTab;
    TQRadioButton* mRadioSpace;
    TQRadioButton* mRadioOther;
    TQLineEdit* mDelimiterEdit;
    TQLineEdit* mDatePatternEdit;
    TQComboBox* mComboLine;
    TQComboBox* mComboQuote;
    TQCheckBox* mIgnoreDuplicates;
    TQComboBox* mCodecCombo;
    TQWidget* mPage;
    KURLRequester* mUrlRequester;

    void initGUI();
    void fillTable();
    void clearTable();
    void fillComboBox();
    void setText( int row, int col, const TQString& text );
    void adjustRows( int rows );
    void resizeColumns();
    TQString getText( int row, int col );
    uint posToType( int pos ) const;
    int typeToPos( uint type ) const;

    void reloadCodecs();
    TQTextCodec *currentCodec();
    TQPtrList<TQTextCodec> mCodecs;

    bool mAdjustRows;
    int mStartLine;
    TQChar mTextQuote;
    TQString mDelimiter;
    TQByteArray mFileArray;
    TQMap<TQString, uint> mTypeMap;
    KABC::AddressBook *mAddressBook;
    int mCustomCounter;
    bool mClearTypeStore;
    TQValueList<int> mTypeStore;
};

#endif
