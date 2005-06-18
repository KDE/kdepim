/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNARTICLEWIDGET_H
#define KNARTICLEWIDGET_H

#include <qbitarray.h>
#include <ktextbrowser.h>


class KNSourceViewWindow : public KTextBrowser {

  Q_OBJECT

  public:
    KNSourceViewWindow( const QString &text );
    ~KNSourceViewWindow();
    virtual void setPalette( const QPalette &pal );

};


//============================================================================================


class KNDisplayedHeader {

  public:
    KNDisplayedHeader();
    ~KNDisplayedHeader();

    //some common headers
    static const char** predefs();

    //name
    const QString& name()               { return n_ame; }
    void setName(const QString &s)      { n_ame = s; }
    bool hasName() const                     { return !n_ame.isEmpty(); }

    //translated name
    QString translatedName();                     // *tries* to translate the name
    void setTranslatedName(const QString &s);     // *tries* to retranslate the name to english
    void setTranslateName(bool b)       { t_ranslateName=b; }
    bool translateName() const                { return t_ranslateName; }

    //header
    const QString& header()             { return h_eader; }
    void setHeader(const QString &s)    { h_eader = s; }

    //flags
    bool flag(int i)                    { return f_lags.at(i); }
    void setFlag(int i, bool b)         { f_lags.setBit(i, b); }

    //HTML-tags
    void createTags();
    const QString& nameOpenTag()        { return t_ags[0]; }
    const QString& nameCloseTag()       { return t_ags[1]; }
    const QString& headerOpenTag()      { return t_ags[2]; }
    const QString& headerCloseTag()     { return t_ags[3]; }

  protected:
    bool t_ranslateName;
    QString n_ame, h_eader, t_ags[4];
    QBitArray f_lags;

};

#endif
