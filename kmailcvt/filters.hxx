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

#include <tqcombobox.h>
#include <tqprogressbar.h>
#include <tqptrlist.h>
#include <tqlistbox.h>
#include <tqlabel.h>

#include "kimportpagedlg.h"

class FilterInfo
{
  public:
    FilterInfo(KImportPageDlg *dlg, TQWidget *parent, bool _removeDupMsg);
   ~FilterInfo();

    void setStatusMsg( const TQString& status );
    void setFrom( const TQString& from );
    void setTo( const TQString& to );
    void setCurrent( const TQString& current );
    void setCurrent( int percent = 0 );
    void setOverall( int percent = 0 );
    void addLog( const TQString& log );
    void clear();
    void alert( const TQString& message );
    static void terminateASAP();
    bool shouldTerminate();

    TQWidget *parent() { return m_parent; }
    bool removeDupMsg;

  private:
    KImportPageDlg *m_dlg;
    TQWidget      *m_parent;
    static bool s_terminateASAP;
};

class Filter
{
  public:
    Filter( const TQString& name, const TQString& author,
            const TQString& info = TQString::null );
    virtual ~Filter() {}
    virtual void import( FilterInfo* ) = 0;
    TQString author() const { return m_author; }
    TQString name() const { return m_name; }
    TQString info() const { return m_info; }

    virtual bool needsSecondPage();

    int count_duplicates; //to count all duplicate messages

  protected:
    void showKMailImportArchiveDialog( FilterInfo* info );
    bool addMessage( FilterInfo* info,
                     const TQString& folder,
                     const TQString& msgFile,
                     const TQString& msgStatusFlags = TQString());
    bool addMessage_fastImport( FilterInfo* info,
                     		    const TQString& folder,
                     		    const TQString& msgFile,
                                const TQString& msgStatusFlags = TQString());
  private:
    TQString m_name;
    TQString m_author;
    TQString m_info;
};



/** 
* Glorified QString[N] for (a) understandability (b) older gcc compatibility. 
*/
template <unsigned int size> class FolderStructureBase
{
public:
	typedef TQString NString[size];
	/** Constructor. Need a default constructor for TQValueList. */
	FolderStructureBase() {} ;

	/** Constructor. Turn N QStrings into a folder structure 
	*   description. 
	*/
	FolderStructureBase(const NString &s)
	{
	    for(unsigned int i=0; i<size; i++) d[i]=s[i];
	} ;

	/** Copy Constructor. */
	FolderStructureBase(const FolderStructureBase &s)
	{
	    for(unsigned int i=0; i<size; i++) d[i]=s[i];
	} ;

	/** Assignment operator. Does the same thing as
	*   the copy constructor.
	*/
	FolderStructureBase &operator =(const FolderStructureBase &s)
	{
	    for(unsigned int i=0; i<size; i++) d[i]=s[i];
	    return *this;
	} ;

	/** Access the different fields. There doesn't seem to
	*   be a real semantics for the fields.
	*/
	const TQString operator [](unsigned int i) const
	{
	    if (i<size) return d[i]; else return TQString::null;
	} ;

	/** Access the different fields, for writing. */
	TQString &operator [](unsigned int i)
	{
	    Q_ASSERT(i<size);
	    if (i<size) return d[i]; else return d[0];
	} ;
private:
	TQString d[size];
} ;

#endif

