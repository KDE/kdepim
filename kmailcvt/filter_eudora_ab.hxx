/***************************************************************************
                        FilterEudoraAb.hxx  -  description
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

#ifndef FILTER_EUDORA_AB
#define FILTER_EUDORA_AB

#include "filters.hxx"

class QFile;

class FilterEudoraAb : public Filter, protected FilterFactory< FilterEudoraAb >

{
  public:
    FilterEudoraAb();
   ~FilterEudoraAb();

    void import(FilterInfo *info);
    void convert(QFile& f,FilterInfo *info);

  protected:
    QString get(const QString& line, const QString& key) const;
    QString comment(const QString& line) const;
    QString email(const QString& line) const;
    QString key(const QString& line) const;
    int     find(const QString& key) const;

  private:
    QStringList keys;
    QStringList emails;
    QStringList names;
    QStringList phones;
    QStringList adr;
    QStringList comments;
};

#endif
