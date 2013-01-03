/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include <KSeparator>
#include <KLocale>
#include <KComboBox>
#include <KLineEdit>
#include <KPushButton>

#include <QWebElement>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>

namespace ComposerEditorNG {

class ExtendAttributesDialogPrivate
{
public:
    ExtendAttributesDialogPrivate(const QWebElement& element, ExtendAttributesDialog::ExtendType extendType, ExtendAttributesDialog *qq)
        : webElement(element), type(extendType), q(qq)
    {
        q->setCaption( i18n( "Extend Attribute" ) );
        q->setButtons( KDialog::Ok | KDialog::Cancel );
        QWidget *page = new QWidget( q );
        q->setMainWidget( page );

        QVBoxLayout *lay = new QVBoxLayout( page );

        treeWidget = new QTreeWidget;
        lay->addWidget(treeWidget);

        QHBoxLayout *hbox = new QHBoxLayout;
        attributes = new KComboBox;
        q->connect(attributes, SIGNAL(activated(int)), q, SLOT(_k_attributeChanged(int)));
        hbox->addWidget(attributes);
        attributeValue = new KLineEdit;
        hbox->addWidget(attributeValue);

        lay->addLayout(hbox);
        removeAttribute = new KPushButton( i18n( "Remove" ) );
        q->connect(removeAttribute, SIGNAL(clicked(bool)), q, SLOT(_k_slotRemoveAttribute()));
        lay->addWidget(removeAttribute);

        KSeparator *sep = new KSeparator;
        lay->addWidget( sep );

        q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));
        fillCombobox();
    }
    void _k_slotOkClicked();
    void _k_slotRemoveAttribute();
    void _k_attributeChanged(int);

    void initialize();
    void fillCombobox();

    QWebElement webElement;
    QTreeWidget *treeWidget;
    KComboBox *attributes;
    KLineEdit *attributeValue;
    KPushButton *removeAttribute;
    ExtendAttributesDialog::ExtendType type;
    ExtendAttributesDialog *q;
};

void ExtendAttributesDialogPrivate::_k_attributeChanged(int)
{
    //TODO
}

void ExtendAttributesDialogPrivate::fillCombobox()
{
    switch(type) {
    case ExtendAttributesDialog::Image:
        break;
    case ExtendAttributesDialog::Table:
        break;
    case ExtendAttributesDialog::Cell:
        break;
    case ExtendAttributesDialog::Link:
        break;
    }

    //TODO
}

void ExtendAttributesDialogPrivate::_k_slotRemoveAttribute()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    delete item;
}

void ExtendAttributesDialogPrivate::initialize()
{
    if (!webElement.isNull()) {

    }
}

void ExtendAttributesDialogPrivate::_k_slotOkClicked()
{
    if (!webElement.isNull()) {

    }
    q->accept();
}

ExtendAttributesDialog::ExtendAttributesDialog(const QWebElement &element, ExtendType type, QWidget *parent)
    : KDialog(parent), d(new ExtendAttributesDialogPrivate(element, type, this))
{
}

ExtendAttributesDialog::~ExtendAttributesDialog()
{
    delete d;
}

}

#include "extendattributesdialog.moc"
