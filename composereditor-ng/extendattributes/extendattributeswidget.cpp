/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "extendattributesutils_p.h"

#include <KComboBox>
#include <QPushButton>
#include <KSeparator>
#include <KLocalizedString>
#include <QLineEdit>

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QWebElement>
#include <QDebug>

namespace ComposerEditorNG {

class ExtendAttributesWidgetPrivate
{
public:
    ExtendAttributesWidgetPrivate(const QWebElement& element, ExtendAttributesDialog::SettingsType settingsType, ExtendAttributesDialog::ExtendType extendType, ExtendAttributesWidget *qq)
        : webElement(element),
          type(extendType),
          settings(settingsType),
          attributes(0),
          attributesLineEdit(0),
          blockSignal(false),
          q(qq)
    {
        QVBoxLayout *lay = new QVBoxLayout(q);

        treeWidget = new QTreeWidget;
        treeWidget->setRootIsDecorated(false);
        QStringList headerStr;

        if (settings == ExtendAttributesDialog::InlineStyle)
            headerStr << i18n("Property")<< i18n("Value");
        else
            headerStr << i18n("Attribute")<< i18n("Value");

        q->connect(treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), q, SLOT(_k_slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

        treeWidget->setHeaderLabels(headerStr);
        lay->addWidget(treeWidget);

        QHBoxLayout *hbox = new QHBoxLayout;

        if (settings == ExtendAttributesDialog::InlineStyle) {
            attributesLineEdit = new QLineEdit;
            q->connect(attributesLineEdit, SIGNAL(textChanged(QString)), q, SLOT(_k_attributeLineEditChanged(QString)));
            hbox->addWidget(attributesLineEdit);
        } else {
            attributes = new KComboBox;
            q->connect(attributes, SIGNAL(activated(QString)), q, SLOT(_k_attributeChanged(QString)));
            hbox->addWidget(attributes);
        }

        attributeValue = new KComboBox;
        attributeValue->setEditable(true);
        q->connect(attributeValue->lineEdit(), SIGNAL(textChanged(QString)), q, SLOT(_k_attributeValueChanged(QString)));
        hbox->addWidget(attributeValue);

        lay->addLayout(hbox);
        removeAttribute = new QPushButton( i18n( "Remove" ) );
        removeAttribute->setEnabled(false);
        q->connect(removeAttribute, SIGNAL(clicked(bool)), q, SLOT(_k_slotRemoveAttribute()));
        lay->addWidget(removeAttribute);

        KSeparator *sep = new KSeparator;
        lay->addWidget( sep );

        fillCombobox();
        initialize();
    }

    void _k_slotRemoveAttribute();
    void _k_attributeChanged(const QString &key);
    void _k_slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);
    void _k_attributeValueChanged(const QString&);
    void _k_attributeLineEditChanged(const QString&);

    void initialize();
    void fillCombobox();
    void changeAttributes();

    QWebElement webElement;

    ExtendAttributesDialog::ExtendType type;

    ExtendAttributesDialog::SettingsType settings;

    QMap<QString, QStringList> attributesMap;
    QTreeWidget *treeWidget;
    KComboBox *attributes;
    QLineEdit *attributesLineEdit;
    KComboBox *attributeValue;
    QPushButton *removeAttribute;

    bool blockSignal;
    ExtendAttributesWidget *q;
};

void ExtendAttributesWidgetPrivate::_k_attributeLineEditChanged(const QString &text)
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (item && item->text(1).isEmpty()) {
        item->setText(0,text);
    } else {
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
        item->setText(0, text);
        treeWidget->setCurrentItem(item);
    }
}

void ExtendAttributesWidgetPrivate::_k_attributeValueChanged(const QString &val)
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
        const QString key = item->text(0);
        _k_attributeChanged(key);
        if (attributes)
            attributes->setCurrentIndex(attributes->findText(key));
        else
            attributesLineEdit->setText(key);
    }
}

void ExtendAttributesWidgetPrivate::_k_attributeChanged(const QString &key)
{
    blockSignal = true;

    attributeValue->clear();
    attributeValue->addItem(QString()); //Add first empty attributes.
    attributeValue->addItems(attributesMap.value(key));

    const QList<QTreeWidgetItem *> lstItems = treeWidget->findItems(key, Qt::MatchCaseSensitive);
    if (lstItems.isEmpty()) {
        QTreeWidgetItem *currentItem = treeWidget->currentItem();
        if (currentItem && currentItem->text(1).isEmpty()) {
            currentItem->setText(0, key);
            treeWidget->setCurrentItem(currentItem);
        } else {
            QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
            item->setText(0, key);
            treeWidget->setCurrentItem(item);
        }
    } else {
        treeWidget->setCurrentItem(lstItems.at(0));
        attributeValue->lineEdit()->setText(lstItems.at(0)->text(1));
    }
    blockSignal = false;
}

void ExtendAttributesWidgetPrivate::fillCombobox()
{
    switch(settings) {
    case ExtendAttributesDialog::HtmlAttributes:
        attributesMap = ComposerEditorNG::ExtendAttributesUtils::attributesMap(type);
        break;
    case ExtendAttributesDialog::InlineStyle:
        //nothing
        break;
    case ExtendAttributesDialog::JavascriptEvents:
        if (type == ExtendAttributesDialog::Body)
            attributesMap = ComposerEditorNG::ExtendAttributesUtils::attributesJavascriptWindowAndBase();
        else
            attributesMap = ComposerEditorNG::ExtendAttributesUtils::attributesJavascript();
        break;
    }
    if (attributes)
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

void ExtendAttributesWidgetPrivate::changeAttributes()
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
}

ExtendAttributesWidget::ExtendAttributesWidget(const QWebElement &element, ExtendAttributesDialog::SettingsType settings, ExtendAttributesDialog::ExtendType type, QWidget *parent)
    : QWidget(parent), d(new ExtendAttributesWidgetPrivate(element, settings, type, this))
{
}

ExtendAttributesWidget::~ExtendAttributesWidget()
{
    delete d;
}

void ExtendAttributesWidget::changeAttributes()
{
    d->changeAttributes();
}

}


#include "moc_extendattributeswidget.cpp"
