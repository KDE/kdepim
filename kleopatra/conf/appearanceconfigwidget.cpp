/*
    appearanceconfigwidget.cpp

    This file is part of kleopatra, the KDE key manager
    Copyright (c) 2002,2004,2008 Klarälvdalens Datakonsult AB
    Copyright (c) 2002,2003 Marc Mutz <mutz@kde.org>

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

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

#include <config-kleopatra.h>

#include "appearanceconfigwidget.h"
#include "ui_appearanceconfigwidget.h"


#include "tooltippreferences.h"

#include "libkleo/kleo/cryptobackendfactory.h"
#include "libkleo/kleo/keyfiltermanager.h"
#include "libkleo/kleo/dn.h"
#include "libkleo/ui/dnattributeorderconfigwidget.h"

#ifdef KDEPIM_ONLY_KLEO
# include <utils/kleo_kicondialog.h>
#else
# include <kicondialog.h>
#endif

#include <kconfig.h>
#include <klocale.h>
#include <kconfiggroup.h>

#include <QColor>
#include <QFont>
#include <QString>
#include <QRegExp>
#include <QApplication>
#include <QColorDialog>
#include <QFontDialog>

#include <boost/range.hpp>
#include <boost/bind.hpp>

#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace Kleo::Config;
using namespace boost;
#ifdef KDEPIM_ONLY_KLEO
using namespace Kleo::KioAvoidance;
#endif

enum {
    HasNameRole = Qt::UserRole + 0x1234, /*!< Records that the user has assigned a name (to avoid comparing with i18n-strings) */
    HasFontRole,                         /*!< Records that the user has chosen  completely different font (as opposed to italic/bold/strikeout) */
    IconNameRole,                        /*!< Records the name of the icon (since QIcon won't give it out again, once set) */
    MayChangeNameRole,
    MayChangeForegroundRole,
    MayChangeBackgroundRole,
    MayChangeFontRole,
    MayChangeItalicRole,
    MayChangeBoldRole,
    MayChangeStrikeOutRole,
    MayChangeIconRole,

    EndDummy
};

static QFont tryToFindFontFor( const QListWidgetItem * item ) {
    if ( item )
        if ( const QListWidget * const lw = item->listWidget() )
            return lw->font();
    return QApplication::font( "QListWidget" );
}   

static bool is( const QListWidgetItem * item, bool (QFont::*func)() const ) {
    if ( !item )
        return false;
    const QVariant v = item->data( Qt::FontRole );
    if ( !v.isValid() || v.type() != QVariant::Font )
        return false;
    return (v.value<QFont>().*func)();
}

static bool is_italic( const QListWidgetItem * item ) {
    return is( item, &QFont::italic );
}
static bool is_bold( const QListWidgetItem * item ) {
    return is( item, &QFont::bold );
}
static bool is_strikeout( const QListWidgetItem * item ) {
    return is( item, &QFont::strikeOut );
}


static void set( QListWidgetItem * item, bool on, void (QFont::*func)(bool) ) {
    if ( !item )
        return;
    const QVariant v = item->data( Qt::FontRole );
    QFont font = v.isValid() && v.type() == QVariant::Font ? v.value<QFont>() : tryToFindFontFor( item ) ;
    (font.*func)( on );
    item->setData( Qt::FontRole, font );
}

static void set_italic( QListWidgetItem * item, bool on ) {
    set( item, on, &QFont::setItalic );
}
static void set_bold( QListWidgetItem * item, bool on ) {
    set( item, on, &QFont::setBold );
}
static void set_strikeout( QListWidgetItem * item, bool on ) {
    set( item, on, &QFont::setStrikeOut );
}


static void apply_config( const KConfigGroup & group, QListWidgetItem * item ) {
    if ( !item )
        return;

    const QString name = group.readEntry( "Name" );
    item->setText( name.isEmpty() ? i18nc("Key filter without user-assigned name", "<unnamed>") : name );
    item->setData( HasNameRole, !name.isEmpty() );
    item->setData( MayChangeNameRole, !group.isEntryImmutable( "Name" ) );

    const QColor fg = group.readEntry( "foreground-color", QColor() );
    item->setData( Qt::ForegroundRole, fg.isValid() ? QBrush( fg ) : QVariant() );
    item->setData( MayChangeForegroundRole, !group.isEntryImmutable( "foreground-color" ) );

    const QColor bg = group.readEntry( "background-color", QColor() );
    item->setData( Qt::BackgroundRole, bg.isValid() ? QBrush( bg ) : QVariant() );
    item->setData( MayChangeBackgroundRole, !group.isEntryImmutable( "background-color" ) );

    const QFont defaultFont = tryToFindFontFor( item );
    if ( group.hasKey( "font" ) ) {
        const QFont font = group.readEntry( "font", defaultFont );
        item->setData( Qt::FontRole, font != defaultFont ? font : QVariant() );
        item->setData( HasFontRole,  font != defaultFont );
    } else {
        QFont font = defaultFont;
        font.setStrikeOut( group.readEntry( "font-strikeout", false ) );
        font.setItalic( group.readEntry( "font-italic", false ) );
        font.setBold( group.readEntry( "font-bold", false ) );
        item->setData( Qt::FontRole, font );
        item->setData( HasFontRole, false );
    }
    item->setData( MayChangeFontRole, !group.isEntryImmutable( "font" ) );
    item->setData( MayChangeItalicRole, !group.isEntryImmutable( "font-italic" ) );
    item->setData( MayChangeBoldRole, !group.isEntryImmutable( "font-bold" ) );
    item->setData( MayChangeStrikeOutRole, !group.isEntryImmutable( "font-strikeout" ) );

    const QString iconName = group.readEntry( "icon" );
    item->setData( Qt::DecorationRole, iconName.isEmpty() ? QVariant() : KIcon( iconName ) );
    item->setData( IconNameRole, iconName.isEmpty() ? QVariant() : iconName );
    item->setData( MayChangeIconRole, !group.isEntryImmutable( "icon" ) );
}

static void erase_if_allowed( QListWidgetItem * item, int role, int allowRole ) {
    if ( item && item->data( allowRole ).toBool() )
        item->setData( role, QVariant() );
}

#if 0
static void erase_if_allowed( QListWidgetItem * item, const int role[], size_t numRoles, int allowRole ) {
    if ( item && item->data( allowRole ).toBool() )
        for ( unsigned int i = 0 ; i < numRoles ; ++i )
            item->setData( role[i], QVariant() );
}

static void erase_if_allowed( QListWidgetItem * item, int role, const int allowRole[], size_t numAllowRoles ) {
    if ( !item )
        return;
    for ( unsigned int i = 0 ; i < numAllowRoles ; ++i )
        if ( !item->data( allowRole[i] ).toBool() )
            return;
    item->setData( role, QVariant() );
}
#endif

static void erase_if_allowed( QListWidgetItem * item, const int role[], size_t numRoles, const int allowRole[], size_t numAllowRoles ) {
    if ( !item )
        return;
    for ( unsigned int i = 0 ; i < numAllowRoles ; ++i )
        if ( !item->data( allowRole[i] ).toBool() )
            return;
    for ( unsigned int i = 0 ; i < numRoles ; ++i )
        item->setData( role[i], QVariant() );
}

static void set_default_appearance( QListWidgetItem * item ) {
    if ( !item )
        return;
    erase_if_allowed( item, Qt::ForegroundRole, MayChangeForegroundRole );
    erase_if_allowed( item, Qt::BackgroundRole, MayChangeBackgroundRole );
    erase_if_allowed( item, Qt::DecorationRole, MayChangeIconRole );
    static const int fontRoles[] = { Qt::FontRole, HasFontRole };
    static const int fontAllowRoles[] = {
        MayChangeFontRole,
        MayChangeItalicRole,
        MayChangeBoldRole,
        MayChangeStrikeOutRole,
    };
    erase_if_allowed( item, fontRoles, size( fontRoles ), fontAllowRoles, size( fontAllowRoles ) );
}

static void writeOrDelete( KConfigGroup & group, const char * key, const QVariant & value ) {
    if ( value.isValid() )
        group.writeEntry( key, value );
    else
        group.deleteEntry( key );
}

static QVariant brush2color( const QVariant & v ) {
    if ( v.isValid() ) {
        if ( v.type() == QVariant::Color )
            return v;
        else if ( v.type() == QVariant::Brush )
            return v.value<QBrush>().color();
    }
    return QVariant();
}

static void save_to_config( const QListWidgetItem * item, KConfigGroup & group ) {
    if ( !item )
        return;
    writeOrDelete( group, "Name", item->data( HasNameRole ).toBool() ? item->text() : QVariant() );
    writeOrDelete( group, "foreground-color", brush2color( item->data( Qt::ForegroundRole ) ) );
    writeOrDelete( group, "background-color", brush2color( item->data( Qt::BackgroundRole ) ) );
    writeOrDelete( group, "icon", item->data( IconNameRole ) );

    group.deleteEntry( "font" );
    group.deleteEntry( "font-strikeout" );
    group.deleteEntry( "font-italic" );
    group.deleteEntry( "font-bold" );

    if ( item->data( HasFontRole ).toBool() ) {
        writeOrDelete( group, "font", item->data( Qt::FontRole ) );
        return;
    }

    if ( is_strikeout( item ) )
        group.writeEntry( "font-strikeout", true );
    if ( is_italic( item ) )
        group.writeEntry( "font-italic", true );
    if ( is_bold( item ) )
        group.writeEntry( "font-bold", true );
}

static void kiosk_enable( QWidget * w, const QListWidgetItem * item, int allowRole ) {
    if ( !w )
        return;
    if ( item && !item->data( allowRole ).toBool() ) {
        w->setEnabled( false );
        w->setToolTip( i18n( "This parameter has been locked down by the system administrator." ) );
    } else {
        w->setEnabled( item );
        w->setToolTip( QString() );
    }
}







class AppearanceConfigWidget::Private : public Ui_AppearanceConfigWidget {
    friend class ::Kleo::Config::AppearanceConfigWidget;
    AppearanceConfigWidget * const q;
public:
    explicit Private( AppearanceConfigWidget * qq )
        : Ui_AppearanceConfigWidget(),
          q( qq ),
          dnOrderWidget( 0 )
    {
        setupUi( q );

        if ( QLayout * const l = q->layout() )
            l->setMargin( 0 );

        QWidget * w = new QWidget;
        dnOrderWidget = Kleo::DNAttributeMapper::instance()->configWidget( w );
        dnOrderWidget->setObjectName( QLatin1String( "dnOrderWidget" ) );
        ( new QVBoxLayout( w ) )->addWidget( dnOrderWidget );

        tabWidget->addTab( w, i18n("DN-Attribute Order") );

        connect( dnOrderWidget, SIGNAL(changed()), q, SIGNAL(changed()) );

        connect( iconButton, SIGNAL(clicked()), q, SLOT(slotIconClicked()) );
#ifndef QT_NO_COLORDIALOG
        connect( foregroundButton, SIGNAL(clicked()), q, SLOT(slotForegroundClicked()) );
        connect( backgroundButton, SIGNAL(clicked()), q, SLOT(slotBackgroundClicked()) );
#else
        foregroundButton->hide();
        backgroundButton->hide();
#endif
#ifndef QT_NO_FONTDIALOG
        connect( fontButton, SIGNAL(clicked()), q, SLOT(slotFontClicked()) );
#else
        fontButton->hide();
#endif
        connect( categoriesLV, SIGNAL(itemSelectionChanged()), q, SLOT(slotSelectionChanged()) );
        connect( defaultLookPB, SIGNAL(clicked()), q, SLOT(slotDefaultClicked()) );
        connect( italicCB, SIGNAL(toggled(bool)), q, SLOT(slotItalicToggled(bool)) );
        connect( boldCB, SIGNAL(toggled(bool)), q, SLOT(slotBoldToggled(bool)) );
        connect( strikeoutCB, SIGNAL(toggled(bool)), q, SLOT(slotStrikeOutToggled(bool)) );
        connect( tooltipValidityCheckBox, SIGNAL(toggled(bool)), q, SLOT(slotTooltipValidityChanged(bool)) );
        connect( tooltipOwnerCheckBox, SIGNAL(toggled(bool)), q, SLOT(slotTooltipOwnerChanged(bool)) );
        connect( tooltipDetailsCheckBox, SIGNAL(toggled(bool)), q, SLOT(slotTooltipDetailsChanged(bool)) );
    }

private:
    void enableDisableActions( QListWidgetItem * item );
    QListWidgetItem * selectedItem() const;

private:
    void slotIconClicked();
#ifndef QT_NO_COLORDIALOG
    void slotForegroundClicked();
    void slotBackgroundClicked();
#endif
#ifndef QT_NO_FONTDIALOG
    void slotFontClicked();
#endif
    void slotSelectionChanged();
    void slotDefaultClicked();
    void slotItalicToggled(bool);
    void slotBoldToggled(bool);
    void slotStrikeOutToggled(bool);
    void slotTooltipValidityChanged(bool);
    void slotTooltipOwnerChanged(bool);
    void slotTooltipDetailsChanged(bool);

private:
    Kleo::DNAttributeOrderConfigWidget * dnOrderWidget;
};

AppearanceConfigWidget::AppearanceConfigWidget( QWidget * p, Qt::WindowFlags f )
    : QWidget( p, f ), d( new Private( this ) )
{
//    load();
}


AppearanceConfigWidget::~AppearanceConfigWidget() {}

void AppearanceConfigWidget::Private::slotSelectionChanged() {
    enableDisableActions( selectedItem() );
}

QListWidgetItem * AppearanceConfigWidget::Private::selectedItem() const {
    const QList<QListWidgetItem*> items = categoriesLV->selectedItems();
    return items.empty() ? 0 : items.front() ;
}

void AppearanceConfigWidget::Private::enableDisableActions( QListWidgetItem * item ) {
    kiosk_enable( iconButton, item, MayChangeIconRole );
#ifndef QT_NO_COLORDIALOG
    kiosk_enable( foregroundButton, item, MayChangeForegroundRole );
    kiosk_enable( backgroundButton, item, MayChangeBackgroundRole );
#endif
#ifndef QT_NO_FONTDIALOG
    kiosk_enable( fontButton, item, MayChangeFontRole );
#endif
    kiosk_enable( italicCB, item, MayChangeItalicRole );
    kiosk_enable( boldCB, item, MayChangeBoldRole );
    kiosk_enable( strikeoutCB, item, MayChangeStrikeOutRole );

    defaultLookPB->setEnabled( item );

    italicCB->setChecked( is_italic( item ) );
    boldCB->setChecked( is_bold( item ) );
    strikeoutCB->setChecked( is_strikeout( item ) );
}

void AppearanceConfigWidget::Private::slotDefaultClicked() {

    QListWidgetItem * const item = selectedItem();
    if ( !item )
        return;

    set_default_appearance( item );
    enableDisableActions( item );

    emit q->changed();
}

void AppearanceConfigWidget::defaults() {

    // This simply means "default look for every category"
    for ( int i = 0, end = d->categoriesLV->count() ; i != end ; ++i )
        set_default_appearance( d->categoriesLV->item( i ) );
    d->tooltipValidityCheckBox->setChecked( true );
    d->tooltipOwnerCheckBox->setChecked( false );
    d->tooltipDetailsCheckBox->setChecked( false );

    d->dnOrderWidget->defaults();

    emit changed();
}

void AppearanceConfigWidget::load() {

    d->dnOrderWidget->load();

    d->categoriesLV->clear();
    KConfig * const config = CryptoBackendFactory::instance()->configObject();
    if ( !config )
        return;
    const QStringList groups = config->groupList().filter( QRegExp( QLatin1String("^Key Filter #\\d+$") ) );
    Q_FOREACH( const QString & group, groups ) {
        //QListWidgetItem * item = new QListWidgetItem( d->categoriesLV );
        apply_config( KConfigGroup( config, group ), new QListWidgetItem( d->categoriesLV ) );
    }

    const TooltipPreferences prefs;
    d->tooltipValidityCheckBox->setChecked( prefs.showValidity() );
    d->tooltipOwnerCheckBox->setChecked( prefs.showOwnerInformation() );
    d->tooltipDetailsCheckBox->setChecked( prefs.showCertificateDetails() );
}

void AppearanceConfigWidget::save() {

    d->dnOrderWidget->save();

    TooltipPreferences prefs;
    prefs.setShowValidity( d->tooltipValidityCheckBox->isChecked() );
    prefs.setShowOwnerInformation( d->tooltipOwnerCheckBox->isChecked() );
    prefs.setShowCertificateDetails( d->tooltipDetailsCheckBox->isChecked() );
    prefs.writeConfig();

    KConfig * const config = CryptoBackendFactory::instance()->configObject();
    if ( !config )
        return;
    // We know (assume) that the groups in the config object haven't changed,
    // so we just iterate over them and over the listviewitems, and map one-to-one.
    const QStringList groups = config->groupList().filter( QRegExp( QLatin1String("^Key Filter #\\d+$") ) );
#if 0
    if ( groups.isEmpty() ) {
        // If we created the default categories ourselves just now, then we need to make up their list
        Q3ListViewItemIterator lvit( categoriesLV );
        for ( ; lvit.current() ; ++lvit )
            groups << lvit.current()->text( 0 );
    }
#endif
    for ( int i = 0, end = std::min( groups.size(), d->categoriesLV->count() ) ; i != end ; ++i ) {
        const QListWidgetItem * const item = d->categoriesLV->item( i );
        assert( item );
        KConfigGroup group( config, groups[i] );
        save_to_config( item, group );
    }

    config->sync();
    KeyFilterManager::instance()->reload();
}

void AppearanceConfigWidget::Private::slotIconClicked() {
    QListWidgetItem * const item = selectedItem();
    if ( !item )
        return;

    const QString iconName = KIconDialog::getIcon( /* repeating default arguments begin */
                                                  KIconLoader::Desktop, KIconLoader::Application, false, 0, false,
                                                  /* repeating default arguments end */
                                                  q );
    if ( iconName.isEmpty() )
        return;

    item->setIcon( KIcon( iconName ) );
    item->setData( IconNameRole, iconName );
    emit q->changed();
}

#ifndef QT_NO_COLORDIALOG
void AppearanceConfigWidget::Private::slotForegroundClicked() {
    QListWidgetItem * const item = selectedItem();
    if ( !item )
        return;

    const QVariant v = brush2color( item->data( Qt::ForegroundRole ) );

    const QColor initial = v.isValid() ? v.value<QColor>() : categoriesLV->palette().color( QPalette::Normal, QPalette::Text );
    const QColor c = QColorDialog::getColor( initial, q );

    if ( c.isValid() ) {
        item->setData( Qt::ForegroundRole, QBrush( c ) );
        emit q->changed();
    }
}

void AppearanceConfigWidget::Private::slotBackgroundClicked() {
    QListWidgetItem * const item = selectedItem();
    if ( !item )
        return;

    const QVariant v = brush2color( item->data( Qt::BackgroundRole ) );

    const QColor initial = v.isValid() ? v.value<QColor>() : categoriesLV->palette().color( QPalette::Normal, QPalette::Base );
    const QColor c = QColorDialog::getColor( initial, q );

    if ( c.isValid() ) {
        item->setData( Qt::BackgroundRole, QBrush( c ) );
        emit q->changed();
    }
}
#endif // QT_NO_COLORDIALOG

#ifndef QT_NO_FONTDIALOG
void AppearanceConfigWidget::Private::slotFontClicked() {
    QListWidgetItem * const item = selectedItem();
    if ( !item )
        return;

    const QVariant v = item->data( Qt::FontRole );

    bool ok = false;
    const QFont defaultFont = tryToFindFontFor( item );
    const QFont initial = v.isValid() && v.type() == QVariant::Font ? v.value<QFont>() : defaultFont ;
    QFont f = QFontDialog::getFont( &ok, initial, q );
    if ( !ok )
        return;

    // disallow circumventing KIOSK:
    if ( !item->data( MayChangeItalicRole ).toBool() )
        f.setItalic( initial.italic() );
    if ( !item->data( MayChangeBoldRole ).toBool() )
        f.setBold( initial.bold() );
    if ( !item->data( MayChangeStrikeOutRole ).toBool() )
        f.setStrikeOut( initial.strikeOut() );

    item->setData( Qt::FontRole, f != defaultFont ? f : QVariant() );
    item->setData( HasFontRole, true );
    emit q->changed();
}
#endif // QT_NO_FONTDIALOG

void AppearanceConfigWidget::Private::slotItalicToggled( bool on ) {
    set_italic( selectedItem(), on );
    emit q->changed();
}

void AppearanceConfigWidget::Private::slotBoldToggled( bool on ) {
    set_bold( selectedItem(), on );
    emit q->changed();
}

void AppearanceConfigWidget::Private::slotStrikeOutToggled( bool on ) {
    set_strikeout( selectedItem(), on );
    emit q->changed();
}

void AppearanceConfigWidget::Private::slotTooltipValidityChanged( bool )
{
    emit q->changed();
}

void AppearanceConfigWidget::Private::slotTooltipOwnerChanged( bool )
{
    emit q->changed();
}

void AppearanceConfigWidget::Private::slotTooltipDetailsChanged( bool )
{
    emit q->changed();
}


#include "moc_appearanceconfigwidget.cpp"
