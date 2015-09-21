/* Copyright (C) 2012-2015 Laurent Montel <montel@kde.org>
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
#include "pimcommon/texteditfindbarbase.h"
#include <QTextDocument>

class QPlainTextEdit;
namespace PimCommon
{
class PlainTextEditFindBarPrivate;
class PIMCOMMON_EXPORT PlainTextEditFindBar : public TextEditFindBarBase
{
    Q_OBJECT

public:
    explicit PlainTextEditFindBar(QPlainTextEdit *view, QWidget *parent = Q_NULLPTR);
    ~PlainTextEditFindBar();

protected:
    bool viewIsReadOnly() const Q_DECL_OVERRIDE;
    bool documentIsEmpty() const Q_DECL_OVERRIDE;
    bool searchInDocument(const QString &text, QTextDocument::FindFlags searchOptions) Q_DECL_OVERRIDE;
    void autoSearchMoveCursor() Q_DECL_OVERRIDE;

public Q_SLOTS:
    void slotSearchText(bool backward = false, bool isAutoSearch = true) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotReplaceText() Q_DECL_OVERRIDE;
    void slotReplaceAllText() Q_DECL_OVERRIDE;

private:
    PlainTextEditFindBarPrivate *const d;
};

}

#endif

