/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef _AKREGATORPART_H_
#define _AKREGATORPART_H_

#include <kparts/browserextension.h>
#include <kparts/part.h>
#include <kurl.h>

#include "config.h"

#include "akregator_partiface.h"

class QDomDocument;
class QTimer;

class KAboutData;
class KConfig;
class KURL;
class KParts::BrowserExtension;

namespace Akregator
{
    namespace Backend
    {
        class Storage;
    }

    typedef KParts::ReadOnlyPart MyBasePart;

    class ActionManagerImpl;
    class View;
    class Part;
    class Feed;
    class Article;
    class TrayIcon;

    class BrowserExtension : public KParts::BrowserExtension
    {
        Q_OBJECT

        public:
            BrowserExtension(Part *p, const char *name );
        public slots:
            void saveSettings();
        private:
            Part *m_part;
    };

    /**
     This is a RSS Aggregator "Part". It does all the real work.
     It is also embeddable into other applications (e.g. for use in Kontact).
     */
    class Part : public MyBasePart, virtual public AkregatorPartIface
    {
        Q_OBJECT
        public:
           typedef MyBasePart inherited;

            /** Default constructor.*/
            Part(QWidget *parentWidget, const char *widgetName,
                          QObject *parent, const char *name, const QStringList&);

	        /** Destructor. */
            virtual ~Part();

            /** Create KAboutData for this KPart. */
            static KAboutData *createAboutData();

            /**
             Opens feedlist
             @param url URL to feedlist
             */
            virtual bool openURL(const KURL& url);

            /** Opens standard feedlist */
            virtual void openStandardFeedList();

            virtual void fetchFeedUrl(const QString&);

            /** Fetch all feeds in the feed tree */
            virtual void fetchAllFeeds();

            /**
             Add a feed to a group.
             @param urls The URL(s) of the feed(s) to add.
             @param group The name of the folder into which the feed is added.
             If the group does not exist, it is created.  The feed is added as the last member
             of the group.
             */
            virtual void addFeedsToGroup(const QStringList& urls, const QString& group);

            virtual void addFeed();

            /**
             This method is called when this app is restored.  The KConfig
             object points to the session management config file that was saved
             with @ref saveProperties
             Calls AkregatorView's saveProperties.
             */
            virtual void readProperties(KConfig* config);

            /** This method is called when it is time for the app to save its
             properties for session management purposes.
             Calls AkregatorView's readProperties. */
            virtual void saveProperties(KConfig* config);

            /** merges a nested part's GUI into the gui of this part
            @return true iff merging was successful, i.e. the GUI factory was not NULL */
            virtual bool mergePart(KParts::Part*);

            void loadTagSet(const QString& path);
            void saveTagSet(const QString& path);

        public slots:
            /** Used to save settings after changing them from configuration dialog. Calls AkregatorPart's saveSettings. */
            virtual void saveSettings();

            /** Saves the standard feed list to it's default location */
            void slotSaveFeedList();

            void fileImport();
            void fileExport();
            void fileGetFeeds();

            void fileSendLink() { fileSendArticle(); }
            void fileSendFile() { fileSendArticle(true); }
            void fileSendArticle(bool attach=false);

            /** Shows configuration dialog */
            void showOptions();
            void showKNotifyOptions();

        signals:
            void showPart();
            void signalSettingsChanged();


        protected:

        /** @return Whether the tray icon is enabled or not */
            virtual bool isTrayIconEnabled() const;

            /** loads all Akregator plugins */
            void loadPlugins();

            /** This must be implemented by each part */
            virtual bool openFile();

            void importFile(const KURL& url);
            void exportFile(const KURL& url);

            /** FIXME: hack to get the tray icon working */
            QWidget* getMainWindow();

            virtual KParts::Part *hitTest(QWidget *widget, const QPoint &globalPos);

            /** reimplemented to load/unload the merged parts on selection/deselection */
            virtual void partActivateEvent(KParts::PartActivateEvent* event);

        protected slots:
            void slotOnShutdown();
            void slotSettingsChanged();

        private: // methods

            bool copyFile(const QString& backup);

            /** fills the font settings with system fonts, if fonts are not set */
            void initFonts();

            /** creates an OPML file containing the initial feeds (KDE feeds) */
            static QDomDocument createDefaultFeedList();

            bool tryToLock(const QString& backendName);

         private: // attributes

            class ApplyFiltersInterceptor;
            ApplyFiltersInterceptor* m_applyFiltersInterceptor;
            QString m_standardFeedList;
            QString m_tagSetPath;
            bool m_standardListLoaded;
            bool m_shuttingDown;

            KParts::BrowserExtension *m_extension;
            KParts::Part* m_mergedPart;
            View* m_view;

            QTimer* m_autosaveTimer;
            /** did we backup the feed list already? */
            bool m_backedUpList;
            Backend::Storage* m_storage;
            ActionManagerImpl* m_actionManager;
    };
}

#endif // _AKREGATORPART_H_

// vim: set et ts=4 sts=4 sw=4:
