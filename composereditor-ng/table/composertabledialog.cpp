/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "composertabledialog.h"

#include <KPIMTextEdit/InsertTableWidget>

#include <KLocalizedString>
#include <KSeparator>

#include <QWebElement>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorNG
{
class ComposerTableDialogPrivate
{
public:
    ComposerTableDialogPrivate(ComposerTableDialog *qq)
        : q(qq)
    {
        initialize();
    }

    QString html() const;

    void initialize();

    void _k_slotOkClicked();

    KPIMTextEdit::InsertTableWidget *insertTableWidget;
    ComposerTableDialog *q;
};

QString ComposerTableDialogPrivate::html() const
{
    const int numberOfColumns(insertTableWidget->columns());
    const int numberRow(insertTableWidget->rows());

    QString htmlTable = QString::fromLatin1("<table border='%1'").arg(insertTableWidget->border());
    htmlTable += QString::fromLatin1(" width='%1%2'").arg(insertTableWidget->length()).arg(insertTableWidget->typeOfLength() == QTextLength::PercentageLength ? QLatin1String("%") : QString());
    htmlTable += QString::fromLatin1(">");
    for (int i = 0; i < numberRow; ++i) {
        htmlTable += QLatin1String("<tr>");
        for (int j = 0; j < numberOfColumns; ++j) {
            htmlTable += QLatin1String("<td><br></td>");
        }
        htmlTable += QLatin1String("</tr>");
    }
    htmlTable += QLatin1String("</table>");
    return htmlTable;
}

void ComposerTableDialogPrivate::initialize()
{
    q->setWindowTitle(i18n("Insert Table"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(q);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    q->connect(buttonBox, &QDialogButtonBox::accepted, q, &ComposerTableDialog::accept);
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, &ComposerTableDialog::reject);
    mainLayout->addWidget(buttonBox);
    okButton->setText(i18n("Insert"));

    QWidget *page = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    page->setLayout(lay);

    insertTableWidget = new KPIMTextEdit::InsertTableWidget(q);
    lay->addWidget(insertTableWidget);

    KSeparator *sep = new KSeparator;
    lay->addWidget(sep);

    mainLayout->addWidget(page);
    q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotOkClicked()));
}

void ComposerTableDialogPrivate::_k_slotOkClicked()
{
    q->accept();
}

ComposerTableDialog::ComposerTableDialog(QWidget *parent)
    : QDialog(parent), d(new ComposerTableDialogPrivate(this))
{
}

ComposerTableDialog::~ComposerTableDialog()
{
    delete d;
}

QString ComposerTableDialog::html() const
{
    return d->html();
}

}

#include "moc_composertabledialog.cpp"
