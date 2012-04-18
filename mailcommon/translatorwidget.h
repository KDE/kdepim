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

#ifndef TRANSLATORWIDGET_H
#define TRANSLATORWIDGET_H
#include "mailcommon_export.h"
#include <QWidget>
#include <kio/job.h>

namespace MailCommon {
class MAILCOMMON_EXPORT TranslatorWidget : public QWidget
{
  Q_OBJECT
public:
  explicit TranslatorWidget( QWidget* parent = 0 );
  explicit TranslatorWidget( const QString& text, QWidget* parent = 0 );
  ~TranslatorWidget();
  void setTextToTranslate( const QString& );
  void writeConfig();
  void readConfig();

  
public Q_SLOTS:
  void slotTranslate();

private Q_SLOTS:
  void slotDataReceived ( KIO::Job *job, const QByteArray &data );
  void slotJobDone(KJob*);
  void slotFromLanguageChanged( int );
  void slotTextChanged();
  
private:
  void init();
  void initLanguage();
  class TranslatorWidgetPrivate;
  TranslatorWidgetPrivate *const d;

};
}

#endif /* TRANSLATORWIDGET_H */

