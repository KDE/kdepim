/*
    This file is part of Akregator2.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR2_PART_H
#define AKREGATOR2_PART_H

#include <kurl.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>
#include <krss/item.h>
#include <boost/shared_ptr.hpp>

class KConfigGroup;
class KUrl;
class KCMultiDialog;

class QDomDocument;
class QTimer;

namespace Akregator2 {

class ActionManagerImpl;
class Feed;
class FeedList;
class MainWidget;
class Part;
class TrayIcon;

class BrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

    public:
        explicit BrowserExtension( Part *p );
    public slots:
        void saveSettings();
    private:
        Part *m_part;
};

/**
    This is a RSS Aggregator "Part". It does all the real work.
    It is also embeddable into other applications (e.g. for use in Kontact).
    */
class Part : public KParts::ReadOnlyPart
{
    Q_OBJECT
    public:
        typedef KParts::ReadOnlyPart inherited;

        /** Default constructor.*/
        Part(QWidget *parentWidget, QObject *parent, const QVariantList&);

	   /** Destructor. */
        ~Part();

        /**
            Opens feedlist
            @param url URL to feedlist
            */
        bool openUrl(const KUrl& url);

        /** Opens standard feedlist */
        void openStandardFeedList();

        void addFeed();
        /** Fetch all feeds in the feed tree */
        void fetchAllFeeds();

        /**
            This method is called when this app is restored.  The KConfig
            object points to the session management config file that was saved
            with @ref saveProperties
            Calls Akregator2 MainWidget's readProperties.
            */
        virtual void readProperties(const KConfigGroup & config);

        /** This method is called when it is time for the app to save its
            properties for session management purposes.
            Calls Akregator2 MainWidget's saveProperties. */
        virtual void saveProperties(KConfigGroup & config);

        /**
            Add a feed to a group.
            @param url The URL of the feed to add.
            @param group The name of the folder into which the feed is added.
            If the group does not exist, it is created.  The feed is added as the last member
            of the group.
            */
        void addFeedsToGroup(const QStringList& urls, const QString& group);


        bool handleCommandLine();

    public slots:
        /** Used to save settings after changing them from configuration dialog. Calls Akregator2Part's saveSettings. */
        void saveSettings();

        /** Shows configuration dialog */
        void showOptions();
        void showNotificationOptions();

    signals:
        void signalSettingsChanged();
        void signalArticlesSelected(const QList<KRss::Item>&);

    private:

    /** @return Whether the tray icon is enabled or not */
        bool isTrayIconEnabled() const;

        /** loads all plugins of a given type
         * @param type The category of plugins to load, currently one of "storage" and "extension"
         */
        void loadPlugins( const QString& type );

        /** This must be implemented by each part */
        bool openFile();

        KParts::Part *hitTest(QWidget *widget, const QPoint &globalPos);

    private slots:
        void slotStarted();

        void slotOnShutdown();
        void slotSettingsChanged();
        void slotSetStatusText( const QString &statusText );

        void slotAkonadiSetUp( KJob* job );

    private: // methods

        /** fills the font settings with system fonts, if fonts are not set */
        void initFonts();

    private: // attributes

        bool m_shuttingDown;

        QPointer<KParts::BrowserExtension> m_extension;
        QPointer<QWidget> m_parentWidget;
        QPointer<ActionManagerImpl> m_actionManager;
        QPointer<MainWidget> m_mainWidget;
        QPointer<KCMultiDialog> m_dialog;
};

} // namespace Akregator2

#endif // AKREGATOR2_PART_H
