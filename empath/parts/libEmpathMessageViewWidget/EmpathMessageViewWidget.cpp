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

// Qt includes
#include <qfile.h>
#include <qcstring.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <krun.h>
#include <kaction.h>
#include <kinstance.h>
#include <kiconloader.h>

// Local includes
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageTextViewWidget.h"
#include "EmpathMessageHeaderViewWidget.h"
#include "EmpathMessageAttachmentViewWidget.h"

#include <RMM_Message.h>
#include <RMM_BodyPart.h>
#include <RMM_ContentType.h>

extern "C"
{
    void *init_libEmpathMessageViewWidget()
    {
        return new EmpathMessageViewPartFactory;
    }
}

KInstance * EmpathMessageViewPartFactory::instance_ = 0L;

EmpathMessageViewPartFactory::EmpathMessageViewPartFactory()
{
}

EmpathMessageViewPartFactory::~EmpathMessageViewPartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
EmpathMessageViewPartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    qDebug("create...");
    QObject * o = new EmpathMessageViewPart((QWidget *)parent, name);
    qDebug("emitting objectCreated...");
    emit objectCreated(o);
    qDebug("create done");
    return o;
}

    KInstance *
EmpathMessageViewPartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("EmpathMessageViewWidget");

    return instance_;
}

// -------------------------------------------------------------------------

EmpathMessageViewPart::EmpathMessageViewPart(
    QWidget * parent,
    const char * name
)
    :   KParts::ReadOnlyPart(parent, name)
{
    qDebug("part ctor ...");
    setInstance(EmpathMessageViewPartFactory::instance());
    qDebug("instance set");

    w = new EmpathMessageViewWidget(parent);
    qDebug("widget created");
    w->setFocusPolicy(QWidget::StrongFocus);
    qDebug("focus set");
    setWidget(w);
    qDebug("setWidget() done. Creating actions");

    messageCompose_ =
        new KAction(
            i18n("Compose"),
            QIconSet(BarIcon("messageCompose",
                    EmpathMessageViewPartFactory::instance())),
            0,
            w,
            SLOT(compose()),
            actionCollection(),
            "messageCompose");

    messageReply_ =
        new KAction(
            i18n("Reply"),
            QIconSet(BarIcon("messageReply",
                    EmpathMessageViewPartFactory::instance())),
            0,
            w,
            SLOT(reply()),
            actionCollection(),
            "messageReply");

    messageReplyAll_ =
        new KAction(
            i18n("ReplyAll"),
            QIconSet(BarIcon("messageReplyAll",
                    EmpathMessageViewPartFactory::instance())),
            0,
            w,
            SLOT(replyAll()),
            actionCollection(),
            "messageReplyAll");

    messageForward_ =
        new KAction(
            i18n("Forward"),
            QIconSet(BarIcon("messageForward",
                    EmpathMessageViewPartFactory::instance())),
            0,
            w,
            SLOT(forward()),
            actionCollection(),
            "messageForward");

    qDebug("Actions created. Creating browser extension");
    extension_ = new EmpathMessageViewBrowserExtension(this);

    qDebug("Browser extension created. Setting XML file");
    setXMLFile("EmpathMessageViewWidget.rc");

    qDebug("Disabling actions");
    enableAllActions(false);
    qDebug("Part ctor done");
}

EmpathMessageViewPart::~EmpathMessageViewPart()
{
    // Empty.
}

// -------------------------------------------------------------------------

EmpathMessageViewWidget::EmpathMessageViewWidget(
    QWidget * parent,
    const char * name
)
    :   QWidget(parent, name)
{
    qDebug("Widget ctor");
    (new QVBoxLayout(this))->setAutoAdd(true);
    qDebug("Layout created");
    
    headerView_     = new EmpathMessageHeaderViewWidget(this);
    textView_       = new EmpathMessageTextViewWidget(this);
    attachmentView_ = new EmpathMessageAttachmentViewWidget(this);

    attachmentView_->hide();
    qDebug("Widget ctor done");
}

EmpathMessageViewWidget::~EmpathMessageViewWidget()
{
    // Empty.
}

    void
EmpathMessageViewWidget::setMessage(RMM::RMessage & m)
{
    qDebug("setMessage()");

    if (!m) {
      qDebug("Message is null");
      return;
    }

    KConfig * config(KGlobal::config());

    config->setGroup("EmpathMessageViewWidget");
    
    QColor defaultQuoteColour1 = Qt::darkBlue;
    QColor defaultQuoteColour2 = Qt::darkCyan;

    QColor quote1 = Qt::darkBlue; // FIXME (config->readColorEntry(UI_QUOTE_ONE, &defaultQuoteColour1));
    QColor quote2 = Qt::darkCyan; // FIXME (config->readColorEntry(UI_QUOTE_TWO, &defaultQuoteColour2));

    RMM::RBodyPart message(m);

    headerView_->useEnvelope(message.envelope());

    QString s;
    
    if (message.body().count() == 0) {
        qDebug("Message body count is 0");
        attachmentView_->hide();
        s = QString::fromUtf8(message.asXML(quote1, quote2));
        textView_->setXML(s);
        return;
    }
    
    else if (message.body().count() == 1) {
        qDebug("Message body count is 1");
        attachmentView_->hide();
        s = QString::fromUtf8(message.body().at(0)->asXML(quote1, quote2));
        textView_->setXML(s);
        return;
    }
    
    else {
        
        qDebug("===================== MULTIPART ====================");
        
        attachmentView_->setMessage(message);
        
        QList<RMM::RBodyPart> body(message.body());
        QListIterator<RMM::RBodyPart> it(body);
        
        int i = 0;
        for (; it.current(); ++it) {
        
            ++i;
        
            qDebug(
                " ===================== PART # "
                + QCString().setNum(i) +
                " =====================");

            if (it.current()->envelope().has(RMM::HeaderContentType)) {
                
                qDebug("Ok this part has a Content-Type");                            
                    
                RMM::RContentType t = it.current()->envelope().contentType();
                
                qDebug("   Type of this part is \"" + t.type() + "\"");
                qDebug("SubType of this part is \"" + t.subType() + "\"");

                if (0 == stricmp(t.type(), "text")) {
                    
                    if (0 == stricmp(t.subType(), "html")) {

                        qDebug("Using this HTML part as body");

                        s = QString::fromUtf8(
                            ((it.current()->body()).at(0))->asString());

                        textView_->setXML(s);
                        return;
    
                    } else if (0 == stricmp(t.subType(), "plain")) {
                    
                        qDebug("Using this plaintext part as body");

                        s = QString::fromUtf8(it.current()
                            ->asXML(quote1, quote2));
                    
                        textView_->setXML(s);
                        return;
                    }
                    
                } else {

                    qDebug("No real idea what to do with this part");
                    s = QString::fromUtf8(it.current()->asString());
                    textView_->setXML(s);
                    return;
                }

            } // End (if this body part is text)

        } // End (if has content type)
        
        qDebug("=================== END MULTIPART =====================");
    }

    qDebug("Fallback");
    s = QString::fromUtf8(message.asXML(quote1, quote2));
    textView_->setXML(s);
}

// -------------------------------------------------------------------------

    void
EmpathMessageViewPart::enableAllActions(bool enable)
{
    int count = actionCollection()->count();

    for (int i = 0; i < count; i++)
        actionCollection()->action(i)->setEnabled(enable);
}

    bool
EmpathMessageViewPart::openFile()
{
    qDebug("openFile `" + QCString(m_file.ascii()) + "'");
    QFile f(m_file);
    f.open(IO_ReadOnly);
    QCString s = QCString(f.readAll());
    RMM::RMessage m(s);
    qDebug("File read and message created");
    w->setMessage(m);

    qDebug("Enabling actions");
    enableAllActions(true);
    qDebug("done");
    return true;
}

// -------------------------------------------------------------------------

EmpathMessageViewBrowserExtension::EmpathMessageViewBrowserExtension(
    EmpathMessageViewPart * parent
)
    :   KParts::BrowserExtension(parent, "EmpathMessageViewBrowserExtension")
{
}

    void
EmpathMessageViewBrowserExtension::compose()
{ ((EmpathMessageViewWidget *)parent())->compose(); }

    void
EmpathMessageViewBrowserExtension::reply()
{ ((EmpathMessageViewWidget *)parent())->reply(); }

    void
EmpathMessageViewBrowserExtension::replyAll()
{ ((EmpathMessageViewWidget *)parent())->replyAll(); }

    void
EmpathMessageViewBrowserExtension::forward()
{ ((EmpathMessageViewWidget *)parent())->forward(); }

#if 0
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
EmpathMessageViewWidget::s_partChanged(RMM::RBodyPart part)
{
    QString errorMsg =
        i18n("<qt type=\"detail\">No viewer for mime type <b>%1</b></qt>");

    KConfig * config(KGlobal::config());

    using namespace EmpathConfig;

    config->setGroup(GROUP_DISPLAY);

    QColor defaultQuoteColour1 = Qt::darkBlue;
    QColor defaultQuoteColour2 = Qt::darkCyan;

    QColor quote1 = Qt::darkBlue; // FIXME (config->readColorEntry(UI_QUOTE_ONE, &defaultQuoteColour1));
    QColor quote2 = Qt::darkCyan; // FIXME (config->readColorEntry(UI_QUOTE_TWO, &defaultQuoteColour2));

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
#endif

// vim:ts=4:sw=4:tw=78
