/***************************************************************************
*   This file is part of the Bilbo Blogger.                               *
*   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
*   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef WAIT_WIDGET_H
#define WAIT_WIDGET_H

#include "ui_waitwidgetbase.h"
#include <QDialog>

/**
  @brief
  This class is the widget for the WaitDialog including a progress bar.
*/

class WaitWidget : public QDialog, public Ui::WaitWidgetBase
{
    Q_OBJECT
public:
    WaitWidget( QWidget* parent = 0 );
    virtual ~WaitWidget();
    // sets the text in the label
    void setText( const QString& text );
    void jobDone();
    // sets the progress bar's max value
    void setMaxJobs( int );
    /**
     * Set progressbar to a busy progressbar
     */
    void setBusyState();
};
#endif
