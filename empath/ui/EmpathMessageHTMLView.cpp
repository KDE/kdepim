/* Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathMessageHTMLView.h"
#endif

#include <ctype.h>

// Qt includes
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>

// Local includes
#include "EmpathMessageHTMLView.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathUtilities.h"
#include <RMM_Message.h>
#include <RMM_Enum.h>


EmpathMessageHTMLWidget::EmpathMessageHTMLWidget(QWidget * parent)
    :   QTextBrowser(parent)
{
    setFrameStyle(QFrame::NoFrame);
    QString welcomeText = i18n("Welcome to Empath");

    QString imgPath =
        KGlobal::dirs()->findResource("appdata", "empath_logo.png");

    setText("<qt bgcolor=\"white\" > 
        <center><img source=\"" + imgPath + "\" /> </qt>");

    setMimeSourceFactory(&(empath->viewFactory()));
    
//    QObject::connect(
 //       this, SIGNAL(popupMenu(QString, const QPoint &)),
  //      this, SLOT(s_popupMenu(QString, const QPoint &)));
}

EmpathMessageHTMLWidget::~EmpathMessageHTMLWidget()
{
}

    void
EmpathMessageHTMLWidget::show(const QString & xml)
{
    if (xml.isNull()) {
        empathDebug("xml is empty");
        setText(i18n("<i>No text to display</i>"));
        return;
    }
    setTextFormat(Qt::RichText);
    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
    
    QFont defaultFixed(KGlobal::fixedFont());
    
    QFont f = c->readFontEntry(UI_FIXED_FONT, &defaultFixed);

    setFont(f, true);
    setLinkColor(c->readColorEntry(UI_LINK, &DFLT_LINK));
    setLinkUnderline(c->readBoolEntry(UI_UNDERLINE_LINKS, DFLT_UNDER_LINKS));

    setText(xml);
}

    void
EmpathMessageHTMLWidget::s_popupMenu(QString s, const QPoint &)
{
    if (s.isEmpty())
        return;
    
    popup_.clear();
    
    empathDebug("URL clicked was: \"" + s + "\"");
    
    if (s.left(16) == "empath://mailto:") {
        popup_.insertItem(empathIcon("menu-compose"),
            i18n("New message to"), empath, SLOT(s_compose()));
    }
    
    if (s.left(7) == "http://"      ||
        s.left(6) == "ftp://"       ||
        s.left(8) == "https://"     ||
        s.left(9) == "gopher://")
    {
        
        popup_.insertItem(empathIcon("menu-view"), i18n("Browse"),
            parent(), SLOT(s_URLSelected()));
        
        popup_.insertItem(empathIcon("menu-view"), i18n("Bookmark"),
            parent(), SLOT(s_URLSelected()));
    }
    
    popup_.exec(QCursor::pos());
}

    QSize
EmpathMessageHTMLWidget::sizeHint() const
{
    return QSize(width(), 400);
//    return QSize(docWidth(), docHeight());
}

    QSize
EmpathMessageHTMLWidget::minimumSizeHint() const
{
    return QSize(0, 0);
//    return QSize(docWidth(), docHeight());
}

// vim:ts=4:sw=4:tw=78
