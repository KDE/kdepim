/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "composertableformatdialog.h"
#include "extendattributes/extendattributesbutton.h"

#include <KPIMTextEdit/InsertTableWidget>

#include <KLocalizedString>
#include <KColorButton>
#include <KSeparator>

#include <QWebElement>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include "composereditorng_debug.h"
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorNG
{
class ComposerTableFormatDialogPrivate
{
public:
    ComposerTableFormatDialogPrivate(const QWebElement &element, ComposerTableFormatDialog *qq)
        : q(qq)
    {
        initialize(element);
    }

    void initialize(const QWebElement &element);

    void applyChanges();
    void updateSettings();

    void _k_slotOkClicked();
    void _k_slotApplyClicked();
    void _k_slotWebElementChanged();

    QWebElement webElement;
    KColorButton *backgroundColor;
    QCheckBox *useBackgroundColor;
    KPIMTextEdit::InsertTableWidget *insertTableWidget;
    ComposerTableFormatDialog *q;
};

void ComposerTableFormatDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
}

void ComposerTableFormatDialogPrivate::applyChanges()
{
    if (!webElement.isNull()) {
        webElement.setAttribute(QLatin1String("border"), QString::number(insertTableWidget->border()));
        const QString width = QStringLiteral("%1%2").arg(insertTableWidget->length()).arg(insertTableWidget->typeOfLength() == QTextLength::PercentageLength ? QLatin1String("%") : QString());
        webElement.setAttribute(QLatin1String("width"), width);
        if (useBackgroundColor->isChecked()) {
            const QColor col = backgroundColor->color();
            if (col.isValid()) {
                webElement.setAttribute(QLatin1String("bgcolor"), backgroundColor->color().name());
            }
        } else {
            webElement.removeAttribute(QLatin1String("bgcolor"));
        }
        //TODO update column/row
    }
}

void ComposerTableFormatDialogPrivate::initialize(const QWebElement &element)
{
    webElement = element;
    q->setWindowTitle(i18n("Table Format"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    q->connect(buttonBox, &QDialogButtonBox::accepted, q, &ComposerTableFormatDialog::accept);
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, &ComposerTableFormatDialog::reject);
    okButton->setText(i18n("Edit"));
    QWidget *page = new QWidget(q);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    QVBoxLayout *lay = new QVBoxLayout(page);
    insertTableWidget = new KPIMTextEdit::InsertTableWidget(q);
    lay->addWidget(insertTableWidget);

    KSeparator *sep = new KSeparator;
    lay->addWidget(sep);

    QHBoxLayout *hbox = new QHBoxLayout;
    useBackgroundColor = new QCheckBox(i18n("Background Color:"));
    hbox->addWidget(useBackgroundColor);
    backgroundColor = new KColorButton;
    backgroundColor->setEnabled(false);
    hbox->addWidget(backgroundColor);

    lay->addLayout(hbox);

    if (!webElement.isNull()) {
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement, ExtendAttributesDialog::Table, q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        lay->addWidget(button);
    }

    sep = new KSeparator;
    lay->addWidget(sep);

    q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotOkClicked()));
    q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotApplyClicked()));

    q->connect(useBackgroundColor, &QCheckBox::toggled, backgroundColor, &KColorButton::setEnabled);
    updateSettings();
}

void ComposerTableFormatDialogPrivate::updateSettings()
{
    if (!webElement.isNull()) {
        if (webElement.hasAttribute(QLatin1String("border"))) {
            insertTableWidget->setBorder(webElement.attribute(QLatin1String("border")).toInt());
        }
        if (webElement.hasAttribute(QLatin1String("width"))) {
            QString width = webElement.attribute(QLatin1String("width"));
            if (width.endsWith(QLatin1Char('%'))) {
                insertTableWidget->setTypeOfLength(QTextLength::PercentageLength);
                width.chop(1);
                insertTableWidget->setLength(width.toInt());
            } else {
                insertTableWidget->setTypeOfLength(QTextLength::FixedLength);
                insertTableWidget->setLength(width.toInt());
            }
        }
        if (webElement.hasAttribute(QLatin1String("bgcolor"))) {
            useBackgroundColor->setChecked(true);
            const QColor color = QColor(webElement.attribute(QLatin1String("bgcolor")));
            backgroundColor->setColor(color);
        } else {
            useBackgroundColor->setChecked(false);
        }
        QWebElementCollection allRows = webElement.findAll(QLatin1String("tr"));
        insertTableWidget->setRows(allRows.count());
        QWebElementCollection allCol = webElement.findAll(QLatin1String("td"));
        insertTableWidget->setColumns(allCol.count() / allRows.count());
    }
}

void ComposerTableFormatDialogPrivate::_k_slotOkClicked()
{
    applyChanges();
    q->accept();
}

void ComposerTableFormatDialogPrivate::_k_slotApplyClicked()
{
    applyChanges();
}

ComposerTableFormatDialog::ComposerTableFormatDialog(const QWebElement &element, QWidget *parent)
    : QDialog(parent), d(new ComposerTableFormatDialogPrivate(element, this))
{
}

ComposerTableFormatDialog::~ComposerTableFormatDialog()
{
    delete d;
}

}

#include "moc_composertableformatdialog.cpp"
