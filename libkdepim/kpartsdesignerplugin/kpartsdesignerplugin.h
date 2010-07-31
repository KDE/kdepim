/*
    Copyright (C) 2005, David Faure <faure@kde.org>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef DESIGNER_PARTPLUGIN_H
#define DESIGNER_PARTPLUGIN_H

#include <tqwidgetplugin.h>
#include <tqwidget.h>
namespace KParts { class ReadOnlyPart; }

/**
 * Generic part loader, able to view any kind of file for which
 * a KParts::ReadOnlyPart is available
 */
class KPartsGenericPart : public TQWidget {
    Q_OBJECT
    Q_PROPERTY( TQString url READ url WRITE setURL )
    Q_PROPERTY( TQString mimetype READ mimetype WRITE setMimetype )
public:
    KPartsGenericPart( TQWidget* parentWidget, const char* name );

    TQString url() const { return m_url; }
    void setURL( const TQString& url ) { m_url = url; load(); }

    // The mimetype, or "auto" if unknown
    TQString mimetype() const { return m_mimetype; }
    void setMimetype( const TQString& mimetype ) { m_mimetype = mimetype; load(); }

private:
    void load();

private:
    TQString m_mimetype;
    TQString m_url;
    KParts::ReadOnlyPart* m_part;
};

/**
 * Qt designer plugin for embedding a KParts using KPartsGenericPart
 */
class KPartsWidgetPlugin : public TQWidgetPlugin {
public:
  TQStringList keys() const;
  TQWidget * create( const TQString & key, TQWidget * parent, const char * name );
  TQString group( const TQString & key ) const;
  //TQIconSet iconSet( const TQString & key ) const;
  TQString includeFile( const TQString & key ) const;
  TQString toolTip( const TQString & key ) const;
  TQString whatsThis( const TQString & key ) const;
  bool isContainer( const TQString & key ) const;
};

#endif
