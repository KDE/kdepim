/*
  Copyright (c) 2013 Montel Laurent <montel.org>

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

#include "extendattributeswidget.h"
#include "extendattributesutil_p.h"

#include <KComboBox>
#include <KPushButton>
#include <KSeparator>
#include <KLocale>

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QWebElement>

namespace ComposerEditorNG {

class ExtendAttributesWidgetPrivate
{
public:
    ExtendAttributesWidgetPrivate(ExtendAttributesWidget *qq)
        :blockSignal(false), q(qq)
    {
        QVBoxLayout *lay = new QVBoxLayout(q);

        treeWidget = new QTreeWidget;
        QStringList headerStr;
        headerStr << i18n("Attribute")<< i18n("Value");
        q->connect(treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), q, SLOT(_k_slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

        treeWidget->setHeaderLabels(headerStr);
        lay->addWidget(treeWidget);

        QHBoxLayout *hbox = new QHBoxLayout;
        attributes = new KComboBox;
        q->connect(attributes, SIGNAL(activated(QString)), q, SLOT(_k_attributeChanged(QString)));
        hbox->addWidget(attributes);

        attributeValue = new KComboBox;
        attributeValue->setEditable(true);
        q->connect(attributeValue->lineEdit(), SIGNAL(textChanged(QString)), q, SLOT(_k_attributeValueChanged(QString)));
        hbox->addWidget(attributeValue);

        lay->addLayout(hbox);
        removeAttribute = new KPushButton( i18n( "Remove" ) );
        removeAttribute->setEnabled(false);
        q->connect(removeAttribute, SIGNAL(clicked(bool)), q, SLOT(_k_slotRemoveAttribute()));
        lay->addWidget(removeAttribute);

        KSeparator *sep = new KSeparator;
        lay->addWidget( sep );

        q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));
        fillCombobox();
        initialize();

    }

    void _k_slotRemoveAttribute();
    void _k_attributeChanged(const QString &key);
    void _k_slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);
    void _k_attributeValueChanged(const QString&);

    void initialize();
    void fillCombobox();

    QWebElement webElement;

    ExtendAttributesDialog::ExtendType type;

    QMap<QString, QStringList> attributesMap;
    QTreeWidget *treeWidget;
    KComboBox *attributes;
    KComboBox *attributeValue;
    KPushButton *removeAttribute;

    bool blockSignal;
    ExtendAttributesWidget *q;
};

void ExtendAttributesWidgetPrivate::_k_attributeValueChanged(const QString& val)
{
    if (blockSignal)
        return;

    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item) {
        item->setText(1, val);
    }
}

void ExtendAttributesWidgetPrivate::_k_slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    removeAttribute->setEnabled(item);
    if (item) {
        _k_attributeChanged(item->text(0));
    }
}

void ExtendAttributesWidgetPrivate::_k_attributeChanged(const QString& key)
{
    blockSignal = true;

    attributeValue->clear();
    attributeValue->addItem(QString()); //Add first empty attributes.
    attributeValue->addItems(attributesMap.value(key));

    const QList<QTreeWidgetItem *> lstItems = treeWidget->findItems(key, Qt::MatchCaseSensitive);
    if (lstItems.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
        item->setText(0, key);
        treeWidget->setCurrentItem(item);
    } else {
        treeWidget->setCurrentItem(lstItems.at(0));
        attributeValue->lineEdit()->setText(lstItems.at(0)->text(1));
    }
    blockSignal = false;
}

void ExtendAttributesWidgetPrivate::fillCombobox()
{
    attributesMap = ComposerEditorNG::ExtendAttributesUtil::attributesMap(type);
    attributes->addItems(attributesMap.keys());
}

void ExtendAttributesWidgetPrivate::_k_slotRemoveAttribute()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    delete item;
}

void ExtendAttributesWidgetPrivate::initialize()
{
    if (!webElement.isNull()) {
        const QStringList keys = attributesMap.keys();
        Q_FOREACH (const QString& str, keys) {
            if (webElement.hasAttribute(str)) {
                QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
                item->setText(0, str);
                item->setText(1, webElement.attribute(str));
            }
        }
    }
}



ExtendAttributesWidget::ExtendAttributesWidget(QWidget *parent)
    : QWidget(parent), d(new ExtendAttributesWidgetPrivate(this))
{
}

ExtendAttributesWidget::~ExtendAttributesWidget()
{
    delete d;
}

}

