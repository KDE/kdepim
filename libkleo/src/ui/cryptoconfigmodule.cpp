/*
    cryptoconfigmodule.cpp

    This file is part of kgpgcertmanager
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "cryptoconfigmodule.h"
#include "cryptoconfigmodule_p.h"
#include "directoryserviceswidget.h"
#include "kdhorizontalline.h"
#include "filenamerequester.h"

#include "libkleo/cryptoconfig.h"

#include <klineedit.h>
#include <KLocalizedString>
#include "kleo_ui_debug.h"
#include <kiconloader.h>
#include <qicon.h>
#include <QDialogButtonBox>

#include <QSpinBox>

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRegExp>
#include <QVBoxLayout>
#include <QList>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QDesktopWidget>
#include <QCheckBox>
#include <QStyle>
#include <QComboBox>
#include <QGroupBox>

#include <cassert>
#include <memory>

using namespace Kleo;

namespace
{

class ScrollArea : public QScrollArea
{
public:
    explicit ScrollArea(QWidget *p) : QScrollArea(p) {}
    QSize sizeHint() const Q_DECL_OVERRIDE
    {
        const QSize wsz = widget() ? widget()->sizeHint() : QSize();
        return QSize(wsz.width() + style()->pixelMetric(QStyle::PM_ScrollBarExtent), QScrollArea::sizeHint().height());
    }
};

}
inline QIcon loadIcon(const QString &s)
{
    QString ss = s;
    return QIcon::fromTheme(ss.replace(QRegExp(QLatin1String("[^a-zA-Z0-9_]")), QStringLiteral("-")));
}

static unsigned int num_components_with_options(const Kleo::CryptoConfig *config)
{
    if (!config) {
        return 0;
    }
    const QStringList components = config->componentList();
    unsigned int result = 0;
    for (QStringList::const_iterator it = components.begin(); it != components.end(); ++it)
        if (const Kleo::CryptoConfigComponent *const comp = config->component(*it))
            if (!comp->groupList().empty()) {
                ++result;
            }
    return result;
}

static KPageView::FaceType determineJanusFace(const Kleo::CryptoConfig *config, Kleo::CryptoConfigModule::Layout layout, bool &ok)
{
    ok = true;
    if (num_components_with_options(config) < 2) {
        ok = false;
        return KPageView::Plain;
    }
    return
        layout == CryptoConfigModule::LinearizedLayout ? KPageView::Plain :
        layout == CryptoConfigModule::TabbedLayout     ? KPageView::Tabbed :
        /* else */                                       KPageView::List;
}

Kleo::CryptoConfigModule::CryptoConfigModule(Kleo::CryptoConfig *config, QWidget *parent)
    : KPageWidget(parent), mConfig(config)
{
    init(IconListLayout);
}

Kleo::CryptoConfigModule::CryptoConfigModule(Kleo::CryptoConfig *config, Layout layout, QWidget *parent)
    : KPageWidget(parent), mConfig(config)
{
    init(layout);
}

void Kleo::CryptoConfigModule::init(Layout layout)
{
    if (QLayout *l = this->layout()) {
        l->setMargin(0);
    }

    Kleo::CryptoConfig *const config = mConfig;

    bool configOK = false;
    const KPageView::FaceType type = determineJanusFace(config, layout, configOK);

    setFaceType(type);

    QVBoxLayout *vlay = 0;
    QWidget *vbox = 0;

    if (type == Plain) {
        QWidget *w = new QWidget(this);
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setMargin(0);
        QScrollArea *s = new QScrollArea(w);
        s->setFrameStyle(QFrame::NoFrame);
        s->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        s->setWidgetResizable(true);
        l->addWidget(s);
        vbox = new QWidget(s->viewport());
        vlay = new QVBoxLayout(vbox);
        vlay->setMargin(0);
        s->setWidget(vbox);
        addPage(w, configOK ? QString() : i18n("GpgConf Error"));
    }

    const QStringList components = config->componentList();
    for (QStringList::const_iterator it = components.begin(); it != components.end(); ++it) {
        //qCDebug(KLEO_UI_LOG) <<"Component" << (*it).toLocal8Bit() <<":";
        Kleo::CryptoConfigComponent *comp = config->component(*it);
        Q_ASSERT(comp);
        if (comp->groupList().empty()) {
            continue;
        }

        std::unique_ptr<CryptoConfigComponentGUI> compGUI(new CryptoConfigComponentGUI(this, comp));
        compGUI->setObjectName(*it);
        // KJanusWidget doesn't seem to have iterators, so we store a copy...
        mComponentGUIs.append(compGUI.get());

        if (type == Plain) {
            QGroupBox *gb = new QGroupBox(comp->description(), vbox);
            (new QVBoxLayout(gb))->addWidget(compGUI.release());
            vlay->addWidget(gb);
        } else {
            vbox = new QWidget(this);
            vlay = new QVBoxLayout(vbox);
            vlay->setMargin(0);
            KPageWidgetItem *pageItem = new KPageWidgetItem(vbox, comp->description());
            if (type != Tabbed) {
                pageItem->setIcon(loadIcon(comp->iconName()));
            }
            addPage(pageItem);

            QScrollArea *scrollArea = type == Tabbed ? new QScrollArea(vbox) : new ScrollArea(vbox);
            scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
            scrollArea->setWidgetResizable(true);

            vlay->addWidget(scrollArea);
            const QSize compGUISize = compGUI->sizeHint();
            scrollArea->setWidget(compGUI.release());

            // Set a nice startup size
            const int deskHeight = QApplication::desktop()->height();
            int dialogHeight;
            if (deskHeight > 1000) { // very big desktop ?
                dialogHeight = 800;
            } else if (deskHeight > 650) { // big desktop ?
                dialogHeight = 500;
            } else { // small (800x600, 640x480) desktop
                dialogHeight = 400;
            }
            assert(scrollArea->widget());
            if (type != Tabbed) {
                scrollArea->setMinimumHeight(qMin(compGUISize.height(), dialogHeight));
            }
        }
    }
    if (mComponentGUIs.empty()) {
        const QString msg = i18n("The gpgconf tool used to provide the information "
                                 "for this dialog does not seem to be installed "
                                 "properly. It did not return any components. "
                                 "Try running \"%1\" on the command line for more "
                                 "information.",
                                 components.empty() ? QLatin1String("gpgconf --list-components") : QLatin1String("gpgconf --list-options gpg"));
        QLabel *label = new QLabel(msg, vbox);
        label->setWordWrap(true);
        label->setMinimumHeight(fontMetrics().lineSpacing() * 5);
        vlay->addWidget(label);
    }
}

bool Kleo::CryptoConfigModule::hasError() const
{
    return mComponentGUIs.empty();
}

void Kleo::CryptoConfigModule::save()
{
    bool changed = false;
    QList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
    for (; it != mComponentGUIs.end(); ++it) {
        if ((*it)->save()) {
            changed = true;
        }
    }
    if (changed) {
        mConfig->sync(true /*runtime*/);
    }
}

void Kleo::CryptoConfigModule::reset()
{
    QList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
    for (; it != mComponentGUIs.end(); ++it) {
        (*it)->load();
    }
}

void Kleo::CryptoConfigModule::defaults()
{
    QList<CryptoConfigComponentGUI *>::Iterator it = mComponentGUIs.begin();
    for (; it != mComponentGUIs.end(); ++it) {
        (*it)->defaults();
    }
}

void Kleo::CryptoConfigModule::cancel()
{
    mConfig->clear();
}

////

Kleo::CryptoConfigComponentGUI::CryptoConfigComponentGUI(
    CryptoConfigModule *module, Kleo::CryptoConfigComponent *component,
    QWidget *parent)
    : QWidget(parent),
      mComponent(component)
{
    QGridLayout *glay = new QGridLayout(this);
    const QStringList groups = mComponent->groupList();
    if (groups.size() > 1) {
        glay->setColumnMinimumWidth(0, KDHorizontalLine::indentHint());
        for (QStringList::const_iterator it = groups.begin(), end = groups.end(); it != end; ++it) {
            Kleo::CryptoConfigGroup *group = mComponent->group(*it);
            Q_ASSERT(group);
            if (!group) {
                continue;
            }
            const QString title = group->description();
            KDHorizontalLine *hl = new KDHorizontalLine(title.isEmpty() ? *it : title, this);
            const int row = glay->rowCount();
            glay->addWidget(hl, row, 0, 1, 3);
            mGroupGUIs.append(new CryptoConfigGroupGUI(module, group, glay, this));
        }
    } else if (!groups.empty()) {
        mGroupGUIs.append(new CryptoConfigGroupGUI(module, mComponent->group(groups.front()), glay, this));
    }
    glay->setRowStretch(glay->rowCount(), 1);
}

bool Kleo::CryptoConfigComponentGUI::save()
{
    bool changed = false;
    QList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
    for (; it != mGroupGUIs.end(); ++it) {
        if ((*it)->save()) {
            changed = true;
        }
    }
    return changed;
}

void Kleo::CryptoConfigComponentGUI::load()
{
    QList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
    for (; it != mGroupGUIs.end(); ++it) {
        (*it)->load();
    }
}

void Kleo::CryptoConfigComponentGUI::defaults()
{
    QList<CryptoConfigGroupGUI *>::Iterator it = mGroupGUIs.begin();
    for (; it != mGroupGUIs.end(); ++it) {
        (*it)->defaults();
    }
}

////

Kleo::CryptoConfigGroupGUI::CryptoConfigGroupGUI(
    CryptoConfigModule *module, Kleo::CryptoConfigGroup *group,
    QGridLayout *glay, QWidget *widget)
    : QObject(module), mGroup(group)
{
    const int startRow = glay->rowCount();
    const QStringList entries = mGroup->entryList();
    for (QStringList::const_iterator it = entries.begin(), end = entries.end(); it != end; ++it) {
        Kleo::CryptoConfigEntry *entry = group->entry(*it);
        Q_ASSERT(entry);
        if (entry->level() > CryptoConfigEntry::Level_Advanced) {
            qCDebug(KLEO_UI_LOG) << "entry" << *it << "too advanced, skipping";
            continue;
        }
        CryptoConfigEntryGUI *entryGUI =
            CryptoConfigEntryGUIFactory::createEntryGUI(module, entry, *it, glay, widget);
        if (entryGUI) {
            mEntryGUIs.append(entryGUI);
            entryGUI->load();
        }
    }
    const int endRow = glay->rowCount() - 1;
    if (endRow < startRow) {
        return;
    }

    const QString iconName = group->iconName();
    if (iconName.isEmpty()) {
        return;
    }

    QLabel *l = new QLabel(widget);
    l->setPixmap(loadIcon(iconName).pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    glay->addWidget(l, startRow, 0, endRow - startRow + 1, 1, Qt::AlignTop);
}

bool Kleo::CryptoConfigGroupGUI::save()
{
    bool changed = false;
    QList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
    for (; it != mEntryGUIs.end(); ++it) {
        if ((*it)->isChanged()) {
            (*it)->save();
            changed = true;
        }
    }
    return changed;
}

void Kleo::CryptoConfigGroupGUI::load()
{
    QList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
    for (; it != mEntryGUIs.end(); ++it) {
        (*it)->load();
    }
}

void Kleo::CryptoConfigGroupGUI::defaults()
{
    QList<CryptoConfigEntryGUI *>::Iterator it = mEntryGUIs.begin();
    for (; it != mEntryGUIs.end(); ++it) {
        (*it)->resetToDefault();
    }
}

////

typedef CryptoConfigEntryGUI *(*constructor)(CryptoConfigModule *, Kleo::CryptoConfigEntry *, const QString &, QGridLayout *, QWidget *);

namespace
{
template <typename T_Widget>
CryptoConfigEntryGUI *_create(CryptoConfigModule *m, Kleo::CryptoConfigEntry *e, const QString &n, QGridLayout *l, QWidget *p)
{
    return new T_Widget(m, e, n, l, p);
}
}

static const struct WidgetsByEntryName {
    const char *entryGlob;
    constructor create;
} widgetsByEntryName[] = {
    { "*/*/debug-level",   &_create<CryptoConfigEntryDebugLevel> },
    { "gpg/*/keyserver",   &_create<CryptoConfigEntryKeyserver>  }
};
static const unsigned int numWidgetsByEntryName = sizeof widgetsByEntryName / sizeof * widgetsByEntryName;

static const constructor listWidgets[CryptoConfigEntry::NumArgType] = {
    // None: A list of options with no arguments (e.g. -v -v -v) is shown as a spinbox
    &_create<CryptoConfigEntrySpinBox>,
    0, // String
    // Int/UInt: Let people type list of numbers (1,2,3....). Untested.
    &_create<CryptoConfigEntryLineEdit>,
    &_create<CryptoConfigEntryLineEdit>,
    0, // Path
    0, // Formerly URL
    &_create<CryptoConfigEntryLDAPURL>,
    0, // DirPath
};

static const constructor scalarWidgets[CryptoConfigEntry::NumArgType] = {
    &_create<CryptoConfigEntryCheckBox>, // None
    &_create<CryptoConfigEntryLineEdit>, // String
    &_create<CryptoConfigEntrySpinBox>,  // Int
    &_create<CryptoConfigEntrySpinBox>,  // UInt
    &_create<CryptoConfigEntryPath>,     // Path
    0,                                   // Formerly URL
    0,                                   // LDAPURL
    &_create<CryptoConfigEntryDirPath>,  // DirPath
};

CryptoConfigEntryGUI *Kleo::CryptoConfigEntryGUIFactory::createEntryGUI(CryptoConfigModule *module, Kleo::CryptoConfigEntry *entry, const QString &entryName, QGridLayout *glay, QWidget *widget)
{
    assert(entry);

    // try to lookup by path:
    const QString path = entry->path();
    for (unsigned int i = 0; i < numWidgetsByEntryName; ++i)
        if (QRegExp(QLatin1String(widgetsByEntryName[i].entryGlob), Qt::CaseSensitive, QRegExp::Wildcard).exactMatch(path)) {
            return widgetsByEntryName[i].create(module, entry, entryName, glay, widget);
        }

    // none found, so look up by type:
    const unsigned int argType = entry->argType();
    assert(argType < CryptoConfigEntry::NumArgType);
    if (entry->isList())
        if (const constructor create = listWidgets[argType]) {
            return create(module, entry, entryName, glay, widget);
        } else {
            qCWarning(KLEO_UI_LOG) << "No widget implemented for list of type" << entry->argType();
        }
    else if (const constructor create = scalarWidgets[argType]) {
        return create(module, entry, entryName, glay, widget);
    } else {
        qCWarning(KLEO_UI_LOG) << "No widget implemented for type" << entry->argType();
    }

    return 0;
}

////

Kleo::CryptoConfigEntryGUI::CryptoConfigEntryGUI(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry,
    const QString &entryName)
    : QObject(module), mEntry(entry), mName(entryName), mChanged(false)
{
    connect(this, &CryptoConfigEntryGUI::changed, module, &CryptoConfigModule::changed);
}

QString Kleo::CryptoConfigEntryGUI::description() const
{
    QString descr = mEntry->description();
    if (descr.isEmpty()) { // shouldn't happen
        return QStringLiteral("<%1>").arg(mName);
    }
    if (i18nc("Translate this to 'yes' or 'no' (use the English words!) "
              "depending on whether your language uses "
              "Sentence style capitalisation in GUI labels (yes) or not (no). "
              "Context: We get some backend strings in that have the wrong "
              "capitalizaion (in English, at least) so we need to force the "
              "first character to upper-case. It is this behaviour you can "
              "control for your language with this translation.", "yes") == QLatin1String("yes")) {
        descr[0] = descr[0].toUpper();
    }
    return descr;
}

void Kleo::CryptoConfigEntryGUI::resetToDefault()
{
    mEntry->resetToDefault();
    load();
}

////

Kleo::CryptoConfigEntryLineEdit::CryptoConfigEntryLineEdit(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry, const QString &entryName,
    QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName)
{
    const int row = glay->rowCount();
    mLineEdit = new KLineEdit(widget);
    QLabel *label = new QLabel(description(), widget);
    label->setBuddy(mLineEdit);
    glay->addWidget(label, row, 1);
    glay->addWidget(mLineEdit, row, 2);
    if (entry->isReadOnly()) {
        label->setEnabled(false);
        mLineEdit->setEnabled(false);
    } else {
        connect(mLineEdit, &KLineEdit::textChanged, this, &CryptoConfigEntryLineEdit::slotChanged);
    }
}

void Kleo::CryptoConfigEntryLineEdit::doSave()
{
    mEntry->setStringValue(mLineEdit->text());
}

void Kleo::CryptoConfigEntryLineEdit::doLoad()
{
    mLineEdit->setText(mEntry->stringValue());
}

////

static const struct {
    const char *label;
    const char *name;
} debugLevels[] = {
    { I18N_NOOP("0 - None (no debugging at all)"),               "none"     },
    { I18N_NOOP("1 - Basic (some basic debug messages)"),        "basic"    },
    { I18N_NOOP("2 - Advanced (more verbose debug messages)"),   "advanced" },
    { I18N_NOOP("3 - Expert (even more detailed messages)"),     "expert"   },
    { I18N_NOOP("4 - Guru (all of the debug messages you can get)"), "guru" },
};
static const unsigned int numDebugLevels = sizeof debugLevels / sizeof * debugLevels;

Kleo::CryptoConfigEntryDebugLevel::CryptoConfigEntryDebugLevel(CryptoConfigModule *module, Kleo::CryptoConfigEntry *entry,
        const QString &entryName, QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName),
      mComboBox(new QComboBox(widget))
{
    QLabel *label = new QLabel(i18n("Set the debugging level to"), widget);
    label->setBuddy(mComboBox);

    for (unsigned int i = 0; i < numDebugLevels; ++i) {
        mComboBox->addItem(i18n(debugLevels[i].label));
    }

    if (entry->isReadOnly()) {
        label->setEnabled(false);
        mComboBox->setEnabled(false);
    } else {
        connect(mComboBox, SIGNAL(currentIndexChanged(int)), SLOT(slotChanged()));
    }

    const int row = glay->rowCount();
    glay->addWidget(label, row, 1);
    glay->addWidget(mComboBox, row, 2);
}

void Kleo::CryptoConfigEntryDebugLevel::doSave()
{
    const unsigned int idx = mComboBox->currentIndex();
    if (idx < numDebugLevels) {
        mEntry->setStringValue(QLatin1String(debugLevels[idx].name));
    } else {
        mEntry->setStringValue(QString());
    }
}

void Kleo::CryptoConfigEntryDebugLevel::doLoad()
{
    const QString str = mEntry->stringValue();
    for (unsigned int i = 0; i < numDebugLevels; ++i)
        if (str == QLatin1String(debugLevels[i].name)) {
            mComboBox->setCurrentIndex(i);
            return;
        }
    mComboBox->setCurrentIndex(0);
}

////

Kleo::CryptoConfigEntryPath::CryptoConfigEntryPath(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry, const QString &entryName,
    QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName),
      mFileNameRequester(0)
{
    const int row = glay->rowCount();
    QWidget *req;
    req = mFileNameRequester = new FileNameRequester(widget);
    mFileNameRequester->setExistingOnly(false);
    mFileNameRequester->setFilter(QDir::Files);
    QLabel *label = new QLabel(description(), widget);
    label->setBuddy(req);
    glay->addWidget(label, row, 1);
    glay->addWidget(req, row, 2);
    if (entry->isReadOnly()) {
        label->setEnabled(false);
        if (mFileNameRequester) {
            mFileNameRequester->setEnabled(false);
        }
    } else {
        if (mFileNameRequester)
            connect(mFileNameRequester, &FileNameRequester::fileNameChanged,
                    this, &CryptoConfigEntryPath::slotChanged);
    }
}

void Kleo::CryptoConfigEntryPath::doSave()
{
    mEntry->setURLValue(QUrl::fromLocalFile(mFileNameRequester->fileName()));
}

void Kleo::CryptoConfigEntryPath::doLoad()
{
    if (mEntry->urlValue().isLocalFile()) {
        mFileNameRequester->setFileName(mEntry->urlValue().toLocalFile());
    } else {
        mFileNameRequester->setFileName(mEntry->urlValue().toString());
    }
}

////

Kleo::CryptoConfigEntryDirPath::CryptoConfigEntryDirPath(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry, const QString &entryName,
    QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName),
      mFileNameRequester(0)
{
    const int row = glay->rowCount();
    QWidget *req;
    req = mFileNameRequester = new FileNameRequester(widget);
    mFileNameRequester->setExistingOnly(false);
    mFileNameRequester->setFilter(QDir::Dirs);
    QLabel *label = new QLabel(description(), widget);
    label->setBuddy(req);
    glay->addWidget(label, row, 1);
    glay->addWidget(req, row, 2);
    if (entry->isReadOnly()) {
        label->setEnabled(false);
        if (mFileNameRequester) {
            mFileNameRequester->setEnabled(false);
        }
    } else {
        if (mFileNameRequester)
            connect(mFileNameRequester, &FileNameRequester::fileNameChanged,
                    this, &CryptoConfigEntryDirPath::slotChanged);
    }
}

void Kleo::CryptoConfigEntryDirPath::doSave()
{
    mEntry->setURLValue(QUrl::fromLocalFile(mFileNameRequester->fileName()));
}

void Kleo::CryptoConfigEntryDirPath::doLoad()
{
    mFileNameRequester->setFileName(mEntry->urlValue().toLocalFile());
}

////

Kleo::CryptoConfigEntrySpinBox::CryptoConfigEntrySpinBox(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry, const QString &entryName,
    QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName)
{

    if (entry->argType() == Kleo::CryptoConfigEntry::ArgType_None && entry->isList()) {
        mKind = ListOfNone;
    } else if (entry->argType() == Kleo::CryptoConfigEntry::ArgType_UInt) {
        mKind = UInt;
    } else {
        Q_ASSERT(entry->argType() == Kleo::CryptoConfigEntry::ArgType_Int);
        mKind = Int;
    }

    const int row = glay->rowCount();
    mNumInput = new QSpinBox(widget);
    QLabel *label = new QLabel(description(), widget);
    label->setBuddy(mNumInput);
    glay->addWidget(label, row, 1);
    glay->addWidget(mNumInput, row, 2);

    if (entry->isReadOnly()) {
        label->setEnabled(false);
        mNumInput->setEnabled(false);
    } else {
        if (mKind == UInt || mKind == ListOfNone) {
            mNumInput->setMinimum(0);
        }
        connect(mNumInput, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CryptoConfigEntrySpinBox::slotChanged);
    }
}

void Kleo::CryptoConfigEntrySpinBox::doSave()
{
    int value = mNumInput->value();
    switch (mKind) {
    case ListOfNone:
        mEntry->setNumberOfTimesSet(value);
        break;
    case UInt:
        mEntry->setUIntValue(value);
        break;
    case Int:
        mEntry->setIntValue(value);
        break;
    }
}

void Kleo::CryptoConfigEntrySpinBox::doLoad()
{
    int value = 0;
    switch (mKind) {
    case ListOfNone:
        value = mEntry->numberOfTimesSet();
        break;
    case UInt:
        value = mEntry->uintValue();
        break;
    case Int:
        value = mEntry->intValue();
        break;
    }
    mNumInput->setValue(value);
}

////

Kleo::CryptoConfigEntryCheckBox::CryptoConfigEntryCheckBox(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry, const QString &entryName,
    QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName)
{
    const int row = glay->rowCount();
    mCheckBox = new QCheckBox(widget);
    glay->addWidget(mCheckBox, row, 1, 1, 2);
    mCheckBox->setText(description());
    if (entry->isReadOnly()) {
        mCheckBox->setEnabled(false);
    } else {
        connect(mCheckBox, &QCheckBox::toggled, this, &CryptoConfigEntryCheckBox::slotChanged);
    }
}

void Kleo::CryptoConfigEntryCheckBox::doSave()
{
    mEntry->setBoolValue(mCheckBox->isChecked());
}

void Kleo::CryptoConfigEntryCheckBox::doLoad()
{
    mCheckBox->setChecked(mEntry->boolValue());
}

Kleo::CryptoConfigEntryLDAPURL::CryptoConfigEntryLDAPURL(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry,
    const QString &entryName,
    QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName)
{
    mLabel = new QLabel(widget);
    mPushButton = new QPushButton(entry->isReadOnly() ? i18n("Show...") : i18n("Edit..."), widget);

    const int row = glay->rowCount();
    QLabel *label = new QLabel(description(), widget);
    label->setBuddy(mPushButton);
    glay->addWidget(label, row, 1);
    QHBoxLayout *hlay = new QHBoxLayout;
    glay->addLayout(hlay, row, 2);
    hlay->addWidget(mLabel, 1);
    hlay->addWidget(mPushButton);

    if (entry->isReadOnly()) {
        mLabel->setEnabled(false);
    }
    connect(mPushButton, &QPushButton::clicked, this, &CryptoConfigEntryLDAPURL::slotOpenDialog);
}

void Kleo::CryptoConfigEntryLDAPURL::doLoad()
{
    setURLList(mEntry->urlValueList());
}

void Kleo::CryptoConfigEntryLDAPURL::doSave()
{
    mEntry->setURLValueList(mURLList);
}

void prepareURLCfgDialog(QDialog *dialog, DirectoryServicesWidget *dirserv, bool readOnly)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    if (!readOnly) {
        buttonBox->addButton(QDialogButtonBox::Cancel);
        buttonBox->addButton(QDialogButtonBox::RestoreDefaults);

        QPushButton *defaultsBtn = buttonBox->button(QDialogButtonBox::RestoreDefaults);

        QObject::connect(defaultsBtn, &QPushButton::clicked, dirserv,
                         &DirectoryServicesWidget::clear);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    }

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(dirserv);
    layout->addWidget(buttonBox);
    dialog->setLayout(layout);
}

void Kleo::CryptoConfigEntryLDAPURL::slotOpenDialog()
{
    // I'm a bad boy and I do it all on the stack. Enough classes already :)
    // This is just a simple dialog around the directory-services-widget
    QDialog dialog(mPushButton->parentWidget());
    dialog.setWindowTitle(i18n("Configure LDAP Servers"));

    DirectoryServicesWidget *dirserv = new DirectoryServicesWidget(&dialog);

    prepareURLCfgDialog(&dialog, dirserv, mEntry->isReadOnly());

    dirserv->setX509ReadOnly(mEntry->isReadOnly());
    dirserv->setAllowedSchemes(DirectoryServicesWidget::LDAP);
    dirserv->setAllowedProtocols(DirectoryServicesWidget::X509Protocol);
    dirserv->addX509Services(mURLList);

    if (dialog.exec()) {
        setURLList(dirserv->x509Services());
        slotChanged();
    }
}

void Kleo::CryptoConfigEntryLDAPURL::setURLList(const QList<QUrl> &urlList)
{
    mURLList = urlList;
    if (mURLList.isEmpty()) {
        mLabel->setText(i18n("No server configured yet"));
    } else {
        mLabel->setText(i18np("1 server configured", "%1 servers configured", mURLList.count()));
    }
}

Kleo::CryptoConfigEntryKeyserver::CryptoConfigEntryKeyserver(
    CryptoConfigModule *module,
    Kleo::CryptoConfigEntry *entry,
    const QString &entryName,
    QGridLayout *glay, QWidget *widget)
    : CryptoConfigEntryGUI(module, entry, entryName)
{
    mLabel = new QLabel(widget);
    mPushButton = new QPushButton(i18n("Edit..."), widget);

    const int row = glay->rowCount();
    QLabel *label = new QLabel(i18n("Use keyserver at"), widget);
    label->setBuddy(mPushButton);
    glay->addWidget(label, row, 1);
    QHBoxLayout *hlay = new QHBoxLayout;
    glay->addLayout(hlay, row, 2);
    hlay->addWidget(mLabel, 1);
    hlay->addWidget(mPushButton);

    if (entry->isReadOnly()) {
        mLabel->setEnabled(false);
        mPushButton->hide();
    } else {
        connect(mPushButton, &QPushButton::clicked, this, &CryptoConfigEntryKeyserver::slotOpenDialog);
    }
}

Kleo::ParsedKeyserver Kleo::parseKeyserver(const QString &str)
{
    const QStringList list = str.split(QRegExp(QLatin1String("[\\s,]")), QString::SkipEmptyParts);
    if (list.empty()) {
        return Kleo::ParsedKeyserver();
    }
    Kleo::ParsedKeyserver result;
    result.url = list.front();
    Q_FOREACH (const QString &kvpair, list.mid(1)) {
        const int idx = kvpair.indexOf(QLatin1Char('='));
        if (idx < 0) {
            result.options.push_back(qMakePair(kvpair, QString()));     // null QString
        } else {
            const QString key = kvpair.left(idx);
            const QString value = kvpair.mid(idx + 1);
            if (value.isEmpty()) {
                result.options.push_back(qMakePair(key, QStringLiteral("")));    // make sure it's not a null QString, only an empty one
            } else {
                result.options.push_back(qMakePair(key, value));
            }
        }
    }
    return result;
}

QString Kleo::assembleKeyserver(const ParsedKeyserver &keyserver)
{
    if (keyserver.options.empty()) {
        return keyserver.url;
    }
    QString result = keyserver.url;
    typedef QPair<QString, QString> Pair;
    Q_FOREACH (const Pair &pair, keyserver.options)
        if (pair.second.isNull()) {
            result += QLatin1Char(' ') + pair.first;
        } else {
            result += QLatin1Char(' ') + pair.first + QLatin1Char('=') + pair.second;
        }
    return result;
}

void Kleo::CryptoConfigEntryKeyserver::doLoad()
{
    mParsedKeyserver = parseKeyserver(mEntry->stringValue());
    mLabel->setText(mParsedKeyserver.url);
}

void Kleo::CryptoConfigEntryKeyserver::doSave()
{
    mParsedKeyserver.url = mLabel->text();
    mEntry->setStringValue(assembleKeyserver(mParsedKeyserver));
}

static QList<QUrl> string2urls(const QString &str)
{
    QList<QUrl> ret;
    if (str.isEmpty()) {
        return ret;
    }
    ret << QUrl::fromUserInput(str);
    return ret;
}

static QString urls2string(const QList<QUrl> &urls)
{
    return urls.empty() ? QString() : urls.front().url();
}

void Kleo::CryptoConfigEntryKeyserver::slotOpenDialog()
{
    // I'm a bad boy and I do it all on the stack. Enough classes already :)
    // This is just a simple dialog around the directory-services-widget
    QDialog dialog(mPushButton->parentWidget());
    dialog.setWindowTitle(i18n("Configure Keyservers"));

    DirectoryServicesWidget *dirserv = new DirectoryServicesWidget(&dialog);

    prepareURLCfgDialog(&dialog, dirserv, mEntry->isReadOnly());

    dirserv->setOpenPGPReadOnly(mEntry->isReadOnly());
    dirserv->setAllowedSchemes(DirectoryServicesWidget::AllSchemes);
    dirserv->setAllowedProtocols(DirectoryServicesWidget::OpenPGPProtocol);
    dirserv->addOpenPGPServices(string2urls(mLabel->text()));

    if (dialog.exec()) {
        mLabel->setText(urls2string(dirserv->openPGPServices()));
        slotChanged();
    }
}

#include "moc_cryptoconfigmodule_p.cpp"
