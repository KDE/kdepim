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

#ifndef KRSS_NETFEEDCREATEDIALOG_H
#define KRSS_NETFEEDCREATEDIALOG_H

#include "krss/krss_export.h"
#include "ui_netfeedcreatedialog.h"

#include <KDialog>

namespace KRss {

class KRSS_EXPORT NetFeedCreateDialog : public KDialog
{
    Q_OBJECT

public:

    explicit NetFeedCreateDialog( QWidget *parent = 0 );
    ~NetFeedCreateDialog();

    void setResourceDescriptions( const QList<QPair<QString, QString> > &resourceDescriptions );

    QString url() const;
    QString resourceIdentifier() const;

    QSize sizeHint() const;

private:

    Ui::NetFeedCreateDialog ui;
};

} // namespace KRss

#endif // KRSS_NETFEEDCREATEDIALOG_H
