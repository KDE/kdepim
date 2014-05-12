/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CSVIMPORTDIALOG_H
#define CSVIMPORTDIALOG_H

#include "contactfields.h"

#include <KABC/Addressee>

#include <KDialog>

#include <QtCore/QList>

class KComboBox;
class KLineEdit;
class KUrlRequester;

class QButtonGroup;
class QCheckBox;
class QCsvModel;
class QTableView;

class CSVImportDialog : public KDialog
{
  Q_OBJECT

  public:
    explicit CSVImportDialog( QWidget *parent = 0 );
    ~CSVImportDialog();

    KABC::AddresseeList contacts() const;

  protected Q_SLOTS:
    virtual void slotButtonClicked( int );

  private Q_SLOTS:
    void setFile( const QString & );
    void setFile( const QUrl & );
    void urlChanged( const QString & );

    void customDelimiterChanged();
    void customDelimiterChanged( const QString &, bool reload = true );
    void delimiterClicked( int, bool reload = true );
    void textQuoteChanged( const QString &, bool reload = true );
    void skipFirstRowChanged( bool, bool reload = true );
    void codecChanged( bool reload = true );

    void modelFinishedLoading();
    void finalizeApplyTemplate();

  private:
    void applyTemplate();
    void saveTemplate();

    QTableView *mTable;
    QButtonGroup *mDelimiterGroup;
    KLineEdit *mDelimiterEdit;
    KLineEdit *mDatePatternEdit;
    KComboBox *mComboQuote;
    KComboBox *mCodecCombo;
    QCheckBox *mSkipFirstRow;
    KUrlRequester *mUrlRequester;
    QCsvModel *mModel;

    void initGUI();

    void reloadCodecs();
    QTextCodec *currentCodec();
    QList<QTextCodec*> mCodecs;

    QChar mTextQuote;
    QString mDelimiter;
    QMap<QString, uint> mTypeMap;
    QIODevice *mDevice;
    ContactFields::Fields mFieldSelection;
};

#endif
