#ifndef KARM_PRINT_H
#define KARM_PRINT_H

#undef Color // X11 headers
#undef GrayScale // X11 headers
#include <kprinter.h>

class TQPainter;
class TQString;

class Task;
class TaskView;

/**
 * Provide printing capabilities.
 */

class MyPrinter : public KPrinter
{
  public:
    MyPrinter( const TaskView *taskView );
    void print();
    void printLine( TQString total, TQString session, TQString name, TQPainter &,
                    int );
    void printTask( Task *task, TQPainter &, int level );  
    int calculateReqNameWidth( Task *task, TQFontMetrics &metrics,
                               int level);
  
  private:
    const TaskView *_taskView;

    int xMargin, yMargin;
    int yoff;
    int timeWidth;
    int sessionTimeWidth;
    int nameFieldWidth;
    int lineHeight;
    int pageHeight;  
};

#endif // KARM_PRINT_H
