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
#include <kdialog.h>

#include <QList>

class KUrlRequester;

class Q3ButtonGroup;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QRadioButton;
class Q3Table;

class CSVImportDialog : public KDialog
{
  Q_OBJECT

  public:
    CSVImportDialog( KABC::AddressBook *ab, QWidget *parent );
    ~CSVImportDialog();

    KABC::AddresseeList contacts() const;

  protected slots:
    virtual void slotOk();

  private slots:
    void returnPressed();
    void delimiterClicked( int id );
    void lineSelected( const QString& line );
    void textquoteSelected( const QString& mark );
    void textChanged ( const QString & );
    void ignoreDuplicatesChanged( int );
    void setFile( const QString& );
    void setFile( const KUrl & );
    void urlChanged( const QString& );
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

    Q3Table* mTable;
    Q3ButtonGroup* mDelimiterBox;
    QRadioButton* mRadioComma;
    QRadioButton* mRadioSemicolon;
    QRadioButton* mRadioTab;
    QRadioButton* mRadioSpace;
    QRadioButton* mRadioOther;
    QLineEdit* mDelimiterEdit;
    QLineEdit* mDatePatternEdit;
    QComboBox* mComboLine;
    QComboBox* mComboQuote;
    QCheckBox* mIgnoreDuplicates;
    QComboBox* mCodecCombo;
    QWidget* mPage;
    KUrlRequester* mUrlRequester;

    void initGUI();
    void fillTable();
    void clearTable();
    void fillComboBox();
    void setText( int row, int col, const QString& text );
    void adjustRows( int rows );
    void resizeColumns();
    QString getText( int row, int col );
    uint posToType( int pos ) const;
    int typeToPos( uint type ) const;

    void reloadCodecs();
    QTextCodec *currentCodec();
    QList<QTextCodec*> mCodecs;

    bool mAdjustRows;
    int mStartLine;
    QChar mTextQuote;
    QString mDelimiter;
    QByteArray mFileArray;
    QMap<QString, uint> mTypeMap;
    KABC::AddressBook *mAddressBook;
    int mCustomCounter;
    bool mClearTypeStore;
    QList<int> mTypeStore;
};

#endif
