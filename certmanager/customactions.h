/*  -*- mode: C++; c-file-style: "gnu" -*-
    customactions.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include <qstringlist.h>

class QLineEdit;

class LabelAction : public KAction {
  Q_OBJECT
public:
  LabelAction( const QString & text, KActionCollection * parent,
	       const char* name );

  int plug( QWidget * widget, int index=-1 );
};

class LineEditAction : public KAction {
  Q_OBJECT
public:
  LineEditAction( const QString & text, KActionCollection * parent,
		  QObject * receiver, const char * member, const char * name );

  int plug( QWidget * widget, int index=-1 );
  void clear();
  void focusAll();
  QString text() const;
  void setText( const QString & txt );
private:
  QLineEdit* _le;
  QObject * _receiver;
  const char * _member;
};

class ComboAction : public KAction {
  Q_OBJECT
public:
  ComboAction( const QStringList & lst,  KActionCollection * parent,
	       QObject * receiver, const char * member, const char * name );

  int plug( QWidget * widget, int index=-1 );

private:
  QStringList _lst;
  QObject * _receiver;
  const char * _member;
};



#endif // __CUSTOMACTIONS_H__
