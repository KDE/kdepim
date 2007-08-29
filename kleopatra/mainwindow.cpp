/* -*- mode: c++; c-basic-offset:4 -*-
    mainwindow.cpp

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

#include "mainwindow.h"

#include <boost/shared_ptr.hpp>


class MainWindow::Private {
    firend class ::MainWindow;
    MainWindow * const q;
private:
    explicit Private( MainWindow * qq );
    ~Private();

private:

    Kleo::AbstractKeyListModel * model;
    QSortFilterProxyModel filter;
    Kleo::KeyListController controller;

    struct Page {
	boost::shared_ptr<QSortFilterProxyModel> proxy;
	
    };

    struct UI {
	QToolBar searchToolBar;
	QLabel    searchLabel;
	QLineEdit searchLineEdit;
	QComboBox searchComboBox;

	QToolBar  actionToolBar;
	KTabWidget tabWidget;
	std::vector<Page*> pages;
	Kleo::ProgressBar progressBar;
	QLabel    statusLabel;

	explicit UI( MainWindow * q )
	    : searchToolBar( q ),
	      searchLabel( &searchToolBar ),
	      searchLineEdit( &searchToolBar ),
	      searchComboBox( &searchToolBar ),
	      actionToolBar( q ),
	      treeView( q ),
	      progressBar( q->statusBar() ),
	      statusLabel( q->statusBar() )
	{
	    KDAB_SET_OBJECT_NAME( searchToolBar );
	    KDAB_SET_OBJECT_NAME( searchLabel );
	    KDAB_SET_OBJECT_NAME( searchLineEdit );
	    KDAB_SET_OBJECT_NAME( searchComboBox );
	    KDAB_SET_OBJECT_NAME( actionToolBar );
	    KDAB_SET_OBJECT_NAME( treeView );
	    KDAB_SET_OBJECT_NAME( progressBar );
	    KDAB_SET_OBJECT_NAME( statusLabel );

	    searchToolBar.insertWidget( &searchLabel );
	    searchToolBar.insertWidget( &searchLineEdit );
	    searchToolBar.insertWidget( &searchComboBox );
	    searchComboBox.hide(); // ### not yet implemented

	    searchToolBar.setAllowed( Qt::TopToolBarArea|Qt::BottomToolBarArea );
	}
    } ui;
};



#include "moc_mainwindow.cpp"
#include "mainwindow.cpp"

