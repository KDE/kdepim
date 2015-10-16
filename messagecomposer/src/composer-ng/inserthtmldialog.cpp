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

#include "inserthtmldialog.h"

#include <KLocalizedString>
#include <KTextEdit>

#include <KPIMTextEdit/TextEditorCompleter>
#include <kpimtextedit/htmlhighlighter.h>
#include "kpimtextedit/plaintexteditorwidget.h"

#include <QCompleter>
#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

namespace MessageComposer
{

class InsertHtmlDialogPrivate
{
public:
    InsertHtmlDialogPrivate(InsertHtmlDialog *qq)
        : q(qq)
    {
        q->setWindowTitle(i18nc("@title:window", "Insert HTML"));
        QVBoxLayout *lay = new QVBoxLayout;
        q->setLayout(lay);
        QLabel *label = new QLabel(i18n("Insert HTML tags and texts:"));
        lay->addWidget(label);
        editor = new InsertHtmlEditor;
        KPIMTextEdit::PlainTextEditorWidget *editorWidget = new KPIMTextEdit::PlainTextEditorWidget(editor);
        lay->addWidget(editorWidget);
        label = new QLabel(i18n("Example: <i> Hello word </i>"));
        QFont font = label->font();
        font.setBold(true);
        label->setFont(font);
        label->setTextFormat(Qt::PlainText);
        lay->addWidget(label);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        okButton->setText(i18nc("@action:button", "Insert"));

        q->connect(buttonBox, &QDialogButtonBox::accepted, q, &QDialog::accept);
        q->connect(buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);

        lay->addWidget(buttonBox);
        q->connect(editor, SIGNAL(textChanged()),
                   q, SLOT(_k_slotTextChanged()));
        okButton->setEnabled(false);
        q->resize(640, 480);
    }

    void _k_slotTextChanged();
    QPushButton *okButton;
    InsertHtmlEditor *editor;
    InsertHtmlDialog *q;
};

void InsertHtmlDialogPrivate::_k_slotTextChanged()
{
    okButton->setEnabled(!editor->toPlainText().isEmpty());
}

InsertHtmlDialog::InsertHtmlDialog(QWidget *parent)
    : QDialog(parent), d(new InsertHtmlDialogPrivate(this))
{
}

InsertHtmlDialog::~InsertHtmlDialog()
{
    delete d;
}

QString InsertHtmlDialog::html() const
{
    return d->editor->toPlainText();
}

InsertHtmlEditor::InsertHtmlEditor(QWidget *parent)
    : KPIMTextEdit::PlainTextEditor(parent)
{
    new KPIMTextEdit::HtmlHighlighter(document());
    setFocus();
    mTextEditorCompleter = new KPIMTextEdit::TextEditorCompleter(this, this);
    QStringList completerList;
    completerList << QStringLiteral("<b></b>")
                  << QStringLiteral("<i></i>")
                  << QStringLiteral("<u></u>");
    //Add more
    mTextEditorCompleter->setCompleterStringList(completerList);
    mTextEditorCompleter->setExcludeOfCharacters(QStringLiteral("~!@#$%^&*()+{}|,./;'[]\\-= "));
}

InsertHtmlEditor::~InsertHtmlEditor()
{

}

void InsertHtmlEditor::keyPressEvent(QKeyEvent *e)
{
    if (mTextEditorCompleter->completer()->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }
    KPIMTextEdit::PlainTextEditor::keyPressEvent(e);
    mTextEditorCompleter->completeText();
}

}

#include "moc_inserthtmldialog.cpp"

