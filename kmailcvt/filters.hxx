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

#ifndef MAX_LINE
#define MAX_LINE 4096
#endif

#include <q3ptrlist.h>
#include <q3listbox.h>

#include "kimportpage.h"

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
            const QString& info = QString() );
    virtual ~Filter() {}
    virtual void import( FilterInfo* ) = 0;
    QString author() const { return m_author; }
    QString name() const { return m_name; }
    QString info() const { return m_info; }

    int count_duplicates; //to count all duplicate messages

  protected:
    bool addMessage( FilterInfo* info,
                     const QString& folder,
                     const QString& msgFile,
                     const QString& msgStatusFlags = QString());
    bool addMessage_fastImport( FilterInfo* info,
                     		    const QString& folder,
                     		    const QString& msgFile,
                                const QString& msgStatusFlags = QString());
  private:
    QString m_name;
    QString m_author;
    QString m_info;
};



/**
* Glorified QString[N] for (a) understandability (b) older gcc compatibility.
*/
template <unsigned int size> class FolderStructureBase
{
public:
	typedef QString NString[size];
	/** Constructor. Need a default constructor for QValueList. */
	FolderStructureBase() {} 

	/** Constructor. Turn N QStrings into a folder structure
	*   description.
	*/
	FolderStructureBase(const NString &s)
	{
	    for(unsigned int i=0; i<size; i++) d[i]=s[i];
	} 

	/** Copy Constructor. */
	FolderStructureBase(const FolderStructureBase &s)
	{
	    for(unsigned int i=0; i<size; i++) d[i]=s[i];
	} 

	/** Assignment operator. Does the same thing as
	*   the copy constructor.
	*/
	FolderStructureBase &operator =(const FolderStructureBase &s)
	{
	    for(unsigned int i=0; i<size; i++) d[i]=s[i];
	    return *this;
	} 

	/** Access the different fields. There doesn't seem to
	*   be a real semantics for the fields.
	*/
	const QString operator [](unsigned int i) const
	{
	    if (i<size) return d[i]; else return QString();
	} 

	/** Access the different fields, for writing. */
	QString &operator [](unsigned int i)
	{
	    Q_ASSERT(i<size);
	    if (i<size) return d[i]; else return d[0];
	} 
private:
	QString d[size];
} ;

#endif

