/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma implementation "EmpathHeaderViewWidget.h"
#endif

// Qt includes
#include <qstrlist.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qapplication.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>

// Local includes
#include "EmpathHeaderViewWidget.h"
#include "EmpathUIUtils.h"
#include "EmpathConfig.h"
#include "EmpathDefines.h"
#include <RMM_Header.h>

EmpathHeaderViewWidget::EmpathHeaderViewWidget(
        QWidget * parent, const char * name)
    :    QWidget(parent, name),
        glowing_(false)
{
    clipIcon_ = empathIcon("clip");
    clipGlow_ = empathIcon("clip-glow");
    setMouseTracking(true);
    setFixedHeight(0);
}

EmpathHeaderViewWidget::~EmpathHeaderViewWidget()
{
    // Empty.
}

    void
EmpathHeaderViewWidget::useEnvelope(RMM::REnvelope & e)
{
    headerList_.clear();
    KConfig * c(KGlobal::config());
    // FIXME Must be QStringList when available.
    c->setGroup(EmpathConfig::GROUP_DISPLAY);
    
    QStrList l;
    c->readListEntry(EmpathConfig::KEY_SHOW_HEADERS, l, ',');
    
    QStrListIterator it(l);
    
    for (; it.current() ; ++it) {
        
        QCString s = it.current();
        s = s.stripWhiteSpace();
    
        RMM::RHeader * h(e.get(s));
        if (h == 0) continue;
        
        headerList_.append(h->headerName() + ":");
        headerList_.append(h->headerBody()->asString());
    }
    
    int th = QFontMetrics(KGlobal::generalFont()).height();
    setFixedHeight(th * l.count() + 4);
    
    paintEvent(0);
}

    void
EmpathHeaderViewWidget::resizeEvent(QResizeEvent * e)
{
    resized_ = true;

    QWidget::resizeEvent(e);
}
    
    void
EmpathHeaderViewWidget::paintEvent(QPaintEvent * e)
{
    if (!resized_ && e) {
        bitBlt(this, e->rect().topLeft(), &buf_, e->rect());
        return;
    }
    
    resized_ = false;

    buf_.resize(width(), height());

    KPixmap px;
    px.resize(width(), height());
    KPixmapEffect::gradient(px,
        QApplication::palette().normal().background(),
        QApplication::palette().normal().base(),
        KPixmapEffect::VerticalGradient);
    
    int th = QFontMetrics(KGlobal::generalFont()).height();
    
    int i(0);
    
    QPainter p;
    p.begin(&buf_);
    
    p.drawPixmap(0, 0, px);
    
    
    p.drawPixmap(width() - 26, 2, clipIcon_);
    
    QRect brect;
    
    int maxWidth(0);
    
    QStrListIterator it(headerList_);
    for (; it.current(); ++it) {
        
        p.drawText(6, i * th + 4, 100, th,
            QPainter::AlignLeft | QPainter::AlignTop | QPainter::SingleLine,
            it.current(),
            -1, &brect);
        
        maxWidth = (maxWidth > brect.width() ? maxWidth : brect.width());
        ++it;
        ++i;
    }
    
    i = 0;
    it.toFirst();
    
    for (; it.current(); ++it) {

        ++it;

        p.drawText(maxWidth + 10, i * th + 4, width() - 30, th,
            QPainter::AlignLeft | QPainter::AlignTop | QPainter::SingleLine,
            it.current(),
            -1, &brect);
        
        ++i;
    }
    
    bitBlt(this, 0, 0, &buf_);
    p.end();
}

    void
EmpathHeaderViewWidget::mouseMoveEvent(QMouseEvent * e)
{
    QPainter p;
    p.begin(this);
    
    if (e != 0 && e->x() > width() - 26 && e->x() < width() - 2 &&
        e->y() > 2 && e->y() < 26) {
        
        if (!glowing_) {
            p.drawPixmap(width() - 26, 2, clipGlow_);
            glowing_ = true;
        }

    } else if (glowing_) {
    
        KPixmap px;
        px.resize(30, height());
        KPixmapEffect::gradient(px,
        QApplication::palette().normal().background(),
        QApplication::palette().normal().base(),
        KPixmapEffect::VerticalGradient);
        p.drawPixmap(width() - 30, 0, px);
        p.drawPixmap(width() - 26, 2, clipIcon_);
        glowing_ = false;
    }
    p.end();
}

    void
EmpathHeaderViewWidget::mousePressEvent(QMouseEvent * e)
{
    if (e != 0 && e->x() > width() - 26 && e->x() < width() - 2 &&
        e->y() > 2 && e->y() < 26)
        emit(clipClicked());
}

// vim:ts=4:sw=4:tw=78
