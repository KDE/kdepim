/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "config-kdepim.h"
#include "knotesimpleconfigdialog.h"
#include "knoteconfigdialog.h"
#include "knotedisplayconfigwidget.h"
#include "knoteeditorconfigwidget.h"

#include "attributes/notelockattribute.h"

#include <KLocalizedString>
#include <KWindowSystem>
#include <KIconLoader>

#include <QTabWidget>
#include <QApplication>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <attributes/notedisplayattribute.h>

KNoteSimpleConfigDialog::KNoteSimpleConfigDialog(const QString &title,
        QWidget *parent)
    : QDialog(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KNoteSimpleConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KNoteSimpleConfigDialog::reject);
    okButton->setDefault(true);

    setWindowTitle(title);
    KWindowSystem::setIcons(winId(),
                            qApp->windowIcon().pixmap(
                                IconSize(KIconLoader::Desktop),
                                IconSize(KIconLoader::Desktop)),
                            qApp->windowIcon().pixmap(
                                IconSize(KIconLoader::Small),
                                IconSize(KIconLoader::Small)));
    mTabWidget = new QTabWidget;

    mEditorConfigWidget = new KNoteEditorConfigWidget(true, this);
    mTabWidget->addTab(mEditorConfigWidget, i18n("Editor Settings"));

    mDisplayConfigWidget = new KNoteDisplayConfigWidget(true, this);
    mTabWidget->addTab(mDisplayConfigWidget, i18n("Display Settings"));

    mainLayout->addWidget(mTabWidget);
    mainLayout->addWidget(buttonBox);

    readConfig();
}

KNoteSimpleConfigDialog::~KNoteSimpleConfigDialog()
{
    writeConfig();
}

void KNoteSimpleConfigDialog::load(Akonadi::Item &item, bool isRichText)
{
    NoteShared::NoteDisplayAttribute *attr = item.attribute<NoteShared::NoteDisplayAttribute>();
    mEditorConfigWidget->load(attr, isRichText);
    mDisplayConfigWidget->load(attr);
}

void KNoteSimpleConfigDialog::slotUpdateCaption(const QString &name)
{
    setWindowTitle(name);
}

void KNoteSimpleConfigDialog::save(Akonadi::Item &item, bool &isRichText)
{
    NoteShared::NoteDisplayAttribute *attr =  item.attribute<NoteShared::NoteDisplayAttribute>(Akonadi::Item::AddIfMissing);
    mEditorConfigWidget->save(attr, isRichText);
    mDisplayConfigWidget->save(attr);
}

void KNoteSimpleConfigDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "KNoteSimpleConfigDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void KNoteSimpleConfigDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "KNoteSimpleConfigDialog");
    group.writeEntry("Size", size());
    group.sync();
}
