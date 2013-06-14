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
#include <KTempDir>

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
    setButtons( Close );
    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Local themes:"));
    lay->addWidget(lab);

    mListThemes = new QListWidget;
    connect(mListThemes, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));
    lay->addWidget(mListThemes);

    mDeleteTheme = new KPushButton(i18n("Delete theme"));
    connect(mDeleteTheme, SIGNAL(clicked()), this, SLOT(slotDeleteTheme()));
    mDeleteTheme->setEnabled(false);
    lay->addWidget(mDeleteTheme);

    w->setLayout(lay);

    initialize();

    setMainWidget(w);
    readConfig();
}

ManageThemes::~ManageThemes()
{
    writeConfig();
}

void ManageThemes::readConfig()
{
    KConfigGroup group( KGlobal::config(), "ManageThemesDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize(300, 150);
    }
}

void ManageThemes::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "ManageThemesDialog" );
    group.writeEntry( "Size", size() );
}

void ManageThemes::slotDeleteTheme()
{
    if (mListThemes->currentItem()) {
        if (KMessageBox::questionYesNo(this, i18n("Do you want to remove selected theme?"), i18n("Remove theme")) == KMessageBox::Yes) {
            const QString localDirectory = KStandardDirs::locateLocal("data",QLatin1String("messageviewer/themes/"));
            if (KTempDir::removeDir(localDirectory + QDir::separator() + mListThemes->currentItem()->text())) {
                delete mListThemes->currentItem();
            } else {
                //TODO give info about with theme we can't delete.
                KMessageBox::error(this, i18n("Can not delete theme. Please contact your administrator."), i18n("Delete theme failed"));
            }
        }
    }
}

void ManageThemes::initialize()
{
    const QString localDirectory = KStandardDirs::locateLocal("data",QLatin1String("messageviewer/themes/"));
    QDir dir(localDirectory);
    if (dir.exists()) {
        bool hasSubDir = false;
        QDirIterator dirIt( localDirectory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
        while ( dirIt.hasNext() ) {
            dirIt.next();
            const QString dirName = dirIt.fileName();
            new QListWidgetItem(dirName, mListThemes);
            hasSubDir = true;
        }
        enableButtonOk(hasSubDir);
    } else {
        enableButtonOk(false);
    }
}

void ManageThemes::slotItemSelectionChanged()
{
    mDeleteTheme->setEnabled(mListThemes->currentItem());
}

#include "managethemes.moc"
