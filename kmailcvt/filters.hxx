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

#include <kabc/addressbook.h>

#include "kimportpagedlg.h"

class FilterInfo
{
  public:
    FilterInfo(KImportPageDlg *dlg, QWidget *parent);
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

    
  private:
    KImportPageDlg *m_dlg;
    QWidget      *m_parent;
    static bool s_terminateASAP;
};

class Filter
{
  public:
    typedef Filter* ( *Creator )();
    typedef QPtrList< Filter > List;
    Filter( const QString& name, const QString& author,
            const QString& info = QString::null );
    virtual ~Filter() {}
    virtual void import( FilterInfo* ) = 0;
    QString author() const { return m_author; }
    QString name() const { return m_name; }
    QString info() const { return m_info; }

    static void registerFilter( Creator );
    static List createFilters();

    // FIXME: Move to protected when pablib goes
    void addContact( const KABC::Addressee& a );
    bool openAddressBook( FilterInfo* info );
    bool closeAddressBook( );

  protected:
    bool addMessage( FilterInfo* info,
                     const QString& folder,
                     const QString& msgFile );
  private:
    QString m_name;
    QString m_author;
    QString m_info;
    KABC::Ticket *saveTicket;
};

template< class T >
class FilterFactory
{
  public:
    static Filter* create()
      { return new T; }

  protected:
    FilterFactory()
    {
      static_cast< void >( s_register ); // Don't remove
    }
  
  private:
    static struct Register
    {
      Register()
        { Filter::registerFilter( create ); }
    } s_register;
};
template< class T >
typename FilterFactory< T >::Register FilterFactory< T >::s_register;

#endif

// vim: ts=2 sw=2 et
