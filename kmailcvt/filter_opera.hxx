/***************************************************************************
                  filter_opera.hxx  -  Opera mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTER_OPERA_HXX
#define FILTER_OPERA_HXX

#include "filters.hxx"

/**
 *imports opera account-archives into KMail
 *@author Danny Kukawka
 */

class FilterOpera : public Filter
{
public:
    FilterOpera();
    ~FilterOpera();

    void import(FilterInfo *info);
  void importBox( const QDir & importDir, const QStringList & , FilterInfo *info, const QString & accountName = QString());
  void importRecursive(const QDir& maildir, FilterInfo *info, const QString &accountName = QString() );
};

#endif
