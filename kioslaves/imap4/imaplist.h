#ifndef _IMAPLIST_H
#define _IMAPLIST_H
/**********************************************************************
 *
 *   imaplist.h  - IMAP4rev1 list response handler
 *   Copyright (C) 2000 Sven Carstens
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *   Send comments and bug fixes to
 *
 *********************************************************************/

#include <qstringlist.h>
#include <qstring.h>

class parseString;
class imapParser;

//the class handling the responses from list
class imapList
{
public:

  imapList ();
  imapList (const QString &, imapParser &);
  imapList (const imapList &);
    imapList & operator = (const imapList &);

  // process the attributes  
  void parseAttributes( parseString & );  

  // return all atributes concatenated
  QString attributesAsString() const 
  { 
    return attributes_.join(","); 
  }

  QString hierarchyDelimiter () const
  {
    return hierarchyDelimiter_;
  }
  void setHierarchyDelimiter (const QString & _str)
  {
    hierarchyDelimiter_ = _str;
  }

  QString name () const
  {
    return name_;
  }
  void setName (const QString & _str)
  {
    name_ = _str;
  }

  bool noInferiors () const
  {
    return noInferiors_;
  }
  void setNoInferiors (bool _val)
  {
    noInferiors_ = _val;
  }

  bool noSelect () const
  {
    return noSelect_;
  }
  void setNoSelect (bool _val)
  {
    noSelect_ = _val;
  }

  bool hasChildren () const
  {
    return hasChildren_;
  }
  void setHasChildren (bool _val)
  {
    hasChildren_ = _val;
  }

  bool hasNoChildren () const
  {
    return hasNoChildren_;
  }
  void setHasNoChildren (bool _val)
  {
    hasNoChildren_ = _val;
  }

  bool marked () const
  {
    return marked_;
  }
  void setMarked (bool _val)
  {
    marked_ = _val;
  }

  bool unmarked () const
  {
    return unmarked_;
  }
  void setUnmarked (bool _val)
  {
    unmarked_ = _val;
  }

private:

  imapParser* parser_;
  QString hierarchyDelimiter_;
  QString name_;
  bool noInferiors_;
  bool noSelect_;
  bool marked_;
  bool unmarked_;
  bool hasChildren_;
  bool hasNoChildren_;
  QStringList attributes_;
};

#endif
