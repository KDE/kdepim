/*
    knconfig.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include "knconfig.h"

#include <stdlib.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <kglobal.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapp.h>
#include <kglobalsettings.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kcharsets.h>

#include "knglobals.h"
#include "knnntpaccount.h"
#include "knaccountmanager.h"
#include "kngroupmanager.h"
#include "knarticlewidget.h"
#include "utilities.h"



KNConfig::Identity::Identity(bool g) : g_lobal(g)
{
  if(g_lobal) {
    KConfig *c=KGlobal::config();
    c->setGroup("IDENTITY");
    loadConfig(c);
  }
}


KNConfig::Identity::~Identity()
{}


void KNConfig::Identity::loadConfig(KConfigBase *c)
{
  n_ame=c->readEntry("Name");
  e_mail=c->readEntry("Email");
  r_eplyTo=c->readEntry("Reply-To");
  o_rga=c->readEntry("Org");
  u_seSigFile=c->readBoolEntry("UseSigFile",false);
  u_seSigGenerator=c->readBoolEntry("UseSigGenerator",false);
  s_igPath=c->readEntry("sigFile");
  s_igText=c->readEntry("sigText");
}


void KNConfig::Identity::saveConfig(KConfigBase *c)
{
  c->writeEntry("Name", n_ame);
  c->writeEntry("Email", e_mail);
  c->writeEntry("Reply-To", r_eplyTo);
  c->writeEntry("Org", o_rga);
  c->writeEntry("UseSigFile", u_seSigFile);
  c->writeEntry("UseSigGenerator",u_seSigGenerator);
  c->writeEntry("sigFile", s_igPath);
  c->writeEntry("sigText", s_igText);
}


void KNConfig::Identity::save()
{
  kdDebug(5003) << "KNConfig::Identity::save()" << endl;
  if(g_lobal) {
    KConfig *c=KGlobal::config();
    c->setGroup("IDENTITY");
    saveConfig(c);
  }
}


bool KNConfig::Identity::isEmpty()
{
  return (  n_ame.isEmpty() &&  e_mail.isEmpty() &&
            r_eplyTo.isEmpty() && o_rga.isEmpty() &&
            s_igPath.isEmpty() && s_igText.isEmpty() );
}


bool KNConfig::Identity::emailIsValid()
{
  if (e_mail.isEmpty())
    return false;
  else
    return e_mail.contains(QRegExp("?*@?*.??*",true,true));
}


QString KNConfig::Identity::getSignature()
{
  s_igContents = QString::null;      // don't cache file contents

  if (u_seSigFile) {
    if(!s_igPath.isEmpty()) {
      if (!u_seSigGenerator) {
        QFile f(s_igPath);
        if(f.open(IO_ReadOnly)) {
          QTextStream ts(&f);
          while(!ts.atEnd()) {
            s_igContents += ts.readLine();
            if (!ts.atEnd())
              s_igContents += "\n";
          }
          f.close();
        }
        else
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot open the signature file!"));
      } else {
        KProcess process;

        // construct command line...
        QStringList command = QStringList::split(' ',s_igPath);
        for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it )
          process << (*it);

        connect(&process, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(slotReceiveStdout(KProcess *, char *, int)));
        connect(&process, SIGNAL(receivedStderr(KProcess *, char *, int)), SLOT(slotReceiveStderr(KProcess *, char *, int)));

        if (!process.start(KProcess::Block,KProcess::AllOutput))
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot run the signature generator!"));
      }
    }
  }
  else
    s_igContents = s_igText;

  if (!s_igContents.isEmpty() && !s_igContents.contains("\n-- \n") && !(s_igContents.left(4) == "-- \n"))
    s_igContents.prepend("-- \n");

  return s_igContents;
}


void KNConfig::Identity::slotReceiveStdout(KProcess *, char *buffer, int buflen)
{
  s_igContents.append(QString::fromLocal8Bit(buffer,buflen));
}


void KNConfig::Identity::slotReceiveStderr(KProcess *, char *buffer, int buflen)
{
  s_igContents.append(QString::fromLocal8Bit(buffer,buflen));
}


//==============================================================================================================


KNConfig::Appearance::Appearance()
{
  KConfig *c=KGlobal::config();
  c->setGroup("VISUAL_APPEARANCE");

  l_ongGroupList=c->readBoolEntry("longGroupList", true);

  //colors
  u_seColors=c->readBoolEntry("customColors", false);

  QColor defCol=kapp->palette().active().base();
  c_olors[background]=c->readColorEntry("backgroundColor",&defCol);
  c_olorNames[background]=i18n("Background");

  defCol=kapp->palette().active().background();
  c_olors[header]=c->readColorEntry("headerColor",&defCol);
  c_olorNames[header]=i18n("Header Decoration");

  defCol=kapp->palette().active().text();
  c_olors[normalText]=c->readColorEntry("textColor",&defCol);
  c_olorNames[normalText]=i18n("Normal Text");

  defCol=kapp->palette().active().text();
  c_olors[quoted1]=c->readColorEntry("quote1Color",&defCol);
  c_olorNames[quoted1]=i18n("Quoted Text - First level");

  defCol=kapp->palette().active().text();
  c_olors[quoted2]=c->readColorEntry("quote2Color",&defCol);
  c_olorNames[quoted2]=i18n("Quoted Text - Second level");

  defCol=kapp->palette().active().text();
  c_olors[quoted3]=c->readColorEntry("quote3Color",&defCol);
  c_olorNames[quoted3]=i18n("Quoted Text - Third level");

  defCol=KGlobalSettings::linkColor();
  c_olors[url]=c->readColorEntry("URLColor",&defCol);
  c_olorNames[url]=i18n("Link");

  defCol=kapp->palette().disabled().text();
  c_olors[readThread]=c->readColorEntry("readThreadColor",&defCol);
  c_olorNames[readThread]=i18n("Read Thread");

  defCol=kapp->palette().active().text();
  c_olors[unreadThread]=c->readColorEntry("unreadThreadColor",&defCol);
  c_olorNames[unreadThread]=i18n("Unread Thread");

  defCol.setRgb(136,136,136);
  c_olors[readArticle]=c->readColorEntry("readArtColor",&defCol);
  c_olorNames[readArticle]=i18n("Read Article");

  defCol.setRgb(183,154,11);
  c_olors[unreadArticle]=c->readColorEntry("unreadArtColor",&defCol);
  c_olorNames[unreadArticle]=i18n("Unread Article");

  defCol=kapp->palette().active().highlight();
  c_olors[activeItem]=c->readColorEntry("activeItemColor",&defCol);
  c_olorNames[activeItem]=i18n("Active Item Background");

  defCol=kapp->palette().active().background();
  c_olors[selectedItem]=c->readColorEntry("selectedItemColor",&defCol);
  c_olorNames[selectedItem]=i18n("Selected Item Background");

  //fonts
  u_seFontsForAllCS = c->readBoolEntry("useFontsForAllCS", false);
  u_seFonts = c->readBoolEntry("customFonts", false);

  QFont defFont=KGlobalSettings::generalFont();
  f_onts[article]=c->readFontEntry("articleFont",&defFont);
  f_ontNames[article]=i18n("Article Body");

  f_onts[composer]=c->readFontEntry("composerFont",&defFont);
  f_ontNames[composer]=i18n("Composer");

  f_onts[groupList]=c->readFontEntry("groupListFont",&defFont);
  f_ontNames[groupList]=i18n("Group List");

  f_onts[articleList]=c->readFontEntry("articleListFont",&defFont);
  f_ontNames[articleList]=i18n("Article List");

  updateHexcodes();

  //icons
  recreateLVIcons();
  i_cons[newFups]         = UserIcon("newsubs");
  i_cons[eyes]            = UserIcon("eyes");
  i_cons[mail]            = SmallIcon("mail_generic");
  i_cons[posting]         = SmallIcon("filenew");
  i_cons[canceledPosting] = SmallIcon("editdelete");
  i_cons[savedRemote]     = SmallIcon("editcopy");
  i_cons[nntp]            = UserIcon("server");
  i_cons[group]           = UserIcon("group");
  i_cons[folder]          = SmallIcon("folder");
  i_cons[sendErr]         = UserIcon("snderr");
}


KNConfig::Appearance::~Appearance()
{
}


void KNConfig::Appearance::save()
{
  kdDebug(5003) << "KNConfig::Appearance::save()" << endl;

  KConfig *c=KGlobal::config();
  c->setGroup("VISUAL_APPEARANCE");

  c->writeEntry("longGroupList", l_ongGroupList);

  c->writeEntry("customColors", u_seColors);
  c->writeEntry("backgroundColor", c_olors[background]);
  c->writeEntry("headerColor", c_olors[header]);
  c->writeEntry("textColor", c_olors[normalText]);
  c->writeEntry("quote1Color", c_olors[quoted1]);
  c->writeEntry("quote2Color", c_olors[quoted2]);
  c->writeEntry("quote3Color", c_olors[quoted3]);
  c->writeEntry("URLColor", c_olors[url]);
  c->writeEntry("readThreadColor", c_olors[readThread]);
  c->writeEntry("unreadThreadColor", c_olors[unreadThread]);
  c->writeEntry("readArtColor", c_olors[readArticle]);
  c->writeEntry("unreadArtColor", c_olors[unreadArticle]);
  c->writeEntry("activeItemColor", c_olors[activeItem]);
  c->writeEntry("selectedItemColor", c_olors[selectedItem]);

  c->writeEntry("useFontsForAllCS", u_seFontsForAllCS);
  c->writeEntry("customFonts", u_seFonts);
  c->writeEntry("articleFont", f_onts[article]);
  c->writeEntry("composerFont", f_onts[composer]);
  c->writeEntry("groupListFont", f_onts[groupList]);
  c->writeEntry("articleListFont", f_onts[articleList]);

  updateHexcodes();
}


QColor KNConfig::Appearance::backgroundColor()
{
  if(u_seColors)
    return c_olors[background];
  else
    return kapp->palette().active().base();
}


QColor KNConfig::Appearance::textColor()
{
  if(u_seColors)
    return c_olors[normalText];
  else
    return kapp->palette().active().text();
}


QColor KNConfig::Appearance::quoteColor1()
{
  if(u_seColors)
    return c_olors[quoted1];
  else
    return kapp->palette().active().text();
}


QColor KNConfig::Appearance::quoteColor2()
{
  if(u_seColors)
    return c_olors[quoted2];
  else
    return kapp->palette().active().text();
}


QColor KNConfig::Appearance::quoteColor3()
{
  if(u_seColors)
    return c_olors[quoted3];
  else
    return kapp->palette().active().text();
}


QColor KNConfig::Appearance::linkColor()
{
  if(u_seColors)
    return c_olors[url];
  else
    return KGlobalSettings::linkColor();

}


QColor KNConfig::Appearance::headerDecoColor()
{
  if(u_seColors)
    return c_olors[header];
  else
    return kapp->palette().active().background();
}


QColor KNConfig::Appearance::unreadThreadColor()
{
  if(u_seColors)
    return c_olors[unreadThread];
  else
    return kapp->palette().active().text();
}


QColor KNConfig::Appearance::readThreadColor()
{
  if(u_seColors)
    return c_olors[readThread];
  else
    return kapp->palette().disabled().text();
}


QColor KNConfig::Appearance::unreadArticleColor()
{
  if(u_seColors)
    return c_olors[unreadArticle];
  else
    return QColor(183,154,11);
}


QColor KNConfig::Appearance::readArticleColor()
{
  if(u_seColors)
    return c_olors[readArticle];
  else
    return QColor(136,136,136);
}


QColor KNConfig::Appearance::activeItemColor()
{
  if(u_seColors)
    return c_olors[activeItem];
  else
    return kapp->palette().active().highlight();
}


QColor KNConfig::Appearance::selectedItemColor()
{
  if(u_seColors)
    return c_olors[selectedItem];
  else
    return kapp->palette().active().background();
}


QFont KNConfig::Appearance::articleFont()
{
  if(u_seFonts)
    return f_onts[article];
  else
    return KGlobalSettings::generalFont();
}


QFont KNConfig::Appearance::composerFont()
{
  if(u_seFonts)
    return f_onts[composer];
  else
    return KGlobalSettings::generalFont();
}


QFont KNConfig::Appearance::groupListFont()
{
  if(u_seFonts)
    return f_onts[groupList];
  else
    return KGlobalSettings::generalFont();
}


QFont KNConfig::Appearance::articleListFont()
{
  if(u_seFonts)
    return f_onts[articleList];
  else
    return KGlobalSettings::generalFont();
}


void KNConfig::Appearance::updateHexcodes()
{
  sprintf(h_excodes[quoted1Hex], "#%2x%2x%2x", quoteColor1().red(), quoteColor1().green(), quoteColor1().blue());
  sprintf(h_excodes[quoted2Hex], "#%2x%2x%2x", quoteColor2().red(), quoteColor2().green(), quoteColor2().blue());
  sprintf(h_excodes[quoted3Hex], "#%2x%2x%2x", quoteColor3().red(), quoteColor3().green(), quoteColor3().blue());
  sprintf(h_excodes[headerHex], "#%2x%2x%2x", headerDecoColor().red(), headerDecoColor().green(), headerDecoColor().blue());
}


QColor KNConfig::Appearance::defaultColor(int i)
{
  switch(i) {

    case background:
      return kapp->palette().active().base();
    break;

    case header:
      return kapp->palette().active().background();
    break;

    case normalText:
    case quoted1:
    case quoted2:
    case quoted3:
    case unreadThread:
      return kapp->palette().active().text();
    break;

    case url:
      return KGlobalSettings::linkColor();
    break;

    case readThread:
      return kapp->palette().disabled().text();
    break;

    case activeItem:
      return kapp->palette().active().highlight();
    break;

    case selectedItem:
      return kapp->palette().active().background();
    break;

    case unreadArticle:
      return QColor(183,154,11);
    break;

    case readArticle:
      return QColor(136,136,136);
  }

  return kapp->palette().disabled().text();
}


QFont KNConfig::Appearance::defaultFont(int)
{
  return KGlobalSettings::generalFont();
}


void KNConfig::Appearance::recreateLVIcons()
{
  QPixmap tempPix = UserIcon("greyball");

  QImage tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, readArticleColor(), 1.0);
  i_cons[greyBall].convertFromImage(tempImg);

  tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, unreadArticleColor(), 1.0);
  i_cons[redBall].convertFromImage(tempImg);

  tempPix = UserIcon("greyballchk");

  tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, readArticleColor(), 1.0);
  i_cons[greyBallChkd].convertFromImage(tempImg);

  tempImg=tempPix.convertToImage();
  KIconEffect::colorize(tempImg, unreadArticleColor(), 1.0);
  i_cons[redBallChkd].convertFromImage(tempImg);
}


//==============================================================================================================


KNConfig::ReadNewsGeneral::ReadNewsGeneral()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("READNEWS");

  a_utoCheck=conf->readBoolEntry("autoCheck", true);
  m_axFetch=conf->readNumEntry("maxFetch", 1000);
  if (m_axFetch<0) m_axFetch = 0;
  a_utoMark=conf->readBoolEntry("autoMark", true);
  m_arkSecs=conf->readNumEntry("markSecs", 5);
  if (m_arkSecs<0) m_arkSecs = 0;
  t_otalExpand=conf->readBoolEntry("totalExpand", true);
  s_howLines=conf->readBoolEntry("showLines3", true);
  s_howScore=conf->readBoolEntry("showScore3", true);
}


KNConfig::ReadNewsGeneral::~ReadNewsGeneral()
{
}


void KNConfig::ReadNewsGeneral::save()
{
  kdDebug(5003) << "KNConfig::ReadNewsGeneral::save()" << endl;

  KConfig *conf=KGlobal::config();
  conf->setGroup("READNEWS");

  conf->writeEntry("autoCheck", a_utoCheck);
  conf->writeEntry("maxFetch", m_axFetch);
  conf->writeEntry("autoMark", a_utoMark);
  conf->writeEntry("markSecs", m_arkSecs);
  conf->writeEntry("totalExpand", t_otalExpand);
  conf->writeEntry("showLines3", s_howLines);
  conf->writeEntry("showScore3", s_howScore);
}


//==============================================================================================================


KNConfig::ReadNewsViewer::ReadNewsViewer()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("READNEWS");

  s_howHeaderDeco=conf->readBoolEntry("showHeaderDeco", true);
  r_ewrapBody=conf->readBoolEntry("rewrapBody", true);
  s_howSig=conf->readBoolEntry("showSig", true);
  i_nterpretFormatTags=conf->readBoolEntry("interpretFormatTags", true);
  q_uoteCharacters=conf->readEntry("quoteCharacters",">:");
  i_nlineAtt=conf->readBoolEntry("inlineAtt", true);
  o_penAtt=conf->readBoolEntry("openAtt", false) ;
  s_howAlts=conf->readBoolEntry("showAlts", false);
  QString s = conf->readEntry("Browser","Konqueror");
  if (s=="Netscape")
    b_rowser = BTnetscape;
  else if (s=="Mozilla")
    b_rowser = BTmozilla;
  else if (s=="Opera")
    b_rowser = BTopera;
  else if (s=="Other")
    b_rowser = BTother;
  else
    b_rowser = BTkonq;
  b_rowserCommand=conf->readEntry("BrowserCommand","netscape %u");
}


KNConfig::ReadNewsViewer::~ReadNewsViewer()
{
}


void KNConfig::ReadNewsViewer::save()
{
  kdDebug(5003) << "KNConfig::ReadNewsViewer::save()" << endl;

  KConfig *conf=KGlobal::config();
  conf->setGroup("READNEWS");

  conf->writeEntry("showHeaderDeco", s_howHeaderDeco);
  conf->writeEntry("rewrapBody", r_ewrapBody);
  conf->writeEntry("showSig", s_howSig);
  conf->writeEntry("interpretFormatTags", i_nterpretFormatTags);
  conf->writeEntry("quoteCharacters",q_uoteCharacters);
  conf->writeEntry("inlineAtt", i_nlineAtt);
  conf->writeEntry("openAtt", o_penAtt);
  conf->writeEntry("showAlts", s_howAlts);
  switch (b_rowser) {
    case BTkonq: conf->writeEntry("Browser","Konqueror");
                 break;
    case BTnetscape: conf->writeEntry("Browser","Netscape");
                     break;
    case BTmozilla: conf->writeEntry("Browser","Mozilla");
                    break;
    case BTopera: conf->writeEntry("Browser","Opera");
                  break;
    case BTother: conf->writeEntry("Browser","Other");
                  break;
  }
  conf->writeEntry("BrowserCommand", b_rowserCommand);
}


//==============================================================================================================


KNConfig::DisplayedHeaders::DisplayedHeaders()
{
  h_drList.setAutoDelete(true);

  QString fname(KGlobal::dirs()->findResource("appdata","headers.rc"));
  if (fname != QString::null) {
    KSimpleConfig headerConf(fname,true);
    QStringList headers = headerConf.groupList();
    headers.remove("<default>");
    headers.sort();

    KNDisplayedHeader *h;
    QValueList<int> flags;

    QStringList::Iterator it;
    for( it = headers.begin(); it != headers.end(); ++it ) {
      h=createNewHeader();
      headerConf.setGroup((*it));
      h->setName(headerConf.readEntry("Name"));
      h->setTranslateName(headerConf.readBoolEntry("Translate_Name",true));
      h->setHeader(headerConf.readEntry("Header"));
      flags=headerConf.readIntListEntry("Flags");
      if(h->name().isNull() || h->header().isNull() || (flags.count()!=8)) {
        kdDebug(5003) << "KNConfig::DisplayedHeaders::DisplayedHeaders() : ignoring invalid/incomplete Header" << endl;
        remove(h);
      }
      else {
        for (int i=0; i<8; i++)
          h->setFlag(i, (flags[i]>0));
        h->createTags();
      }
    }
  }
}


KNConfig::DisplayedHeaders::~DisplayedHeaders()
{
}


void KNConfig::DisplayedHeaders::save()
{
  kdDebug(5003) << "KNConfig::DisplayedHeaders::save()" << endl;

  QString dir(KGlobal::dirs()->saveLocation("appdata"));
  if (dir==QString::null) {
    KNHelper::displayInternalFileError();
    return;
  }
  KSimpleConfig headerConf(dir+"headers.rc");
  QStringList oldHeaders = headerConf.groupList();

  QStringList::Iterator oldIt=oldHeaders.begin();
  for( ;oldIt != oldHeaders.end(); ++oldIt )      // remove all old groups
    headerConf.deleteGroup((*oldIt));             // find a better way to do it?

  QValueList<int> flags;
  int idx=0;
  QString group;

  for(Iterator it(h_drList); it.current(); ++it) {
    group.setNum(idx++);
    while (group.length()<3)
      group.prepend("0");
    headerConf.setGroup(group);
    headerConf.writeEntry("Name",(*it)->name());
    headerConf.writeEntry("Translate_Name",(*it)->translateName());
    headerConf.writeEntry("Header",(*it)->header());
    flags.clear();
    for (int i=0; i<8; i++) {
      if ((*it)->flag(i))
        flags << 1;
      else
        flags << 0;
    }
    headerConf.writeEntry("Flags",flags);
  }
}


KNDisplayedHeader* KNConfig::DisplayedHeaders::createNewHeader()
{
  KNDisplayedHeader *h=new KNDisplayedHeader();
  h_drList.append(h);

  return h;
}


void KNConfig::DisplayedHeaders::remove(KNDisplayedHeader *h)
{
  if (!h_drList.remove(h))
    kdDebug(5003) << "KNConfig::DisplayedHeaders::remove() : cannot find pointer in list!" << endl;

}


void KNConfig::DisplayedHeaders::up(KNDisplayedHeader *h)
{
  int idx=h_drList.findRef(h);
  if(idx!=-1) {
    h_drList.take(idx);
    h_drList.insert(idx-1, h);
  }
  else kdDebug(5003) << "KNConfig::DisplayedHeaders::up() : item not found in list" << endl;
}


void KNConfig::DisplayedHeaders::down(KNDisplayedHeader *h)
{
  int idx=h_drList.findRef(h);
  if(idx!=-1) {
    h_drList.take(idx);
    h_drList.insert(idx+1, h);
  }
  else kdDebug(5003) << "KNConfig::DisplayedHeaders::down() : item not found in list" << endl;
}


//==============================================================================================================


KNConfig::XHeader::XHeader(const QString &s)
{
  if(s.left(2)=="X-") {
    int pos=s.find(": ");
    if(pos!=-1) {
      n_ame=s.mid(2, pos-2).latin1();
      pos+=2;
      v_alue=s.mid(pos, s.length()-pos);
    }
  }
}


//==============================================================================================================


KNConfig::PostNewsTechnical::PostNewsTechnical()
 : findComposerCSCache(113)
{
  findComposerCSCache.setAutoDelete(true);

  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

  c_omposerCharsets=conf->readListEntry("ComposerCharsets");
  if (c_omposerCharsets.isEmpty())
    c_omposerCharsets=QStringList::split(',',"us-ascii,utf-8,iso-8859-1,iso-8859-2,"
    "iso-8859-3,iso-8859-4,iso-8859-5,iso-8859-6,iso-8859-7,iso-8859-8,"
    "iso-8859-9,iso-8859-10,iso-8859-13,iso-8859-14,iso-8859-15,koi8-r,koi8-u,"
    "iso-2022-jp,iso-2022-jp-2,iso-2022-kr,euc-jp,euc-kr,Big5,gb2312");

  c_harset=conf->readEntry("Charset").latin1();
  if (c_harset.isEmpty()) {
    c_harset=findComposerCharset(KGlobal::charsets()->charsetForLocale());
    if (c_harset.isEmpty())
      c_harset="iso-8859-1";  // shit
  }

  h_ostname=conf->readEntry("MIdhost").latin1();
  a_llow8BitBody=conf->readBoolEntry("8BitEncoding",true);
  u_seOwnCharset=conf->readBoolEntry("UseOwnCharset",true);
  a_llow8BitHeaders=conf->readBoolEntry("allow8bitChars", false);
  g_enerateMID=conf->readBoolEntry("generateMId", false);
  d_ontIncludeUA=conf->readBoolEntry("dontIncludeUA", false);
  u_seKmail=conf->readBoolEntry("useKmail", false);

  QString dir(KGlobal::dirs()->saveLocation("appdata"));
  if (dir!=QString::null) {
    QFile f(dir+"xheaders");
    if(f.open(IO_ReadOnly)) {
      QTextStream ts(&f);
      while(!ts.eof())
        x_headers.append( XHeader(ts.readLine()) );

      f.close();
    }
  }
}


KNConfig::PostNewsTechnical::~PostNewsTechnical()
{
}


void KNConfig::PostNewsTechnical::save()
{
  kdDebug(5003) << "KNConfig::PostNewsTechnical::save()" << endl;

  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

  conf->writeEntry("ComposerCharsets", c_omposerCharsets);
  conf->writeEntry("Charset", QString::fromLatin1(c_harset));
  conf->writeEntry("8BitEncoding", a_llow8BitBody);
  conf->writeEntry("UseOwnCharset", u_seOwnCharset);
  conf->writeEntry("allow8bitChars", a_llow8BitHeaders);
  conf->writeEntry("generateMId", g_enerateMID);
  conf->writeEntry("MIdhost", QString::fromLatin1(h_ostname));
  conf->writeEntry("dontIncludeUA", d_ontIncludeUA);
  conf->writeEntry("useKmail", u_seKmail);

  QString dir(KGlobal::dirs()->saveLocation("appdata"));
  if (dir==QString::null)
    KNHelper::displayInternalFileError();
  else {
    QFile f(dir+"xheaders");
    if(f.open(IO_WriteOnly)) {
      QTextStream ts(&f);
      XHeaders::Iterator it;
      for(it=x_headers.begin(); it!=x_headers.end(); ++it)
        ts << (*it).header() << "\n";
      f.close();
    }
    else
      KNHelper::displayInternalFileError();
  }
}


int KNConfig::PostNewsTechnical::indexForCharset(const QCString &str)
{
  int i=0;
  bool found=false;
  for ( QStringList::Iterator it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
    if ((*it).lower() == str.lower().data()) {
      found = true;
      break;
    }
    i++;
  }
  if (!found) {
    i=0;
    for ( QStringList::Iterator it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
      if ((*it).lower() == c_harset.lower().data()) {
        found = true;
        break;
      }
      i++;
    }
    if (!found)
      i=0;
  }
  return i;
}


QCString KNConfig::PostNewsTechnical::findComposerCharset(QCString cs)
{
  QCString *ret=findComposerCSCache.find(cs);
  if (ret)
    return *ret;

  QFont::CharSet qfcs = KGlobal::charsets()->charsetForEncoding(cs);
  QCString s;

  QStringList::Iterator it;
  for( it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {

    // match by name
    if ((*it).lower()==cs.lower().data()) {
      s = (*it).latin1();
      break;
    }

    // match by charset, avoid to return "us-ascii" for iso-8859-1
    if (((*it).lower()!="us-ascii")&&
        (KGlobal::charsets()->charsetForEncoding(*it)==qfcs)) {
      s = (*it).latin1();
      break;
    }
  }

  if (s.isEmpty())
    s = "us-ascii";

  findComposerCSCache.insert(cs, new QCString(s));

  return s;
}


QCString KNConfig::PostNewsTechnical::findComposerCharset(QFont::CharSet cs)
{
  if (cs==QFont::ISO_8859_1)  // avoid to return "us-ascii"
    return "iso-8859-1";

  QCString *ret=findComposerCSCache.find(QString::number((int)(cs)).latin1());
  if (ret)
    return *ret;

  QCString s;
  QStringList::Iterator it;
  for( it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
    if ((KGlobal::charsets()->charsetForEncoding(*it)==cs)) {
      s = (*it).latin1();
      break;
    }
  }

  if (s.isEmpty())
    s = "us-ascii";

  findComposerCSCache.insert(QString::number((int)(cs)).latin1(), new QCString(s));

  return s;
}


//==============================================================================================================


KNConfig::PostNewsComposer::PostNewsComposer()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

  w_ordWrap=conf->readBoolEntry("wordWrap",true);
  m_axLen=conf->readNumEntry("maxLength", 76);
  a_ppSig=conf->readBoolEntry("appSig",true);
  r_ewrap=conf->readBoolEntry("rewrap",true);
  i_ncSig=conf->readBoolEntry("incSig",false);
  u_seExtEditor=conf->readBoolEntry("useExternalEditor",false);
  i_ntro=conf->readEntry("Intro","%NAME wrote:");
  e_xternalEditor=conf->readEntry("externalEditor","kwrite %f");
}


KNConfig::PostNewsComposer::~PostNewsComposer()
{
}


void KNConfig::PostNewsComposer::save()
{
  kdDebug(5003) << "KNConfig::PostNewsComposer::save()" << endl;

  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

  conf->writeEntry("wordWrap", w_ordWrap);
  conf->writeEntry("maxLength", m_axLen);
  conf->writeEntry("appSig", a_ppSig);
  conf->writeEntry("rewrap",r_ewrap);
  conf->writeEntry("incSig", i_ncSig);
  conf->writeEntry("useExternalEditor", u_seExtEditor);
  conf->writeEntry("Intro", i_ntro);
  conf->writeEntry("externalEditor", e_xternalEditor);

}

//==============================================================================================================


KNConfig::Privacy::Privacy()
  : v_ersions ( QStringList() << "GnuPG 1.x" << "PGP 2.6.x" << "PGP 5" << "PGP 6" ),
    e_ncodings ( QStringList() << "Text" )
{
  KConfig *conf = KGlobal::config();
  conf->setGroup("PRIVACY");

  v_ersion = conf->readNumEntry("pgpVersion",GPG1);
  k_eeppasswd = conf->readBoolEntry("keepPassword",false);
  k_eyserv = conf->readEntry("keyserver");
  e_ncoding = conf->readNumEntry("encoding",0);
  p_rogpath = conf->readEntry("progPath");
  if (p_rogpath.isEmpty())
    p_rogpath = defaultProg(v_ersion);
}


KNConfig::Privacy::~Privacy()
{
}


// overrides KNConfig::Base::save()
void KNConfig::Privacy::save()
{
  kdDebug(5003) << "KNConfig::Privacy::save()" << endl;
  KConfig *conf = KGlobal::config();
  conf->setGroup("PRIVACY");

  conf->writeEntry("pgpVersion",v_ersion);
  conf->writeEntry("keepPassword",k_eeppasswd);
  conf->writeEntry("keyserver",k_eyserv);
  conf->writeEntry("encoding",e_ncoding);
  conf->writeEntry("progPath",p_rogpath);
}


QString KNConfig::Privacy::defaultProg(int version)
{
  QString pgp;
  if (version == GPG1) pgp = "/gpg";
  else pgp = "/pgp";

  QStringList pSearchPaths=QStringList::split(':',QString::fromLocal8Bit(getenv("PATH")));

  for (QStringList::Iterator it = pSearchPaths.begin(); it != pSearchPaths.end(); ++it ) {
    if (QFileInfo((*it)+pgp).isExecutable())
      return (*it)+pgp;
  }

  return QString::null;
}


//==============================================================================================================


KNConfig::Cleanup::Cleanup()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("EXPIRE");

  d_oExpire=conf->readBoolEntry("doExpire", true);
  p_reserveThr=conf->readBoolEntry("saveThreads",true);
  d_oCompact=conf->readBoolEntry("doCompact", true);
  e_xpireInterval=conf->readNumEntry("expInterval", 5);
  r_eadMaxAge=conf->readNumEntry("readDays",10);
  u_nreadMaxAge=conf->readNumEntry("unreadDays",15);
  c_ompactInterval=conf->readNumEntry("comInterval", 5);
}


KNConfig::Cleanup::~Cleanup()
{
}


void KNConfig::Cleanup::save()
{
  kdDebug(5003) << "KNConfig::Cleanup::save()" << endl;

  KConfig *conf=KGlobal::config();
  conf->setGroup("EXPIRE");

  conf->writeEntry("doExpire", d_oExpire);
  conf->writeEntry("saveThreads", p_reserveThr);
  conf->writeEntry("doCompact", d_oCompact);
  conf->writeEntry("expInterval", e_xpireInterval);
  conf->writeEntry("readDays", r_eadMaxAge);
  conf->writeEntry("unreadDays", u_nreadMaxAge);
  conf->writeEntry("comInterval", c_ompactInterval);
}


bool KNConfig::Cleanup::expireToday()
{
  if(!d_oExpire)
    return false;

  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");

  QDate today=QDate::currentDate();
  QDate lastExpDate=c->readDateTimeEntry("lastExpire").date();

  if(lastExpDate==today) {
    c->writeEntry("lastExpire", QDateTime::currentDateTime());  // important! otherwise lastExpDate will be at its default value (current date) forever
    return false;
  }

  return (lastExpDate.daysTo(today) >= e_xpireInterval);
}


void KNConfig::Cleanup::setLastExpireDate()
{
  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");
  c->writeEntry("lastExpire", QDateTime::currentDateTime());
}


bool KNConfig::Cleanup::compactToday()
{
  if(!d_oCompact)
    return false;

  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");

  QDate today=QDate::currentDate();
  QDate lastComDate=c->readDateTimeEntry("lastCompact").date();

  if(lastComDate==today) {
    c->writeEntry("lastCompact", QDateTime::currentDateTime());  // important! otherwise lastComDate will be at its default value (current date) forever
    return false;
  }

  return (lastComDate.daysTo(today) >= c_ompactInterval);
}


void KNConfig::Cleanup::setLastCompactDate()
{
  KConfig *c=KGlobal::config();
  c->setGroup("EXPIRE");
  c->writeEntry("lastCompact", QDateTime::currentDateTime());
}



//==============================================================================================================



KNConfig::Cache::Cache()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("CACHE");

  m_emMaxArt=conf->readNumEntry("memMaxArt", 1000);
  m_emMaxKB=conf->readNumEntry("memMaxKB", 1024);

  d_iskMaxArt=conf->readNumEntry("diskMaxArt", 1000);
  d_iskMaxKB=conf->readNumEntry("diskMaxKB", 1024);
}


KNConfig::Cache::~Cache()
{
}


void KNConfig::Cache::save()
{
  kdDebug(5003) << "KNConfig::Cache::save()" << endl;

  KConfig *conf=KGlobal::config();
  conf->setGroup("CACHE");

  conf->writeEntry("memMaxArt", m_emMaxArt);
  conf->writeEntry("memMaxKB", m_emMaxKB);

  conf->writeEntry("diskMaxArt", d_iskMaxArt);
  conf->writeEntry("diskMaxKB", d_iskMaxKB);

}

