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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "appearanceconfigwidget.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/keyfiltermanager.h>

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
#include <q3header.h>
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qcheckbox.h>

#include <assert.h>

using namespace Kleo;

class CategoryListViewItem : public Q3ListViewItem
{
public:
  CategoryListViewItem( Q3ListView* lv, Q3ListViewItem* prev, const KConfigBase& config )
    : Q3ListViewItem( lv, prev ) {

    setName( config.readEntry( "Name", i18n("<unnamed>") ) );
    mForegroundColor = config.readEntry( "foreground-color" );
    mBackgroundColor = config.readEntry( "background-color" );
    mHasFont = config.hasKey( "font" );
    if ( mHasFont ) {
      setFont( config.readFontEntry( "font" ) ); // sets mItalic and mBold
    }
    else {
      mItalic = config.readEntry( "font-italic", false );
      mBold = config.readEntry( "font-bold", false );
    }
    mStrikeOut = config.readEntry( "font-strikeout", false );
    mIsExpired = config.readEntry( "is-expired", false );
    mDirty = false;
  }

  void save( KConfigBase& config ) {
    config.writeEntry( "Name", text( 0 ) );
    config.writeEntry( "foreground-color", mForegroundColor );
    config.writeEntry( "background-color", mBackgroundColor );
    if ( mHasFont )
      config.writeEntry( "font", mFont );
    else {
      config.deleteEntry( "font" );
      config.writeEntry( "font-italic", mItalic );
      config.writeEntry( "font-bold", mBold );
    }
    config.writeEntry( "font-strikeout", mStrikeOut );
  }

  void setForegroundColor( const QColor& foreground ) { mForegroundColor = foreground; mDirty = true; }
  void setBackgroundColor( const QColor& background ) { mBackgroundColor = background; mDirty = true; }
  void setFont( const QFont& font ) {
    mFont = font;
    mHasFont = true;
    mItalic = font.italic();
    mBold = font.bold();
    mDirty = true;
  }

  QColor foregroundColor() const { return mForegroundColor; }
  QColor backgroundColor() const { return mBackgroundColor; }
  QFont font() const { return mFont; }

  void setDefaultAppearance() {
    mForegroundColor = mIsExpired ? Qt::red : QColor();
    mBackgroundColor = QColor();
    mHasFont = false;
    mFont = QFont();
    mBold = false;
    mItalic = false;
    mStrikeOut = false;
    mDirty = true;
  }

  bool isDirty() const { return mDirty; }
  bool isItalic() const { return mItalic; }
  bool isBold() const { return mBold; }
  bool isStrikeout() const { return mStrikeOut; }
  bool hasFont() const { return mHasFont; }

  void toggleItalic() { mItalic = !mItalic; if ( mHasFont ) mFont.setItalic( mItalic ); mDirty = true; }
  void toggleBold() { mBold = !mBold; if ( mHasFont ) mFont.setBold( mBold ); mDirty = true; }
  void toggleStrikeout() { mStrikeOut = !mStrikeOut; mDirty = true; }

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
  bool mItalic;
  bool mBold;
  bool mStrikeOut;
  bool mDirty;
};

void CategoryListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment ) {
  QColorGroup _cg = cg;
  QFont font = p->font();
  if ( mHasFont )
    font = mFont;
  else {
    if ( mItalic )
      font.setItalic( true );
    if ( mBold )
      font.setBold( true );
  }
  if ( mStrikeOut )
    font.setStrikeOut( true );
  p->setFont( font );

  if ( mForegroundColor.isValid() )
    _cg.setColor( QColorGroup::Text, mForegroundColor );
  if ( mBackgroundColor.isValid() )
    _cg.setColor( QColorGroup::Base, mBackgroundColor );

  Q3ListViewItem::paintCell( p, _cg, column, width, alignment );
}

////

Kleo::AppearanceConfigWidget::AppearanceConfigWidget (
  QWidget* parent,  const char* name, Qt::WFlags fl )
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


void AppearanceConfigWidget::slotSelectionChanged( Q3ListViewItem* item )
{
  bool sel = item != 0;
  foregroundButton->setEnabled( sel );
  backgroundButton->setEnabled( sel );
  fontButton->setEnabled( sel );
  italicCB->setEnabled( item );
  boldCB->setEnabled( item );
  strikeoutCB->setEnabled( item );
  defaultLookPB->setEnabled( sel );
  if ( item ) {
    CategoryListViewItem* clvi = static_cast<CategoryListViewItem *>( item );
    italicCB->setChecked( clvi->isItalic() );
    boldCB->setChecked( clvi->isBold() );
    strikeoutCB->setChecked( clvi->isStrikeout() );
  } else {
    italicCB->setChecked( false );
    boldCB->setChecked( false );
    strikeoutCB->setChecked( false );
  }
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
  item->repaint();
  slotSelectionChanged( item );
  emit changed();
}

void AppearanceConfigWidget::load()
{
  categoriesLV->clear();
  KConfig * config = Kleo::CryptoBackendFactory::instance()->configObject();
  if ( !config )
    return;
  QStringList groups = config->groupList().grep( QRegExp( "^Key Filter #\\d+$" ) );
  for ( QStringList::const_iterator it = groups.begin() ; it != groups.end() ; ++it ) {
    KConfigGroup cfg( config, *it );
    (void) new CategoryListViewItem( categoriesLV, categoriesLV->lastItem(), cfg );
  }
}

void AppearanceConfigWidget::save()
{
  KConfig * config = Kleo::CryptoBackendFactory::instance()->configObject();
  if ( !config )
    return;
  // We know (assume) that the groups in the config object haven't changed,
  // so we just iterate over them and over the listviewitems, and map one-to-one.
  QStringList groups = config->groupList().grep( QRegExp( "^Key Filter #\\d+$" ) );
  if ( groups.isEmpty() ) {
    // If we created the default categories ourselves just now, then we need to make up their list
    Q3ListViewItemIterator lvit( categoriesLV );
    for ( ; lvit.current() ; ++lvit )
      groups << lvit.current()->text( 0 );
  }

  Q3ListViewItemIterator lvit( categoriesLV );
  for ( QStringList::const_iterator it = groups.begin() ; it != groups.end() && lvit.current(); ++it, ++lvit ) {
    CategoryListViewItem* item = static_cast<CategoryListViewItem*>(lvit.current() );
    KConfigGroup cfg( config, *it );
    item->save( cfg );
  }
  config->sync();
  Kleo::KeyFilterManager::instance()->reload();
}


void AppearanceConfigWidget::slotForegroundClicked() {
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  Q_ASSERT( item );
  if( !item )
    return;
  QColor fg = item->foregroundColor();
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
  QColor bg = item->backgroundColor();
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
  QFont font = item->font();
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
  Q3ListViewItemIterator lvit( categoriesLV );
  for ( ; lvit.current() ; ++lvit ) {
    CategoryListViewItem* item = static_cast<CategoryListViewItem *>( lvit.current() );
    item->setDefaultAppearance();
    item->repaint();
  }
  emit changed();
}

void AppearanceConfigWidget::slotItalicClicked()
{
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  if ( item ) {
    item->toggleItalic();
    item->repaint();
    emit changed();
  }
}

void AppearanceConfigWidget::slotBoldClicked()
{
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  if ( item ) {
    item->toggleBold();
    item->repaint();
    emit changed();
  }
}

void AppearanceConfigWidget::slotStrikeoutClicked()
{
  CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
  if ( item ) {
    item->toggleStrikeout();
    item->repaint();
    emit changed();
  }
}

#include "appearanceconfigwidget.moc"
