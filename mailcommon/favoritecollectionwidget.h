/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
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

#include <akonadi/entitylistview.h>

class KXMLGUIClient;

namespace MailCommon {

class MAILCOMMON_EXPORT FavoriteCollectionWidget : public Akonadi::EntityListView
{
  Q_OBJECT
public:
  explicit FavoriteCollectionWidget( KXMLGUIClient *xmlGuiClient, QWidget *parent = 0 );
  ~FavoriteCollectionWidget();

protected: 
  void readConfig();

protected slots:
  void slotGeneralFontChanged();
};
}

#endif /* MAILCOMMON_FAVORITECOLLECTIONWIDGET_H */

