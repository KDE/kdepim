/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   teemu.rytilahti@kde-fi.org                                            *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef VIEWER_H
#define VIEWER_H

#include <kurl.h>
#include <khtml_part.h>

namespace Akregator
{
    class Viewer : public KHTMLPart
    {
        Q_OBJECT
        public:
            Viewer(QWidget* parent, const char* name);
	    bool closeURL();
	
        signals:
           void urlClicked(const KURL& url, bool background=false);

        protected:
           KURL m_url;

        protected slots:
           virtual bool slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
           void slotPopupMenu(KXMLGUIClient*, const QPoint&, const KURL&, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t);

           /**
            * Copies current link to clipboard.
            */
           void slotCopyToClipboard();

           /**
            * Opens link in internal viewer.
            */
           virtual void slotOpenLinkInternal();

           /**
            * Opens link in external viewer, eg. Konqueror
            */
           void slotOpenLinkExternal();

           /**
            * This changes cursor to wait cursor
            */
           void slotStarted(KIO::Job *);

           /**
            * This reverts cursor back to normal one
            */
           void slotCompleted();
           
        private:
            /**
             * Display article in external browser.
             */
            void displayInExternalBrowser(const KURL &url);
    };
}

#endif // VIEWER_H
