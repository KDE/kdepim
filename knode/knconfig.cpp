/*
    knconfig.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
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

#include <qtextcodec.h>

#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kprocess.h>

#include "knglobals.h"
#include "kngroupmanager.h"
#include "knarticlewidget.h"
#include "utilities.h"



KNConfig::Identity::Identity(bool g)
 :  u_seSigFile(false), u_seSigGenerator(false), g_lobal(g)
{
  if(g_lobal) {
    KConfig *c=knGlobals.config();
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
  m_ailCopiesTo=c->readEntry("Mail-Copies-To");
  o_rga=c->readEntry("Org");
  s_igningKey = c->readEntry("SigningKey").local8Bit();
  u_seSigFile=c->readBoolEntry("UseSigFile",false);
  u_seSigGenerator=c->readBoolEntry("UseSigGenerator",false);
  s_igPath=c->readPathEntry("sigFile");
  s_igText=c->readEntry("sigText");
}


void KNConfig::Identity::saveConfig(KConfigBase *c)
{
  c->writeEntry("Name", n_ame);
  c->writeEntry("Email", e_mail);
  c->writeEntry("Reply-To", r_eplyTo);
  c->writeEntry("Mail-Copies-To", m_ailCopiesTo);
  c->writeEntry("Org", o_rga);
  c->writeEntry("SigningKey", QString(s_igningKey));
  c->writeEntry("UseSigFile", u_seSigFile);
  c->writeEntry("UseSigGenerator",u_seSigGenerator);
  c->writePathEntry("sigFile", s_igPath);
  c->writeEntry("sigText", s_igText);
  c->sync();
}


void KNConfig::Identity::save()
{
  kdDebug(5003) << "KNConfig::Identity::save()" << endl;
  if(g_lobal) {
    KConfig *c=knGlobals.config();
    c->setGroup("IDENTITY");
    saveConfig(c);
  }
}


bool KNConfig::Identity::isEmpty()
{
  return (  n_ame.isEmpty() &&  e_mail.isEmpty() &&
            r_eplyTo.isEmpty() && m_ailCopiesTo.isEmpty() &&
            o_rga.isEmpty() && s_igPath.isEmpty() && s_igText.isEmpty() &&
            s_igningKey.isEmpty() );
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
  s_igStdErr = QString::null;

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
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot open the signature file."));
      } else {
        KProcess process;

        // construct command line...
        QStringList command = QStringList::split(' ',s_igPath);
        for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it )
          process << (*it);

        connect(&process, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(slotReceiveStdout(KProcess *, char *, int)));
        connect(&process, SIGNAL(receivedStderr(KProcess *, char *, int)), SLOT(slotReceiveStderr(KProcess *, char *, int)));

        if (!process.start(KProcess::Block,KProcess::AllOutput))
          KMessageBox::error(knGlobals.topWidget, i18n("Cannot run the signature generator."));
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
  s_igStdErr.append(QString::fromLocal8Bit(buffer,buflen));
}


//==============================================================================================================


KNConfig::Appearance::Appearance()
{
  KConfig *c=knGlobals.config();
  c->setGroup("VISUAL_APPEARANCE");

  //colors
  u_seColors=c->readBoolEntry("customColors", false);

  QColor defCol=kapp->palette().active().base();
  c_olors[background]=c->readColorEntry("backgroundColor",&defCol);
  c_olorNames[background]=i18n("Background");

  defCol=KGlobalSettings::alternateBackgroundColor();
  c_olors[alternateBackground]=c->readColorEntry("alternateBackgroundColor",&defCol);
  c_olorNames[alternateBackground]=i18n("Alternate Background");

  defCol=kapp->palette().active().background();
  c_olors[header]=c->readColorEntry("headerColor",&defCol);
  c_olorNames[header]=i18n("Header Decoration");

  defCol=kapp->palette().active().text();
  c_olors[normalText]=c->readColorEntry("textColor",&defCol);
  c_olorNames[normalText]=i18n("Normal Text");

  defCol=QColor( 0x00, 0x80, 0x00 );
  c_olors[quoted1]=c->readColorEntry("quote1Color",&defCol);
  c_olorNames[quoted1]=i18n("Quoted Text - First level");

  defCol=QColor( 0x00, 0x70, 0x00 );
  c_olors[quoted2]=c->readColorEntry("quote2Color",&defCol);
  c_olorNames[quoted2]=i18n("Quoted Text - Second level");

  defCol=QColor( 0x00, 0x60, 0x00 );
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
  u_seFonts = c->readBoolEntry("customFonts", false);
  QFont defFont=KGlobalSettings::generalFont();
  f_onts[article]=c->readFontEntry("articleFont",&defFont);
  f_ontNames[article]=i18n("Article Body");

  defFont=KGlobalSettings::fixedFont();
  f_onts[articleFixed]=c->readFontEntry("articleFixedFont",&defFont);
  f_ontNames[articleFixed]=i18n("Article Body (Fixed)");

  f_onts[composer]=c->readFontEntry("composerFont",&defFont);
  f_ontNames[composer]=i18n("Composer");

  defFont=KGlobalSettings::generalFont();
  f_onts[groupList]=c->readFontEntry("groupListFont",&defFont);
  f_ontNames[groupList]=i18n("Group List");

  f_onts[articleList]=c->readFontEntry("articleListFont",&defFont);
  f_ontNames[articleList]=i18n("Article List");

  //icons
  KGlobal::iconLoader()->addAppDir("knode");
  recreateLVIcons();
  i_cons[newFups]         = UserIcon("newsubs");
  i_cons[eyes]            = UserIcon("eyes");
  i_cons[ignore]          = UserIcon("ignore");
  i_cons[mail]            = SmallIcon("mail_generic");
  i_cons[posting]         = SmallIcon("filenew");
  i_cons[canceledPosting] = SmallIcon("editdelete");
  i_cons[savedRemote]     = SmallIcon("editcopy");
  i_cons[nntp]            = SmallIcon("server");
  i_cons[group]           = UserIcon("group");
  i_cons[folder]          = SmallIcon("folder_cyan");
  i_cons[rootFolder]      = SmallIcon("folder");
  i_cons[customFolder]    = SmallIcon("folder_green");
  i_cons[sendErr]         = UserIcon("snderr");
}


KNConfig::Appearance::~Appearance()
{
}


void KNConfig::Appearance::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::Appearance::save()" << endl;

  KConfig *c=knGlobals.config();
  c->setGroup("VISUAL_APPEARANCE");

  c->writeEntry("customColors", u_seColors);
  c->writeEntry("backgroundColor", c_olors[background]);
  c->writeEntry("alternateBackgroundColor", c_olors[alternateBackground]);
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

  c->writeEntry("customFonts", u_seFonts);
  c->writeEntry("articleFont", f_onts[article]);
  c->writeEntry("articleFixedFont", f_onts[articleFixed]);
  c->writeEntry("composerFont", f_onts[composer]);
  c->writeEntry("groupListFont", f_onts[groupList]);
  c->writeEntry("articleListFont", f_onts[articleList]);
  c->sync();
  d_irty = false;
}


QColor KNConfig::Appearance::backgroundColor()
{
  if(u_seColors)
    return c_olors[background];
  else
    return kapp->palette().active().base();
}


QColor KNConfig::Appearance::alternateBackgroundColor()
{
  if(u_seColors)
    return c_olors[alternateBackground];
  else
    return KGlobalSettings::alternateBackgroundColor();
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


QFont KNConfig::Appearance::articleFixedFont()
{
  if(u_seFonts)
    return f_onts[articleFixed];
  else
    return KGlobalSettings::fixedFont();
}


QFont KNConfig::Appearance::composerFont()
{
  if(u_seFonts)
    return f_onts[composer];
  else
    return KGlobalSettings::fixedFont();
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


QString KNConfig::Appearance::quotedTextHexcode(int i)
{
  switch(i) {
    case quoted1Hex:
      return quoteColor1().name();
    case quoted2Hex:
      return quoteColor2().name();
    case quoted3Hex:
      return quoteColor3().name();
  }
  return textColor().name();
}


QColor KNConfig::Appearance::defaultColor(int i)
{
  switch(i) {

    case background:
      return kapp->palette().active().base();
    break;

    case alternateBackground:
      return KGlobalSettings::alternateBackgroundColor();

    case header:
      return kapp->palette().active().background();
    break;

  case quoted1:
      return QColor( 0x00, 0x80, 0x00 );
      break;
    case quoted2:
        return QColor( 0x00, 0x70, 0x00 );
        break;
    case quoted3:
        return QColor( 0x00, 0x60, 0x00 );
        break;

    case normalText:
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
      return kapp->palette().active().text();
    break;

    case readArticle:
      return kapp->palette().inactive().text();
      break;
  }

  return kapp->palette().disabled().text();
}


QFont KNConfig::Appearance::defaultFont(int i)
{
  if (i == 1 || i == 2)
    return KGlobalSettings::fixedFont();
  else
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
  KConfig *conf=knGlobals.config();
  conf->setGroup("READNEWS");

  a_utoCheck=conf->readBoolEntry("autoCheck", true);
  m_axFetch=conf->readNumEntry("maxFetch", 1000);
  if (m_axFetch<0) m_axFetch = 0;
  a_utoMark=conf->readBoolEntry("autoMark", true);
  m_arkSecs=conf->readNumEntry("markSecs", 0);
  if (m_arkSecs<0) m_arkSecs = 0;
  m_arkCrossposts=conf->readBoolEntry("markCrossposts", true);
  s_martScrolling=conf->readBoolEntry("smartScrolling", true);
  t_otalExpand=conf->readBoolEntry("totalExpand", true);
  d_efaultExpand=conf->readBoolEntry("defaultExpand", false);
  s_howLines=conf->readBoolEntry("showLines3", true);
  s_howScore=conf->readBoolEntry("showScore3", true);
  s_howUnread=conf->readBoolEntry("showUnread", true);
  s_howThreads = conf->readBoolEntry("showThreads", true);
  a_utoCheckPgpSigs = conf->readBoolEntry("autoCheckPgpSigs",false);

  conf->setGroup("CACHE");
  c_ollCacheSize=conf->readNumEntry("collMemSize", 2048);
  a_rtCacheSize=conf->readNumEntry("artMemSize", 1024);
}


KNConfig::ReadNewsGeneral::~ReadNewsGeneral()
{
}


void KNConfig::ReadNewsGeneral::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::ReadNewsGeneral::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("READNEWS");

  conf->writeEntry("autoCheck", a_utoCheck);
  conf->writeEntry("maxFetch", m_axFetch);
  conf->writeEntry("autoMark", a_utoMark);
  conf->writeEntry("markSecs", m_arkSecs);
  conf->writeEntry("markCrossposts", m_arkCrossposts);
  conf->writeEntry("smartScrolling", s_martScrolling);
  conf->writeEntry("totalExpand", t_otalExpand);
  conf->writeEntry("defaultExpand", d_efaultExpand);
  conf->writeEntry("showLines3", s_howLines);
  conf->writeEntry("showScore3", s_howScore);
  conf->writeEntry("showUnread", s_howUnread);
  conf->writeEntry("showThreads", s_howThreads);
  conf->writeEntry("autoCheckPgpSigs", a_utoCheckPgpSigs);

  conf->setGroup("CACHE");
  conf->writeEntry("collMemSize", c_ollCacheSize);
  conf->writeEntry("artMemSize", a_rtCacheSize);
  conf->sync();
  d_irty = false;
}


//==============================================================================================================


KNConfig::ReadNewsNavigation::ReadNewsNavigation()
{
  KConfig *conf=knGlobals.config();
  conf->setGroup("READNEWS_NAVIGATION");

  m_arkAllReadGoNext=conf->readBoolEntry("markAllReadGoNext", false);
  m_arkThreadReadGoNext=conf->readBoolEntry("markThreadReadGoNext", false);
  m_arkThreadReadCloseThread=conf->readBoolEntry("markThreadReadCloseThread", false);
  i_gnoreThreadGoNext=conf->readBoolEntry("ignoreThreadGoNext", false);
  i_gnoreThreadCloseThread=conf->readBoolEntry("ignoreThreadCloseThread", false);
}


KNConfig::ReadNewsNavigation::~ReadNewsNavigation()
{
}


void KNConfig::ReadNewsNavigation::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::ReadNewsNavigation::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("READNEWS_NAVIGATION");

  conf->writeEntry("markAllReadGoNext", m_arkAllReadGoNext);
  conf->writeEntry("markThreadReadGoNext", m_arkThreadReadGoNext);
  conf->writeEntry("markThreadReadCloseThread", m_arkThreadReadCloseThread);
  conf->writeEntry("ignoreThreadGoNext", i_gnoreThreadGoNext);
  conf->writeEntry("ignoreThreadCloseThread", i_gnoreThreadCloseThread);
  conf->sync();
  d_irty = false;
}


//==============================================================================================================


KNConfig::ReadNewsViewer::ReadNewsViewer()
{
  KConfig *conf=knGlobals.config();
  conf->setGroup("READNEWS");

  s_howHeaderDeco=conf->readBoolEntry("showHeaderDeco", true);
  r_ewrapBody=conf->readBoolEntry("rewrapBody", true);
  r_emoveTrailingNewlines=conf->readBoolEntry("removeTrailingNewlines", true);
  s_howSig=conf->readBoolEntry("showSig", true);
  i_nterpretFormatTags=conf->readBoolEntry("interpretFormatTags", true);
  q_uoteCharacters=conf->readEntry("quoteCharacters",">:");
  i_nlineAtt=conf->readBoolEntry("inlineAtt", true);
  o_penAtt=conf->readBoolEntry("openAtt", false) ;
  s_howAlts=conf->readBoolEntry("showAlts", false);
  f_ullHdrs=conf->readBoolEntry("fullHdrs", false);
  u_seFixedFont=conf->readBoolEntry("articleBodyFixedFont", false);
  QString s = conf->readEntry("Browser","Default");
  if (s=="Konqueror")
    b_rowser = BTkonq;
  else if (s=="Netscape")
    b_rowser = BTnetscape;
  else if (s=="Mozilla")
    b_rowser = BTmozilla;
  else if (s=="Opera")
    b_rowser = BTopera;
  else if (s=="Other")
    b_rowser = BTother;
  else
    b_rowser = BTdefault;
  b_rowserCommand=conf->readPathEntry("BrowserCommand","netscape %u");
}


KNConfig::ReadNewsViewer::~ReadNewsViewer()
{
}


void KNConfig::ReadNewsViewer::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::ReadNewsViewer::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("READNEWS");

  conf->writeEntry("showHeaderDeco", s_howHeaderDeco);
  conf->writeEntry("rewrapBody", r_ewrapBody);
  conf->writeEntry("removeTrailingNewlines", r_emoveTrailingNewlines);
  conf->writeEntry("showSig", s_howSig);
  conf->writeEntry("interpretFormatTags", i_nterpretFormatTags);
  conf->writeEntry("quoteCharacters",q_uoteCharacters);
  conf->writeEntry("inlineAtt", i_nlineAtt);
  conf->writeEntry("openAtt", o_penAtt);
  conf->writeEntry("showAlts", s_howAlts);
  conf->writeEntry("fullHdrs", f_ullHdrs);
  conf->writeEntry("articleBodyFixedFont", u_seFixedFont);
  switch (b_rowser) {
    case BTdefault: conf->writeEntry("Browser","Default");
		    break;
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
  conf->writePathEntry("BrowserCommand", b_rowserCommand);
  conf->sync();
  d_irty = false;
}


//==============================================================================================================


KNConfig::DisplayedHeaders::DisplayedHeaders()
{
  h_drList.setAutoDelete(true);

  QString fname( locate("data","knode/headers.rc") );

  if (!fname.isNull()) {
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
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::DisplayedHeaders::save()" << endl;

  QString dir(locateLocal("data","knode/"));
  if (dir.isNull()) {
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
  headerConf.sync();
  d_irty = false;
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


KNConfig::Scoring::Scoring()
{
  KConfig *conf=knGlobals.config();
  conf->setGroup("SCORING");

  i_gnoredThreshold=conf->readNumEntry("ignoredThreshold", -100);
  w_atchedThreshold=conf->readNumEntry("watchedThreshold", 100);
}


KNConfig::Scoring::~Scoring()
{
}


void KNConfig::Scoring::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::Scoring::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("SCORING");

  conf->writeEntry("ignoredThreshold", i_gnoredThreshold);
  conf->writeEntry("watchedThreshold", w_atchedThreshold);
  conf->sync();
  d_irty = false;
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

  KConfig *conf=knGlobals.config();
  conf->setGroup("POSTNEWS");

  c_omposerCharsets=conf->readListEntry("ComposerCharsets");
  if (c_omposerCharsets.isEmpty())
    c_omposerCharsets=QStringList::split(',',"us-ascii,utf-8,iso-8859-1,iso-8859-2,"
    "iso-8859-3,iso-8859-4,iso-8859-5,iso-8859-6,iso-8859-7,iso-8859-8,"
    "iso-8859-9,iso-8859-10,iso-8859-13,iso-8859-14,iso-8859-15,koi8-r,koi8-u,"
    "iso-2022-jp,iso-2022-jp-2,iso-2022-kr,euc-jp,euc-kr,Big5,gb2312");

  c_harset=conf->readEntry("Charset").latin1();
  if (c_harset.isEmpty()) {
    QCString localeCharset(QTextCodec::codecForLocale()->mimeName());

    // special logic for japanese users:
    // "euc-jp" is default encoding for them, but in the news
    // "iso-2022-jp" is used
    if (localeCharset.lower() == "euc-jp")
      localeCharset = "iso-2022-jp";

    c_harset=findComposerCharset(localeCharset);
    if (c_harset.isEmpty())
      c_harset="iso-8859-1";  // shit
  }

  h_ostname=conf->readEntry("MIdhost").latin1();
  a_llow8BitBody=conf->readBoolEntry("8BitEncoding",true);
  u_seOwnCharset=conf->readBoolEntry("UseOwnCharset",true);
  g_enerateMID=conf->readBoolEntry("generateMId", false);
  d_ontIncludeUA=conf->readBoolEntry("dontIncludeUA", false);
  u_seExternalMailer=conf->readBoolEntry("useExternalMailer", false);

  QString dir(locateLocal("data","knode/"));
  if (!dir.isNull()) {
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
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::PostNewsTechnical::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("POSTNEWS");

  conf->writeEntry("ComposerCharsets", c_omposerCharsets);
  conf->writeEntry("Charset", QString::fromLatin1(c_harset));
  conf->writeEntry("8BitEncoding", a_llow8BitBody);
  conf->writeEntry("UseOwnCharset", u_seOwnCharset);
  conf->writeEntry("generateMId", g_enerateMID);
  conf->writeEntry("MIdhost", QString::fromLatin1(h_ostname));
  conf->writeEntry("dontIncludeUA", d_ontIncludeUA);
  conf->writeEntry("useExternalMailer", u_seExternalMailer);

  QString dir(locateLocal("data","knode/"));
  if (dir.isNull())
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
  conf->sync();
  d_irty = false;
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

  QCString s;

  QStringList::Iterator it;
  for( it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
    // match by name
    if ((*it).lower()==cs.lower().data()) {
      s = (*it).latin1();
      break;
    }
  }

  if (s.isEmpty()) {
    for( it = c_omposerCharsets.begin(); it != c_omposerCharsets.end(); ++it ) {
    // match by charset, avoid to return "us-ascii" for iso-8859-1
      if ((*it).lower()!="us-ascii") {
        QTextCodec *composerCodec = QTextCodec::codecForName((*it).latin1());
        QTextCodec *csCodec = QTextCodec::codecForName(cs);
        if ((composerCodec != 0) &&
            (csCodec != 0) &&
            (0 == strcmp(composerCodec->name(), csCodec->name()))) {
      s = (*it).latin1();
      break;
    }
  }
    }
  }

  if (s.isEmpty())
    s = "us-ascii";

  findComposerCSCache.insert(cs, new QCString(s));

  return s;
}


//==============================================================================================================


KNConfig::PostNewsComposer::PostNewsComposer()
{
  KConfig *conf=knGlobals.config();
  conf->setGroup("POSTNEWS");

  w_ordWrap=conf->readBoolEntry("wordWrap",true);
  m_axLen=conf->readNumEntry("maxLength", 76);
  a_ppSig=conf->readBoolEntry("appSig",true);
  r_ewrap=conf->readBoolEntry("rewrap",true);
  i_ncSig=conf->readBoolEntry("incSig",false);
  c_ursorOnTop=conf->readBoolEntry("cursorOnTop",false);
  u_seExtEditor=conf->readBoolEntry("useExternalEditor",false);
  i_ntro=conf->readEntry("Intro","%NAME wrote:");
  e_xternalEditor=conf->readEntry("externalEditor","kwrite %f");
}


KNConfig::PostNewsComposer::~PostNewsComposer()
{
}


void KNConfig::PostNewsComposer::save()
{
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::PostNewsComposer::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("POSTNEWS");

  conf->writeEntry("wordWrap", w_ordWrap);
  conf->writeEntry("maxLength", m_axLen);
  conf->writeEntry("appSig", a_ppSig);
  conf->writeEntry("rewrap",r_ewrap);
  conf->writeEntry("incSig", i_ncSig);
  conf->writeEntry("cursorOnTop", c_ursorOnTop);
  conf->writeEntry("useExternalEditor", u_seExtEditor);
  conf->writeEntry("Intro", i_ntro);
  conf->writeEntry("externalEditor", e_xternalEditor);
  conf->sync();

  d_irty = false;
}

//==============================================================================================================




// BEGIN: Cleanup configuration ===============================================


KNConfig::Cleanup::Cleanup( bool global ) :
  // default values for new accounts / groups
  d_oExpire( true ), r_emoveUnavailable( true ), p_reserveThr( true ),
  e_xpireInterval( 5 ), r_eadMaxAge( 10 ), u_nreadMaxAge( 15 ),
  mGlobal(global), mDefault(!global), mLastExpDate( QDate::currentDate() )
{
  if (mGlobal) {
    KConfig *conf = knGlobals.config();
    conf->setGroup( "EXPIRE" );
    loadConfig( conf );
  }
}


void KNConfig::Cleanup::loadConfig(KConfigBase *conf)
{
  // group expire settings
  d_oExpire = conf->readBoolEntry( "doExpire", true );
  r_emoveUnavailable = conf->readBoolEntry( "removeUnavailable", true );
  p_reserveThr = conf->readBoolEntry( "saveThreads", true );
  e_xpireInterval = conf->readNumEntry( "expInterval", 5 );
  r_eadMaxAge = conf->readNumEntry( "readDays", 10 );
  u_nreadMaxAge = conf->readNumEntry( "unreadDays", 15 );
  mLastExpDate = conf->readDateTimeEntry( "lastExpire" ).date();

  // folder compaction settings (only available globally)
  if (mGlobal) {
    d_oCompact = conf->readBoolEntry( "doCompact", true );
    c_ompactInterval = conf->readNumEntry( "comInterval", 5 );
    mLastCompDate = conf->readDateTimeEntry( "lastCompact" ).date();
  }

  if (!mGlobal)
    mDefault = conf->readBoolEntry( "UseDefaultExpConf", true );
}


void KNConfig::Cleanup::saveConfig(KConfigBase *conf)
{
  // group expire settings
  conf->writeEntry( "doExpire", d_oExpire );
  conf->writeEntry( "removeUnavailable", r_emoveUnavailable );
  conf->writeEntry( "saveThreads", p_reserveThr );
  conf->writeEntry( "expInterval", e_xpireInterval );
  conf->writeEntry( "readDays", r_eadMaxAge );
  conf->writeEntry( "unreadDays", u_nreadMaxAge );
  conf->writeEntry( "lastExpire", mLastExpDate );

  // folder compaction settings (only available globally)
  if (mGlobal) {
    conf->writeEntry( "doCompact", d_oCompact );
    conf->writeEntry( "comInterval", c_ompactInterval );
    conf->writeEntry( "lastCompact", mLastCompDate );
  }

  if (!mGlobal)
    conf->writeEntry( "UseDefaultExpConf", mDefault );

  conf->sync();
}


void KNConfig::Cleanup::save()
{
  kdDebug(5003) << "KNConfig::Cleanup::save()" << endl;
  if (mGlobal) {
    KConfig *conf = knGlobals.config();
    conf->setGroup( "EXPIRE" );
    saveConfig( conf );
  }
}


bool KNConfig::Cleanup::expireToday()
{
  if (!d_oExpire)
    return false;

  QDate today = QDate::currentDate();
  if (mLastExpDate == today)
    return false;

  return (mLastExpDate.daysTo( today ) >= e_xpireInterval);
}


void KNConfig::Cleanup::setLastExpireDate()
{
  mLastExpDate = QDateTime::currentDateTime();
}


bool KNConfig::Cleanup::compactToday()
{
  if (!d_oCompact)
    return false;

  QDate today = QDate::currentDate();
  if (mLastCompDate == today)
    return false;

  return (mLastCompDate.daysTo( today ) >= c_ompactInterval);
}


void KNConfig::Cleanup::setLastCompactDate()
{
  mLastCompDate = QDateTime::currentDateTime();
}


// END: Cleanup configuration =================================================



/*KNConfig::Cache::Cache()
{
  KConfig *conf=knGlobals.config();
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
  if(!d_irty)
    return;

  kdDebug(5003) << "KNConfig::Cache::save()" << endl;

  KConfig *conf=knGlobals.config();
  conf->setGroup("CACHE");

  conf->writeEntry("memMaxArt", m_emMaxArt);
  conf->writeEntry("memMaxKB", m_emMaxKB);

  conf->writeEntry("diskMaxArt", d_iskMaxArt);
  conf->writeEntry("diskMaxKB", d_iskMaxKB);

  d_irty = false;
}
*/
