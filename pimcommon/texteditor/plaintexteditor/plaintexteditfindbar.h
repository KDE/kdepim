/* Copyright (C) 2012, 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PIMCOMMON_FINDBARBASE_H
#define PIMCOMMON_FINDBARBASE_H

#include "pimcommon_export.h"
#include <QWidget>
#include <QTextDocument>

class QPlainTextEdit;

namespace PimCommon {
class TextFindWidget;
class TextReplaceWidget;
class PIMCOMMON_EXPORT PlainTextEditFindBar : public QWidget
{
    Q_OBJECT

public:
    explicit PlainTextEditFindBar( QPlainTextEdit *view, QWidget *parent = 0 );
    ~PlainTextEditFindBar();

    QString text() const;
    void setText( const QString&text );

    void focusAndSetCursor();

    void showReplace();
    void showFind();

protected:
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
    void slotSearchText( bool backward = false, bool isAutoSearch = true );
    void closeBar();

private slots:
    void slotClearSearch();
    void slotUpdateSearchOptions();
    void slotReplaceText();
    void slotReplaceAllText();

protected:
    QString mLastSearchStr;
    TextFindWidget *mFindWidget;
    TextReplaceWidget *mReplaceWidget;
    QPlainTextEdit *mView;
};

}

#endif

