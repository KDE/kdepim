/*
    knarticlefilter.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qcombobox.h>

#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include "kngroup.h"
#include "knmime.h"
#include "utilities.h"
#include "knarticlefilter.h"

//=============================================================================================================


// the names of our default filters
static const char *defFil[] = { "all","unread","new","watched","threads with unread",
                                "threads with new","own articles","threads with own articles", 0 };
void dummyFilter()
{
  i18n("default filter name","all");
  i18n("default filter name","unread");
  i18n("default filter name","new");
  i18n("default filter name","watched");
  i18n("default filter name","threads with unread");
  i18n("default filter name","threads with new");
  i18n("default filter name","own articles");
  i18n("default filter name","threads with own articles");
}


//=============================================================================================================


KNArticleFilter::KNArticleFilter(int id)
: i_d(id), c_ount(0), l_oaded(false), e_nabled(true), translateName(true), apon(articles)
{}



// constructs a copy of org
KNArticleFilter::KNArticleFilter(const KNArticleFilter& org)
: i_d(-1), c_ount(0), l_oaded(false), e_nabled(org.e_nabled), translateName(true), apon(org.apon)
{
  status = org.status;
  score = org.score;
  age = org.age;
  lines = org.lines;
  subject = org.subject;
  from = org.from;
}



KNArticleFilter::~KNArticleFilter()
{}



bool KNArticleFilter::loadInfo()
{
  if (i_d!=-1) {
    QString fname(KGlobal::dirs()->findResource("appdata",QString("filters/%1.fltr").arg(i_d)));
    if (fname==QString::null)
      return false;
    KSimpleConfig conf(fname,true);

    conf.setGroup("GENERAL");
    n_ame=conf.readEntry("name");
    translateName = conf.readBoolEntry("Translate_Name",true);
    e_nabled=conf.readBoolEntry("enabled", true);
    apon=(ApOn) conf.readNumEntry("applyOn", 0);
    return true;
  }
  return false;
}



void KNArticleFilter::load()
{
  QString fname(KGlobal::dirs()->findResource("appdata",QString("filters/%1.fltr").arg(i_d)));
  if (fname==QString::null)
    return;
  KSimpleConfig conf(fname,true);
  
  conf.setGroup("STATUS");
  status.load(&conf);
  
  conf.setGroup("SCORE");
  score.load(&conf);
  
  conf.setGroup("AGE");
  age.load(&conf);
  
  conf.setGroup("LINES");
  lines.load(&conf);
  
  conf.setGroup("SUBJECT");
  subject.load(&conf);
  
  conf.setGroup("FROM");
  from.load(&conf);
  
  l_oaded=true;
  
  kdDebug(5003) << "KNMessageFilter: filter loaded \"" << n_ame << "\" " << endl;
  
}



void KNArticleFilter::save()
{
  if (i_d==-1)
    return;
  QString dir(KGlobal::dirs()->saveLocation("appdata","filters/"));
  if (dir==QString::null) {
    displayInternalFileError();
    return;
  }
  KSimpleConfig conf(dir+QString("%1.fltr").arg(i_d));
  
  conf.setGroup("GENERAL");
  conf.writeEntry("name", QString(n_ame));
  conf.writeEntry("Translate_Name",translateName);
  conf.writeEntry("enabled", e_nabled);
  conf.writeEntry("applyOn", (int) apon);
  
  conf.setGroup("STATUS");
  status.save(&conf);
  
  conf.setGroup("SCORE");
  score.save(&conf);
  
  conf.setGroup("AGE");
  age.save(&conf);
  
  conf.setGroup("LINES");
  lines.save(&conf);
  
  conf.setGroup("SUBJECT");
  subject.save(&conf);
  
  conf.setGroup("FROM");
  from.save(&conf);
  
  kdDebug(5003) << "KNMessageFilter: filter saved \"" << n_ame << "\" " << endl;
}



void KNArticleFilter::doFilter(KNGroup *g)
{
  c_ount=0;
  KNRemoteArticle *art, *ref;
  int idRef;
  
  if(!l_oaded) load();

  subject.expand(g);  // replace placeholders
  from.expand(g);

  for(int idx=0; idx<g->length(); idx++) {
    art=g->at(idx);
    art->setFiltered(false);
    art->setVisibleFollowUps(false);
  }
  
    
  for(int idx=0; idx<g->length(); idx++) {
  
    art=g->at(idx);

    if(!art->isFiltered() && applyFilter(art)) {
      if(apon==threads) {
        idRef=art->idRef();
        while(idRef!=0) {
          ref=g->byId(idRef);
          ref->setFilterResult(true);
          ref->setFiltered(true);
          ref->setVisibleFollowUps(true);
          idRef=ref->idRef();
        }
      }
      else {
        if(art->idRef() > 0) {
          ref=g->byId(art->idRef());
          if(ref)
            ref->setVisibleFollowUps(true);
        }
        c_ount++;
      }
    }
  }
    
      
  if(apon==threads) {
    bool inThread;
    for(int idx=0; idx<g->length(); idx++) {
      art=g->at(idx);
      if(!art->filterResult()) {
        inThread=false;
        idRef=art->idRef();
        
        while(idRef!=0 && !inThread) {
          ref=g->byId(idRef);
          inThread=ref->filterResult();
          idRef=ref->idRef();
        }
        if(inThread) {
          art->setFilterResult(true);
          g->byId(art->idRef())->setVisibleFollowUps(true);
          c_ount++;
        }
      }
      else c_ount++;
    }
  }
}



// *trys* to translate the name
QString KNArticleFilter::translatedName()
{
  if (translateName) {
    // major hack alert !!!
    if (!n_ame.isEmpty()) {
      if (i18n("default filter name",n_ame.local8Bit())!=n_ame.local8Bit().data())    // try to guess if this english or not
        return i18n("default filter name",n_ame.local8Bit());
      else
        return n_ame;
    } else
      return QString::null;
  } else
    return n_ame;
}



// *trys* to retranslate the name to english
void KNArticleFilter::setTranslatedName(const QString &s)
{
  bool retranslated = false;
  for (const char **c=defFil;(*c)!=0;c++)   // ok, try if it matches any of the standard filter names
    if (s==i18n("default filter name",*c)) {
      n_ame = QString::fromLatin1(*c);
      retranslated = true;
      break;
    }

  if (!retranslated) {      // ok, we give up and store the maybe non-english string
    n_ame = s;
    translateName = false;  // and don't try to translate it, so a german user *can* use the original english name
  } else
    translateName = true;
}



bool KNArticleFilter::applyFilter(KNRemoteArticle *a)
{
  bool result=true;
  
  if(result) result=status.doFilter(a);
  if(result) result=score.doFilter(a->score());
  if(result) result=lines.doFilter(a->lines()->numberOfLines());
  if(result) result=age.doFilter(a->date()->ageInDays());
  if(result) result=subject.doFilter(a->subject()->asUnicodeString());
  if(result) {
    QString tmp = (a->from()->name()+"##") + QString(a->from()->email().data());
    result=from.doFilter(tmp);
  }

  a->setFilterResult(result);
  a->setFiltered(true);
    
  return result;
}



/*bool KNArticleFilter::applyFilter(KNSavedArticle *a)
{
  bool result=true;
  
  if(result) result=age.doFilter(a->age()); 
  if(result) result=subject.doFilter(a->subject());
  //if(result) result=from.doFilter(a->fromName());
      
  return result;
} */
