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

// Qt includes
#include <qpopupmenu.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstddirs.h>
#include <kiconloader.h>

// Local includes
#include "EmpathMessageTextViewWidget.h"

EmpathMessageTextViewWidget::EmpathMessageTextViewWidget(QWidget * parent)
    :   QTextBrowser(parent)
{
    qDebug("Text view ctor");
    setFrameStyle(QFrame::NoFrame);

//    QString welcomeText = i18n("Welcome to Empath");

//    QString imgPath = KGlobal::dirs()->findResource("appdata", "empath_logo.png");

    setText("<qt bgcolor=\"white\"><center>Empath message viewer</center></qt>");

    popup_ = new QPopupMenu(this);

#if 0
    setMimeSourceFactory(&(empath->viewFactory()));
    
    QObject::connect(
        this, SIGNAL(popupMenu(QString, const QPoint &)),
        this, SLOT(s_popupMenu(QString, const QPoint &)));
#endif
    qDebug("Text view ctor done");
}

EmpathMessageTextViewWidget::~EmpathMessageTextViewWidget()
{
}

    void
EmpathMessageTextViewWidget::setXML(const QString & xml)
{
    qDebug("text view -> setText");
    if (xml.isEmpty()) {
        setText(i18n("<i>No text to display</i>"));
        return;
    }

    setTextFormat(Qt::RichText);
    KConfig * c(KGlobal::config());

    c->setGroup("EmpathMessageTextViewWidget");
    
    QFont defaultFixed(KGlobalSettings::fixedFont());
    
    QFont f = c->readFontEntry("Font", &defaultFixed);

    setFont(f, true);

    QColor defaultLinkColour = Qt::red;

    setLinkColor(c->readColorEntry("LinkColour", &defaultLinkColour));
    setLinkUnderline(c->readBoolEntry("UnderlineLinks", true));

    setText(xml);
}

    void
EmpathMessageTextViewWidget::s_popupMenu(QString s, const QPoint &)
{
    if (s.isEmpty())
        return;
    
    popup_->clear();
    
    if (s.left(16) == "mailto:") {

        popup_->insertItem(BarIcon("menu-compose"),
            i18n("New message to"), this, SLOT(s_compose()));

    } else {
    
        if (s.left(7) == "http://"      ||
            s.left(6) == "ftp://"       ||
            s.left(8) == "https://"     ||
            s.left(9) == "gopher://")
        {
            
            popup_->insertItem(BarIcon("menu-view"), i18n("Browse"),
                parent(), SLOT(s_URLSelected()));
            
            popup_->insertItem(BarIcon("menu-view"), i18n("Bookmark"),
                parent(), SLOT(s_URLSelected()));
        }
    }
    
    popup_->exec(QCursor::pos());
}

    QSize
EmpathMessageTextViewWidget::sizeHint() const
{
    return QSize(width(), 400);
//    return QSize(docWidth(), docHeight());
}

    QSize
EmpathMessageTextViewWidget::minimumSizeHint() const
{
    return QSize(0, 0);
//    return QSize(docWidth(), docHeight());
}

// vim:ts=4:sw=4:tw=78
