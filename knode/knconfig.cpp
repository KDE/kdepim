#include "knconfig.h"

#include <qfile.h>
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

#include "knglobals.h"
#include "knnntpaccount.h"
#include "knaccountmanager.h"
#include "kngroupmanager.h"
#include "knviewheader.h"
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
  e_mail=c->readEntry("Email").latin1();
  r_eplyTo=c->readEntry("Reply-To");
  o_rga=c->readEntry("Org");
  u_seSigFile=c->readBoolEntry("UseSigFile",false);
  s_igPath=c->readEntry("sigFile");
  s_igText=c->readEntry("sigText");
}


void KNConfig::Identity::saveConfig(KConfigBase *c)
{
  c->writeEntry("Name", n_ame);
  c->writeEntry("Email", e_mail.data());
  c->writeEntry("Reply-To", r_eplyTo);
  c->writeEntry("Org", o_rga);
  c->writeEntry("UseSigFile", u_seSigFile);
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
            r_eplyTo.isEmpty()   && o_rga.isEmpty() &&
            ( s_igPath.isEmpty() || s_igText.isEmpty() ) );
}


QString KNConfig::Identity::getSignature()
{
  s_igContents = "";      // don't cache file contents

  if (u_seSigFile) {
    if(!s_igPath.isEmpty()) {
      QFile f(s_igPath);
      if(f.open(IO_ReadOnly)) {
        QTextStream ts(&f);
        while(!ts.atEnd())
          s_igContents += (ts.readLine()+"\n");
        f.close();
      }
      else
        KMessageBox::error(knGlobals.topWidget, i18n("Cannot open the signature file!"));
    }
  }
  else
    s_igContents = s_igText;

  if (!s_igContents.isEmpty() && !s_igContents.contains("\n-- \n") && !(s_igContents.left(4) == "-- \n"))
    s_igContents.prepend("-- \n");

  return s_igContents;
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
  c_olors[readArticle]=c->readColorEntry("readArticleColor",&defCol);
  c_olorNames[readArticle]=i18n("Read Article");

  //fonts
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
  i_cons[greyBall]        = UserIcon("greyball");
  i_cons[redBall]         = UserIcon("redball");
  i_cons[greyBallChkd]    = UserIcon("greyballchk");
  i_cons[redBallChkd]     = UserIcon("redballchk");
  i_cons[newFups]         = UserIcon("newsubs");
  i_cons[eyes]            = UserIcon("eyes");
  i_cons[mail]            = SmallIcon("mail_generic");
  i_cons[posting]         = SmallIcon("filenew");
  i_cons[canceledPosting] = SmallIcon("editdelete");
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
  c->writeEntry("readArticleColor", c_olors[readArticle]);

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


QColor KNConfig::Appearance::readArticleColor()
{
  if(u_seColors)
    return c_olors[readArticle];
  else
    return kapp->palette().disabled().text();
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


//==============================================================================================================


KNConfig::ReadNewsGeneral::ReadNewsGeneral()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("READNEWS");

  a_utoCheck=conf->readBoolEntry("autoCheck", true);
  a_utoMark=conf->readBoolEntry("autoMark", true);
  s_howThreads=conf->readBoolEntry("showThreads", true);
  t_otalExpand=conf->readBoolEntry("totalExpand", true);
  s_howFullHdrs=conf->readBoolEntry("fullHdrs", false);
  s_howSig=conf->readBoolEntry("showSig", true);
  i_nlineAtt=conf->readBoolEntry("inlineAtt", true);
  o_penAtt=conf->readBoolEntry("openAtt", false) ;
  s_howAlts=conf->readBoolEntry("showAlts", false);
  m_axFetch=conf->readNumEntry("maxFetch", 1000);
  m_arkSecs=conf->readNumEntry("markSecs", 5);
  b_rowser=(browserType)(conf->readNumEntry("Browser", 0));
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
  conf->writeEntry("autoMark", a_utoMark);
  conf->writeEntry("showThreads", s_howThreads);
  conf->writeEntry("totalExpand", t_otalExpand);
  conf->writeEntry("fullHdrs", s_howFullHdrs);
  conf->writeEntry("showSig", s_howSig);
  conf->writeEntry("inlineAtt", i_nlineAtt);
  conf->writeEntry("openAtt", o_penAtt);
  conf->writeEntry("showAlts", s_howAlts);
  conf->writeEntry("maxFetch", m_axFetch);
  conf->writeEntry("markSecs", m_arkSecs);
  conf->writeEntry("Browser", (int)b_rowser);

}





KNConfig::DisplayedHeaders::DisplayedHeaders()
{
  i_nstances.setAutoDelete(true);

  QString fname(KGlobal::dirs()->findResource("appdata","headers.rc"));
  if (fname != QString::null) {
    KSimpleConfig headerConf(fname,true);
    QStringList headers = headerConf.groupList();
    headers.remove("<default>");
    headers.sort();

    KNViewHeader *h;
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
    displayInternalFileError();
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

  for(Iterator it(i_nstances); it.current(); ++it) {
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


KNViewHeader* KNConfig::DisplayedHeaders::createNewHeader()
{
  KNViewHeader *h=new KNViewHeader();
  i_nstances.append(h);

  return h;
}


void KNConfig::DisplayedHeaders::remove(KNViewHeader *h)
{
  if (!i_nstances.remove(h))
    kdDebug(5003) << "KNConfig::DisplayedHeaders::remove() : cannot find pointer in list !!" << endl;

}


void KNConfig::DisplayedHeaders::up(KNViewHeader *h)
{
  int idx=i_nstances.findRef(h);
  if(idx!=-1) {
    i_nstances.take(idx);
    i_nstances.insert(idx-1, h);
  }
  else kdDebug(5003) << "KNConfig::DisplayedHeaders::up() : item not found in list" << endl;
}


void KNConfig::DisplayedHeaders::down(KNViewHeader *h)
{
  int idx=i_nstances.findRef(h);
  if(idx!=-1) {
    i_nstances.take(idx);
    i_nstances.insert(idx+1, h);
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
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

  c_harset=conf->readEntry("Charset","ISO-8859-1").upper().latin1();  // we should use charsetForLocale, but it has the wrong format
  h_ostname=conf->readEntry("MIdhost").latin1();
  e_ncoding=conf->readNumEntry("Encoding",1);
  a_llow8Bit=conf->readBoolEntry("allow8bitChars", false);
  g_enerateMID=conf->readBoolEntry("generateMId", false);
  d_ontIncludeUA=conf->readBoolEntry("dontIncludeUA", false);

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
  kdDebug(5003) << "KNConfig::PostNewsTechnical::saveAndNotify(" << endl;

  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

  conf->writeEntry("Charset", QString::fromLatin1(c_harset));
  conf->writeEntry("Encoding", e_ncoding);
  conf->writeEntry("allow8bitChars", a_llow8Bit);
  conf->writeEntry("generateMId", g_enerateMID);
  conf->writeEntry("MIdhost", QString::fromLatin1(h_ostname));
  conf->writeEntry("dontIncludeUA", d_ontIncludeUA);

  QString dir(KGlobal::dirs()->saveLocation("appdata"));
  if (dir==QString::null)
    displayInternalFileError();
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
      displayInternalFileError();
  }
}


//==============================================================================================================


KNConfig::PostNewsComposer::PostNewsComposer()
{
  KConfig *conf=KGlobal::config();
  conf->setGroup("POSTNEWS");

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

  conf->writeEntry("maxLength", m_axLen);
  conf->writeEntry("appSig", a_ppSig);
  conf->writeEntry("rewrap",r_ewrap);
  conf->writeEntry("incSig", i_ncSig);
  conf->writeEntry("useExternalEditor", u_seExtEditor);
  conf->writeEntry("Intro", i_ntro);
  conf->writeEntry("externalEditor", e_xternalEditor);

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
