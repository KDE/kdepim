/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef RICHTEXTCOMPOSER_INSERTHTMLDIALOG_H
#define RICHTEXTCOMPOSER_INSERTHTMLDIALOG_H

#include <KTextEdit>
#include <QDialog>

#include "kpimtextedit/plaintexteditor.h"

namespace KPIMTextEdit
{
class TextEditorCompleter;
}
namespace MessageComposer
{
class TextEditorCompleter;
class InsertHtmlDialogPrivate;
class InsertHtmlEditor : public KPIMTextEdit::PlainTextEditor
{
    Q_OBJECT
public:
    explicit InsertHtmlEditor(QWidget *parent = Q_NULLPTR);
    ~InsertHtmlEditor();
protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
private:
    KPIMTextEdit::TextEditorCompleter *mTextEditorCompleter;
};

class InsertHtmlDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InsertHtmlDialog(QWidget *parent = Q_NULLPTR);
    ~InsertHtmlDialog();
    QString html() const;

private:
    friend class InsertHtmlDialogPrivate;
    InsertHtmlDialogPrivate *const d;
    Q_PRIVATE_SLOT(d, void _k_slotTextChanged())
};

}

#endif
