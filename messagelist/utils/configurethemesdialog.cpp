/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "utils/configurethemesdialog.h"
#include "utils/configurethemesdialog_p.h"

#include "utils/themeeditor.h"
#include "core/theme.h"

#include "core/manager.h"

#include <QGridLayout>
#include <QPushButton>
#include <QFrame>
#include <QHash>

#include <KLocale>
#include <KIconLoader>
#include <KMessageBox>

namespace MessageList
{

namespace Utils
{

class ThemeListWidgetItem : public QListWidgetItem
{
public:
  ThemeListWidgetItem( QListWidget * par, const Core::Theme &set )
    : QListWidgetItem( set.name(), par )
  {
    mTheme = new Core::Theme( set );
  }
  ~ThemeListWidgetItem()
  {
    delete mTheme;
  }

  Core::Theme * theme() const
  {
    return mTheme;
  }
  void forgetTheme()
  {
    mTheme = 0;
  }
private:
  Core::Theme * mTheme;
};

class ThemeListWidget : public QListWidget
{
public:
  ThemeListWidget( QWidget * parent )
    : QListWidget( parent )
    {}
public:
  // need a larger but shorter QListWidget
  QSize sizeHint() const
    { return QSize( 450, 128 ); }
};

} // namespace Utils

} // namespace MessageList

using namespace MessageList::Core;
using namespace MessageList::Utils;

ConfigureThemesDialog::ConfigureThemesDialog( QWidget *parent )
  : KDialog( parent ), d( new Private( this ) )
{
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowModality( Qt::ApplicationModal ); // FIXME: Sure ?
  setButtons( Ok | Cancel );
  setWindowTitle( i18n( "Customize Themes" ) );

  QWidget * base = new QWidget( this );
  setMainWidget( base );

  QGridLayout * g = new QGridLayout( base );

  d->mThemeList = new ThemeListWidget( base );
  d->mThemeList->setSortingEnabled( true );
  g->addWidget( d->mThemeList, 0, 0, 5, 1 );

  connect( d->mThemeList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
           SLOT(themeListCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)) );

  d->mNewThemeButton = new QPushButton( i18n( "New Theme" ), base );
  d->mNewThemeButton->setIcon( KIcon( QLatin1String( "document-new" ) ) );
  d->mNewThemeButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  g->addWidget( d->mNewThemeButton, 0, 1 );

  connect( d->mNewThemeButton, SIGNAL(clicked()),
           SLOT(newThemeButtonClicked()) );

  d->mCloneThemeButton = new QPushButton( i18n( "Clone Theme" ), base );
  d->mCloneThemeButton->setIcon( KIcon( QLatin1String( "edit-copy" ) ) );
  d->mCloneThemeButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  g->addWidget( d->mCloneThemeButton, 1, 1 );

  connect( d->mCloneThemeButton, SIGNAL(clicked()),
           SLOT(cloneThemeButtonClicked()) );

  QFrame * f = new QFrame( base );
  f->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  f->setMinimumHeight( 24 );
  g->addWidget( f, 2, 1, Qt::AlignVCenter );

  d->mDeleteThemeButton = new QPushButton( i18n( "Delete Theme" ), base );
  d->mDeleteThemeButton->setIcon( KIcon( QLatin1String( "edit-delete" ) ) );
  d->mDeleteThemeButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  g->addWidget( d->mDeleteThemeButton, 3, 1 );

  connect( d->mDeleteThemeButton, SIGNAL(clicked()),
           SLOT(deleteThemeButtonClicked()) );

  d->mEditor = new ThemeEditor( base );
  g->addWidget( d->mEditor, 5, 0, 1, 2 );

  connect( d->mEditor, SIGNAL(themeNameChanged()),
           SLOT(editedThemeNameChanged()) );

  g->setColumnStretch( 0, 1 );
  g->setRowStretch( 4, 1 );

  connect( this, SIGNAL(okClicked()),
           SLOT(okButtonClicked()) );

  d->fillThemeList();
}

ConfigureThemesDialog::~ConfigureThemesDialog()
{
  delete d;
}

void ConfigureThemesDialog::selectTheme( const QString &themeId )
{
  ThemeListWidgetItem * item = d->findThemeItemById( themeId );
  if ( item )
    d->mThemeList->setCurrentItem( item );
}

void ConfigureThemesDialog::Private::okButtonClicked()
{
  commitEditor();

  Manager::instance()->removeAllThemes();

  const int c = mThemeList->count();
  int i = 0;
  while ( i < c )
  {
    ThemeListWidgetItem * item = dynamic_cast< ThemeListWidgetItem * >( mThemeList->item( i ) );
    if ( item )
    {
      Manager::instance()->addTheme( item->theme() );
      item->forgetTheme();
    }
    i++;
  }

  Manager::instance()->themesConfigurationCompleted();

  q->close(); // this will delete too
}

void ConfigureThemesDialog::Private::commitEditor()
{
  Theme * editedTheme = mEditor->editedTheme();
  if ( !editedTheme )
    return;

  mEditor->commit();

  ThemeListWidgetItem * editedItem = findThemeItemByTheme( editedTheme );
  if ( editedItem )
    return;

  // We must reset the runtime column state as the columns might have
  // totally changed in the editor
  editedTheme->resetColumnState();

  QString goodName = uniqueNameForTheme( editedTheme->name(), editedTheme );
  editedTheme->setName( goodName );
  editedItem->setText( goodName );
}

void ConfigureThemesDialog::Private::editedThemeNameChanged()
{
  Theme * set = mEditor->editedTheme();
  if ( !set )
    return;

  ThemeListWidgetItem * it = findThemeItemByTheme( set );
  if ( !it )
    return;

  QString goodName = uniqueNameForTheme( set->name(), set );

  it->setText( goodName );
}

void ConfigureThemesDialog::Private::fillThemeList()
{
  const QHash< QString, Theme * >& sets = Manager::instance()->themes();

  QHash< QString, Theme * >::ConstIterator end( sets.constEnd() );
  for( QHash< QString, Theme * >::ConstIterator it = sets.constBegin(); it != end; ++it )
    (void)new ThemeListWidgetItem( mThemeList, *( *it ) );
}


void ConfigureThemesDialog::Private::themeListCurrentItemChanged( QListWidgetItem * cur, QListWidgetItem * )
{
  commitEditor();

  ThemeListWidgetItem * item = cur ? dynamic_cast< ThemeListWidgetItem * >( cur ) : 0;
  mDeleteThemeButton->setEnabled( item && !item->theme()->readOnly() );
  mCloneThemeButton->setEnabled( item );
  mEditor->editTheme( item ? item->theme() : 0 );

  if ( item && !item->isSelected() )
    item->setSelected( true ); // make sure it's true
}

ThemeListWidgetItem * ConfigureThemesDialog::Private::findThemeItemById( const QString &themeId )
{
  const int c = mThemeList->count();
  int i = 0;
  while ( i < c )
  {
    ThemeListWidgetItem * item = dynamic_cast< ThemeListWidgetItem * >( mThemeList->item( i ) );
    if ( item )
    {
      if ( item->theme()->id() == themeId )
        return item;
    }
    i++;
  }
  return 0;
}


ThemeListWidgetItem * ConfigureThemesDialog::Private::findThemeItemByName( const QString &name, Theme * skipTheme )
{
  const int c = mThemeList->count();
  int i = 0;
  while ( i < c )
  {
    ThemeListWidgetItem * item = dynamic_cast< ThemeListWidgetItem * >( mThemeList->item( i ) );
    if ( item )
    {
      if ( item->theme() != skipTheme )
      {
        if ( item->theme()->name() == name )
          return item;
      }
    }
    i++;
  }
  return 0;
}

ThemeListWidgetItem * ConfigureThemesDialog::Private::findThemeItemByTheme( Theme * set )
{
  const int c = mThemeList->count();
  int i = 0;
  while ( i < c )
  {
    ThemeListWidgetItem * item = dynamic_cast< ThemeListWidgetItem * >( mThemeList->item( i ) );
    if ( item )
    {
      if ( item->theme() == set )
        return item;
    }
    i++;
  }
  return 0;
}


QString ConfigureThemesDialog::Private::uniqueNameForTheme( QString baseName, Theme * skipTheme )
{
  QString ret = baseName;
  if( ret.isEmpty() )
    ret = i18n( "Unnamed Theme" );

  int idx = 1;

  ThemeListWidgetItem * item = findThemeItemByName( ret, skipTheme );
  while ( item )
  {
    idx++;
    ret = QString::fromLatin1( "%1 %2" ).arg( baseName ).arg( idx );
    item = findThemeItemByName( ret, skipTheme );
  }
  return ret;
}

void ConfigureThemesDialog::Private::newThemeButtonClicked()
{
  Theme emptyTheme;
  emptyTheme.setName( uniqueNameForTheme( i18n( "New Theme" ) ) );
  Theme::Column * col = new Theme::Column();
  col->setLabel( i18n( "New Column" ) );
  col->setVisibleByDefault( true );
  col->addMessageRow( new Theme::Row() );
  col->addGroupHeaderRow( new Theme::Row() );
  emptyTheme.addColumn( col );
  ThemeListWidgetItem * item = new ThemeListWidgetItem( mThemeList, emptyTheme );

  mThemeList->setCurrentItem( item );
  mDeleteThemeButton->setEnabled( item && !item->theme()->readOnly() );

}

void ConfigureThemesDialog::Private::cloneThemeButtonClicked()
{
  ThemeListWidgetItem * item = dynamic_cast< ThemeListWidgetItem * >( mThemeList->currentItem() );
  if ( !item )
    return;

  item->setSelected(false);
  Theme copyTheme( *( item->theme() ) );
  copyTheme.setReadOnly( false );
  copyTheme.detach(); // detach shared data
  copyTheme.generateUniqueId(); // regenerate id so it becomes different
  copyTheme.setName( uniqueNameForTheme( item->theme()->name() ) );
  item = new ThemeListWidgetItem( mThemeList, copyTheme );

  mThemeList->setCurrentItem( item );
  mDeleteThemeButton->setEnabled( item && !item->theme()->readOnly() );

}

void ConfigureThemesDialog::Private::deleteThemeButtonClicked()
{
  ThemeListWidgetItem * item = dynamic_cast< ThemeListWidgetItem * >( mThemeList->currentItem() );
  if ( !item )
    return;
  if ( mThemeList->count() < 2 )
    return; // no way: desperately try to keep at least one option set alive :)

  mEditor->editTheme( 0 ); // forget it

  delete item; // this will trigger themeListCurrentItemChanged()

  mDeleteThemeButton->setEnabled( item && !item->theme()->readOnly() );
}

#include "configurethemesdialog.moc"
