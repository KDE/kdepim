/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef MAILCOMMON_FAVORITECOLLECTIONWIDGET_H
#define MAILCOMMON_FAVORITECOLLECTIONWIDGET_H

#include "mailcommon_export.h"

#include <EntityListView>

class KXMLGUIClient;
class KActionCollection;

namespace MailCommon {

class MAILCOMMON_EXPORT FavoriteCollectionWidget : public Akonadi::EntityListView
{
    Q_OBJECT
public:
    explicit FavoriteCollectionWidget( KXMLGUIClient *xmlGuiClient, QWidget *parent = 0 );
    ~FavoriteCollectionWidget();

    void readConfig();
    void updateMode();

    void changeViewMode(QListView::ViewMode mode);

protected slots:
    void slotGeneralFontChanged();
    void slotGeneralPaletteChanged();
    void slotChangeIconSize(bool);
    void slotChangeMode(bool);

protected:
    void paintEvent(QPaintEvent*);

private:
    void createMenu(KActionCollection *ac);
    class Private;
    Private *const d;
};

}

#endif /* MAILCOMMON_FAVORITECOLLECTIONWIDGET_H */

