/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  The program is free software; you can redistribute it and/or modify it
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

#ifndef TRANSLATEWIDGET_H
#define TRANSLATEWIDGET_H
#include "mailcommon_export.h"
#include <QWidget>
#include <kio/job.h>

namespace MailCommon {
class MAILCOMMON_EXPORT TranslateWidget : public QWidget
{
  Q_OBJECT
public:
  explicit TranslateWidget( QWidget* parent = 0 );
  ~TranslateWidget();
  void setTextToTranslate( const QString& );
public Q_SLOTS:
  void slotTranslate();

private Q_SLOTS:
  void slotDataReceived ( KIO::Job *job, const QByteArray &data );
  void slotJobDone(KJob*);

private:
  void initLanguage();
  class TranslateWidgetPrivate;
  TranslateWidgetPrivate *const d;

};
}

#endif /* TRANSLATEWIDGET_H */

