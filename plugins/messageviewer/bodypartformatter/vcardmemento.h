/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef VCARDMEMENTO_H
#define VCARDMEMENTO_H

#include <messageviewer/interfaces/bodypart.h>

#include <messageviewer/viewer/viewer.h>

#include <QObject>
#include <QMap>
#include <KABC/kabc/Addressee>

class KJob;

namespace MessageViewer {

struct VCard {
    VCard(const QString& str, bool b)
        : email(str), found(b)
    {
    }
    KABC::Addressee address;
    QString email;
    bool found;
};

class VcardMemento : public QObject, public Interface::BodyPartMemento
{
Q_OBJECT
public:
  explicit VcardMemento( const QStringList& emails );
  ~VcardMemento();

   bool finished() const;

   virtual void detach();

   bool vcardExist(int index) const;

   KABC::Addressee address( int index ) const;

private Q_SLOTS:
   void slotSearchJobFinished( KJob *job );


Q_SIGNALS:
  // TODO: Factor our update and detach into base class
  void update( MessageViewer::Viewer::UpdateMode );

private:
  void checkEmail();
  void continueToCheckEmail();
  QList<VCard> mVCardList;
  int mIndex;
  bool mFinished;
};
}

#endif // VCARDMEMENTO_H
