/* Copyright 2009 James Bendig <james@imptalk.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "themecombobox.h"
#include "messagelist/utils/themecombobox.h"
#include "messagelist/utils/themecombobox_p.h"

#include "messagelist/core/manager.h"
#include "messagelist/core/theme.h"

#include <KDE/KGlobal>

using namespace MessageList::Core;
using namespace MessageList::Utils;

ThemeComboBox::ThemeComboBox( QWidget * parent )
: KComboBox( parent ), d( new ThemeComboBoxPrivate( this ) )
{
  d->slotLoadThemes();
}

ThemeComboBox::~ThemeComboBox()
{
  delete d;
}

void ThemeComboBox::writeDefaultConfig() const
{
  KConfigGroup group( KGlobal::config(), "MessageListView::StorageModelThemes" );

  const QString themeID = d->currentTheme()->id();
  group.writeEntry( QString( "DefaultSet" ), themeID );

  Manager::instance()->themesConfigurationCompleted();
}

void ThemeComboBox::writeStorageModelConfig( StorageModel *storageModel, bool isPrivateSetting ) const
{
  QString themeID;
  if ( isPrivateSetting ) {
    themeID = d->currentTheme()->id();
  } else { // explicitly use default theme id when using default theme.
    themeID = Manager::instance()->defaultTheme()->id();
  }
  Manager::instance()->saveThemeForStorageModel( storageModel, themeID, isPrivateSetting );
  Manager::instance()->themesConfigurationCompleted();
}

void ThemeComboBox::readStorageModelConfig( StorageModel *storageModel, bool &isPrivateSetting )
{
  const Theme *theme = Manager::instance()->themeForStorageModel( storageModel, &isPrivateSetting );
  d->setCurrentTheme( theme );
}

void ThemeComboBox::selectDefault()
{
  const Theme *defaultTheme = Manager::instance()->defaultTheme();
  d->setCurrentTheme( defaultTheme );
}

static bool themeNameLessThan( const Theme * lhs, const Theme * rhs )
{
  return lhs->name() < rhs->name();
}

void ThemeComboBoxPrivate::slotLoadThemes()
{
  q->clear();

  // Get all message list themes and sort them into alphabetical order.
  QList< Theme * > themes = Manager::instance()->themes().values();
  qSort( themes.begin(), themes.end(), themeNameLessThan );

  foreach( const Theme * theme, themes )
  {
    q->addItem( theme->name(), QVariant( theme->id() ) );
  }
}

void ThemeComboBoxPrivate::setCurrentTheme( const Theme *theme )
{
  Q_ASSERT( theme != 0 );

  const QString themeID = theme->id();
  const int themeIndex = q->findData( QVariant( themeID ) );
  q->setCurrentIndex( themeIndex );
}

const Theme *ThemeComboBoxPrivate::currentTheme() const
{
  const QVariant currentThemeVariant = q->itemData( q->currentIndex() );
  const QString currentThemeID = currentThemeVariant.toString();
  return Manager::instance()->theme( currentThemeID );
}

#include "themecombobox.moc"
