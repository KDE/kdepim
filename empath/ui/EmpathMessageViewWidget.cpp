/*
    Empath - Mailer for KDE
    
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
#pragma implementation "EmpathMessageViewWidget.h"
#endif

// Qt includes
#include <qmessagebox.h>
#include <qcstring.h>
#include <qimage.h>

// KDE includes
#include <klocale.h>
#include <krun.h>

// Local includes
#include "EmpathConfig.h"
#include "EmpathMessageStructureWidget.h"
#include "EmpathMessageHTMLView.h"
#include "EmpathHeaderViewWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathComposeWindow.h"
#include "EmpathAttachmentViewWidget.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
#include <RMM_Message.h>
#include <RMM_BodyPart.h>
#include <RMM_ContentType.h>

EmpathMessageViewWidget::EmpathMessageViewWidget
    (const EmpathURL & url, QWidget *parent)
    :   QWidget(parent, "MessageViewWidget"),
        url_(url),
        viewingSource_(false)
{
    structureWidget_ =
        new EmpathMessageStructureWidget(0, "structureWidget");
    
    QVBoxLayout * layout = new QVBoxLayout(this);
    
    layout->setAutoAdd(true);
    
    headerViewWidget_ =
        new EmpathHeaderViewWidget(this, "headerViewWidget");
    
    messageWidget_ = new EmpathMessageHTMLWidget(this);
    
    attachmentViewWidget_ = new EmpathAttachmentViewWidget(this);
    
    QObject::connect(
        headerViewWidget_,  SIGNAL(clipClicked()),
        this,               SLOT(s_clipClicked()));
    
    QObject::connect(
        structureWidget_,    SIGNAL(partChanged(RMM::RBodyPart)),
        this,                SLOT(s_partChanged(RMM::RBodyPart)));

    QObject::connect(
        structureWidget_,    SIGNAL(showText(const QString &)),
        this,                SLOT(s_showText(const QString &)));

    layout->activate();
    show();    
}

    void
EmpathMessageViewWidget::s_retrieveJobFinished(EmpathRetrieveJob j)
{
    if (!j.success())
        return;

    url_ = j.url();

    RMM::RMessage m(j.message());
    
    if (!m) {
        empathDebug("Couldn't get supposedly retrieved message from job");
        return;
    }

    KConfig * config(KGlobal::config());

    using namespace EmpathConfig;

    config->setGroup(GROUP_DISPLAY);

    QColor quote1(config->readColorEntry(UI_QUOTE_ONE, DFLT_Q_1));
    QColor quote2(config->readColorEntry(UI_QUOTE_TWO, DFLT_Q_2));

    RMM::RBodyPart message(m);

    structureWidget_->setMessage(message);

    headerViewWidget_->useEnvelope(message.envelope());

    QString s;
    
    if (message.body().count() == 0) {
        empathDebug("Message body count is 0");
        attachmentViewWidget_->hide();
        s = QString::fromUtf8(message.asXML(quote1, quote2));
        messageWidget_->show(s);
        return;
    }
    
    else if (message.body().count() == 1) {
        empathDebug("Message body count is 1");
        attachmentViewWidget_->hide();
        s = QString::fromUtf8(message.body().at(0)->asXML(quote1, quote2));
        messageWidget_->show(s);
        return;
    }
    
    else {
        
        empathDebug("===================== MULTIPART ====================");
        
        attachmentViewWidget_->setMessage(message);
        attachmentViewWidget_->show();
        
        QList<RMM::RBodyPart> body(message.body());
        QListIterator<RMM::RBodyPart> it(body);
        
        int i = 0;
        for (; it.current(); ++it) {
        
            ++i;
        
            empathDebug(
                " ===================== PART # "
                + QString().setNum(i) +
                " =====================");

            if (it.current()->envelope().has(RMM::HeaderContentType)) {
                
                empathDebug("Ok this part has a Content-Type");                            
                    
                RMM::RContentType t = it.current()->envelope().contentType();
                
                empathDebug("   Type of this part is \"" + t.type() + "\"");
                empathDebug("SubType of this part is \"" + t.subType() + "\"");

                if (0 == stricmp(t.type(), "text")) {
                    
                    if (0 == stricmp(t.subType(), "html")) {

                        empathDebug("Using this HTML part as body");

                        s =QString::fromUtf8(
                            ((it.current()->body()).at(0))->asString());

                        messageWidget_->show(s);
                        return;
    
                    } else if (0 == stricmp(t.subType(), "plain")) {
                    
                        empathDebug("Using this plaintext part as body");

                        s = QString::fromUtf8(it.current()
                            ->asXML(quote1, quote2));
                    
                        messageWidget_->show(s);
                        return;
                    }
                    
                } else {

                    empathDebug("No real idea what to do with this part");
                    s = QString::fromUtf8(it.current()->asString());
                    messageWidget_->show(s);
                    return;
                }

            } // End (if this body part is text)

        } // End (if has content type)
        
        empathDebug("=================== END MULTIPART =====================");
    }

    empathDebug("Fallback");
    s = QString::fromUtf8(message.asXML(quote1, quote2));
    messageWidget_->show(s);
}

    void
EmpathMessageViewWidget::s_print()
{
    empathDebug("STUB");
}

EmpathMessageViewWidget::~EmpathMessageViewWidget()
{
    delete structureWidget_;
}

    void
EmpathMessageViewWidget::s_setMessage(const EmpathURL & url)
{
    url_ = url;
    empath->retrieve(url_, this);
}

    void
EmpathMessageViewWidget::s_URLSelected(QString fixedURL, int button)
{
    fixedURL = fixedURL.stripWhiteSpace();

    if (fixedURL.left(7) == "mailto:") {
        
        fixedURL = fixedURL.mid(7);

        if (button == 1) {
            empath->s_compose();
        }
    
    } else {

        // It's an URL we don't handle. Pass to KRun.
        
        new KRun(fixedURL);
    }
}

    void
EmpathMessageViewWidget::s_clipClicked()
{
    if (structureWidget_->isVisible())
        structureWidget_->hide();
    else
        structureWidget_->show();
}

    void
EmpathMessageViewWidget::s_partChanged(RMM::RBodyPart part)
{
    empathDebug("");

    QString errorMsg =
        i18n("<qt type=\"detail\">No viewer for mime type <b>%1</b></qt>");

    KConfig * config(KGlobal::config());

    using namespace EmpathConfig;

    config->setGroup(GROUP_DISPLAY);

    QColor quote1(config->readColorEntry(UI_QUOTE_ONE, DFLT_Q_1));
    QColor quote2(config->readColorEntry(UI_QUOTE_TWO, DFLT_Q_2));

    RMM::RContentType t = part.envelope().contentType();

    if (0 == stricmp(t.type(), "text")) {

        if (
            (0 == stricmp(t.subType(), "plain")) ||
            (0 == stricmp(t.subType(), "unknown"))
            )
            messageWidget_->show(part.asXML(quote1, quote2));

        else if (
            (0 == stricmp(t.subType(), "html")) ||
            (0 == stricmp(t.subType(), "richtext"))
            )
            messageWidget_->show(part.asString());

    } else if (0 == stricmp(t.type(), "image")) {

        // TODO: Enable this when we work out how to use QMimeSourceFactory.
        if (
            (0 == stricmp(t.subType(), "png")) ||
            (0 == stricmp(t.subType(), "jpeg")) ||
            (0 == stricmp(t.subType(), "xpm"))) {

            QByteArray data = part.decode();

            if (data.size() == 0) {

              empathDebug("Decoded to nothing")
              messageWidget_->show("Could not decode image");

            } else {

              QMimeSourceFactory::defaultFactory()
                  ->setImage("tempimg", QImage(data));

              messageWidget_->show("<img source=\"tempimg\"/>");
            }
        }

        else
            messageWidget_->show(errorMsg.arg(t.type() + "/" + t.subType()));

    } else {

        messageWidget_->show(errorMsg.arg(t.type() + "/" + t.subType()));
    }
}

    void
EmpathMessageViewWidget::s_showText(const QString & s)
{
    messageWidget_->show(s);
}


    void
EmpathMessageViewWidget::s_switchView()
{
    empathDebug("STUB");
}

