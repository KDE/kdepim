/*
    knscoring.h

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

#ifndef KNSCORING_H
#define KNSCORING_H

#include <kscoring.h>

class KDialogBase;
class KNRemoteArticle;
class KNGroup;


class KNScorableArticle : public ScorableArticle
{
public:
  KNScorableArticle(KNRemoteArticle*);
  virtual ~KNScorableArticle();

  virtual void addScore(short s);
  virtual void changeColor(const QColor&);
  virtual void displayMessage(const QString&);
  virtual QString from() const;
  virtual QString subject() const;
  virtual QString getHeaderByType(const QString&) const;

  static NotifyCollection* notifyC;

private:
  KNRemoteArticle *_a;
};


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
};

#endif
