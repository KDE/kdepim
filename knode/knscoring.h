/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNSCORING_H
#define KNSCORING_H

#include "knarticle.h"
#include "kscoring.h"

using namespace KPIM;

/** Article interface for the scoring system. */
class KNScorableArticle : public ScorableArticle
{
public:
  explicit KNScorableArticle( KNRemoteArticle::Ptr a );
  virtual ~KNScorableArticle();

  virtual void addScore(short s);
  virtual void changeColor(const QColor&);
  virtual void displayMessage(const QString&);
  virtual QString from() const;
  virtual QString subject() const;
  virtual QString getHeaderByType(const QString&) const;
  virtual void markAsRead();

  static NotifyCollection* notifyC;

private:
  KNRemoteArticle::Ptr _a;
};


/** Group interface for the scoring system. */
class KNScorableGroup : public ScorableGroup
{
public:
  KNScorableGroup();
  virtual ~KNScorableGroup();
};


// class KNScorableServer : public ScorableServer
// {
// public:
//   virtual ~KNScorableServer();
// };


/** Scoring manager. */
class KNScoringManager : public KScoringManager
{
  Q_OBJECT

public:
  KNScoringManager();
  virtual ~KNScoringManager();
  virtual QStringList getGroups() const;
  virtual QStringList getDefaultHeaders() const;

  void configure();
  bool canColors()const { return true; }
  bool canMarkAsRead() const { return true; }
};

#endif
