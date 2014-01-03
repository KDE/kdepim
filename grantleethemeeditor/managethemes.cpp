/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include <KLocalizedString>
#include <KStandardDirs>
#include <KPushButton>
#include <KMessageBox>
#include <KTempDir>
#include <KSharedConfig>

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDir>
#include <QDirIterator>

static const KCatalogLoader loader( QLatin1String("libgrantleethemeeditor") );

using namespace GrantleeThemeEditor;

ManageThemes::ManageThemes(const QString &relativeThemePath, QWidget *parent)
    : KDialog(parent)
{
    mLocalDirectory = KStandardDirs::locateLocal("data", relativeThemePath);
    setCaption( i18n( "Manage Theme" ) );
    setButtons( Close );
    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;

    QLabel *lab = new QLabel(i18n("Local themes:"));
    lay->addWidget(lab);

    mListThemes = new QListWidget;
    mListThemes->setSelectionMode(QAbstractItemView::MultiSelection);
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
    const QSize sizeDialog = group.readEntry( "Size", QSize(300, 150) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void ManageThemes::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "ManageThemesDialog" );
    group.writeEntry( "Size", size() );
}

void ManageThemes::slotDeleteTheme()
{
    QList<QListWidgetItem *> selectItems = mListThemes->selectedItems ();
    if (!selectItems.isEmpty()) {
        if (KMessageBox::questionYesNo(this, i18np("Do you want to remove selected theme?", "Do you want to remove %1 selected themes?", selectItems.count()), i18n("Remove theme")) == KMessageBox::Yes) {
            Q_FOREACH(QListWidgetItem *item, selectItems) {
                if (KTempDir::removeDir(mLocalDirectory + QDir::separator() + item->text())) {
                    delete item;
                } else {
                    KMessageBox::error(this, i18n("Theme \"%1\" cannot be deleted. Please contact your administrator.", item->text()), i18n("Delete theme failed"));
                }
            }
        }
    }
}

void ManageThemes::initialize()
{
    QDir dir(mLocalDirectory);
    if (dir.exists()) {
        bool hasSubDir = false;
        QDirIterator dirIt( mLocalDirectory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
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
    mDeleteTheme->setEnabled(!mListThemes->selectedItems().isEmpty());
}

