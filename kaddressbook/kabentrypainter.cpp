/* -*- C++ -*-
   This file implements kab´s printing methods. It is a part of the
   TopLevelWidget implementation.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

// #include "kab_mainwidget.h"
#include "kabentrypainter.h"
#include "detailledview/look_basic.h"

#include <kprinter.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>


#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <knotifyclient.h>
#include <kurl.h>

KABEntryPainter::KABEntryPainter(QColor foreColor_,
                                 QColor headerColor_,
                                 bool useHeaderColor_,
                                 QColor backColor_,
                                 QFont header_,
                                 QFont headlines_,
                                 QFont body_,
                                 QFont fixed_,
                                 QFont comment_,
                                 bool showAddresses_,
                                 bool showEmails_,
                                 bool showTelephones_,
                                 bool showURLs_)
  : foreColor(foreColor_),
    headerColor(headerColor_),
    useHeaderColor(useHeaderColor_),
    backColor(backColor_),
    header(header_),
    headlines(headlines_),
    body(body_),
    fixed(fixed_),
    comment(comment_),
    showAddresses(showAddresses_),
    showEmails(showEmails_),
    showTelephones(showTelephones_),
    showURLs(showURLs_)
{
}

KABEntryPainter::~KABEntryPainter()
{
  emails.clear();
  telephones.clear();
  URLs.clear();
}

void KABEntryPainter::setShowAddresses(bool b)
{
    showAddresses=b;
}

void KABEntryPainter::setShowEmails(bool b)
{
    showEmails=b;
}

void KABEntryPainter::setShowTelephones(bool b)
{
    showTelephones=b;
}

void KABEntryPainter::setShowURLs(bool b)
{
    showURLs=b;
}

int KABEntryPainter::hitsEmail(QPoint p)
{
    return hits(emails, p);
}

int KABEntryPainter::hitsURLs(QPoint p)
{
    return hits(URLs, p);
}
int KABEntryPainter::hitsTelephones(QPoint p)
{
    return hits(telephones, p);
}
int KABEntryPainter::hitsTalkAddresses(QPoint p)
{
    return hits(talk, p);
}

int KABEntryPainter::hits(const QRectList& list, QPoint p)
{
    QRectList::const_iterator pos;
    int count=0;
    // -----
    for(pos=list.begin(); pos!=list.end(); ++pos)
    {
        if((*pos).contains(p))
        {
            return count;
        }
        ++count;
    }
    return -1;
}

bool KABEntryPainter::printEntry(const KABC::Addressee& entry,
                                 const QRect& window,
                                 QPainter *painter,
                                 int top, bool fake, QRect *brect)
{
    // TODO: custom fields, custom (?) for Entry
    const int Width=window.width();
    const int Height=window.height();
    const int Ruler1=Width/32;
    const int Ruler2=2*Ruler1;
    const int Ruler3=3*Ruler1;
    QString text, line1, line2, line3, line4;
    QRect rect;
    // settings derived from the options:
    QFontMetrics fmheader(header);
    QFontMetrics fmheadline(headlines);
    QFontMetrics fmbody(body);
    QFontMetrics fmfixed(fixed);
    QFontMetrics fmcomment(comment);
    int y=top;
    KABC::Address address;
    // this is used to prepare some fields for printing and decide about
    // the layout later:
    QValueList<QStringList> parts;
    QValueList<QRectList*> contents;

    emails.clear(); telephones.clear(); URLs.clear();

    // ----- set window the painter works on:
    painter->setWindow(window);
    // ----- first draw a black rectangle on top, containing the entries
    //       name, centered:
    painter->setFont(header);
    painter->setBrush(QBrush(backColor));
    painter->setPen(backColor);
    text=entry.realName();
    // replacement for: api->addressbook()->literalName(entry, text);
    rect=painter->boundingRect(Ruler1, y, Width, Height,
                               Qt::AlignVCenter | Qt::AlignLeft, text);
    rect.setHeight((int)(1.25*rect.height()));
    if(!fake && useHeaderColor)
    {
        painter->drawRect(0, y, Width, rect.height());
    }
    painter->setPen(useHeaderColor ? headerColor : foreColor);
    if(!fake)
    {
        // create a little (1/8) space on top of the letters:
        float ypos=y+((float)rect.height())*0.125;
        painter->drawText(Ruler1, (int)ypos, Width, rect.height(),
                          Qt::AlignVCenter | Qt::AlignLeft, text);
    }
    //       paint the birthday to the right:
    QDateTime dt=entry.birthday();
    if(dt.isValid())
    {
        line1=KGlobal::locale()->formatDate(dt.date(), true);
        if(!fake)
        {
            // create a little (1/8) space on top of the letters:
            float ypos=y+((float)rect.height())*0.125;
            painter->drawText(0, (int)ypos, Width-Ruler1, rect.height(),
                              Qt::AlignVCenter | Qt::AlignRight, line1);
        }
    }
    y+=rect.height();
    // ----- now draw the data according to the person:
    painter->setFont(body);
    y+=fmbody.lineSpacing()/2;
    painter->setPen(foreColor);
    if(!entry.prefix().isEmpty())
    {
        line1=entry.prefix();
        line1=line1.stripWhiteSpace();
        if(fake)
        {
            rect=painter->boundingRect(Ruler1, y, Width-Ruler1, Height,
                                       Qt::AlignTop | Qt::AlignLeft,
                                       line1);
        } else {
            painter->drawText(Ruler1, y, Width-Ruler1, Height,
                              Qt::AlignTop | Qt::AlignLeft,
                              line1, -1, &rect);
        }
        y+=rect.height();
    }
    /*
      if(!entry.title.isEmpty())
      {
      line2=entry.title;
      line2=line2.stripWhiteSpace();
      if(fake)
      {
      rect=painter->boundingRect(Ruler1, y, Width-Ruler1, Height,
      Qt::AlignTop | Qt::AlignLeft,
      line2);
      } else {
      painter->drawText(Ruler1, y, Width-Ruler1, Height,
      Qt::AlignTop | Qt::AlignLeft,
      line2, -1, &rect);
      }
      y+=rect.height();
    }
    */
    if(!(entry.prefix().isEmpty()))    {
        y+=fmbody.lineSpacing()/2;
    }
    // ----- fill the parts stringlist, it contains "parts" (printable
    // areas) that will be combined to fill the page as effectively as
    // possible:
    //       Email addresses:
    if(!entry.emails().isEmpty() && showEmails)
    {
        contents.push_back(&emails);
        QStringList list;
        // -----
        list.append(entry.emails().count()==1
                    ? i18n("Email address:")
                    : i18n("Email addresses:"));
        list+=entry.emails();
        parts.push_back(list);
    }
    //       Telephones:
    KABC::PhoneNumber::List phoneNumbers=entry.phoneNumbers();
    if(!phoneNumbers.isEmpty() && showTelephones)
    {
        contents.push_back(&telephones);
        QStringList list;
        QString line;
        // -----
        list.append(phoneNumbers.count()==1
                    ? i18n("Telephone:")
                    : i18n("Telephones:"));
        // ----- this is more complex:
        for(KABC::PhoneNumber::List::Iterator it=phoneNumbers.begin();
            it!=phoneNumbers.end();
            ++it)
        {
            line=(*it).typeLabel();
            line+=": " + (*it).number();
            list.append(line.stripWhiteSpace());
        }
        parts.push_back(list);
    }
    //       Web pages/URLs:
    if(!entry.url().isEmpty() && entry.url().isValid() && showURLs)
    {
        contents.push_back(&URLs);
        QStringList list;
        // -----
//         list.append(entry.URLs.count()==1
//                     ? i18n("Web page:")
//                     : i18n("Web pages:"));
        list.append(i18n("Web page:"));
        list+=entry.url().prettyURL();
        parts.push_back(list);
    }
    /*
    //       Talk addresses:
    if(!entry.talk.isEmpty())
    {
    contents.push_back(&talk);
    QStringList list;
    // -----
    list.append(entry.talk.count()==1
    ? i18n("Talk address:")
    : i18n("Talk addresses:"));
    list+=entry.talk;
    parts.push_back(list);
    }
    */
    // -----
    QRect limits[]= {
        QRect(0, y, Width/2, Height),
        QRect(Width/2, y, Width/2, Height),
        QRect(0, y, Width/2, Height),
        QRect(Width/2, y, Width/2, Height) };
    int heights[4]= { 0, 0, 0, 0 };
    // -----
    QValueList<QStringList>::iterator pos=parts.begin();
    QValueList<QRectList*>::iterator rpos=contents.begin();
    for(unsigned counter=0; counter<parts.count(); ++counter)
    {
        const int Offset=counter>1 ? QMAX(heights[0], heights[1]) : 0;
        QStringList list=*pos;
        // -----
        painter->setFont(headlines);
        if(fake)
        {
            rect=painter->boundingRect
                 (limits[counter].left(),
                  limits[counter].top()+heights[counter]+Offset,
                  limits[counter].width(),
                  limits[counter].height(),
                  Qt::AlignTop | Qt::AlignLeft,
                  *list.at(0));
        } else {
            painter->drawText
                (limits[counter].left(),
                 limits[counter].top()+heights[counter]+Offset,
                 limits[counter].width(),
                 limits[counter].height(),
                 Qt::AlignTop | Qt::AlignLeft,
                 *list.at(0), -1, &rect);
        }
        heights[counter]+=rect.height();
        // ----- paint the other elements at Ruler1:
        painter->setFont(fixed);
        for(unsigned c2=1; c2<list.count(); ++c2)
        { // @todo: implement proper line breaking!
            if(fake)
            {
                rect=painter->boundingRect
                     (limits[counter].left()+Ruler1,
                      limits[counter].top()+heights[counter]+Offset,
                      limits[counter].width()-Ruler1,
                      limits[counter].height(),
                      Qt::AlignTop | Qt::AlignLeft,
                      *list.at(c2));
            } else {
                painter->drawText
                    (limits[counter].left()+Ruler1,
                     limits[counter].top()+heights[counter]+Offset,
                     limits[counter].width()-Ruler1,
                     limits[counter].height(),
                     Qt::AlignTop | Qt::AlignLeft,
                     *list.at(c2), -1, &rect);
            }
            (*rpos)->push_back(rect);
            heights[counter]+=rect.height();
        }
        // ----- done
        ++pos;
        ++rpos;
    }
    y=y+QMAX(heights[0], heights[1])+QMAX(heights[2], heights[3]);
    // ^^^^^ done with emails, telephone, URLs and talk addresses
    // ----- now print the addresses:
    KABC::Address::List addresses=entry.addresses();
    if(addresses.count()>0 && showAddresses)
    {
        y+=fmbody.lineSpacing()/2;
        painter->setFont(headlines);
        if(fake)
        {
            rect=painter->boundingRect
                 (0, y, Width, Height,
                  Qt::AlignTop | Qt::AlignLeft,
                  addresses.count()==1 ? i18n("Address:") : i18n("Addresses:"));
        } else {
            painter->drawText(0, y, Width, Height,
                              Qt::AlignTop | Qt::AlignLeft,
                              addresses.count()==1
                              ? i18n("Address:") : i18n("Addresses:"),
                              -1, &rect);
        }
        y+=rect.height();
        y+=fmbody.lineSpacing()/4;
        painter->setFont(body);
        for(KABC::Address::List::iterator it=addresses.begin();
            it!=addresses.end();
            ++it)
        {
            bool org; org=false;
            address=*it;
            switch(address.type())
            {
            case KABC::Address::Dom: // domestic
                line1=i18n("Domestic Address");
                break;
            case KABC::Address::Intl: // domestic
                line1=i18n("International Address");
                break;
            case KABC::Address::Postal:
                line1=i18n("Postal Address");
                break;
            case KABC::Address::Parcel:
                line1=i18n("Parcel Address");
                break;
            case KABC::Address::Home:
                line1=i18n("Home Address");
                break;
            case KABC::Address::Work:
                line1=i18n("Work Address");
                break;
            case KABC::Address::Pref:
            default:
                line1=i18n("Preferred Address");
            };
            line1+=QString::fromLatin1(":");
            text=QString::null;
            /*
              if(!address.position.isEmpty())
              {
              text+=address.position;
              }
              if(!address.org.isEmpty())
              {
              if(!text.isEmpty())
              {
              text+=", ";
              }
              text+=address.org;
              org=true;
              }
              if(!address.orgUnit.isEmpty())
              {
              if(!org)
              {
              text+=", ";
              } else {
              text+=" / ";
              }
              text+=address.orgUnit;
              org=true;
              }
              if(!address.orgSubUnit.isEmpty())
              {
              if(!org)
              {
              text+=", ";
              } else {
              text+=" / ";
              }
              text+=address.orgSubUnit;
              org=true;
              }
            */
            if(!address.extended().isEmpty())
            {
                text=address.extended();
            }
            text=text.stripWhiteSpace();
            if(!text.isEmpty())
            {
                line1=line1+QString::fromLatin1(" (")
                      +text+QString::fromLatin1(")");
            }
            line1=line1.stripWhiteSpace();
            line2=address.street();
            if(!address.postOfficeBox().isEmpty())
            {
                line2+=QString::fromLatin1(" - ") + address.postOfficeBox();
            }
            // ----- print address in american style, this will need
            // localisation:
            line3=address.locality()
                  +(address.region().isEmpty()
                    ? QString::fromLatin1("")
                    : QString::fromLatin1(", ")+address.region())
                  +(address.postalCode().isEmpty()
                    ? QString::fromLatin1("")
                    : QString::fromLatin1(" ")+address.postalCode());
            line4=address.country();
            // -----
            /*
              if(line1.isEmpty())
              {
              line1=i18n("Unnamed address:");
              }
            */
            if(fake)
            {
                rect=painter->boundingRect(Ruler1, y, Width-Ruler1, Height,
                                           Qt::AlignTop | Qt::AlignLeft,
                                           line1);
            } else {
                painter->drawText(Ruler1, y, Width-Ruler1, Height,
                                  Qt::AlignTop | Qt::AlignLeft,
                                  line1, -1, &rect);
            }
            y+=rect.height();
            if(!line2.isEmpty())
            {
                if(fake)
                {
                    rect=painter->boundingRect(Ruler2, y, Width-Ruler2, Height,
                                               Qt::AlignTop | Qt::AlignLeft,
                                               line2);
                } else {
                    painter->drawText(Ruler2, y, Width-Ruler2, Height,
                                      Qt::AlignTop | Qt::AlignLeft,
                                      line2, -1, &rect);
                }
                y+=rect.height();
            }
            if(!line3.isEmpty())
            {
                if(fake)
                {
                    rect=painter->boundingRect(Ruler2, y, Width-Ruler2, Height,
                                               Qt::AlignTop | Qt::AlignLeft,
                                               line3);
                } else {
                    painter->drawText(Ruler2, y, Width-Ruler2, Height,
                                      Qt::AlignTop | Qt::AlignLeft,
                                      line3, -1, &rect);
                }
                y+=rect.height();
            }
            if(!line4.isEmpty())
            {
                if(fake)
                {
                    rect=painter->boundingRect(Ruler2, y, Width-Ruler2, Height,
                                               Qt::AlignTop | Qt::AlignLeft,
                                               line4);
                } else {
                    painter->drawText(Ruler2, y, Width-Ruler2, Height,
                                      Qt::AlignTop | Qt::AlignLeft,
                                      line4, -1, &rect);
                }
                y+=rect.height();
            }
            y+=fmbody.lineSpacing()/4;
            if(!address.label().isEmpty())
            {
                if(fake)
                {
                    rect=painter->boundingRect(Ruler2, y, Width-Ruler2, Height,
                                               Qt::AlignTop | Qt::AlignLeft,
                                               i18n("(Deliver to:)"));
                } else {
                    painter->drawText(Ruler2, y, Width-Ruler2, Height,
                                      Qt::AlignTop | Qt::AlignLeft,
                                      i18n("(Deliver to:)"), -1, &rect);
                }
                y+=rect.height();
                y+=fmbody.lineSpacing()/4;
                if(fake)
                {
                    rect=painter->boundingRect(Ruler3, y, Width-Ruler3, Height,
                                               Qt::AlignTop | Qt::AlignLeft,
                                               address.label());
                } else {
                    painter->drawText(Ruler3, y, Width-Ruler3, Height,
                                      Qt::AlignTop | Qt::AlignLeft,
                                      address.label(), -1, &rect);
                }
                y+=rect.height();
                y+=fmbody.lineSpacing()/2;
            }
        }
    }
    if(!entry.note().isEmpty())
    {
        painter->setFont(comment);
        y+=fmbody.lineSpacing()/2;
        if(fake)
        {
            rect=painter->boundingRect(0, y, Width, Height,
                                       Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak,
                                       entry.note());
        } else {
            painter->drawText(0, y, Width, Height,
                              Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak,
                              entry.note(), -1, &rect);
        }
        y+=rect.height();
    }
    y+=fmbody.lineSpacing()/2;
    // ----- we are done:
    if(brect!=0)
    {
        *brect=QRect(0, top, Width, y-top);
    }
    if(y<Height)
    {
        return true;
    } else {
        return false;
    }
    return true;
}



/*
  bool TopLevelWidget::printEntries(const BunchOfKeys keys)
  {
  KPrinter printer;
  QPainter painter;
  // QRect rect;
  // ----- variables used to define MINIMAL MARGINS entered by the user:
  int marginTop=0,
  marginLeft=64, // to allow stapling
  marginRight=0,
  marginBottom=0;
  register int left, top, width, height;
  // -----
  if(!printer.setup(this))
  {
  emit(setStatus(i18n("Rejected.")));
  KNotifyClient::beep();
  return false;
  }
  // ----- create the metrics in accordance to the selection:
  printer.setFullPage(true); // use whole page
  QPaintDeviceMetrics metrics(&printer);
  kdDebug() << "TopLevelWidget::slotPrintEntries: printing on a "
  << metrics.width() << "x" << metrics.height()
  << " size area," << endl << "   "
  << "margins are "
  << printer.margins().width() << " (left/right) and "
  << printer.margins().height() << " (top/bottom)." << endl;
  left=QMAX(printer.margins().width(), marginLeft); // left margin
  top=QMAX(printer.margins().height(), marginTop); // top margin
  width=metrics.width()-left
  -QMAX(printer.margins().width(), marginRight); // page width
  height=metrics.height()-top
  -QMAX(printer.margins().height(), marginBottom); // page height
  // -----
  painter.begin(&printer);
  painter.setViewport(left, top, width, height);
  return printEntries(keys, &printer, &painter,
  QRect(0, 0, metrics.width(), metrics.height()));
  painter.end();
  }

  bool TopLevelWidget::printEntries(const BunchOfKeys keys,
  KPrinter *printer,
  QPainter *painter,
  const QRect& window)
  {
  BunchOfKeys::const_iterator pos;
  AddressBook::Entry entry;
  QRect brect;
  int ypos=0;
  KABEntryPainter p(Qt::black, Qt::white, Qt::black,
  QFont("Utopia", 12, QFont::Normal, true),
  QFont("Utopia", 10, QFont::Bold, true),
  QFont("Utopia", 10, QFont::Normal, false),
  QFont("Helvetica", 10, QFont::Normal, false),
  QFont("Utopia", 8, QFont::Normal, false));
  // -----
  for(pos=keys.begin(); pos!=keys.end(); ++pos)
  {
  if(api->addressbook()->getEntry(*pos, entry)==AddressBook::NoError)
  {
  // ----- do a faked print to get the bounding rect:
  if(!p.printEntry(entry, window, painter, api, ypos, true, &brect))
  { // it does not fit on the page beginning at ypos:
  printer->newPage();
  ypos=0; // WORK_TO_DO: this assumes the entry fits on the whole page
  }
  p.printEntry(entry, window, painter, api, ypos, false, &brect);
  ypos+=brect.height();
  } else {
  kdDebug() << "TopLevelWidget::printEntries: cannot get entry "
  << (*pos).getKey() << ", skipping." << endl;
  }
  }
  return true;
  }

  void TopLevelWidget::slotPrintEntries()
  { // this slot prints all entries in the order they are displayed
  BunchOfKeys bunch;
  // -----
  KabKey key;
  for(unsigned counter=0; counter<api->addressbook()->noOfEntries(); ++counter)
  {
  if(api->addressbook()->getKey(counter, key)!=AddressBook::NoError)
  {
  kdDebug() << "TopLevelWidget::slotPrintEntries: cannot get entry at "
  << "index " << counter << "!" << endl;
  } else {
  bunch.push_back(key);
  }
  }
  printEntries(bunch);
  }

  void TopLevelWidget::slotPrintEntry()
  {
  KPrinter printer;
  QPainter painter;
  AddressBook::Entry entry;
  KABEntryPainter p(Qt::black, Qt::white, Qt::black,
  QFont("Utopia", 12, QFont::Normal, true),
  QFont("Utopia", 10, QFont::Bold, true),
  QFont("Utopia", 10, QFont::Normal, false),
  QFont("Helvetica", 10, QFont::Normal, false),
  QFont("Utopia", 8, QFont::Normal, false));
  // ----- variables used to define MINIMAL MARGINS entered by the user:
  int marginTop=0,
  marginLeft=64, // to allow stapling
  marginRight=0,
  marginBottom=0;
  register int left, top, width, height;
  // -----
  if(!printer.setup(this))
  {
  emit(setStatus(i18n("Rejected.")));
  KNotifyClient::beep();
  return;
  }
  // ----- create the metrics in accordance to the selection:
  printer.setFullPage(true); // use whole page
  QPaintDeviceMetrics metrics(&printer);
  kdDebug() << "TopLevelWidget::slotPrintEntry: printing on a "
  << metrics.width() << "x" << metrics.height()
  << " size area," << endl << "   "
  << "margins are "
  << printer.margins().width() << " (left/right) and "
  << printer.margins().height() << " (top/bottom)." << endl;
  left=QMAX(printer.margins().width(), marginLeft); // left margin
  top=QMAX(printer.margins().height(), marginTop); // top margin
  width=metrics.width()-left
  -QMAX(printer.margins().width(), marginRight); // page width
  height=metrics.height()-top
  -QMAX(printer.margins().height(), marginBottom); // page height
  // ----- get the current entry from the view:
  mainwidget->getView()->getEntry(entry);
  // ----- call the painting method:
  painter.begin(&printer);
  // ----- two per page:
  // painter.setViewport(left, top+height, height/2, width);
  // painter.rotate(270);
  // ----- four per page:
  // painter.setViewport(left, top, width/2, height/2);
  // one per page:
  painter.setViewport(left, top, width, height);
  if(p.printEntry(entry, QRect(0, 0, metrics.width(), metrics.height()),
  &painter, api, 0, false))
  {
  emit(setStatus(i18n("Printing finished.")));
  } else {
  emit(setStatus(i18n("Printing failed.")));
  }
  painter.end();
  }

*/
