/*
    knarticlefilter.cpp

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

#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include "kngroup.h"
#include "knfolder.h"
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
: i_d(id), c_ount(0), l_oaded(false), e_nabled(true), translateName(true), s_earchFilter(false), apon(articles)
{}



// constructs a copy of org
KNArticleFilter::KNArticleFilter(const KNArticleFilter& org)
: i_d(-1), c_ount(0), l_oaded(false), e_nabled(org.e_nabled), translateName(true), s_earchFilter(org.s_earchFilter), apon(org.apon)
{
  status = org.status;
  score = org.score;
  age = org.age;
  lines = org.lines;
  subject = org.subject;
  from = org.from;
  messageId = org.messageId;
  references = org.messageId;
}



KNArticleFilter::~KNArticleFilter()
{}



bool KNArticleFilter::loadInfo()
{
  if (i_d!=-1) {
    QString fname(KGlobal::dirs()->findResource("appdata",QString("filters/%1.fltr").arg(i_d)));
    if (fname.isNull())
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
  if (fname.isNull())
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

  conf.setGroup("MESSAGEID");
  messageId.load(&conf);

  conf.setGroup("REFERENCES");
  references.load(&conf);
  
  l_oaded=true;
  
  kdDebug(5003) << "KNMessageFilter: filter loaded \"" << n_ame << "\" " << endl;
  
}



void KNArticleFilter::save()
{
  if (i_d==-1)
    return;
  QString dir(KGlobal::dirs()->saveLocation("appdata","filters/"));
  if (dir.isNull()) {
    KNHelper::displayInternalFileError();
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

  conf.setGroup("MESSAGEID");
  messageId.save(&conf);

  conf.setGroup("REFERENCES");
  references.save(&conf);
  
  kdDebug(5003) << "KNMessageFilter: filter saved \"" << n_ame << "\" " << endl;
}



void KNArticleFilter::doFilter(KNGroup *g)
{
  c_ount=0;
  KNRemoteArticle *art=0, *ref=0;
  KNRemoteArticle::List orphant_threads;
  int idRef;
  int mergeCnt=0;
  bool inThread=false;

  if(!l_oaded) load();

  subject.expand(g);  // replace placeholders
  from.expand(g);
  messageId.expand(g);
  references.expand(g);

  for(int idx=0; idx<g->length(); idx++) {
    art=g->at(idx);
    art->setFiltered(false);
    art->setVisibleFollowUps(false);
    art->setDisplayedReference(0);
  }

  for(int idx=0; idx<g->length(); idx++) {
  
    art=g->at(idx);

    if(!art->isFiltered() && applyFilter(art) && apon==threads) {
      idRef=art->idRef();
      while(idRef!=0) {
        ref=g->byId(idRef);
        ref->setFilterResult(true);
        ref->setFiltered(true);
        if ( idRef==ref->idRef() ) break;
        idRef=ref->idRef();
      }
    }

  }

  for(int idx=0; idx<g->length(); idx++) {

    art=g->at(idx);

    if( apon==threads && !art->filterResult() ) {
      inThread=false;
      idRef=art->idRef();
      while(idRef!=0 && !inThread) {
        ref=g->byId(idRef);
        inThread=ref->filterResult();
        idRef=ref->idRef();
      }
      art->setFilterResult(inThread);
    }

    if(art->filterResult()) {
      c_ount++;

      ref = (art->idRef()>0) ? g->byId(art->idRef()) : 0;
      while(ref && !ref->filterResult())
        ref = (ref->idRef()>0) ? g->byId(ref->idRef()) : 0;

      art->setDisplayedReference(ref);
      if(ref)
        ref->setVisibleFollowUps(true);
      else if(art->idRef()>0) {
        orphant_threads.append(art);
      }
    }

  }

  if( orphant_threads.count() > 0 ) {
    // try to merge orphant threads by subject
    KNRemoteArticle::List same_subjects;
    QString s;
    for(art=orphant_threads.first(); art; art=orphant_threads.next()) {
      if(art->displayedReference()) // already processed
        continue;

      s=art->subject()->asUnicodeString();
      same_subjects.clear();
      for(QPtrListIterator<KNRemoteArticle> it(orphant_threads); it.current(); ++it) {
        if(it.current()!=art && it.current()->subject()->asUnicodeString()==s)
          same_subjects.append(it.current());
      }

      art->setVisibleFollowUps( art->hasVisibleFollowUps() || (same_subjects.count()>0) );
      for(QPtrListIterator<KNRemoteArticle> it(same_subjects); it.current(); ++it) {
        it.current()->setDisplayedReference(art);
        mergeCnt++;
      }
    }
  }

  kdDebug(5003) << "KNArticleFilter::doFilter() : matched " << c_ount
                << " articles , merged " << mergeCnt
                << " threads by subject" << endl;

}


void KNArticleFilter::doFilter(KNFolder *f)
{
  c_ount=0;
  KNLocalArticle *art=0;

  if(!l_oaded) load();

  subject.expand(0);  // replace placeholders
  from.expand(0);
  messageId.expand(0);
  references.expand(0);

  for(int idx=0; idx<f->length(); idx++) {
    art=f->at(idx);
    if (applyFilter(art))
      c_ount++;
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
  if(result) result=messageId.doFilter(a->messageID()->asUnicodeString());
  if(result) result=references.doFilter(a->references()->asUnicodeString());

  a->setFilterResult(result);
  a->setFiltered(true);
    
  return result;
}


bool KNArticleFilter::applyFilter(KNLocalArticle *a)
{
  bool result=true;

  if (isSearchFilter()) {
    if(result) result=lines.doFilter(a->lines()->numberOfLines());
    if(result) result=age.doFilter(a->date()->ageInDays());
    if(result) result=subject.doFilter(a->subject()->asUnicodeString());
    if(result) {
      QString tmp = (a->from()->name()+"##") + QString(a->from()->email().data());
      result=from.doFilter(tmp);
    }
    if(result) result=messageId.doFilter(a->messageID()->asUnicodeString());
    if(result) result=references.doFilter(a->references()->asUnicodeString());
  }

  a->setFilterResult(result);

  return result;
}
