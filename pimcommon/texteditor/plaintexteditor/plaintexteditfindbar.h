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
#include "pimcommon/texteditor/commonwidget/texteditfindbarbase.h"
#include <QTextDocument>

class QPlainTextEdit;
namespace PimCommon
{
class PIMCOMMON_EXPORT PlainTextEditFindBar : public TextEditFindBarBase
{
    Q_OBJECT

public:
    explicit PlainTextEditFindBar(QPlainTextEdit *view, QWidget *parent = 0);
    ~PlainTextEditFindBar();

protected:
    virtual bool viewIsReadOnly() const;
    virtual bool documentIsEmpty() const;
    virtual bool searchInDocument(const QString &text, QTextDocument::FindFlags searchOptions);
    virtual void autoSearchMoveCursor();

public slots:
    virtual void slotSearchText(bool backward = false, bool isAutoSearch = true);

private slots:
    virtual void slotReplaceText();
    virtual void slotReplaceAllText();

private:
    QPlainTextEdit *mView;
};

}

#endif

