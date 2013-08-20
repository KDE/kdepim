/*
  This file is part of libkdepim.

  Copyright (c) 20013 Franck Arrecot

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "completionaddressedelegate.h"
#include <QSize>
#include <qpainter.h>
#include <KDebug>
#include <KPeople/PersonData>
#include <KStandardDirs>
#include <KCompletionBox>
#include <QItemSelection>
#include <KJob>

#define SIZE_STANDARD_PIXMAP 25

using namespace KPIM;

class avatarHelper : public KJob
{
  Q_OBJECT

public :

    QString m_email;
    QPixmap m_defaultPixmap;

    avatarHelper(const QString& email) {
      m_email = email ;
      m_defaultPixmap = KStandardDirs::locate("data", "kdepimwidgets/pics/dummy_avatar.png");

      if (m_defaultPixmap.isNull()) kDebug() << "Failed to load default pixmap" ;

      m_defaultPixmap = m_defaultPixmap.scaled(QSize(SIZE_STANDARD_PIXMAP, SIZE_STANDARD_PIXMAP),
                                               Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
Q_SIGNALS:
  void avatarReady(QString &email, QPixmap def);

public Q_SLOTS:
    virtual void start()
    {
      QMetaObject::invokeMethod(this, "fetchAvatarHelper", Qt::QueuedConnection);
    }

    void fetchAvatarHelper()
    {
      KPeople::PersonDataPtr person = KPeople::PersonData::createFromContactId(m_email);
      QPixmap avatar(person->avatar().toLocalFile());
      avatar = (!avatar) ? m_defaultPixmap : avatar.scaled(QSize(SIZE_STANDARD_PIXMAP, SIZE_STANDARD_PIXMAP),
                                                                Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

      emit avatarReady(m_email, avatar);
      //deleteLater();
    }
};

CompletionAddresseDelegate::CompletionAddresseDelegate(KCompletionBox* parent)
{
  m_view = parent;
  m_defaultPixmap = KStandardDirs::locate("data", "kdepimwidgets/pics/dummy_avatar.png");
  m_defaultPixmap = m_defaultPixmap.scaled(QSize(SIZE_STANDARD_PIXMAP, SIZE_STANDARD_PIXMAP), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

CompletionAddresseDelegate::~CompletionAddresseDelegate()
{}

void CompletionAddresseDelegate::paint ( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  // Save some space for the pixmap
  QStyleOptionViewItem opt = QStyleOptionViewItem(option);
  opt.rect.translate(m_defaultPixmap.width(), 0);

  // set up the name and the email
  QStyledItemDelegate::paint(painter, opt, index);

  // retrieve the mail inside the string
  QString source = index.data(Qt::DisplayRole).toString() , email;
  QString begin("<"), end(">");
  int startIndex = source.indexOf(begin)+begin.length();
  int endIndex = source.indexOf(end,startIndex);

  if (source.indexOf(begin)+begin.length() > 0 && source.indexOf(end,startIndex) > 0) {

    const QString email = source.mid(startIndex,endIndex - startIndex) ;
    // TODO : make the standard pixmap here to avoid several person with the standard pix ?
    if (QPixmap* pix = m_cachedPixmaps[email]) {
      painter->drawPixmap(option.rect.topLeft() ,*pix);
    } else {
      if (!email.isEmpty() && !email.isNull()) {

        avatarHelper* helper = new avatarHelper(email);
        connect(helper, SIGNAL(avatarReady(QString&, QPixmap)), SLOT(onAvatarReady(QString&, QPixmap)));
        helper->start();
      }
    }
  }
  // some debug
//   foreach (const QString key , m_cachedPixmaps.keys()) {
//     kDebug() << key << "-> " << m_cachedPixmaps[key];
//   }
}

void CompletionAddresseDelegate::onAvatarReady(QString& email , QPixmap avatar)
{
  kDebug() << "Avatar Helper reception done !" ;

  QPixmap* avatarPtr = new QPixmap(avatar);
  m_cachedPixmaps.insert(email, avatarPtr);
  m_view->update();
}

QSize CompletionAddresseDelegate::sizeHint ( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  return QSize(
    option.fontMetrics.boundingRect(index.data(Qt::DisplayRole).toString()).height(),
    m_defaultPixmap.height() + 4
  );
}

#include "moc_completionaddressedelegate.cpp"
#include "completionaddressedelegate.moc"