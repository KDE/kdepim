 /*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_FEATUREPLAN_H
#define KCAL_FEATUREPLAN_H

namespace KCalFeaturePlan {

class Responsible
{
  public:
    QString name() { return mName; }
    QString email() { return mEmail; }

  private:
    QString mName;
    QString mEmail;
};

class Feature
{
  public:
    QString summary() { return mSummary; }
    QValueList<Responsible> responsibleList() { return mResponsibleList; }

    QString status() { return mStatus; }
    QString target() { return mTarget; }

  private:
    QString mSummary;
    QValueList<Responsible> mReponsibleList;
    
    QString mStatus;
    QString mTarget;
};

class Category
{
  public:
    QValueList<Category> categoryList() { return mCategoryList; }
    QValueList<Feature> featureList() { return mFeatureList; }

    QString name() { return mName; }

  private:
    QValueList<Category> mCategoryList;
    QValueList<Feature> mFeatureList;

    QString mName;
};

class Features
{
  public:
    QValueList<Category> categoryList() { return mCategoryList; }

    bool readFromFile( const QString &filename );

  private:
    QValueList<Category> mCategoryList;
};

}

#endif
