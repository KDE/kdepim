/* -*- mode: c++; c-basic-offset:4 -*-
    utils/dragqueen.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "dragqueen.h"

#include <QDrag>
#include <QMouseEvent>
#include <QStringList>
#include <QVariant>
#include <QApplication>
#include <QUrl>
#include <QStyle>

#include <algorithm>

using namespace Kleo;

namespace
{
class MimeDataProxy : public QMimeData
{
    Q_OBJECT
public:
    explicit MimeDataProxy(QMimeData *source)
        : QMimeData(), m_source(source)
    {

    }

    /* reimp */ QStringList formats() const
    {
        if (m_source) {
            return m_source->formats();
        } else {
            return QStringList();
        }
    }

    /* reimp */ bool hasFormat(const QString &format) const
    {
        return m_source && m_source->hasFormat(format);
    }

protected:
    /* reimp */ QVariant retrieveData(const QString &format, QVariant::Type type) const
    {
        if (!m_source) {
            return QVariant();
        }
        // Doesn't work, is protected:
        // return m_source->retrieveData( format, type );

        switch (type) {
        case QVariant::String:
            if (format == QLatin1String("text/plain")) {
                return m_source->text();
            }
            if (format == QLatin1String("text/html")) {
                return m_source->html();
            }
            break;
        case QVariant::Color:
            if (format == QLatin1String("application/x-color")) {
                return m_source->colorData();
            }
            break;
        case QVariant::Image:
            if (format == QLatin1String("application/x-qt-image")) {
                return m_source->imageData();
            }
            break;
        case QVariant::List:
        case QVariant::Url:
            if (format == QLatin1String("text/uri-list")) {
                const QList<QUrl> urls = m_source->urls();
                if (urls.size() == 1) {
                    return urls.front();
                }
                QList<QVariant> result;
                std::copy(urls.begin(), urls.end(),
                          std::back_inserter(result));
                return result;
            }
            break;
        default:
            break;
        }

        QVariant v = m_source->data(format);
        v.convert(type);
        return v;
    }
private:
    QPointer<QMimeData> m_source;
};
}

DragQueen::DragQueen(QWidget *p, Qt::WindowFlags f)
    : QLabel(p, f),
      m_data(),
      m_dragStartPosition()
{

}

DragQueen::DragQueen(const QString &t, QWidget *p, Qt::WindowFlags f)
    : QLabel(t, p, f),
      m_data(),
      m_dragStartPosition()
{

}

DragQueen::~DragQueen()
{
    delete m_data;
}

void DragQueen::setUrl(const QString &url)
{
    QMimeData *data = new QMimeData;
    QList<QUrl> urls;
    urls.push_back(QUrl(url));
    data->setUrls(urls);
    setMimeData(data);
}

QString DragQueen::url() const
{
    if (!m_data || !m_data->hasUrls()) {
        return QString();
    }
    const QList<QUrl> urls = m_data->urls();
    if (urls.empty()) {
        return QString();
    }
    return urls.front().toString();
}

void DragQueen::setMimeData(QMimeData *data)
{
    if (data == m_data) {
        return;
    }
    delete m_data;
    m_data = data;
}

QMimeData *DragQueen::mimeData() const
{
    return m_data;
}

void DragQueen::mousePressEvent(QMouseEvent *e)
{
#ifndef QT_NO_DRAGANDDROP
    if (m_data && e->button() == Qt::LeftButton) {
        m_dragStartPosition = e->pos();
    }
#endif
    QLabel::mousePressEvent(e);
}

static QPoint calculate_hot_spot(const QPoint &mouse, const QSize &pix, const QLabel *label)
{
    const Qt::Alignment align = label->alignment();
    const int margin = label->margin();
    const QRect cr = label->contentsRect().adjusted(margin, margin, -margin, -margin);
    const QRect rect = QStyle::alignedRect(QApplication::layoutDirection(), align, pix, cr);
    return mouse - rect.topLeft();
}

void DragQueen::mouseMoveEvent(QMouseEvent *e)
{
#ifndef QT_NO_DRAGANDDROP
    if (m_data &&
            (e->buttons() & Qt::LeftButton) &&
            (m_dragStartPosition - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QDrag *drag = new QDrag(this);
        if (const QPixmap *const pix = pixmap()) {
            drag->setPixmap(*pix);
            drag->setHotSpot(calculate_hot_spot(e->pos(), pix->size(), this));
        }
        drag->setMimeData(new MimeDataProxy(m_data));
        drag->exec();
    } else {
#endif
        QLabel::mouseMoveEvent(e);
#ifndef QT_NO_DRAGANDDROP
    }
#endif
}

#include "dragqueen.moc"
