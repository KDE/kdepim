/***************************************************************************
                          filters.hxx  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTERS_HXX
#define FILTERS_HXX

#include <qcombobox.h>
#include <qprogressbar.h>
#include <qptrlist.h>
#include <qlistbox.h>
#include <qlabel.h>

#include "kimportpagedlg.h"

class FilterInfo
{
  public:
    FilterInfo(KImportPageDlg *dlg, QWidget *parent, bool _removeDupMsg);
   ~FilterInfo();

    void setStatusMsg( const QString& status );
    void setFrom( const QString& from );
    void setTo( const QString& to );
    void setCurrent( const QString& current );
    void setCurrent( int percent = 0 );
    void setOverall( int percent = 0 );
    void addLog( const QString& log );
    void clear();
    void alert( const QString& message );
    static void terminateASAP();
    bool shouldTerminate();

    QWidget *parent() { return m_parent; }
    bool removeDupMsg;

  private:
    KImportPageDlg *m_dlg;
    QWidget      *m_parent;
    static bool s_terminateASAP;
};

class Filter
{
  public:
    Filter( const QString& name, const QString& author,
            const QString& info = QString::null );
    virtual ~Filter() {}
    virtual void import( FilterInfo* ) = 0;
    QString author() const { return m_author; }
    QString name() const { return m_name; }
    QString info() const { return m_info; }
    
    int count_duplicates; //to count all duplicate messages
    
  protected:
    bool addMessage( FilterInfo* info,
                     const QString& folder,
                     const QString& msgFile );
    bool addMessage_fastImport( FilterInfo* info,
                     		const QString& folder,
                     		const QString& msgFile );
    bool endImport();
  private:
    QString m_name;
    QString m_author;
    QString m_info;
};


#endif

// vim: ts=2 sw=2 et
