/*
  This file is part of the PimPrint library.

  Copyright (C) 2012  Allen Winter <winter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

//
// arguments:
//   <D> printer() (mPrinter)
//   <D> style
//   <D> calendar
//   <D> calendarsystem
//   <D> flagsList
//
// properties:
//   <D>useColor()
//   <D>useLandscape()
//   <D>pageWidth()
//   <D>pageHeight()
//   <D>headerHeight()
//   <D>subheaderHeight()
//   <D>footerHeight()
//   TODO: headerBackgroundColor?
//   <D> margins()
//   <D> padding()
//   <D> timeLineWidth() (TIMELINE_WIDTH)
//   <D> boxBorderWidth() (BOX_BORDER_WIDTH)
//   <D> itemBoxBorderWidth() (EVENT_BORDER_WIDTH)
//
#ifndef PIMPRINT_CALPRINTBASE_H
#define PIMPRINT_CALPRINTBASE_H

#include "pimprint_calendar_export.h"
#include "cellitem.h"

#include <KCalCore/Calendar>
#include <KCalCore/Event>
#include <KCalCore/Incidence>

#include <QPainter>
#include <QPrinter>

class KCalendarSystem;

namespace PimPrint {

namespace Calendar {

class PIMPRINT_CALENDAR_EXPORT CalPrintBase : public QObject
{
  Q_OBJECT
  Q_ENUMS( Style )
  Q_FLAGS( InfoOptions )
  Q_FLAGS( TypeOptions )
  Q_FLAGS( RangeOptions )
  Q_FLAGS( ExtraOptions )

  Q_PROPERTY( bool useColor
              READ useColor WRITE setUseColor )

  Q_PROPERTY( bool useLandscape
              READ useLandscape WRITE setUseLandscape )

  Q_PROPERTY( int pageWidth
              READ pageWidth WRITE setPageWidth )

  Q_PROPERTY( int pageHeight
              READ pageHeight WRITE setPageHeight )

  Q_PROPERTY( int headerHeight
              READ headerHeight WRITE setHeaderHeight )

  Q_PROPERTY( int subheaderHeight
              READ subHeaderHeight WRITE setSubHeaderHeight )

  Q_PROPERTY( int footerHeight
              READ footerHeight WRITE setFooterHeight )

  Q_PROPERTY( int padding
              READ padding WRITE setPadding )

  Q_PROPERTY( int margins READ margins )

  Q_PROPERTY( int boxBorderWidth READ boxBorderWidth )

  Q_PROPERTY( int itemBoxBorderWidth READ itemBoxBorderWidth )

  Q_PROPERTY( int timeLineWidth READ timeLineWidth )

  public:
    /**
     * Constructor.
     */
    CalPrintBase( QPrinter *printer );

    /**
     * Destructor.
     */
    ~CalPrintBase();

    virtual void print( QPainter &p ) = 0;

    enum Style {
      None,               //< No print type is set
      Incidence,          //< Generalized for Events, To-dos and Journals
      DayFiloFax,         //<
      DayTimeTable,       //<
      DaySingleTimeTable, //<
      WeekFiloFax,        //<
      WeekTimeTable,      //<
      WeekSplitWeek,      //<
      MonthClassic,       //<
      TodoList,           //<
      Journal,            //< (how is this different from Incidence for Journals?)
      Year                //<
    };

    /**
     * Sets if a color print is desired.
     * @param useColor if true, then colors are used in the print; greyscale otherwise.
     * @see useColor()
     */
    void setUseColor( const bool useColor );

    /**
     * Returns if a color printing is currently set.
     * @see setUseColor()
     */
    bool useColor() const;

    /**
     * Sets the printed page orientation to landscape.
     * @param landscape if true use landscape when printing; portrait otherwise.
     * @see useLandscape()
     */
    void setUseLandscape( const bool landscape );

    /**
     * Returns if the current printed page orientation is landscape.
     * @see setUseLandscape()
     */
    bool useLandscape() const;

    /**
     * Sets the printed page height.
     * @param height is the height of the desired printed page.
     * @see pageHeight()
     */
    void setPageHeight( const int height ) const;

    /**
     * Returns the current printed page height.
     * @see setPageHeight().
     */
    int pageHeight() const;

    /**
     * Sets the printed page width.
     * @param width is the width of the desired printed page.
     * @see pageWidth()
     */
    void setPageWidth( const int width ) const;

    /**
     * Returns the current printed page width.
     * @see setPageWidth().
     */
    int pageWidth() const;

    /**
     * Sets the height of the page header.  If the height is not explicitly set,
     * a default value based on the printer orientation is used.
     * @see headerHeight()
     */
    void setHeaderHeight( const int height );

    /**
     * Returns the current height of the page header.  If a height has not been set
     * explicitly, then a default value based on the printer orientation is returned.
     * @return height of the page header of the printout.
     * @see setHeaderHeight()
     */
    int headerHeight() const;

    /**
     * Sets the height of the page sub-header.  If the height is not explicitly set,
     * a default value based on the printer orientation is used.
     * @see subHeaderHeight()
     */
    void setSubHeaderHeight( const int height );

    /**
     * Returns the current height of the page sub-header.  If a height has not been set
     * explicitly, then a default value based on the printer orientation is returned.
     * @return height of the page sub-header of the printout.
     * @see setSubHeaderHeight()
     */
    int subHeaderHeight() const;

    /**
     * Sets the height of the page footer.  If the height is not explicitly set,
     * a default value based on the printer orientation is used.
     * @see footerHeight()
     */
    void setFooterHeight( const int height );

    /**
     * Returns the current height of the page footer.  If a height has not been set
     * explicitly, then a default value based on the printer orientation is returned.
     * @return height of the page footer of the printout.
     * @see setFooterHeight()
     */
    int footerHeight() const;

    /**
     * Sets the padding margin.  If not explicitly set, a nice default value is used.
     * @see padding()
     */
    void setPadding( const int padding );

    /**
     * Returns the current padding margin.  If this value has not been set explicitly,
     * a nice default value is returned.
     * @return padding margin of of the printout.
     * @see setPadding()
     */
    int padding() const;

    /**
     * Returns the size of the page margins. Not settable.
     * The same margin is used on all 4 sides of the page.
     * A nice default value is used.
     */
    int margins() const;

    /**
     * Returns the width of box borders. Not settable.
     * A nice default value is used.
     */
    int boxBorderWidth() const;

    /**
     * Returns the width of item box borders. Not settable.
     * A nice default value is used.
     */
    int itemBoxBorderWidth() const;

    /**
     * Returns the width of timelines. Not settable.
     * A nice default value is used.
     */
    int timeLineWidth() const;

    enum InfoOption {
      InfoAll          = 0x00000001,
      InfoDescription  = 0x00000002,
      InfoLocation     = 0x00000004,
      InfoAttendees    = 0x00000008,
      InfoCategories   = 0x00000010,
      InfoTimeRange    = 0x00000020
    };
    Q_DECLARE_FLAGS( InfoOptions, InfoOption )

    enum TypeOption {
      TypeAll          = 0x00000001,
      TypeEvent        = 0x00000002,
      TypeTodo         = 0x00000004,
      TypeJournal      = 0x00000008,
      TypeConfidential = 0x00000010,
      TypePrivate      = 0x00000020
    };
    Q_DECLARE_FLAGS( TypeOptions, TypeOption )

    enum RangeOption {
      RangeTimeExpand  = 0x00000001
    };
    Q_DECLARE_FLAGS( RangeOptions, RangeOption )

    enum ExtraOption {
      ExtraSingleLine  = 0x00000001,
      ExtraNoteLines   = 0x00000002,
      ExtraFooter      = 0x00000004
    };
    Q_DECLARE_FLAGS( ExtraOptions, ExtraOption )

    /**
     * Sets the QPrinter.
     * @param printer is a pointer to a valid QPrinter where the printout will be made.
     * @see thePrinter()
     */
    void setThePrinter( QPrinter *printer );

    /**
     * Returns a pointer to the currently set QPrinter.
     * @see setThePrinter()
     */
    QPrinter *thePrinter() const;

    /**
     * Sets the calendar containing the data to print.
     * @param calendar is a pointer to a Calendar object containing the data to print.
     * @see printCalendar()
     */
    void setPrintCalendar( const KCalCore::Calendar::Ptr &calendar );

    /**
     * Returns a pointer to the currently set print calendar.
     * @see setPrintCalendar()
     */
    KCalCore::Calendar::Ptr printCalendar() const;

    /**
     * Sets the calendar system to use when printing.
     * @param calSystem is a pointer to a valid KCalendarSystem object.
     * @see calendarSystem()
     */
    void setCalendarSystem( KCalendarSystem *calSystem );

    /**
     * Returns a pointer to the currently set calendar system.
     * @see setCalendarSystem()
     */
    KCalendarSystem *calendarSystem() const;

    /**
     * Sets the printing Style.
     * @param style is the Style of print desired.
     * @see style()
     */
    void setPrintStyle( const Style style );

    /**
     * Returns the current printing Style.
     * @see setStyle()
     */
    Style printStyle() const;

    /**
     * Sets information options for printing.
     * @param flags bitwise ORd InfoOption flags.
     * @see infoFlags()
     */
    void setInfoOptions( InfoOptions flags );

    /**
     * Returns the current information option flags.
     * @see setInfoOptions()
     */
    InfoOptions infoOptions() const;

    /**
     * Sets type options for printing.
     * @param flags bitwise ORd TypeOption flags.
     * @see typeFlags()
     */
    void setTypeOptions( TypeOptions flags );

    /**
     * Returns the current type option flags.
     * @see setTypeOptions()
     */
    TypeOptions typeOptions() const;

    /**
     * Sets time-range options for printing.
     * @param flags bitwise ORd RangeOption flags.
     * @see rangeFlags()
     */
    void setRangeOptions( RangeOptions flags );

    /**
     * Returns the current time-range option flags.
     * @see setRangeOptions()
     */
    RangeOptions rangeOptions() const;

    /**
     * Sets extra options for printing.
     * @param flags bitwise ORd ExtraOption flags.
     * @see extraFlags()
     */
    void setExtraOptions( ExtraOptions flags );

    /**
     * Returns the current extra option flags.
     * @see setExtraOptions()
     */
    ExtraOptions extraOptions() const;

  protected:
    /**
     * Draws the gray header bar of the printout to the QPainter.
     * It prints the given text and optionally one or two small month views, as
     * specified by the two QDate. The printed text can also contain a line feed.
     * If month2 is invalid, only the month that contains month1 is printed.
     * E.g. the filofax week view draws just the current month, while the month
     * view draws the previous and the next month.
     *
     * @param p QPainter of the printout.
     * @param box coordinates of the header box.
     * @param title The string printed as the title of the page.
     * @param leftMonth The month-year to draw for the left one of the small month
     *        views in the title bar.
     *        If an invalid QDate, the left small month view will not be printed.
     * @param rightMonth The month-year to draw for the right one of the small month
     *        views in the title bar.
     *        If an invalid QDate, the right small month view will not be printed.
     * @param expand Whether to expand the box vertically to fit the
     *        whole title in it.
     * @param backColor background color for the header box.
     *
     * @return The bottom of the printed box. If expand==false, this
     *         is box.bottom, otherwise it is larger than box.bottom
     *         and matches the y-coordinate of the surrounding rectangle.
     */
    int drawHeader( QPainter &p,
                    const QRect &box,
                    const QString &title,
                    const QDate &leftMonth = QDate(),
                    const QDate &rightMonth = QDate(),
                    const bool expand = false,
                    const QColor &backColor = QColor() ) const;
    /**
     * Draws a subheader with a shaded background and the specified string.
     *
     * @param p QPainter of the printout.
     * @param box Coordinates of the box.
     * @param str Text to be printed inside the box.
     */
    void drawSubHeader( QPainter &p, const QRect &box, const QString &str ) const;

    /**
     * Draws a page footer containing the printing date and possibly other things,
     * like a page number.
     *
     * @param p QPainter of the printout
     * @param box coordinates of the footer box.
     * @return The bottom of the printed box.
     */
    int drawFooter( QPainter &p, const QRect &box ) const;

    /**
     * Draws a box with given width at the given coordinates.
     *
     * @param p The printer to be used.
     * @param linewidth The border width of the box.
     * @param box Coordinates of the box.
     */
  //TODO: make box second arg
    void drawBox( QPainter &p, const int linewidth, const QRect &box ) const;

    /**
     * Draws a shaded box with given width at the given coordinates.
     *
     * @param p The printer to be used.
     * @param linewidth The border width of the box.
     * @param brush The brush to fill the box.
     * @param box Coordinates of the box.
     */
  //TODO: make box second arg
    void drawShadedBox( QPainter &p, const int linewidth,
                        const QBrush &brush, const QRect &box ) const;

    /**
     * Draws an event box with vertical text.
     *
     * @param p QPainter of the printout
     * @param linewidth is the width of the line used to draw the box, ignored if less than 1.
     * @param box Coordinates of the box
     * @param str ext to be printed inside the box
     * @param flags is a bitwise OR of Qt::AlignmentFlags and Qt::TextFlags values.
     */
  //TODO: make box second arg
    void drawVerticalBox( QPainter &p,
                          const int linewidth,
                          const QRect &box,
                          const QString &str,
                          int flags = -1 ) const;

    /**
     * Draws the box for the specified item with the given string.
     *
     * @param p QPainter of the printout.
     * @param linewidth is the width of the line used to draw the box.
     * @param box Coordinates of the incidence's box.
     * @param incidence The incidence (if available), from which the category color
     *        will be deduced, if applicable.
     * @param str The string to print inside the box.
     * @param flags is a bitwise OR of Qt::AlignmentFlags and Qt::TextFlags values.
     */
  //TODO: make box the second arg
    void drawItemBox( QPainter &p,
                      int linewidth,
                      const QRect &box,
                      const KCalCore::Incidence::Ptr &incidence,
                      const QString &str,
                      int flags = -1 ) const;

    /**
     * Draws the given string (incidence summary) in the given rectangle.
     * Margins and justification (centered or not) are automatically adjusted.
     *
     * @param p QPainter of the printout.
     * @param box Coordinates of the surrounding item box.
     * @param str The text to be printed in the box.
     */
    void drawItemString( QPainter &p, const QRect &box,
                         const QString &str, int flags = -1 ) const;

    /**
     * Draws the box containing a list of all events and to-dos of the given day.
     * (with their timesof course). Used in the Filofax and the month print style.
     *
     * @param p QPainter of the printout.
     * @param date The date of the currently printed day.
     *        All events and to-dos occurring on this day will be printed.
     * @param startTime Start time of the time range to display.
     * @param endTime End time of the time range to display.
     * @param box coordinates of the day box.
     * @param fullDate Whether the title bar of the box should contain the full
     *        date string or just a short.
     * @param printRecurDaily Whether daily recurring incidences should be printed.
     * @param printRecurWeekly Whether weekly recurring incidences should be printed.
     */
  //TODO: make box second arg
    void drawDayBox( QPainter &p, const QDate &date,
                     const QTime &startTime, const QTime &endTime,
                     const QRect &box,
                     bool fullDate = false,
                     bool printRecurDaily = true,
                     bool printRecurWeekly = true ) const;

    /**
     * Draws the agenda box.
     *
     * Also draws a grid with half-hour spacing of the grid lines.
     * Does NOT draw allday events.  Use drawAllDayBox for allday events.
     *
     * @param p QPainter of the printout
     * @param date The date of the currently printed day
     * @param eventList The list of the events that are supposed to be printed
     *        inside this box
     * The height of the box will not be affected by this (but the height of one hour
     * will be scaled down so that the whole range fits into  by this function).
     *
     * @param startTime Start of the time range to be printed.
     *        Might be adjusted to include all events if @p expandable is set to true.
     * @param endTime End of the time range to be printed.
     *        Might be adjusted to include all events if @p expandable is set to true.
     *
     * @param workDays List of workDays
     */
    void drawAgendaDayBox( QPainter &p,
                           const QRect &box,
                           const QDate &date,
                           const KCalCore::Event::List &eventList,
                           const QTime &startTime,
                           const QTime &endTime,
                           const QList<QDate> &workDays ) const;

  //TODO: make QPainter the first arg
    void drawAgendaItem( PrintCellItem *item, QPainter &p,
                         const KDateTime &startPrintDate,
                         const KDateTime &endPrintDate,
                         float minlen, const QRect &box ) const;

    /**
     * Draws a (vertical) time scale from time startTime to endTime inside the
     * given area of the painter. Every hour will have a one-pixel line over
     * the whole width, every half-hour the line will only span the left half
     * of the width. This is used in the day and timetable print styles
     *
     * @param p QPainter of the printout.
     * @param startTime Start time of the time range to display.
     * @param endTime End time of the time range to display.
     * @param box coordinates of the timeline.
     */
  //TODO: make box the second arg
    void drawTimeLine( QPainter &p,
                       const QTime &startTime,
                       const QTime &endTime,
                       const QRect &box ) const;

    /**
     * Draws the timetable view of the given time range from startDate to endDate.
     * On the left side the time scale is printed (using drawTimeLine), then each
     * day gets one column (printed using drawAgendaDayBox), and the events are
     * displayed as boxes (like in korganizer's day/week view).
     *
     * The first cell of each column contains the all-day events (using
     * drawAllDayBox with expandable=false).
     *
     * @param p QPainter of the printout.
     * @param box coordinates of the time table.
     * @param startDate First day to be included in the page.
     * @param endDate Last day to be included in the page.
     * @param startTime Start time of the displayed time range.
     * @param endTime End time of the displayed time range.
     * @param expandAll If true, the start and end times are adjusted to include the
     *        whole range of all events of that day, not just of the given time range.
     */
    void drawTimeTable( QPainter &p, const QRect &box,
                        const QDate &startDate, const QDate &endDate,
                        const QTime &startTime, const QTime &endTime,
                        bool expandAll = false ) const;

    /**
     * Determines the column of the given weekday ( 1=Monday, 7=Sunday ), taking the
     * start of the week setting into account as given in the user's locale.
     * @param weekday Index of the weekday
     */
    int weekdayColumn( int weekday ) const;

    /**
     * Cleans a string of newlines and other characters that shouldn't be printed.
     * The unwanted chars are either replaces with white-space are simply removed.
     *
     * @param str is the QString to clean.
     * @return a QString with the unwanted characters replaced or removed.
     */
    QString cleanString( const QString &str ) const;

    /**
     * Converts possible rich text to plain text.
     *
     * @param htmlText is a QString containing possible RichText to convert.
     * @return a plain text string representation of the input string.
     * @see cleanStr()
     */
    QString toPlainText( const QString &htmlText ) const;

  private:
  //TODO: move to dpointer
    /**
     * Initializes the QPainter, page height and width, etc.
     */
    void init( QPrinter *printer ) const;

    /**
     * De-Initializes everything.
     */
    void finish() const;

    /**
     * Draws a small calendar with the days of a month into the given area.
     * Used for example in the header of some print styles.
     *
     * @param p QPainter of the printout.
     * @param date Arbitrary Date within the month to be printed.
     * @param box coordinates of the small calendar.
     */
    void drawSmallMonth( QPainter &p, const QDate &date, const QRect &box ) const;

    /**
     * Draws dotted lines for notes in a box.
     *
     * @param p QPainter of the printout.
     * @param box coordinates of the box where the lines will be placed.
     * @param startY starting y-coordinate for the first line.
     */
    void drawNoteLines( QPainter &p, const QRect &box, const int startY ) const;

    void drawDayIncidence( QPainter &p, const QRect &dayBox, const QString &time,
                           const QString &summary, const QString &description,
                           int &textY, bool richDescription ) const;

    QColor categoryColor( const QStringList &categories ) const;

    QColor categoryBgColor( const KCalCore::Incidence::Ptr &incidence ) const;

    /**
     * Sets the QPainter's brush and pen color according to the Incidence's category.
     */
    void setColorsByIncidenceCategory( QPainter &p,
                                       const KCalCore::Incidence::Ptr &incidence ) const;

    QString holidayString( const QDate &date ) const;

    KCalCore::Event::Ptr holidayEvent( const QDate &date ) const;

    /**
     * Returns a nice QColor for text, give the input color &c.
     */
    QColor getTextColor( const QColor &c ) const;

    /**
      Draws a horizontal bar with the weekday names of the given date range
      in the given area of the painter.
      This is used for the weekday-bar on top of the timetable view and the month view.

      @param p QPainter of the printout
      @param box coordinates of the box for the days of the week
      @param fromDate First date of the printed dates
      @param toDate Last date of the printed dates
    */
    void drawDaysOfWeek( QPainter &p, const QRect &box,
                         const QDate &fromDate, const QDate &toDate ) const;

    /**
      Draws a single weekday name in a box inside the given area of the painter.
      This is called in a loop by drawDaysOfWeek.

      @param p QPainter of the printout
      @param box coordinates of the weekbox
      @param date Date of the printed day
    */
    void drawDaysOfWeekBox( QPainter &p, const QRect &box, const QDate &date ) const;

    /**
      Draws the all-day box for the agenda print view (the box on top which
      doesn't have a time on the time scale associated). If expandable is set,
      height is the cell height of a single cell, and the returned height will
      be the total height used for the all-day events. If !expandable, only one
      cell will be used, and multiple events are concatenated using ", ".

      @param p QPainter of the printout
      @param box coordinates of the all day box.
      @param qd The date of the currently printed day
      @param eventList The list of all-day events that are supposed to be printed
             inside this box
      @param expandable If true, height is the height of one single cell, the printout
             will use as many cells as events in the list and return the total height
             needed for all of them. If false, height specifies the total height
             allowed for all events, and the events are displayed in one cell,
             with their summaries concatenaated by ", ".

      @return The height used for the all-day box.
    */
    int drawAllDayBox( QPainter &p, const QRect &box,
                       const QDate &date,
                       const KCalCore::Event::List &eventList,
                       bool expandAll = false ) const;

  private:
    Q_DISABLE_COPY( CalPrintBase )
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

Q_DECLARE_OPERATORS_FOR_FLAGS( CalPrintBase::InfoOptions )
Q_DECLARE_OPERATORS_FOR_FLAGS( CalPrintBase::TypeOptions )
Q_DECLARE_OPERATORS_FOR_FLAGS( CalPrintBase::RangeOptions )
Q_DECLARE_OPERATORS_FOR_FLAGS( CalPrintBase::ExtraOptions )

}

}

#if 0
  public:
    enum DisplayFlags {
      Text=0x0001,
      TimeBoxes=0x0002
    };

  public:
      Actually do the printing.

      @param p QPainter the print result is painted to
      @param width Width of printable area
      @param height Height of printable area
    */
    virtual void print( QPainter &p, int width, int height ) = 0;
    /**
      Start printing.
    */
    virtual void doPrint( QPrinter *printer );

  public:
    bool printFooter() const;
    void setPrintFooter( bool printFooter );

    /** Helper functions to hide the KOrg::CoreHelper */
    QTime dayStart();

    int margin() const;
    void setMargin( const int margin );

  /*****************************************************************
   **               PRINTING HELPER FUNCTIONS                     **
   *****************************************************************/

  public:

    /**
     * Draws a component box with a heading (printed in bold).
     *
     * @param p QPainter of the printout
     * @param box Coordinates of the box
     * @param caption Caption string to be printed inside the box
     * @param contents Normal text contents of the box. If contents.isNull(),
     *        then no text will be printed, only the caption.
     * @param sameLine Whether the contents should start on the same line as the
     *        caption (the space below the caption text will be used as indentation
     *        in the subsequent lines) or on the next line (no indentation of the
     *        contents).
     * @param expand Whether to expand the box vertically to fit the whole text in it.
     * @param richContents Whether contents contains rich text.
     *
     * @return The bottom of the printed box. If expand==true, the bottom of the drawn
     *         box is returned, if expand is false, the vertical end of the printed
     *         contents inside the box is returned.  If you want to print some custom
     *         graphics or text below the contents, use the return value as the
     *         top-value of your custom contents in that case.
     */
    int drawBoxWithCaption( QPainter &p, const QRect &box, const QString &caption,
                            const QString &contents, bool sameLine, bool expand,
                            const QFont &captionFont, const QFont &textFont,
                            bool richContents = false );
};

}

}
#endif //if 0

#endif
