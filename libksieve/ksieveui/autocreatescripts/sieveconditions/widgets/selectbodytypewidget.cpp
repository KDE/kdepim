/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "selectbodytypewidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>
#include <KComboBox>
#include <QLineEdit>

#include <QHBoxLayout>

using namespace KSieveUi;

SelectBodyTypeWidget::SelectBodyTypeWidget(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

SelectBodyTypeWidget::~SelectBodyTypeWidget()
{
}

void SelectBodyTypeWidget::initialize()
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    setLayout(lay);

    mBodyCombobox = new KComboBox;
    lay->addWidget(mBodyCombobox);
    mBodyCombobox->addItem(i18n("raw"), QLatin1String(":raw"));
    mBodyCombobox->addItem(i18n("content"), QLatin1String(":content"));
    mBodyCombobox->addItem(i18n("text"), QLatin1String(":text"));
    connect(mBodyCombobox, SIGNAL(activated(int)), this, SLOT(slotBodyTypeChanged(int)));

    mBodyLineEdit = new QLineEdit;
    connect(mBodyLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    lay->addWidget(mBodyLineEdit);
    mBodyLineEdit->hide();
}

QString SelectBodyTypeWidget::code() const
{
    QString value = mBodyCombobox->itemData(mBodyCombobox->currentIndex()).toString();
    if (value == QLatin1String(":content")) {
        value += QString::fromLatin1(" \"%1\"").arg(mBodyLineEdit->text());
    }
    return value;
}

void SelectBodyTypeWidget::slotBodyTypeChanged(int index)
{
    const QString value = mBodyCombobox->itemData(index).toString();
    if (value == QLatin1String(":content")) {
        mBodyLineEdit->show();
    } else {
        mBodyLineEdit->hide();
    }
    Q_EMIT valueChanged();
}

void SelectBodyTypeWidget::setCode(const QString &type, const QString &content, const QString &name, QString &error)
{
    const int index = mBodyCombobox->findData(type);
    if (index != -1) {
        mBodyCombobox->setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(type, name, error);
        mBodyCombobox->setCurrentIndex(0);
    }
    slotBodyTypeChanged(index);
    mBodyLineEdit->setText(content);
}

