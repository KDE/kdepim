/*

  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "favoritecollectionwidget.h"
#include "kernel/mailkernel.h"
#include "mailcommonsettings_base.h"

#include <messagecore/settings/globalsettings.h>

#include <KLocalizedString>
#include <KXMLGUIClient>
#include <KActionMenu>
#include <KActionCollection>
#include <AkonadiWidgets/CollectionStatisticsDelegate>

#include <QPainter>
#include <QFontDatabase>

using namespace MailCommon;

class Q_DECL_HIDDEN FavoriteCollectionWidget::Private
{
public:
    Private()
        : listMode(0),
          iconMode(0)
    {
    }
    QColor textColor;
    QAction *listMode;
    QAction *iconMode;
};

FavoriteCollectionWidget::FavoriteCollectionWidget(KXMLGUIClient *xmlGuiClient, QWidget *parent)
    : Akonadi::EntityListView(xmlGuiClient, parent), d(new Private)
{
    setFocusPolicy(Qt::NoFocus);

    Akonadi::CollectionStatisticsDelegate *delegate = new Akonadi::CollectionStatisticsDelegate(this);
    delegate->setProgressAnimationEnabled(true);

    setItemDelegate(delegate);

    delegate->setUnreadCountShown(true);

    readConfig();

    createMenu(xmlGuiClient->actionCollection());
}

FavoriteCollectionWidget::~FavoriteCollectionWidget()
{
    delete d;
}

void FavoriteCollectionWidget::updateMode()
{
    switch (viewMode()) {
    case ListMode:
        d->listMode->setChecked(true);
        d->iconMode->setChecked(false);
        break;
    case IconMode:
        d->listMode->setChecked(false);
        d->iconMode->setChecked(true);
        break;
    }
}

void FavoriteCollectionWidget::createMenu(KActionCollection *ac)
{
    KActionMenu *iconSizeMenu  = new KActionMenu(i18n("Icon size"), this);
    ac->addAction(QStringLiteral("favorite_icon_size"), iconSizeMenu);

    static const int icon_sizes[] = { 16, 22, 32 /*, 48, 64, 128 */ };

    QActionGroup *grp = new QActionGroup(iconSizeMenu);
    const int nbElement((int)(sizeof(icon_sizes) / sizeof(int)));
    QAction *act = Q_NULLPTR;
    for (int i = 0; i < nbElement; ++i) {
        act = new QAction(QStringLiteral("%1x%2").arg(icon_sizes[ i ]).arg(icon_sizes[ i ]), iconSizeMenu);
        iconSizeMenu->addAction(act);
        act->setCheckable(true);
        grp->addAction(act);
        if (iconSize().width() == icon_sizes[ i ]) {
            act->setChecked(true);
        }
        act->setData(QVariant(icon_sizes[ i ]));
        connect(act, &QAction::triggered, this, &FavoriteCollectionWidget::slotChangeIconSize);
    }

    KActionMenu *modeFavoriteMenu = new KActionMenu(i18n("Mode"), this);
    ac->addAction(QStringLiteral("favorite_mode"), modeFavoriteMenu);

    grp = new QActionGroup(modeFavoriteMenu);
    d->listMode = new QAction(i18n("List Mode"), modeFavoriteMenu);
    modeFavoriteMenu->addAction(d->listMode);
    d->listMode->setCheckable(true);
    grp->addAction(d->listMode);
    if (viewMode() ==  ListMode) {
        d->listMode->setChecked(true);
    }
    d->listMode->setData(QVariant(MailCommon::MailCommonSettings::EnumFavoriteCollectionViewMode::ListMode));
    connect(d->listMode, &QAction::triggered, this, &FavoriteCollectionWidget::slotChangeMode);

    d->iconMode = new QAction(i18n("Icon Mode"), modeFavoriteMenu);
    modeFavoriteMenu->addAction(d->iconMode);
    grp->addAction(d->iconMode);
    d->iconMode->setCheckable(true);
    if (viewMode() == IconMode) {
        d->iconMode->setChecked(true);
    }
    d->iconMode->setData(QVariant(MailCommon::MailCommonSettings::EnumFavoriteCollectionViewMode::IconMode));
    connect(d->iconMode, &QAction::triggered, this, &FavoriteCollectionWidget::slotChangeMode);
}

void FavoriteCollectionWidget::slotChangeMode(bool)
{
    QAction *act = dynamic_cast< QAction * >(sender());
    if (!act) {
        return;
    }

    QVariant data = act->data();

    bool ok;
    const int mode = data.toInt(&ok);
    if (!ok) {
        return;
    }

    switch (mode) {
    case MailCommon::MailCommonSettings::EnumFavoriteCollectionViewMode::IconMode:
        changeViewMode(IconMode);
        break;
    case MailCommon::MailCommonSettings::EnumFavoriteCollectionViewMode::ListMode:
        changeViewMode(ListMode);
        break;
    }

    MailCommon::MailCommonSettings::self()->setFavoriteCollectionViewMode(mode);
    MailCommon::MailCommonSettings::self()->save();
}

void FavoriteCollectionWidget::changeViewMode(QListView::ViewMode mode)
{
    setViewMode(mode);
    setDragEnabled(true);
    setAcceptDrops(true);
}

void FavoriteCollectionWidget::slotChangeIconSize(bool)
{
    QAction *act = dynamic_cast< QAction * >(sender());
    if (!act) {
        return;
    }

    QVariant data = act->data();

    bool ok;
    const int size = data.toInt(&ok);
    if (!ok) {
        return;
    }

    const QSize newIconSize(QSize(size, size));
    if (newIconSize == iconSize()) {
        return;
    }
    setIconSize(newIconSize);
    MailCommon::MailCommonSettings::self()->setIconSize(iconSize().width());
    MailCommon::MailCommonSettings::self()->save();
}

void FavoriteCollectionWidget::slotGeneralPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha(128);
    d->textColor = color;
}

void FavoriteCollectionWidget::slotGeneralFontChanged()
{
    // Custom/System font support
    if (MessageCore::GlobalSettings::self()->useDefaultFonts()) {
        setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    }
}

void FavoriteCollectionWidget::readConfig()
{
    // Custom/System font support
    if (!MessageCore::GlobalSettings::self()->useDefaultFonts()) {
        KConfigGroup fontConfig(KernelIf->config(), "Fonts");
        setFont(fontConfig.readEntry("folder-font", QFontDatabase::systemFont(QFontDatabase::GeneralFont)));
    } else {
        setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    }

    int iIconSize = MailCommon::MailCommonSettings::self()->iconSize();
    if (iIconSize < 16 || iIconSize > 32) {
        iIconSize = 22;
    }
    setIconSize(QSize(iIconSize, iIconSize));
}

void FavoriteCollectionWidget::paintEvent(QPaintEvent *event)
{
    if (!model() || model()->rowCount() == 0) {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        if (!d->textColor.isValid()) {
            slotGeneralPaletteChanged();
        }
        p.setPen(d->textColor);

        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, i18n("Drop your favorite folders here..."));
    } else {
        Akonadi::EntityListView::paintEvent(event);
    }
}

