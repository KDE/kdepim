/***************************************************************************
                        filter_eudora_ab.hxx  -  description
                        ------------------------------------
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

#include "filters.hxx"

#ifndef __FILTER_EUDORA_AB__
#define __FILTER_EUDORA_AB__

class filter_eudora_ab : public filter
{
  private:
    QString CAP;
    int     LINES;
  private:
    QStringList keys;
    QStringList emails;
    QStringList names;
    QStringList phones;
    QStringList adr;
    QStringList comments;
  public:
    filter_eudora_ab();
   ~filter_eudora_ab();
  public:
    void import(filterInfo *info);
  private:
    void convert(FILE *f,filterInfo *info);
  private:
    QString get(QString line,QString key);
    QString getcomment(QString line);
    QString getemail(QString line);
    QString getkey(QString line);
    int     find(QString key);
};

#endif
