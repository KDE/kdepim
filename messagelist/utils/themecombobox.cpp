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
#include "messagelist/storagemodel.h"
#include "messagelist/core/manager.h"
#include "messagelist/core/theme.h"
#include "messagelist/core/settings.h"

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

QString ThemeComboBox::currentTheme() const
{
  return itemData( currentIndex() ).toString();
}

void ThemeComboBox::writeDefaultConfig() const
{
  KConfigGroup group( Settings::self()->config(), "MessageListView::StorageModelThemes" );

  const QString themeID = currentTheme();
  group.writeEntry( QLatin1String( "DefaultSet" ), themeID );

  Manager::instance()->themesConfigurationCompleted();
}

void ThemeComboBox::writeStorageModelConfig( const Akonadi::Collection &col, bool isPrivateSetting ) const
{
  if ( !col.isValid() )
    return;
  writeStorageModelConfig( QString::number( col.id() ), isPrivateSetting );
}

void ThemeComboBox::writeStorageModelConfig( MessageList::Core::StorageModel *storageModel, bool isPrivateSetting ) const
{
  writeStorageModelConfig( storageModel->id(), isPrivateSetting );
}

void ThemeComboBox::writeStorageModelConfig( const QString &id, bool isPrivateSetting )const
{
  QString themeID;
  if ( isPrivateSetting ) {
    themeID = currentTheme();
  } else { // explicitly use default theme id when using default theme.
    themeID = Manager::instance()->defaultTheme()->id();
  }
  Manager::instance()->saveThemeForStorageModel( id, themeID, isPrivateSetting );
  Manager::instance()->themesConfigurationCompleted();
}


void ThemeComboBox::readStorageModelConfig( const Akonadi::Collection& col, bool &isPrivateSetting )
{
  const Theme *theme = Manager::instance()->themeForStorageModel( col, &isPrivateSetting );
  d->setCurrentTheme( theme );

}

void ThemeComboBox::readStorageModelConfig( MessageList::Core::StorageModel *storageModel, bool &isPrivateSetting )
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

#include "themecombobox.moc"
