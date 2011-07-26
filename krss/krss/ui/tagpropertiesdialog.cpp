/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tagpropertiesdialog.h"
#include "ui_tagpropertieswidget.h"

#include <KLocale>

using namespace KRss;

namespace KRss {

class TagPropertiesDialogPrivate
{
public:

    Ui::TagPropertiesWidget m_ui;
};

} // namespace KRss

TagPropertiesDialog::TagPropertiesDialog( QWidget *parent )
     : KDialog( parent ), d( new TagPropertiesDialogPrivate() )
{
    QWidget *widget = new QWidget( this );
    d->m_ui.setupUi( widget );
    setMainWidget( widget );
    setCaption( i18n( "Tag Properties" ) );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setInitialSize( QSize( 350, 180 ) );
}

TagPropertiesDialog::~TagPropertiesDialog()
{
    delete d;
}

QString TagPropertiesDialog::label() const
{
    return d->m_ui.labelEdit->text();
}

void TagPropertiesDialog::setLabel( const QString& label )
{
    d->m_ui.labelEdit->setText( label );
}

QString TagPropertiesDialog::description() const
{
    return d->m_ui.descriptionEdit->text();
}

void TagPropertiesDialog::setDescription( const QString& description )
{
    d->m_ui.descriptionEdit->setText( description );
}

QString TagPropertiesDialog::icon() const
{
    return QString();
}

void TagPropertiesDialog::setIcon( const QString& icon )
{
    Q_UNUSED( icon )
}

#include "tagpropertiesdialog.moc"
