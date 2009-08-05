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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CSVIMPORTDIALOG_H
#define CSVIMPORTDIALOG_H

#include <kabc/addressee.h>
#include <kdialog.h>

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
    CSVImportDialog( QWidget *parent = 0 );
    ~CSVImportDialog();

    KABC::AddresseeList contacts() const;

  protected Q_SLOTS:
    virtual void slotButtonClicked( int );

  private Q_SLOTS:
    void setFile( const QString& );
    void setFile( const KUrl & );
    void urlChanged( const QString& );

    void customDelimiterChanged();
    void customDelimiterChanged( const QString& );
    void delimiterClicked( int );
    void textQuoteChanged( const QString& );
    void skipFirstRowChanged( bool );
    void codecChanged();

    void modelFinishedLoading();

  private:
    void applyTemplate();
    void saveTemplate();

    QTableView* mTable;
    QButtonGroup* mDelimiterGroup;
    KLineEdit* mDelimiterEdit;
    KLineEdit* mDatePatternEdit;
    KComboBox* mComboQuote;
    KComboBox* mCodecCombo;
    QCheckBox* mSkipFirstRow;
    KUrlRequester* mUrlRequester;
    QCsvModel *mModel;

    void initGUI();

    void reloadCodecs();
    QTextCodec *currentCodec();
    QList<QTextCodec*> mCodecs;

    QChar mTextQuote;
    QString mDelimiter;
    QMap<QString, uint> mTypeMap;
    QIODevice *mDevice;
};

#endif
