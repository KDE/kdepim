/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2005, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#include "config-kdepim.h"
#include "knoteconfigdialog.h"
#include "notes/knote.h"
#include "print/knoteprintselectthemecombobox.h"
#include "knotedisplayconfigwidget.h"
#include "knoteeditorconfigwidget.h"
#include "knotecollectionconfigwidget.h"
#include "knotesglobalconfig.h"
#include "notesharedglobalconfig.h"
#include "config/noteactionconfig.h"

#include "kdepim-version.h"
#include <QLineEdit>
#include <qapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfontrequester.h>
#include <kiconloader.h>
#include <KLocalizedString>
#include <kwindowsystem.h>
#include <QIcon>
#include <KNS3/DownloadDialog>
#include <QDialog>

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QToolButton>

#include <config/notenetworkconfig.h>

KNoteConfigDialog::KNoteConfigDialog(const QString &title,
                                     QWidget *parent)
    : KCMultiDialog(parent)
{
    setFaceType(KPageDialog::List);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    button(QDialogButtonBox::Ok)->setDefault(true);

    setWindowTitle(title);
    KWindowSystem::setIcons(winId(),
                            qApp->windowIcon().pixmap(
                                IconSize(KIconLoader::Desktop),
                                IconSize(KIconLoader::Desktop)),
                            qApp->windowIcon().pixmap(
                                IconSize(KIconLoader::Small),
                                IconSize(KIconLoader::Small)));
    addModule(QStringLiteral("knote_config_display"));
    addModule(QStringLiteral("knote_config_editor"));
    addModule(QStringLiteral("knote_config_action"));
    addModule(QStringLiteral("knote_config_network"));
    addModule(QStringLiteral("knote_config_print"));
    addModule(QStringLiteral("knote_config_collection"));
    addModule(QStringLiteral("knote_config_misc"));
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &KNoteConfigDialog::slotOk);
    connect(button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &KNoteConfigDialog::slotDefaultClicked);
}

KNoteConfigDialog::~KNoteConfigDialog()
{
}

void KNoteConfigDialog::slotOk()
{
    NoteShared::NoteSharedGlobalConfig::self()->save();
    KNotesGlobalConfig::self()->save();
}

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knote_config_display(QWidget *parent)
    {
        return new KNoteDisplayConfig(parent);
    }
}

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knote_config_collection(QWidget *parent)
    {
        return new KNoteCollectionConfig(parent);
    }
}

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knote_config_editor(QWidget *parent)
    {
        return new KNoteEditorConfig(parent);
    }
}

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knote_config_action(QWidget *parent)
    {
        return new NoteShared::NoteActionConfig(parent);
    }
}

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knote_config_network(QWidget *parent)
    {
        return new NoteShared::NoteNetworkConfig(parent);
    }
}

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knote_config_print(QWidget *parent)
    {
        return new KNotePrintConfig(parent);
    }
}

extern "C"
{
    Q_DECL_EXPORT KCModule *create_knote_config_misc(QWidget *parent)
    {
        return new KNoteMiscConfig(parent);
    }
}

KNoteDisplayConfig::KNoteDisplayConfig(QWidget *parent)
    : KCModule(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    QWidget *w =  new KNoteDisplayConfigWidget(true);
    lay->addWidget(w);
    lay->addStretch();
    addConfig(KNotesGlobalConfig::self(), w);
    load();
}

void KNoteDisplayConfig::load()
{
    KCModule::load();
}

void KNoteDisplayConfig::save()
{
    KCModule::save();
}

KNoteEditorConfig::KNoteEditorConfig(QWidget *parent)
    : KCModule(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    QWidget *w =  new KNoteEditorConfigWidget(true);
    lay->addWidget(w);
    lay->addStretch();
    addConfig(KNotesGlobalConfig::self(), w);
    load();
}

void KNoteEditorConfig::save()
{
    KCModule::save();
}

void KNoteEditorConfig::load()
{
    KCModule::load();
}

KNoteMiscConfig::KNoteMiscConfig(QWidget *parent)
    : KCModule(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    QWidget *w =  new QWidget(this);
    topLayout->addWidget(w);

    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    QCheckBox *kcfg_SystemTrayShowNotes = new QCheckBox(i18n("Show number of notes in tray icon"), this);

    kcfg_SystemTrayShowNotes->setObjectName(QStringLiteral("kcfg_SystemTrayShowNotes"));
    lay->addWidget(kcfg_SystemTrayShowNotes);

    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);
    QLabel *label_DefaultTitle = new QLabel(i18n("Default Title:"), this);
    hbox->addWidget(label_DefaultTitle);

    mDefaultTitle = new QLineEdit(this);
    label_DefaultTitle->setBuddy(mDefaultTitle);
    hbox->addWidget(mDefaultTitle);

    QLabel *howItWorks = new QLabel(i18n("<a href=\"whatsthis\">How does this work?</a>"));
    connect(howItWorks, &QLabel::linkActivated, this, &KNoteMiscConfig::slotHelpLinkClicked);
    lay->addWidget(howItWorks);
    addConfig(KNotesGlobalConfig::self(), w);
    howItWorks->setContextMenuPolicy(Qt::NoContextMenu);
    lay->addStretch();
    load();
    connect(mDefaultTitle, SIGNAL(textChanged(QString)), SLOT(changed()));
}

void KNoteMiscConfig::load()
{
    KCModule::load();
    mDefaultTitle->setText(NoteShared::NoteSharedGlobalConfig::self()->defaultTitle());
}

void KNoteMiscConfig::save()
{
    KCModule::save();
    NoteShared::NoteSharedGlobalConfig::self()->setDefaultTitle(mDefaultTitle->text());
    NoteShared::NoteSharedGlobalConfig::self()->save();
}

void KNoteMiscConfig::defaults()
{
    KCModule::defaults();
    const bool bUseDefaults = NoteShared::NoteSharedGlobalConfig::self()->useDefaults(true);
    mDefaultTitle->setText(NoteShared::NoteSharedGlobalConfig::self()->defaultTitle());
    NoteShared::NoteSharedGlobalConfig::self()->useDefaults(bUseDefaults);
}

void KNoteMiscConfig::slotHelpLinkClicked(const QString &)
{
    const QString help =
        i18n("<qt>"
             "<p>You can customize title note. "
             "You can use:</p>"
             "<ul>"
             "<li>%d current date (short format)</li>"
             "<li>%l current date (long format)</li>"
             "<li>%t current time</li>"
             "</ul>"
             "</qt>");

    QWhatsThis::showText(QCursor::pos(), help);
}

KNotePrintConfig::KNotePrintConfig(QWidget *parent)
    : KCModule(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    QWidget *w =  new QWidget(this);
    lay->addWidget(w);
    QGridLayout *layout = new QGridLayout(w);
    layout->setMargin(0);

    QLabel *label_PrintAction = new QLabel(i18n("Theme:"), this);
    layout->addWidget(label_PrintAction, 0, 0);

    mSelectTheme = new KNotePrintSelectThemeComboBox(this);
    connect(mSelectTheme, SIGNAL(activated(int)), SLOT(slotThemeChanged()));
    label_PrintAction->setBuddy(mSelectTheme);
    layout->addWidget(mSelectTheme, 0, 1);

    QToolButton *getNewTheme = new QToolButton;
    getNewTheme->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    getNewTheme->setToolTip(i18n("Download new printing themes"));
    connect(getNewTheme, &QToolButton::clicked, this, &KNotePrintConfig::slotDownloadNewThemes);
    layout->addWidget(getNewTheme, 0, 2);
    lay->addStretch();
    load();
}

void KNotePrintConfig::slotDownloadNewThemes()
{
    QPointer<KNS3::DownloadDialog> downloadThemesDialog = new KNS3::DownloadDialog(QStringLiteral("knotes_printing_theme.knsrc"));
    if (downloadThemesDialog->exec()) {
        if (!downloadThemesDialog->changedEntries().isEmpty()) {
            mSelectTheme->loadThemes();
        }
    }
    delete downloadThemesDialog;
}

void KNotePrintConfig::slotThemeChanged()
{
    Q_EMIT changed(true);
}

void KNotePrintConfig::save()
{
    KNotesGlobalConfig::self()->setTheme(mSelectTheme->selectedTheme());
}

void KNotePrintConfig::load()
{
    mSelectTheme->loadThemes();
}

void KNotePrintConfig::defaults()
{
    mSelectTheme->selectDefaultTheme();
    Q_EMIT changed(true);
}

KNoteCollectionConfig::KNoteCollectionConfig(QWidget *parent)
    : KCModule(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mCollectionConfigWidget = new KNoteCollectionConfigWidget;
    lay->addWidget(mCollectionConfigWidget);
    connect(mCollectionConfigWidget, SIGNAL(emitChanged(bool)), this, SLOT(changed()));
    setLayout(lay);
    load();
}

void KNoteCollectionConfig::save()
{
    mCollectionConfigWidget->save();
}

void KNoteCollectionConfig::load()
{
    //Nothing
}

