// #include <iostream>

#include <tqdatetime.h>
#include <tqpaintdevicemetrics.h>
#include <tqpainter.h>

#include <kglobal.h>
#include <klocale.h>            // i18n

#include "karmutility.h"        // formatTime()
#include "print.h"
#include "task.h"
#include "taskview.h"

const int levelIndent = 10;

MyPrinter::MyPrinter(const TaskView *taskView)
{
  _taskView = taskView;
}

void MyPrinter::print()
{
  // FIXME: make a better caption for the printingdialog
  if (setup(0L, i18n("Print Times"))) {
    // setup
    TQPainter painter(this);
    TQPaintDeviceMetrics deviceMetrics(this);
    TQFontMetrics metrics = painter.fontMetrics();
    pageHeight = deviceMetrics.height();
    int pageWidth = deviceMetrics.width();
    xMargin = margins().width();
    yMargin = margins().height();
    yoff = yMargin;
    lineHeight = metrics.height();
    
    // Calculate the totals
    // Note the totals are only calculated at the top most levels, as the
    // totals are increased together with its children.
    int totalTotal = 0;
    int sessionTotal = 0;
    for (Task* task = _taskView->first_child();
               task;
               task = static_cast<Task *>(task->nextSibling())) {
      totalTotal += task->totalTime();
      sessionTotal += task->totalSessionTime();
    }

    // Calculate the needed width for each of the fields
    timeWidth = QMAX(metrics.width(i18n("Total")),
                     metrics.width(formatTime(totalTotal)));
    sessionTimeWidth = QMAX(metrics.width(i18n("Session")),
                            metrics.width(formatTime(sessionTotal)));

    nameFieldWidth = pageWidth - xMargin - timeWidth - sessionTimeWidth - 2*5;
    
    int maxReqNameFieldWidth= metrics.width(i18n("Task Name "));
    
    for ( Task* task = _taskView->first_child();
          task;
          task = static_cast<Task *>(task->nextSibling()))
    {
      int width = calculateReqNameWidth(task, metrics, 0);
      maxReqNameFieldWidth = QMAX(maxReqNameFieldWidth, width);
    }
    nameFieldWidth = QMIN(nameFieldWidth, maxReqNameFieldWidth);

    int realPageWidth = nameFieldWidth + timeWidth + sessionTimeWidth + 2*5;

    // Print the header
    TQFont origFont, newFont;
    origFont = painter.font();
    newFont = origFont;
    newFont.setPixelSize( static_cast<int>(origFont.pixelSize() * 1.5) );
    painter.setFont(newFont);
    
    int height = metrics.height();
    TQString now = KGlobal::locale()->formatDateTime(TQDateTime::currentDateTime());
    
    painter.drawText(xMargin, yoff, pageWidth, height,
         TQPainter::AlignCenter, 
         i18n("KArm - %1").arg(now));
    
    painter.setFont(origFont);
    yoff += height + 10;

    // Print the second header.
    printLine(i18n("Total"), i18n("Session"), i18n("Task Name"), painter, 0);
    
    yoff += 4;
    painter.drawLine(xMargin, yoff, xMargin + realPageWidth, yoff);
    yoff += 2;
    
    // Now print the actual content
    for ( Task* task = _taskView->first_child();
                task;
                task = static_cast<Task *>(task->nextSibling()) )
    {
      printTask(task, painter, 0);
    }

    yoff += 4;
    painter.drawLine(xMargin, yoff, xMargin + realPageWidth, yoff);
    yoff += 2;
    
    // Print the Totals
    printLine( formatTime( totalTotal ),
               formatTime( sessionTotal ),
               TQString(), painter, 0);
  }
}

int MyPrinter::calculateReqNameWidth( Task* task,
                                      TQFontMetrics &metrics,
                                      int level) 
{
  int width = metrics.width(task->name()) + level * levelIndent;

  for ( Task* subTask = task->firstChild();
              subTask;
              subTask = subTask->nextSibling() ) {
    int subTaskWidth = calculateReqNameWidth(subTask, metrics, level+1);
    width = QMAX(width, subTaskWidth);
  }
  return width;
}

void MyPrinter::printTask(Task *task, TQPainter &painter, int level)
{
  TQString time = formatTime(task->totalTime());
  TQString sessionTime = formatTime(task->totalSessionTime());
  TQString name = task->name();
  printLine(time, sessionTime, name, painter, level);

  for ( Task* subTask = task->firstChild();
              subTask;
              subTask = subTask->nextSibling())
  {
    printTask(subTask, painter, level+1);
  }      
}

void MyPrinter::printLine( TQString total, TQString session, TQString name, 
                           TQPainter &painter, int level )
{
  int xoff = xMargin + 10 * level;
  
  painter.drawText( xoff, yoff, nameFieldWidth, lineHeight,
                    TQPainter::AlignLeft, name);
  xoff = xMargin + nameFieldWidth;
  
  painter.drawText( xoff, yoff, sessionTimeWidth, lineHeight,
                    TQPainter::AlignRight, session);
  xoff += sessionTimeWidth+ 5;
  
  painter.drawText( xoff, yoff, timeWidth, lineHeight,
                    TQPainter::AlignRight, total);
  xoff += timeWidth+5;

  yoff += lineHeight;
  
  if (yoff + 2* lineHeight > pageHeight) {
    newPage();
    yoff = yMargin;
  }
}
