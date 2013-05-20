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

#include "managethemes.h"

#include <KLocale>
#include <KStandardDirs>
#include <KPushButton>
#include <KMessageBox>

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

ManageThemes::ManageThemes(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Manage Theme" ) );
    setButtons( Ok|Cancel );
    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Local themes:"));
    lay->addWidget(lab);

    mListThemes = new QListWidget;
    connect(mListThemes, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotItemSelected(QListWidgetItem*)));
    lay->addWidget(mListThemes);

    mDeleteTheme = new KPushButton(i18n("Delete theme"));
    connect(mDeleteTheme, SIGNAL(clicked()), this, SLOT(slotDeleteTheme()));
    mDeleteTheme->setEnabled(false);
    lay->addWidget(mDeleteTheme);

    w->setLayout(lay);

    initialize();

    setMainWidget(w);
    resize(300,150);
}

ManageThemes::~ManageThemes()
{
}

void ManageThemes::slotDeleteTheme()
{
    if (mListThemes->currentItem()) {
        if (KMessageBox::questionYesNo(this, i18n("Do you want to remove selected theme?"), i18n("Remove theme")) == KMessageBox::Yes) {
            const QString localDirectory = KStandardDirs::locateLocal("data",QLatin1String("messageviewer/themes/"));
            QDir themeDir(localDirectory);
            themeDir.remove(mListThemes->currentItem()->text());
            delete mListThemes->currentItem();
        }
    }
}

void ManageThemes::initialize()
{
    const QString localDirectory = KStandardDirs::locateLocal("data",QLatin1String("messageviewer/themes/"));
    QDir dir(localDirectory);
    if (dir.exists()) {
        QDirIterator dirIt( localDirectory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
        while ( dirIt.hasNext() ) {
            dirIt.next();
            const QString dirName = dirIt.fileName();
            new QListWidgetItem(dirName, mListThemes);
        }
        enableButtonOk(true);
    } else {
        enableButtonOk(false);
    }
}

void ManageThemes::slotItemSelected(QListWidgetItem* item)
{
    mDeleteTheme->setEnabled(item);
}

#include "managethemes.moc"
