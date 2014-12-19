/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "contentstypecombobox.h"
#include <QLabel>
#include <QHBoxLayout>
#include <KLocalizedString>
#include <KComboBox>

using namespace MailCommon;

ContentsTypeComboBox::ContentsTypeComboBox(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);

    QLabel *label = new QLabel( i18n( "Generate free/&busy and activate alarms for:" ), this );
    label->setObjectName(QLatin1String("contentstypelabel"));
    hbox->addWidget(label);
    mIncidencesForComboBox = new KComboBox( this );
    label->setBuddy( mIncidencesForComboBox );
    hbox->addWidget(mIncidencesForComboBox);

    mIncidencesForComboBox->addItem( i18n( "Nobody" ) );
    mIncidencesForComboBox->addItem( i18n( "Admins of This Folder" ) );
    mIncidencesForComboBox->addItem( i18n( "All Readers of This Folder" ) );
    const QString whatsThisForMyOwnFolders =
            i18n( "This setting defines which users sharing "
                  "this folder should get \"busy\" periods in their freebusy lists "
                  "and should see the alarms for the events or tasks in this folder. "
                  "The setting applies to Calendar and Task folders only "
                  "(for tasks, this setting is only used for alarms).\n\n"
                  "Example use cases: if the boss shares a folder with his secretary, "
                  "only the boss should be marked as busy for his meetings, so he should "
                  "select \"Admins\", since the secretary has no admin rights on the folder.\n"
                  "On the other hand if a working group shares a Calendar for "
                  "group meetings, all readers of the folders should be marked "
                  "as busy for meetings.\n"
                  "A company-wide folder with optional events in it would use \"Nobody\" "
                  "since it is not known who will go to those events." );

    mIncidencesForComboBox->setObjectName(QLatin1String("contentstypecombobox"));
    mIncidencesForComboBox->setWhatsThis( whatsThisForMyOwnFolders );
    connect(mIncidencesForComboBox, SIGNAL(currentIndexChanged(int)), SIGNAL(indexChanged(int)));
}

ContentsTypeComboBox::~ContentsTypeComboBox()
{

}

int ContentsTypeComboBox::currentIndex() const
{
    return mIncidencesForComboBox->currentIndex();
}

void ContentsTypeComboBox::setCurrentIndex(int index)
{
    mIncidencesForComboBox->setCurrentIndex(index);
}

