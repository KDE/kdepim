#ifndef ___print_h
#define ___print_h

#undef Color // X11 headers
#undef GrayScale // X11 headers
#include <kprinter.h>
#include <qpainter.h>
#include "taskview.h"

/**
 * Provide printing capabilities.
 */

class MyPrinter : public KPrinter
{
  public:
    MyPrinter( const TaskView *taskView );
    void print();
    void printLine( QString total, QString session, QString name, QPainter &,
                    int );
    void printTask( QListViewItem *item, QPainter &, int level );  
    int calculateReqNameWidth( QListViewItem *item, QFontMetrics &metrics,
                               int level);
  
  private:
    const TaskView *_taskView;

    int xMargin, yMargin;
    int yoff;
    int totalTimeWidth;
    int sessionTimeWidth;
    int nameFieldWidth;
    int lineHeight;
   int pageHeight;  
};

#endif

