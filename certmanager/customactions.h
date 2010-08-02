/*
    customactions.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�lvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __CUSTOMACTIONS_H__
#define __CUSTOMACTIONS_H__

#include <kaction.h>

#include <tqstringlist.h>

class TQLineEdit;

class LabelAction : public KAction {
  Q_OBJECT
public:
  LabelAction( const TQString & text, KActionCollection * parent,
	       const char* name );

  int plug( TQWidget * widget, int index=-1 );
};

class LineEditAction : public KAction {
  Q_OBJECT
public:
  LineEditAction( const TQString & text, KActionCollection * parent,
		  TQObject * receiver, const char * member, const char * name );

  int plug( TQWidget * widget, int index=-1 );
  void clear();
  void focusAll();
  TQString text() const;
  void setText( const TQString & txt );
private:
  TQLineEdit* _le;
  TQObject * _receiver;
  const char * _member;
};

class ComboAction : public KAction {
  Q_OBJECT
public:
  ComboAction( const TQStringList & lst,  KActionCollection * parent,
	       TQObject * receiver, const char * member, const char * name,
               int selectedID );

  int plug( TQWidget * widget, int index=-1 );

private:
  TQStringList _lst;
  TQObject * _receiver;
  const char * _member;
  int _selectedId;
};



#endif // __CUSTOMACTIONS_H__
