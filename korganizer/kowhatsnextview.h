/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOWHATSNEXTVIEW_H
#define KOWHATSNEXTVIEW_H

#include <tqtextbrowser.h>

#include <korganizer/baseview.h>

class TQListView;

class KOEventViewerDialog;

class WhatsNextTextBrowser : public TQTextBrowser {
    Q_OBJECT
  public:
    WhatsNextTextBrowser(TQWidget *parent) : TQTextBrowser(parent) {}

    void setSource(const TQString &);

  signals:
    void showIncidence(const TQString &uid);
};


/**
  This class provides a view of the next events and todos
*/
class KOWhatsNextView : public KOrg::BaseView
{
    Q_OBJECT
  public:
    KOWhatsNextView(Calendar *calendar, TQWidget *parent = 0,
                    const char *name = 0);
    ~KOWhatsNextView();

    virtual int currentDateCount();
    virtual Incidence::List selectedIncidences() { return Incidence::List(); }
    DateList selectedIncidenceDates() { return DateList(); }

    bool supportsDateNavigation() const { return true; }

  public slots:
    virtual void updateView();
    virtual void showDates(const TQDate &start, const TQDate &end);
    virtual void showIncidences( const Incidence::List &incidenceList, const TQDate &date );

    void changeIncidenceDisplay(Incidence *, int);

  protected:
    void appendEvent( Incidence *, const TQDateTime &start = TQDateTime(),
                      const TQDateTime &end = TQDateTime() );
    void appendTodo( Incidence * );

  private slots:
    void showIncidence(const TQString &);

  private:
    TQTextBrowser *mView;
    TQString mText;
    TQDate mStartDate;
    TQDate mEndDate;

    Incidence::List mTodos;
};

#endif
