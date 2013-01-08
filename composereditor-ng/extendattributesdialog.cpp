/*
  Copyright (c) 2012-2013 Montel Laurent <montel.org>

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
#include "extendattributesutil_p.h"

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
        : webElement(element),
          type(extendType),
          blockSignal(false),
          q(qq)
    {
        q->setCaption( i18n( "Extend Attribute" ) );
        q->setButtons( KDialog::Ok | KDialog::Cancel );
        QWidget *page = new QWidget( q );
        q->setMainWidget( page );

        QVBoxLayout *lay = new QVBoxLayout( page );

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
    void _k_slotOkClicked();
    void _k_slotRemoveAttribute();
    void _k_attributeChanged(const QString &key);
    void _k_slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);
    void _k_attributeValueChanged(const QString&);

    void initialize();
    void fillCombobox();

    QWebElement webElement;
    QTreeWidget *treeWidget;
    KComboBox *attributes;
    KComboBox *attributeValue;
    KPushButton *removeAttribute;
    ExtendAttributesDialog::ExtendType type;
    QMap<QString, QStringList> attributesMap;
    bool blockSignal;
    ExtendAttributesDialog *q;
};

void ExtendAttributesDialogPrivate::_k_attributeValueChanged(const QString& val)
{
    if (blockSignal)
        return;

    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item) {
        item->setText(1, val);
    }
}

void ExtendAttributesDialogPrivate::_k_slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    removeAttribute->setEnabled(item);
    if (item) {
        _k_attributeChanged(item->text(0));
    }
}

void ExtendAttributesDialogPrivate::_k_attributeChanged(const QString& key)
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

void ExtendAttributesDialogPrivate::fillCombobox()
{
    attributesMap = ComposerEditorNG::ExtendAttributesUtil::attributesMap(type);
    attributes->addItems(attributesMap.keys());
}

void ExtendAttributesDialogPrivate::_k_slotRemoveAttribute()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    delete item;
}

void ExtendAttributesDialogPrivate::initialize()
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

void ExtendAttributesDialogPrivate::_k_slotOkClicked()
{
    if (!webElement.isNull()) {
        const QStringList keys = attributesMap.keys();
        Q_FOREACH (const QString& str, keys) {
            const QList<QTreeWidgetItem *> lstItems = treeWidget->findItems(str, Qt::MatchCaseSensitive);
            if (lstItems.isEmpty()) {
                if (webElement.hasAttribute(str)) {
                    webElement.removeAttribute(str);
                }
            } else {
                const QString value = lstItems.at(0)->text(1);
                if (!value.isEmpty()) {
                    webElement.setAttribute(str, value);
                }
            }
        }
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
