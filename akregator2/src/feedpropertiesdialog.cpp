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

#include "feedpropertiesdialog.h"
#include "ui_feedpropertiesdialog.h"

#include <KLocale>

using namespace KRss;

namespace KRss {

class FeedPropertiesDialogPrivate
{
public:

    Ui::FeedPropertiesDialog m_ui;
};

} // namespace KRss

FeedPropertiesDialog::FeedPropertiesDialog( QWidget *parent )
     : KDialog( parent ), d( new FeedPropertiesDialogPrivate() )
{
    QWidget *widget = new QWidget( this );
    d->m_ui.setupUi( widget );

    connect( d->m_ui.updateCheck, SIGNAL(toggled(bool)),
             d->m_ui.updateLabel, SLOT(setEnabled(bool)) );
    connect( d->m_ui.updateCheck, SIGNAL(toggled(bool)),
             d->m_ui.updateInput, SLOT(setEnabled(bool)) );

    setMainWidget( widget );
    setCaption( i18n( "Feed Properties" ) );
    setButtons( KDialog::Ok | KDialog::Cancel );
}

FeedPropertiesDialog::~FeedPropertiesDialog()
{
    delete d;
}

QString FeedPropertiesDialog::feedTitle() const
{
    return d->m_ui.feedTitleEdit->text();
}

void FeedPropertiesDialog::setFeedTitle( const QString &feedTitle )
{
    d->m_ui.feedTitleEdit->setText( feedTitle );
}

QString FeedPropertiesDialog::url() const
{
    return d->m_ui.urlEdit->text();
}

void FeedPropertiesDialog::setUrl( const QString &url )
{
    d->m_ui.urlEdit->setText( url );
}

bool FeedPropertiesDialog::hasCustomFetchInterval() const
{
    return d->m_ui.updateCheck->isChecked();
}

void FeedPropertiesDialog::setCustomFetchInterval( bool enable )
{
    d->m_ui.updateCheck->setChecked( enable );
}

int FeedPropertiesDialog::fetchInterval() const
{
    return d->m_ui.updateInput->value();
}

void FeedPropertiesDialog::setFetchInterval( int minutes )
{
    return d->m_ui.updateInput->setValue( minutes );
}

