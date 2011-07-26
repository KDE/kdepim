/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#include "netfeedcreatedialog.h"

#include <KLocale>

#include <QtCore/QListIterator>

using namespace KRss;

NetFeedCreateDialog::NetFeedCreateDialog( QWidget *parent )
    : KDialog( parent )
{
    QWidget *widget = new QWidget( this );
    ui.setupUi( widget );

    setMainWidget( widget );
    setCaption( i18n( "New feed" ) );
    setButtons( KDialog::Ok | KDialog::Cancel );
}

NetFeedCreateDialog::~NetFeedCreateDialog()
{
}

void NetFeedCreateDialog::setResourceDescriptions( const QList<QPair<QString, QString> > &resourceDescriptions )
{
    // for some unknown reason Q_FOREACH doesn't compile
    QListIterator<QPair<QString, QString> > it_description( resourceDescriptions );
    while ( it_description.hasNext() ) {
        QPair<QString, QString> description = it_description.next();
        ui.resourceCombo->addItem( description.second, description.first );
    }
}

QString NetFeedCreateDialog::url() const
{
    return ui.urlEdit->text();
}

QString NetFeedCreateDialog::resourceIdentifier() const
{
    return ui.resourceCombo->itemData( ui.resourceCombo->currentIndex() ).toString();
}

QSize NetFeedCreateDialog::sizeHint() const
{
    return QSize( 342, 72 );
}

#include "netfeedcreatedialog.moc"
