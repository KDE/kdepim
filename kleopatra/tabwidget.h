/* -*- mode: c++; c-basic-offset:4 -*-
    tabwidget.h

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

#ifndef __KLEOPATRA_TABWIDGET_H__
#define __KLEOPATRA_TABWIDGET_H__

#include <QWidget>

#include <utils/pimpl_ptr.h>

#include <boost/shared_ptr.hpp>

class QAbstractItemView;

namespace Kleo {
    class AbstractKeyListModel;
    class KeyFilter;
}

class KConfigGroup;

class TabWidget : public QWidget {
    Q_OBJECT
public:
    explicit TabWidget( QWidget * parent=0, Qt::WindowFlags f=0 );
    ~TabWidget();

    void setOpenNewTabAction( QAction * action );
    void setCloseCurrentTabAction( QAction * action );
    //void setHierarchicalViewAction( QAction * action );

    QAbstractItemView * addView( Kleo::AbstractKeyListModel * model, const QString & title=QString(), const QString & keyFilterID=QString(), const QString & searchString=QString() );
    QAbstractItemView * addView( Kleo::AbstractKeyListModel * model, const KConfigGroup & group );

    void saveTab( unsigned int idx, KConfigGroup & group ) const;

    QAbstractItemView * currentView() const;

    unsigned int count() const;

public Q_SLOTS:
    void setKeyFilter( const boost::shared_ptr<Kleo::KeyFilter> & filter );
    void setStringFilter( const QString & filter );

Q_SIGNALS:
    void currentViewChanged( QAbstractItemView * view );
    void enableCloseCurrentTabAction( bool enable );

protected:
    void resizeEvent( QResizeEvent * );

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;

    Q_PRIVATE_SLOT( d, void currentIndexChanged( int ) );
};


#endif /* __KLEOPATRA_TABWIDGET_H__ */
