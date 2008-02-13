/****************************************************************************
** Copyright (C) 2001-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
**********************************************************************/

#ifndef __KDTOOLSGUI_KDLOGTEXTWIDGET_H__
#define __KDTOOLSGUI_KDLOGTEXTWIDGET_H__

#include <utils/kdtoolsglobal.h>
#include <utils/pimpl_ptr.h>
#include <QtGui/QAbstractScrollArea>

class QString;
class QStringList;
class QColor;

class KDLogTextWidget : public QAbstractScrollArea {
    Q_OBJECT
    Q_PROPERTY( uint historySize READ historySize WRITE setHistorySize )
    Q_PROPERTY( QString text READ text )
    Q_PROPERTY( QStringList lines READ lines WRITE setLines )
    Q_PROPERTY( uint minimumVisibleLines READ minimumVisibleLines WRITE setMinimumVisibleLines )
    Q_CLASSINFO( "description", "High-speed text display widget" )
public:
    explicit KDLogTextWidget( QWidget * parent=0 );
    ~KDLogTextWidget();

    void setHistorySize( unsigned int size );
    unsigned int historySize() const;

    QString text() const;

    void setLines( const QStringList & list );
    QStringList lines() const;

    void setMinimumVisibleLines( unsigned int num );
    unsigned int minimumVisibleLines() const;

    /*! \reimp */ QSize minimumSizeHint() const;
    /*! \reimp */ QSize sizeHint() const;

public Q_SLOTS:
    void clear();
    void message( const QString & msg, const QColor & color );
    void message( const QString & msg );

protected:
    /*! \reimp */ void paintEvent( QPaintEvent * );
    /*! \reimp */ void timerEvent( QTimerEvent * );
    /*! \reimp */ void resizeEvent( QResizeEvent * );
    /*! \reimp */ void changeEvent( QEvent * );

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif /* __KDTOOLSGUI_KDLOGTEXTWIDGET_H__ */

