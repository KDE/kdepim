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

#ifndef KRSS_FEEDPROPERTIESDIALOG_H
#define KRSS_FEEDPROPERTIESDIALOG_H

#include "krss/krss_export.h"

#include <KDialog>

namespace KRss {

class FeedPropertiesDialogPrivate;

class KRSS_EXPORT FeedPropertiesDialog : public KDialog
{
    Q_OBJECT

public:

    explicit FeedPropertiesDialog( QWidget *parent = 0 );
    ~FeedPropertiesDialog();

    QString feedTitle() const;
    void setFeedTitle( const QString &feedTitle );
    QString url() const;
    void setUrl( const QString &url );

    bool hasCustomFetchInterval() const;
    void setCustomFetchInterval( bool enable );
    int fetchInterval() const;
    void setFetchInterval( int minutes );

private:

    Q_DISABLE_COPY( FeedPropertiesDialog )
    FeedPropertiesDialogPrivate * const d;
};

} // namespace KRss

#endif // KRSS_FEEDPROPERTIESDIALOG_H
