// -*- tab-width: 2 -*-

/***************************************************************************
                          knviewheader.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include "utilities.h"
#include "knviewheader.h"


//=============================================================================================================


// some standard headers
static const char *predef[] = { "Approved","Content-Transfer-Encoding","Content-Type","Control","Date","Distribution",
                                "Expires","Followup-To","From","Lines","Message-ID","Mime-Version","NNTP-Posting-Host",
                                "Newsgroups","Organization","Path","References","Reply-To","Sender","Subject","Supersedes",
                                "To", "User-Agent","X-Mailer","X-Newsreader","X-No-Archive","XRef",0 };

// default display names KNode uses
static const char *disp[] = { "Groups", 0 };

void dummyHeader()
{
  i18n("it's not very important to translate this","Approved");
  i18n("it's not very important to translate this","Content-Transfer-Encoding");
  i18n("it's not very important to translate this","Content-Type");
  i18n("it's not very important to translate this","Control");
  i18n("it's not very important to translate this","Date");
  i18n("it's not very important to translate this","Distribution");
  i18n("it's not very important to translate this","Expires");
  i18n("it's not very important to translate this","Followup-To");
  i18n("it's not very important to translate this","From");
  i18n("it's not very important to translate this","Lines");
  i18n("it's not very important to translate this","Message-ID");
  i18n("it's not very important to translate this","Mime-Version");
  i18n("it's not very important to translate this","NNTP-Posting-Host");
  i18n("it's not very important to translate this","Newsgroups");
  i18n("it's not very important to translate this","Organization");
  i18n("it's not very important to translate this","Path");
  i18n("it's not very important to translate this","References");
  i18n("it's not very important to translate this","Reply-To");
  i18n("it's not very important to translate this","Sender");
  i18n("it's not very important to translate this","Subject");
  i18n("it's not very important to translate this","Supersedes");
  i18n("it's not very important to translate this","To");
  i18n("it's not very important to translate this","User-Agent");
  i18n("it's not very important to translate this","X-Mailer");
  i18n("it's not very important to translate this","X-Newsreader");
  i18n("it's not very important to translate this","X-No-Archive");
  i18n("it's not very important to translate this","XRef");

  i18n("it's not very important to translate this","Groups");
}


//=============================================================================================================


KNViewHeader::KNViewHeader()
 : t_ranslateName(true)
{
  f_lags.fill(false, 8);
  f_lags[1] = true;   // header name bold by default
}



KNViewHeader::~KNViewHeader()
{
}


// some common headers
const char** KNViewHeader::predefs()
{
  return predef;
}


// *trys* to translate the name
QString KNViewHeader::translatedName()
{
  if (t_ranslateName) {
    // major hack alert !!!
    if (!n_ame.isEmpty()) {
      if (i18n("it's not very important to translate this",n_ame.local8Bit())!=n_ame.local8Bit().data())    // try to guess if this english or not
        return i18n("it's not very important to translate this",n_ame.local8Bit());
      else
        return n_ame;
    } else
      return QString::null;
  } else
    return n_ame;
}


// *trys* to retranslate the name to english
void KNViewHeader::setTranslatedName(const QString &s)
{
  bool retranslated = false;
  for (const char **c=predef;(*c)!=0;c++) {  // ok, first the standard header names
    if (s==i18n("it's not very important to translate this",*c)) {
      n_ame = QString::fromLatin1(*c);
      retranslated = true;
      break;
    }
  }

  if (!retranslated) {
    for (const char **c=disp;(*c)!=0;c++)   // now our standard display names
      if (s==i18n("it's not very important to translate this",*c)) {
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


void  KNViewHeader::createTags()
{
  const char *tokens[] = {  "<large>","</large>","<b>","</b>",
                            "<i>","</i>","<u>","</u>" };

  for(int i=0; i<4; i++) t_ags[i]=QString::null;

  if(f_lags.at(0)) {    // <font>
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


