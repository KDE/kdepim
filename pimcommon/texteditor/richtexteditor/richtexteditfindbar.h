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

#ifndef RICHTEXTEDITFINDBAR_H
#define RICHTEXTEDITFINDBAR_H

#include "pimcommon_export.h"
#include "pimcommon/texteditor/commonwidget/texteditfindbarbase.h"
#include <QTextDocument>

class QTextEdit;
namespace PimCommon
{
class PIMCOMMON_EXPORT RichTextEditFindBar : public TextEditFindBarBase
{
    Q_OBJECT
public:
    explicit RichTextEditFindBar(QTextEdit *view, QWidget *parent = Q_NULLPTR);
    ~RichTextEditFindBar();

protected:
    bool viewIsReadOnly() const Q_DECL_OVERRIDE;
    bool documentIsEmpty() const Q_DECL_OVERRIDE;
    bool searchInDocument(const QString &text, QTextDocument::FindFlags searchOptions) Q_DECL_OVERRIDE;
    void autoSearchMoveCursor() Q_DECL_OVERRIDE;

public Q_SLOTS:
    virtual void slotSearchText(bool backward = false, bool isAutoSearch = true) Q_DECL_OVERRIDE;

private Q_SLOTS:
    virtual void slotReplaceText() Q_DECL_OVERRIDE;
    virtual void slotReplaceAllText() Q_DECL_OVERRIDE;

private:
    QTextEdit *mView;
};

}

#endif // RICHTEXTEDITFINDBAR_H
