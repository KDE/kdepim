/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "composerlistdialog.h"
#include "extendattributes/extendattributesbutton.h"
#include "helper/listhelper_p.h"

#include <KLocale>
#include <KSeparator>
#include <KComboBox>

#include <QVBoxLayout>
#include <QWebElement>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>

namespace ComposerEditorNG
{

class ComposerListDialogPrivate
{
public:
    ComposerListDialogPrivate(const QWebElement& element, ComposerListDialog *qq)
        : webElement(element),
          listType(0),
          listStyle(0),
          start(0),
          styleLabel(0),
          q(qq),
          type(ExtendAttributesDialog::ListUL)
    {
        initialize();
    }
    void _k_slotOkClicked();
    void _k_slotWebElementChanged();

    enum ListType {
        None = 0,
        Bullet = 1,
        Numbered = 2,
        Definition = 3
    };

    void initialize();
    void initializeTypeList();
    void updateSettings();
    void updateListHtml();
    void fillStyle();
    QWebElement webElement;
    KComboBox *listType;
    KComboBox *listStyle;
    QSpinBox *start;
    QLabel *styleLabel;
    ComposerListDialog *q;
    ExtendAttributesDialog::ExtendType type;
};

void ComposerListDialogPrivate::initialize()
{

    q->setButtons( KDialog::Ok | KDialog::Cancel );
    q->setCaption( i18n( "Edit List" ) );

    QVBoxLayout *vbox = new QVBoxLayout(q->mainWidget());

    QLabel *lab = new QLabel(i18n("List Type:"));
    vbox->addWidget(lab);

    listType = new KComboBox;
    vbox->addWidget(listType);
    listType->addItem(i18n("None"), None);
    listType->addItem(i18n("Bullet List"), Bullet);
    listType->addItem(i18n("Numbered List"), Numbered);
    listType->addItem(i18n("Definition List"), Definition);
    listType->setEnabled(false);

    styleLabel = new QLabel(i18n("List Type:"));
    vbox->addWidget(styleLabel);

    listStyle = new KComboBox;
    vbox->addWidget(listStyle);

    lab = new QLabel(i18n("Start Number:"));
    vbox->addWidget(lab);
    start = new QSpinBox;
    start->setMinimum(1);
    start->setMaximum(9999);
    vbox->addWidget(start);


    initializeTypeList();

    KSeparator *sep = 0;
    if (!webElement.isNull()) {
        sep = new KSeparator;
        vbox->addWidget( sep );
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement,type,q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        vbox->addWidget( button );
    }

    sep = new KSeparator;
    vbox->addWidget( sep );

    q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));
    fillStyle();    
    updateSettings();
    q->resize(300,200);
}

void ComposerListDialogPrivate::initializeTypeList()
{
    if (!webElement.isNull()) {
        type = ListHelper::listType(webElement);
        switch(type) {
        case ExtendAttributesDialog::ListOL: {
            listType->setCurrentIndex(2);
            styleLabel->setText(i18n("Number Style:"));
            break;
        }
        case ExtendAttributesDialog::ListUL: {
            listType->setCurrentIndex(1);
            styleLabel->setText(i18n("Bullet Style:"));
            break;
        }
        case ExtendAttributesDialog::ListDL: {
            listType->setCurrentIndex(3);
            styleLabel->setText(i18n("Bullet Style:"));
            break;
        }
        default:
           return;
        }
    }
}

void ComposerListDialogPrivate::fillStyle()
{
    listStyle->clear();
    if (type == ExtendAttributesDialog::ListUL) {
        listStyle->addItem(i18n("Automatic"),QString());
        listStyle->addItem(i18n("Solid circle"),QLatin1String("disc"));
        listStyle->addItem(i18n("Open circle"),QLatin1String("circle"));
        listStyle->addItem(i18n("Solid square"),QLatin1String("square"));
        listStyle->setEnabled(true);
        start->setEnabled(false);
    } else if (type == ExtendAttributesDialog::ListOL) {
        listStyle->addItem(i18n("Automatic"),QString());
        listStyle->addItem(i18n("1,2,3..."),QLatin1String("1"));
        listStyle->addItem(i18n("A,B,C..."),QLatin1String("A"));
        listStyle->addItem(i18n("a,b,c..."),QLatin1String("a"));
        listStyle->addItem(i18n("I,II,III..."),QLatin1String("I"));
        listStyle->addItem(i18n("i,ii,iii..."),QLatin1String("i"));
        listStyle->setEnabled(true);
        start->setEnabled(true);
    } else {
        listStyle->setEnabled(false);
        start->setEnabled(false);
    }
}

void ComposerListDialogPrivate::updateSettings()
{
    if (!webElement.isNull()) {
        if (webElement.hasAttribute(QLatin1String("type"))) {
            const QString newType = webElement.attribute(QLatin1String("type"));
            const int itemIndex = listStyle->findData(newType);
            if (itemIndex!=-1) {
                listStyle->setCurrentIndex(itemIndex);
            }
        }
        if (webElement.hasAttribute(QLatin1String("start"))) {
            const int startValue = webElement.attribute(QLatin1String("start"), QLatin1String("1")).toInt();
            start->setValue(startValue);
        }
    }
}

void ComposerListDialogPrivate::updateListHtml()
{
    if ((type == ExtendAttributesDialog::ListUL) || (type == ExtendAttributesDialog::ListOL)) {
        const QString newType = listStyle->itemData(listStyle->currentIndex()).toString();
        if (newType.isEmpty()) {
            if (webElement.hasAttribute(QLatin1String("type"))) {
                webElement.removeAttribute(QLatin1String("type"));
            }
        } else {
            webElement.setAttribute(QLatin1String("type"), newType);
        }
        if (start->isEnabled()) {
            const int startValue = start->value();
            webElement.setAttribute(QLatin1String("start"), QString::number(startValue));
        } else {
            webElement.removeAttribute(QLatin1String("start"));
        }
    } else {
        //TODO ?
    }
}

void ComposerListDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
}

void ComposerListDialogPrivate::_k_slotOkClicked()
{
    if (!webElement.isNull()) {
        updateListHtml();
    }
    q->accept();
}

ComposerListDialog::ComposerListDialog(const QWebElement& element, QWidget *parent)
    : KDialog(parent), d(new ComposerListDialogPrivate(element, this))
{
}

ComposerListDialog::~ComposerListDialog()
{
    delete d;
}

}

#include "composerlistdialog.moc"
