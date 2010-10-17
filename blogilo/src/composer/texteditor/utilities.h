/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "utilities.h" from
    FlashQard project.

    Copyright (C) 2008-2009 Shahab <shahab@flashqard-project.org>
    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdlib>
#include <ctime>
#include <QString>
#include <QObject>
#include <QToolBar>
#include <QProcess>
#include <QToolButton>
class QWebPage;

class HtmlParser
{
   public:
      static HtmlParser *htmlParser();
      QString htmlToPlainText(const QString&);
      bool setTagAttribute(QString&, int&, const QString&, const QString&, const QString&);
      bool getTagAttribute(const QString&, int&, const QString&, const QString&, QString&);
      bool setTagStyleAttribute(QString&, int, const QString&, const QString&, const QString&);
      bool contains(const QString&, const QString&, bool) const;
      QString contentsOfTag(const QString&, int, const QString&) const;

      bool findNextTag(const QString&, int, QString&, int&, int&) const;
      bool findTag(const QString&, int, const QString&, int&, int&) const;

   private:
      static HtmlParser *_htmlParser;
      QWebPage *webPage;

      HtmlParser();
};


class Random
{
   public:
      static Random* random();
      //get random number x where min <= x <= max
      int getARandomNumber(int min, int max);
      //get a name which includes characters a-z and 0-9.
      QString getARandomName(int maxLength = 10);
   private:
      Random();
      static Random *_random;

};

#endif

