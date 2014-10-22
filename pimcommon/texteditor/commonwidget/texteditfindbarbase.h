/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TEXTEDITFINDBARBASE_H
#define TEXTEDITFINDBARBASE_H

#include "pimcommon_export.h"
#include <QWidget>
#include <QTextDocument>


namespace PimCommon {
class TextFindWidget;
class TextReplaceWidget;
class PIMCOMMON_EXPORT TextEditFindBarBase : public QWidget
{
    Q_OBJECT

public:
    explicit TextEditFindBarBase( QWidget *parent = 0 );
    ~TextEditFindBarBase();

    QString text() const;
    void setText( const QString &text );

    void focusAndSetCursor();

    void showReplace();
    void showFind();

Q_SIGNALS:
    void displayMessageIndicator(const QString &message);

protected:
    virtual bool viewIsReadOnly() const = 0;
    virtual bool documentIsEmpty() const = 0;
    virtual bool searchInDocument(const QString &text, QTextDocument::FindFlags searchOptions) = 0;
    virtual void autoSearchMoveCursor() = 0;

    bool event(QEvent* e);
    void clearSelections();
    void updateHighLight(bool);
    bool searchText( bool backward, bool isAutoSearch );
    void updateSensitivity( bool );

    void setFoundMatch( bool match );
    void messageInfo( bool backward, bool isAutoSearch, bool found );

public slots:
    void findNext();
    void findPrev();
    void autoSearch( const QString &str );
    virtual void slotSearchText( bool backward = false, bool isAutoSearch = true ) = 0;
    void closeBar();

private slots:
    void slotClearSearch();
    void slotUpdateSearchOptions();
    virtual void slotReplaceText() = 0;
    virtual void slotReplaceAllText() = 0;

protected:
    QString mLastSearchStr;
    TextFindWidget *mFindWidget;
    TextReplaceWidget *mReplaceWidget;
};

}

#endif // TEXTEDITFINDBARBASE_H
