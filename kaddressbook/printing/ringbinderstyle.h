/*
  RingBinderPrintStyle - a print style for ring binders

  Author: Jost Schenck
  Copyright: (C) 2002 Jost Schenck <jost@schenck.de>
  License: LGPL 

*/

#ifndef RINGBINDERSTYLE_H
#define RINGBINDERSTYLE_H

#include <kabc/addressee.h>
#include "printstyle.h"

class KPrinter;
class RingBinderStyleAppearanceForm;

namespace KABPrinting
{

class RingBinderPrintStyle : public PrintStyle
{
    Q_OBJECT

  public:
    RingBinderPrintStyle( PrintingWizard* parent, const char* name = 0 );
    ~RingBinderPrintStyle();
    void print( KABC::Addressee::List &contacts, PrintProgress* );

  protected:
    bool printEntries( KABC::Addressee::List &contacts, KPrinter *printer, 
                       QPainter *painter, const QRect& window );
    void fillEmpty( const QRect& window, KPrinter *printer, QPainter* painter, 
                    int top, int grpnum );
    bool printEntry( const KABC::Addressee& contact, const QRect& window, 
                     QPainter *painter, int top, bool fake = false, 
                     QRect* brect = 0 );
    QRect entryMetrics( const KABC::Addressee& contact, const QRect& window, 
                        QPainter* painter, int top );
    bool printEmptyEntry( const QRect& window, QPainter* painter, int top );
    QRect emptyEntryMetrics( const QRect& window, QPainter* painter, int top );
    bool printPageHeader( const QString section, const QRect& window, 
                          QPainter* painter );
    QRect pageHeaderMetrics( const QRect& window, QPainter* painter );

  private:
    RingBinderStyleAppearanceForm *mPageAppearance;
    PrintProgress *mPrintProgress;
};

class RingBinderPrintStyleFactory : public PrintStyleFactory
{
  public:
    RingBinderPrintStyleFactory( PrintingWizard* parent_,
                                 const char* name_ = 0 );
    PrintStyle *create();
    QString description();
};

}

#endif
// vim:tw=78 cin et sw=2 comments=sr\:/*,mb\:\ ,ex\:*/,\://
