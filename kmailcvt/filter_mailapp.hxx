/***************************************************************************
                          filter_mailapp.hxx  -  OS X Mail App import
                             -------------------
    copyright            : (C) 2004 by Chris Howells
    email                : howells@kde.org
 
    Derived from code by:
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 
 ***************************************************************************/

#ifndef FILTER_MAILAPP_HXX
#define FILTER_MAILAPP_HXX

#include "filters.hxx"

/**
 *imports mbox archives messages into KMail
 *@author Chris Howells
 */

class FilterMailApp : public Filter
{
public:
    FilterMailApp();
    ~FilterMailApp();

    void import(FilterInfo *info);

private:
    QStringList mMboxFiles;
    void traverseDirectory(FilterInfo *info, const QString &);
};

#endif
