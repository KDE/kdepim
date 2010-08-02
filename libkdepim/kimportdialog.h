/*
    This file is part of libkdepim.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
#ifndef KIMPORTDIALOG_H
#define KIMPORTDIALOG_H

#include <tqintdict.h>
#include <tqstringlist.h>
#include <tqspinbox.h>
#include <tqptrvector.h>
#include <tqvaluevector.h>

#include <kdialogbase.h>

class TQTable;
class TQListView;

class KImportDialog;
class KComboBox;

class KImportColumn
{
  public:
    enum { FormatUndefined = 0, FormatPlain, FormatUnquoted, FormatBracketed, FormatLast };

    KImportColumn(KImportDialog *dlg, const TQString &header, int count = 0);
    virtual ~KImportColumn() {}

    TQString header() const { return m_header; }

    TQValueList<int> formats();
    TQString formatName(int format);
    int defaultFormat();

    TQString convert();
//    virtual void convert(const TQString &value,int format) = 0;
    TQString preview(const TQString &value,int format);

    void addColId(int i);
    void removeColId(int i);

    TQValueList<int> colIdList();

  protected:

  private:
    int m_maxCount, m_refCount;

    TQString m_header;
    TQValueList<int> mFormats;
    int mDefaultFormat;
    
    TQValueList<int> mColIds;
    
    KImportDialog *mDialog;
};

class KImportDialog : public KDialogBase
{
    Q_OBJECT
  public:
    KImportDialog(TQWidget* parent);

  public slots:
    bool setFile(const TQString& file);

    TQString cell(uint row);

    void addColumn(KImportColumn *);

  protected:
    void readFile( int rows = 10 );
  
    void fillTable();
    void registerColumns();
    int findFormat(int column);

    virtual void convertRow() {}

  protected slots:
    void separatorClicked(int id);
    void formatSelected(TQListViewItem* item);
    void headerSelected(TQListViewItem* item);
    void assignColumn(TQListViewItem *);
    void assignColumn();
    void assignTemplate();
    void removeColumn();
    void applyConverter();
    void tableSelected();
    void slotUrlChanged(const TQString & );
    void saveTemplate();

  private:
    void updateFormatSelection(int column);
    void setCellText(int row, int col, const TQString& text);

    void setData( uint row, uint col, const TQString &text );
    TQString data( uint row, uint col );

    TQListView *mHeaderList;
    TQSpinBox *mStartRow;
    TQSpinBox *mEndRow;
    TQTable *mTable;

    KComboBox *mFormatCombo;
    KComboBox *mSeparatorCombo;

    TQString mSeparator;
    int mCurrentRow;
    TQString mFile;
    TQIntDict<KImportColumn> mColumnDict;
    TQIntDict<uint> mTemplateDict;
    TQMap<int,int> mFormats;
    TQPtrList<KImportColumn> mColumns;
    TQPtrVector<TQValueVector<TQString> > mData;
};

#endif
