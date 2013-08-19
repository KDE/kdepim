/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <klocale.h>

#include "kndisplayedheader.h"


// some standard headers
static QStringList predef;

// default display names KNode uses
static const char *disp[] = { "Groups", 0 };

void dummyHeader()
{
  i18nc("collection of article headers","Approved");
  i18nc("collection of article headers","Content-Transfer-Encoding");
  i18nc("collection of article headers","Content-Type");
  i18nc("collection of article headers","Control");
  i18nc("collection of article headers","Date");
  i18nc("collection of article headers","Distribution");
  i18nc("collection of article headers","Expires");
  i18nc("collection of article headers","Followup-To");
  i18nc("collection of article headers","From");
  i18nc("collection of article headers","Lines");
  i18nc("collection of article headers","Mail-Copies-To");
  i18nc("collection of article headers","Message-ID");
  i18nc("collection of article headers","Mime-Version");
  i18nc("collection of article headers","NNTP-Posting-Host");
  i18nc("collection of article headers","Newsgroups");
  i18nc("collection of article headers","Organization");
  i18nc("collection of article headers","Path");
  i18nc("collection of article headers","References");
  i18nc("collection of article headers","Reply-To");
  i18nc("collection of article headers","Sender");
  i18nc("collection of article headers","Subject");
  i18nc("collection of article headers","Supersedes");
  i18nc("collection of article headers","To");
  i18nc("collection of article headers","User-Agent");
  i18nc("collection of article headers","X-Mailer");
  i18nc("collection of article headers","X-Newsreader");
  i18nc("collection of article headers","X-No-Archive");
  i18nc("collection of article headers","XRef");

  i18nc("collection of article headers","Groups");
}


//=============================================================================================================


KNDisplayedHeader::KNDisplayedHeader()
 : t_ranslateName(true)
{
  f_lags.fill(false, 8);
  f_lags[1] = true;   // header name bold by default
}


KNDisplayedHeader::~KNDisplayedHeader()
{
}


// some common headers
QStringList KNDisplayedHeader::predefs()
{
  if ( predef.isEmpty() )
    predef << "Approved"
        << "Content-Transfer-Encoding"
        << "Content-Type"
        << "Control"
        << "Date"
        << "Distribution"
        << "Expires"
        << "Followup-To"
        << "From"
        << "Lines"
        << "Mail-Copies-To"
        << "Message-ID"
        << "Mime-Version"
        << "NNTP-Posting-Host"
        << "Newsgroups"
        << "Organization"
        << "Path"
        << "References"
        << "Reply-To"
        << "Sender"
        << "Subject"
        << "Supersedes"
        << "To"
        << "User-Agent"
        << "X-Mailer"
        << "X-Newsreader"
        << "X-No-Archive"
        << "XRef";
  return predef;
}


// *tries* to translate the name
QString KNDisplayedHeader::translatedName()
{
  if (t_ranslateName) {
    // major hack alert !!!
    if (!n_ame.isEmpty()) {
      if (i18nc("collection of article headers",n_ame.toLocal8Bit())!=n_ame.toLocal8Bit().data())    // try to guess if this english or not
        return i18nc("collection of article headers",n_ame.toLocal8Bit());
      else
        return n_ame;
    } else
      return QString();
  } else
    return n_ame;
}


// *tries* to retranslate the name to english
void KNDisplayedHeader::setTranslatedName(const QString &s)
{
  bool retranslated = false;
  QStringList pd = predefs();
  Q_FOREACH( const QString &c, pd ) {
    if ( s == i18nc("collection of article headers", c.toLatin1()) ) {
      n_ame = c;
      retranslated = true;
      break;
    }
  }

  if (!retranslated) {
    for (const char **c=disp;(*c)!=0;c++)   // now our standard display names
      if (s==i18nc("collection of article headers",*c)) {
        n_ame = QString::fromLatin1(*c);
        retranslated = true;
        break;
      }
  }

  if (!retranslated) {      // ok, we give up and store the maybe non-english string
    n_ame = s;
    t_ranslateName = false;  // and don't try to translate it, so a german user *can* use the original english name
  } else
    t_ranslateName = true;
}


void  KNDisplayedHeader::createTags()
{
  const char *tokens[] = {  "<big>","</big>","<b>","</b>",
                            "<i>","</i>","<u>","</u>" };

  for ( int i = 0; i < 4; ++i )
    t_ags[i].clear();

  if(f_lags.at(0)) {    // <big>
    t_ags[0]=tokens[0];
    t_ags[1]=tokens[1];
  }
  if(f_lags.at(4)) {
    t_ags[2]=tokens[0];
    t_ags[3]=tokens[1];
  }

  if(f_lags.at(1)) {     // <b>
    t_ags[0]+=(tokens[2]);
    t_ags[1].prepend(tokens[3]);
  }
  if(f_lags.at(5)) {
    t_ags[2]+=tokens[2];
    t_ags[3].prepend(tokens[3]);
  }

  if(f_lags.at(2)) {     // <i>
    t_ags[0]+=tokens[4];
    t_ags[1].prepend(tokens[5]);
  }
  if(f_lags.at(6)) {
    t_ags[2]+=tokens[4];
    t_ags[3].prepend(tokens[5]);
  }

  if(f_lags.at(3)) {    // <u>
    t_ags[0]+=tokens[6];
    t_ags[1].prepend(tokens[7]);
  }
  if(f_lags.at(7)) {
    t_ags[2]+=tokens[6];
    t_ags[3].prepend(tokens[7]);
  }
}
