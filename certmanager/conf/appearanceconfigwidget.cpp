/*
    appearanceconfigwidget.cpp

    This file is part of kleopatra, the KDE key manager
    Copyright (c) 2002,2004 Klarälvdalens Datakonsult AB
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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "appearanceconfigwidget.h"

#include <klistview.h>
#include <kconfig.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kfontdialog.h>
#include <kcolordialog.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>
#include <qpainter.h>
#include <qregexp.h>

#include <assert.h>
#include <cryptplugfactory.h>

using namespace Kleo;

class CategoryListViewItem : public QListViewItem
{
public:
 CategoryListViewItem( QListView* lv, QListViewItem* prev )
    : QListViewItem( lv, prev ) {}

  void load( const KConfigBase& config ) {
    setName( config.readEntry( "name", i18n("<unnamed>") ) );
    mForegroundColor = config.readColorEntry( "foreground-color" );
    mBackgroundColor = config.readColorEntry( "background-color" );
    mHasFont = config.hasKey( "font" );
    if ( mHasFont )
      mFont = config.readFontEntry( "font" );
    mIsExpired = config.readBoolEntry( "is-expired", false );
    // TODO support for partial font changes (italic, bold, strikeout) (see kconfigbasedkeyfilter)
  }

  void save( KConfigBase& config ) {
    config.writeEntry( "name", text( 0 ) );
    config.writeEntry( "foreground-color", mForegroundColor );
    config.writeEntry( "background-color", mBackgroundColor );
    if ( mHasFont )
      config.writeEntry( "font", mFont );
    else
      config.deleteEntry( "font" );
  }

  void setForegroundColor( const QColor& foreground ) { mForegroundColor = foreground; }
  void setBackgroundColor( const QColor& background ) { mBackgroundColor = background; }
  void setFont( const QFont& font ) { mFont = font; }

  void setDefaultAppearance() {
    mForegroundColor = mIsExpired ? Qt::red : QColor();
    mBackgroundColor = QColor();
    mHasFont = false;
    mFont = QFont();
  }

private:
  void setName( const QString& name ) {
    setText( 0, name );
  }

  void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

private:
  QColor mForegroundColor, mBackgroundColor;
  QFont mFont;
  bool mHasFont;
  bool mIsExpired; // used for default settings
};

void CategoryListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment ) {
  QColorGroup _cg = cg;
  if ( mHasFont )
    p->setFont( mFont );
  if ( mForegroundColor.isValid() )
    _cg.setColor( QColorGroup::Text, mForegroundColor );
  if ( mBackgroundColor.isValid() )
    _cg.setColor( QColorGroup::Base, mBackgroundColor );

  QListViewItem::paintCell( p, _cg, column, width, alignment );
}

////

Kleo::AppearanceConfigWidget::AppearanceConfigWidget (
  QWidget* parent,  const char* name, WFlags fl )
  : AppearanceConfigWidgetBase( parent, name, fl )
{
    categoriesLV->setSorting( -1 );
    load();
}


/*
 *  Destroys the object and frees any allocated resources
 */

AppearanceConfigWidget::~AppearanceConfigWidget()
{
  // no need to delete child widgets, Qt does it all for us
}


void AppearanceConfigWidget::slotSelectionChanged( QListViewItem* item )
{
  bool sel = item != 0;
  foregroundButton->setEnabled( sel );
  backgroundButton->setEnabled( sel );
  fontButton->setEnabled( sel );
  defaultLookPB->setEnabled( sel );
}

/*
 * set default appearance for selected category
 */
void AppearanceConfigWidget::slotDefaultClicked()
{
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  if ( !item )
    return;
  item->setDefaultAppearance();
  emit changed();
}

static const struct {
  const char* categoryName;
  // This is limited to one key (with its value)
  const char* configKey;
  const char* value;
} defaultCategoriesList[] = {
  { I18N_NOOP( "Revoked key" ), "is-revoked", "true" },
  { I18N_NOOP( "Expired key" ), "is-expired", "true" },
  { I18N_NOOP( "Disabled key" ), "is-disabled", "true" },
  { I18N_NOOP( "Can encrypt" ), "can-encrypt", "true" },
  { I18N_NOOP( "Can sign" ), "can-sign", "true" },
  { I18N_NOOP( "Can authenticate" ), "can-authenticate", "true" },
  { I18N_NOOP( "Has secret key" ), "has-secret-key", "true" },
  { I18N_NOOP( "OpenPGP key" ), "is-openpgp-key", "true" },
  { I18N_NOOP( "Key was validated" ), "was-validated", "true" }
};

QStringList AppearanceConfigWidget::createDefaultCategories( KConfig* config )
{
  QStringList groups;
  for( unsigned int i = 0; i < sizeof defaultCategoriesList / sizeof *defaultCategoriesList; ++i ) {
    const QString name = QString::fromLatin1( "Key Filter #" ) + QString::number( i );
    KConfigGroup group( config, name );
    group.writeEntry( "name", i18n( defaultCategoriesList[i].categoryName ) );
    group.writeEntry( defaultCategoriesList[i].configKey, defaultCategoriesList[i].value );
    groups << name;
  }
  kdDebug() << k_funcinfo << groups << endl;
  return groups;
}

void AppearanceConfigWidget::load()
{
  KConfig * config = Kleo::CryptPlugFactory::instance()->configObject();
  if ( !config )
    return;
  QStringList groups = config->groupList().grep( QRegExp( "^Key Filter #\\d+$" ) );
  kdDebug() << k_funcinfo << groups << endl;
  bool setDefaults = groups.isEmpty();
  if ( setDefaults )
    groups = createDefaultCategories( config );

  for ( QStringList::const_iterator it = groups.begin() ; it != groups.end() ; ++it ) {
    KConfigGroup cfg( config, *it );
    CategoryListViewItem* item = new CategoryListViewItem( categoriesLV, categoriesLV->lastItem() );
    item->load( cfg );
  }
  if ( setDefaults )
    defaults(); // set the default colors for the default categories
}

void AppearanceConfigWidget::save()
{
  KConfig * config = Kleo::CryptPlugFactory::instance()->configObject();
  if ( !config )
    return;
  // We know (assume) that the groups in the config object haven't changed,
  // so we just iterate over them and over the listviewitems, and map one-to-one.
  QStringList groups = config->groupList().grep( QRegExp( "^Key Filter #\\d+$" ) );
  if ( groups.isEmpty() ) {
    // If we created the default categories ourselves just now, then we need to make up their list
    QListViewItemIterator lvit( categoriesLV );
    for ( ; lvit.current() ; ++lvit )
      groups << lvit.current()->text( 0 );
  }

  QListViewItemIterator lvit( categoriesLV );
  for ( QStringList::const_iterator it = groups.begin() ; it != groups.end() && lvit.current(); ++it, ++lvit ) {
    CategoryListViewItem* item = static_cast<CategoryListViewItem*>(lvit.current() );
    KConfigGroup cfg( config, *it );
    item->save( cfg );
  }
}


void AppearanceConfigWidget::slotForegroundClicked() {
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  Q_ASSERT( item );
  if( !item )
    return;
  QColor fg;
  int result = KColorDialog::getColor( fg );
  if ( result == KColorDialog::Accepted ) {
    item->setForegroundColor( fg );
    item->repaint();
    emit changed();
  }
}

void AppearanceConfigWidget::slotBackgroundClicked() {
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  Q_ASSERT( item );
  if( !item )
    return;
  QColor bg;
  int result = KColorDialog::getColor( bg );
  if ( result == KColorDialog::Accepted ) {
    item->setBackgroundColor( bg );
    item->repaint();
    emit changed();
  }
}

void AppearanceConfigWidget::slotFontClicked() {
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  Q_ASSERT( item );
  if( !item )
    return;
  QFont font;
  int result = KFontDialog::getFont( font );
  if ( result == KFontDialog::Accepted ) {
    item->setFont( font );
    item->repaint();
    emit changed();
  }
}

void AppearanceConfigWidget::defaults()
{
  // This simply means "default look for every category"
  QListViewItemIterator lvit( categoriesLV );
  for ( ; lvit.current() ; ++lvit ) {
    CategoryListViewItem* item = static_cast<CategoryListViewItem *>( lvit.current() );
    item->setDefaultAppearance();
  }

}

#include "appearanceconfigwidget.moc"
