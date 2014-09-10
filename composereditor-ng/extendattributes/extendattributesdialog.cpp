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

#include "extendattributesdialog.h"
#include "extendattributeswidget.h"
#include "extendattributesutils_p.h"

#include <KLocalizedString>
#include <QPushButton>
#include <QDebug>

#include <QWebElement>
#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <KConfigGroup>
#include <QDialogButtonBox>

namespace ComposerEditorNG
{

class ExtendAttributesDialogPrivate
{
public:
    ExtendAttributesDialogPrivate(const QWebElement &element, ExtendAttributesDialog::ExtendType extendType, ExtendAttributesDialog *qq)
        : q(qq)
    {
        q->setWindowTitle(i18n("Extend Attribute"));
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        q->setLayout(mainLayout);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        q->connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
        q->connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));
        QWidget *page = new QWidget(q);
        mainLayout->addWidget(page);
        mainLayout->addWidget(buttonBox);


        QVBoxLayout *lay = new QVBoxLayout(page);
        QString tagName;
        switch (extendType) {
        case ExtendAttributesDialog::Image:
            tagName = QLatin1String("img");
            break;
        case ExtendAttributesDialog::Table:
            tagName = QLatin1String("table");
            break;
        case ExtendAttributesDialog::Cell:
            tagName = QLatin1String("cell");
            break;
        case ExtendAttributesDialog::Link:
            tagName = QLatin1String("a");
            break;
        case ExtendAttributesDialog::Body:
            tagName = QLatin1String("body");
            break;
        case ExtendAttributesDialog::ListUL:
            tagName = QLatin1String("ul");
            break;
        case ExtendAttributesDialog::ListOL:
            tagName = QLatin1String("ol");
            break;
        case ExtendAttributesDialog::ListDL:
            tagName = QLatin1String("dl");
            break;
        default:
            qDebug() << " extendattribute not implemented" << extendType;
            break;
        }

        QLabel *lab = new QLabel(i18n("Current attributes for: \"%1\"", tagName));
        lay->addWidget(lab);

        QTabWidget *tab = new QTabWidget;

        htmlAttributes = new ExtendAttributesWidget(element, ExtendAttributesDialog::HtmlAttributes, extendType, q);
        javascriptAttributes = new ExtendAttributesWidget(element, ExtendAttributesDialog::JavascriptEvents, extendType, q);
        inlineStyleAttributes = new ExtendAttributesWidget(element, ExtendAttributesDialog::InlineStyle, extendType, q);

        tab->addTab(htmlAttributes, i18n("Html Attributes"));
        tab->addTab(javascriptAttributes, i18n("Javascript"));
        tab->addTab(inlineStyleAttributes, i18n("Inline Style"));

        lay->addWidget(tab);
        q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotOkClicked()));
        q->resize(400, 300);
    }
    void _k_slotOkClicked();

    ExtendAttributesWidget *htmlAttributes;
    ExtendAttributesWidget *javascriptAttributes;
    ExtendAttributesWidget *inlineStyleAttributes;
    ExtendAttributesDialog *q;
};

void ExtendAttributesDialogPrivate::_k_slotOkClicked()
{
    htmlAttributes->changeAttributes();
    javascriptAttributes->changeAttributes();
    inlineStyleAttributes->changeAttributes();
    q->accept();
}

ExtendAttributesDialog::ExtendAttributesDialog(const QWebElement &element, ExtendType type, QWidget *parent)
    : QDialog(parent), d(new ExtendAttributesDialogPrivate(element, type, this))
{
}

ExtendAttributesDialog::~ExtendAttributesDialog()
{
    delete d;
}

}

#include "moc_extendattributesdialog.cpp"
