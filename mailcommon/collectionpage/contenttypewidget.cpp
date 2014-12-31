/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "contenttypewidget.h"
#include "collectiontypeutil.h"
#include <KLocalizedString>
#include <KComboBox>
#include <QHBoxLayout>
#include <QLabel>

using namespace MailCommon;
ContentTypeWidget::ContentTypeWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);

    QLabel *label = new QLabel(i18n("&Folder contents:"), this);
    label->setObjectName(QLatin1String("contenttypewidgetlabel"));
    hbox->addWidget(label);
    mContentsComboBox = new KComboBox(this);
    mContentsComboBox->setObjectName(QLatin1String("contentcombobox"));
    label->setBuddy(mContentsComboBox);
    hbox->addWidget(mContentsComboBox);
    MailCommon::CollectionTypeUtil collectionUtil;
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeMail));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeCalendar));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeContact));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeNote));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeTask));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeJournal));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeConfiguration));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeFreebusy));
    mContentsComboBox->addItem(collectionUtil.folderContentDescription(CollectionTypeUtil::ContentsTypeFile));

    connect(mContentsComboBox, SIGNAL(activated(int)), SIGNAL(activated(int)));
}

ContentTypeWidget::~ContentTypeWidget()
{

}

int ContentTypeWidget::currentIndex() const
{
    return mContentsComboBox->currentIndex();
}

void ContentTypeWidget::setCurrentIndex(int index)
{
    mContentsComboBox->setCurrentIndex(index);
}

void ContentTypeWidget::setCurrentItem(const QString &name)
{
    mContentsComboBox->setCurrentItem(name);
}

QString ContentTypeWidget::currentText() const
{
    return mContentsComboBox->currentText();
}
