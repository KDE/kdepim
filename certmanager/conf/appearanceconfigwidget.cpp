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

#include <assert.h>

using namespace Kleo;

class CategoryListViewItem : public QListViewItem
{
public:
 CategoryListViewItem( QListView* lv, QListViewItem* prev,
                     const QString& categoryName, QColor fg, QColor bg )
    : QListViewItem( lv, prev, categoryName )  {
    setName( categoryName );
    setForegroundColor( fg );
    setBackgroundColor( bg );
  }

  void setForegroundColor( QColor foreground ) { mForegroundColor = foreground; }
  void setBackgroundColor( QColor background ) { mBackgroundColor = background; }
  void setFont( QFont font ) { mFont = font; }
  void setName( const QString& name ) {
    mName = name;
    setText( 0, mName );
  }

  QString categoryName() const { return mName; }
  QColor getForegroundColor() const { return mForegroundColor; }
  QColor getBackgroundColor() const { return mBackgroundColor; }
  QFont  getFont() const { return mFont; }

  void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment );

private:
  QString mName;
  QColor mForegroundColor, mBackgroundColor;
  QFont mFont;
};

void CategoryListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment ) {
  QColorGroup _cg = cg;
   p->setFont( mFont );
  _cg.setColor( QColorGroup::Text, mForegroundColor );
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
  if( item ) {
    foregroundButton->setEnabled( true );
    backgroundButton->setEnabled( true );
    fontButton->setEnabled( true );
    removeCategoryPB->setEnabled( true );
  }
  else {
    foregroundButton->setEnabled( false );
    backgroundButton->setEnabled( false );
    fontButton->setEnabled( false );
    removeCategoryPB->setEnabled( false );
  }
}

/*
 * protected slot
 */
void AppearanceConfigWidget::slotCategorySelected( QListViewItem* item )
{
  if( item ) {
    foregroundButton->setEnabled( true );
    backgroundButton->setEnabled( true );
    fontButton->setEnabled( true );
    removeCategoryPB->setEnabled( true );
  }
  else {
    foregroundButton->setEnabled( false );
    backgroundButton->setEnabled( false );
    fontButton->setEnabled( false );
    removeCategoryPB->setEnabled( false );
  }

}



/*
 * protected slot
 */
void AppearanceConfigWidget::slotAddCategory()
{
  /*
   * pending MICHEL: We should be able to do that - create a dialog ?
   */
  /*
    AddCategoryDialog* dlg = new AddCategoryDialog( this );
    if( dlg->exec() == QDialog::Accepted ) {
        (void)new  CategoryListViewItem( categoriesLV,
	   categoriesLV->lastItem(), dlg->nameED->text() );
        emit changed();
    }
  */
}


/*
 * protected slot
 */
void AppearanceConfigWidget::slotDeleteCategory()
{
    QListViewItem* item = categoriesLV->selectedItem();
    Q_ASSERT( item );
    if( !item )
        return;
    else
        delete item;
    categoriesLV->triggerUpdate();
    slotSelectionChanged( categoriesLV->selectedItem() );
    emit changed();
}


void AppearanceConfigWidget::setInitialConfiguration( const QStringList& categories )
{
    categoriesLV->clear();
    for( QStringList::const_iterator it = categories.begin(); it != categories.end(); ++it ) {
        QString catName = *it;
        (void)new CategoryListViewItem( categoriesLV, categoriesLV->lastItem(), catName, Qt::black, Qt::white );
    }
}

QStringList AppearanceConfigWidget::categoriesList()
{
    QStringList lst;
    QListViewItemIterator it( categoriesLV );
    for ( ; it.current() ; ++it ) {
        QListViewItem* item = it.current();
        QString name = item->text( 0 );

        kdDebug() << name << endl;
        lst << name;
    }
    return lst;
}


void AppearanceConfigWidget::clear()
{
    categoriesLV->clear();
    emit changed();
}

void AppearanceConfigWidget::load()
{
  // PENDING Michel ( dummy data ) - should be read from the config file
  QStringList list;
  list.append( "Times" );
  list += "Courier";
  list += "Courier New";
  list << "Helvetica [Cronyx]" << "Helvetica [Adobe]";
  setInitialConfiguration( list );

}

void AppearanceConfigWidget::save()
{
  //PENDING Michel : save to the config file.
}


void AppearanceConfigWidget::slotForegroundClicked() {
  QColor fg;
  int result = KColorDialog::getColor( fg );
  if ( result == KColorDialog::Accepted ) {
     CategoryListViewItem* item = static_cast<CategoryListViewItem*>(categoriesLV->selectedItem() );
     Q_ASSERT( item );
     if( !item )
        return;
     else {
       item->setForegroundColor( fg );
       item->repaint();
     }
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
  }
}
void AppearanceConfigWidget::defaults()
{
  //Pending Michel : which defaults ?
}

#include "appearanceconfigwidget.moc"
