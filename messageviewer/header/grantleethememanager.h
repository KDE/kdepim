/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef GRANTLEETHEMEMANAGER_H
#define GRANTLEETHEMEMANAGER_H

#include "header/grantleetheme.h"

#include <QObject>
#include <QMap>
class QActionGroup;
class KXMLGUIClient;
class KActionCollection;
class KToggleAction;

namespace MessageViewer {
class GrantleeThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit GrantleeThemeManager(KXMLGUIClient *guiClient, KActionCollection *actionCollection, const QString &path, QObject *parent = 0);
    ~GrantleeThemeManager();

    void updateThemeList();


    QMap<QString, GrantleeTheme> themes() const;
    GrantleeTheme findTheme( const QString &themeName) const;

    void setXmlGuiClient( KXMLGUIClient *guiClient );
    void setActionGroup( QActionGroup *actionGroup );

    void activateTheme(const QString &themeName);

    KToggleAction *actionForHeaderStyle();

Q_SIGNALS:
    void themesChanged();
    void grantleeThemeSelected();

private:
    Q_PRIVATE_SLOT( d, void directoryChanged() )
    Q_PRIVATE_SLOT( d, void slotDownloadHeaderThemes() )
    Q_PRIVATE_SLOT( d, void slotNewStuffFinished() )
    Q_PRIVATE_SLOT( d, void slotThemeSelected() )
    class Private;
    Private *const d;
};
}
#endif // GRANTLEETHEMEMANAGER_H
