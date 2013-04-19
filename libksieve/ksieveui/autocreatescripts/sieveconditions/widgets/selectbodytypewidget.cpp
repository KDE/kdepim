/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <KLocale>
#include <KComboBox>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QDebug>

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
    setLayout(lay);

    mBodyCombobox = new KComboBox;
    lay->addWidget(mBodyCombobox);
    mBodyCombobox->addItem(i18n("raw"), QLatin1String(":raw"));
    mBodyCombobox->addItem(i18n("content"), QLatin1String(":content"));
    mBodyCombobox->addItem(i18n("text"), QLatin1String(":text"));
    connect(mBodyCombobox, SIGNAL(activated(int)), this, SLOT(slotBodyTypeChanged(int)));

    mBodyLineEdit = new KLineEdit;
    lay->addWidget(mBodyLineEdit);
    mBodyLineEdit->hide();
}

QString SelectBodyTypeWidget::code() const
{
    qDebug()<<" thus "<<this;
    QString value = mBodyCombobox->itemData(mBodyCombobox->currentIndex()).toString();
    if (mBodyLineEdit->isVisible())
        value += QString::fromLatin1(" \"%1\"").arg(mBodyLineEdit->text());
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
}

#include "selectbodytypewidget.moc"
