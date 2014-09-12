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

#include "composertablecellformatdialog.h"
#include "composercellsizewidget.h"
#include "extendattributes/extendattributesbutton.h"

#include <KLocalizedString>
#include <KColorButton>
#include <KComboBox>
#include <KSeparator>

#include <QWebElement>
#include <QVBoxLayout>
#include <QCheckBox>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorNG
{

class ComposerTableCellFormatDialogPrivate
{
public:
    ComposerTableCellFormatDialogPrivate(const QWebElement &element, ComposerTableCellFormatDialog *qq)
        : webElement(element)
        , q(qq)
    {
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Apply);
        QWidget *mainWidget = new QWidget(q);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        q->setLayout(mainLayout);
        mainLayout->addWidget(mainWidget);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        q->connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
        q->connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));
        mainLayout->addWidget(buttonBox);
        q->setWindowTitle(i18n("Edit Cell Format"));

        QVBoxLayout *layout = new QVBoxLayout(mainWidget);

        width = new ComposerCellSizeWidget;
        width->setLabel(i18n("Width:"));
        layout->addWidget(width);

        height = new ComposerCellSizeWidget;
        height->setLabel(i18n("Height:"));
        layout->addWidget(height);

        KSeparator *sep = new KSeparator;
        layout->addWidget(sep);

        QHBoxLayout *hbox = new QHBoxLayout;

        useHorizontalAlignment = new QCheckBox(i18n("Horizontal Alignment:"));
        hbox->addWidget(useHorizontalAlignment);
        horizontalAlignment = new KComboBox;
        horizontalAlignment->addItem(i18nc("@item:inlistbox Horizontal Alignment:", "Top"), QLatin1String("top"));
        horizontalAlignment->addItem(i18nc("@item:inlistbox Horizontal Alignment:", "Middle"), QLatin1String("middle"));
        horizontalAlignment->addItem(i18nc("@item:inlistbox Horizontal Alignment:", "Bottom"), QLatin1String("bottom"));
        horizontalAlignment->addItem(i18nc("@item:inlistbox Horizontal Alignment:", "Baseline"), QLatin1String("baseline"));
        horizontalAlignment->setEnabled(false);
        hbox->addWidget(horizontalAlignment);
        layout->addLayout(hbox);
        q->connect(useHorizontalAlignment, &QCheckBox::toggled, horizontalAlignment, &KComboBox::setEnabled);

        hbox = new QHBoxLayout;
        useVerticalAlignment = new QCheckBox(i18n("Vertical Alignment:"));
        hbox->addWidget(useVerticalAlignment);
        verticalAlignment = new KComboBox;
        verticalAlignment->addItem(i18nc("@item:inlistbox Vertical Alignment:", "Left"), QLatin1String("left"));
        verticalAlignment->addItem(i18nc("@item:inlistbox Vertical Alignment:", "Center"), QLatin1String("center"));
        verticalAlignment->addItem(i18nc("@item:inlistbox Vertical Alignment:", "Right"), QLatin1String("right"));
        verticalAlignment->addItem(i18nc("@item:inlistbox Vertical Alignment:", "Justify"), QLatin1String("justify"));
        verticalAlignment->addItem(i18nc("@item:inlistbox Vertical Alignment:", "Char"), QLatin1String("char"));
        verticalAlignment->setEnabled(false);
        hbox->addWidget(verticalAlignment);
        layout->addLayout(hbox);
        q->connect(useVerticalAlignment, &QCheckBox::toggled, verticalAlignment, &KComboBox::setEnabled);

        sep = new KSeparator;
        layout->addWidget(sep);

        hbox = new QHBoxLayout;
        useBackgroundColor = new QCheckBox(i18n("Background Color:"));
        hbox->addWidget(useBackgroundColor);
        backgroundColor = new KColorButton;
        backgroundColor->setEnabled(false);
        hbox->addWidget(backgroundColor);

        layout->addLayout(hbox);

        if (!webElement.isNull()) {
            ExtendAttributesButton *button = new ExtendAttributesButton(webElement, ExtendAttributesDialog::Cell, q);
            q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
            layout->addWidget(button);
        }

        sep = new KSeparator;
        layout->addWidget(sep);

        q->connect(useBackgroundColor, &QCheckBox::toggled, backgroundColor, &KColorButton::setEnabled);

        q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotOkClicked()));

        q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotApplyClicked()));
        updateSettings();
    }

    void _k_slotOkClicked();
    void _k_slotApplyClicked();
    void _k_slotWebElementChanged();

    void applyChanges();
    void updateSettings();

    QWebElement webElement;
    KColorButton *backgroundColor;

    KComboBox *verticalAlignment;
    KComboBox *horizontalAlignment;

    QCheckBox *useBackgroundColor;
    QCheckBox *useVerticalAlignment;
    QCheckBox *useHorizontalAlignment;

    ComposerCellSizeWidget *width;
    ComposerCellSizeWidget *height;
    ComposerTableCellFormatDialog *q;
};

void ComposerTableCellFormatDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
}

void ComposerTableCellFormatDialogPrivate::updateSettings()
{
    if (!webElement.isNull()) {
        if (webElement.hasAttribute(QLatin1String("bgcolor"))) {
            useBackgroundColor->setChecked(true);
            const QColor color = QColor(webElement.attribute(QLatin1String("bgcolor")));
            backgroundColor->setColor(color);
        }
        if (webElement.hasAttribute(QLatin1String("valign"))) {
            useVerticalAlignment->setChecked(true);
            const QString valign = webElement.attribute(QLatin1String("valign"));
            verticalAlignment->setCurrentIndex(verticalAlignment->findData(valign));
        }
        if (webElement.hasAttribute(QLatin1String("align"))) {
            useHorizontalAlignment->setChecked(true);
            const QString align = webElement.attribute(QLatin1String("align"));
            horizontalAlignment->setCurrentIndex(horizontalAlignment->findData(align));
        }
        if (webElement.hasAttribute(QLatin1String("width"))) {
            const QString widthVal = webElement.attribute(QLatin1String("width"));
            width->setValue(widthVal);
        }
        if (webElement.hasAttribute(QLatin1String("height"))) {
            const QString heightVal = webElement.attribute(QLatin1String("height"));
            height->setValue(heightVal);
        }
    }

}

void ComposerTableCellFormatDialogPrivate::_k_slotApplyClicked()
{
    applyChanges();
}

void ComposerTableCellFormatDialogPrivate::applyChanges()
{
    if (!webElement.isNull()) {
        if (useBackgroundColor->isChecked()) {
            const QColor col = backgroundColor->color();
            if (col.isValid()) {
                webElement.setAttribute(QLatin1String("bgcolor"), col.name());
            }
        } else {
            webElement.removeAttribute(QLatin1String("bgcolor"));
        }
        if (useVerticalAlignment->isChecked()) {
            webElement.setAttribute(QLatin1String("valign"), verticalAlignment->itemData(verticalAlignment->currentIndex()).toString());
        } else {
            webElement.removeAttribute(QLatin1String("valign"));
        }
        if (useHorizontalAlignment->isChecked()) {
            webElement.setAttribute(QLatin1String("align"), horizontalAlignment->itemData(horizontalAlignment->currentIndex()).toString());
        } else {
            webElement.removeAttribute(QLatin1String("align"));
        }
        const QString widthStr = width->value();
        if (widthStr.isEmpty()) {
            webElement.removeAttribute(QLatin1String("width"));
        } else {
            webElement.setAttribute(QLatin1String("width"), widthStr);
        }
        const QString heightStr = height->value();
        if (heightStr.isEmpty()) {
            webElement.removeAttribute(QLatin1String("height"));
        } else {
            webElement.setAttribute(QLatin1String("height"), heightStr);
        }
    }
}

void ComposerTableCellFormatDialogPrivate::_k_slotOkClicked()
{
    applyChanges();
    q->accept();
}

ComposerTableCellFormatDialog::ComposerTableCellFormatDialog(const QWebElement &element, QWidget *parent)
    : QDialog(parent), d(new ComposerTableCellFormatDialogPrivate(element, this))
{
}

ComposerTableCellFormatDialog::~ComposerTableCellFormatDialog()
{
    delete d;
}

}

#include "moc_composertablecellformatdialog.cpp"
