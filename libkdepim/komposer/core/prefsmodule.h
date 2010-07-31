/*
 * prefsmodule.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef KOMPOSER_PREFSMODULE_H
#define KOMPOSER_PREFSMODULE_H

#include <kprefsdialog.h>
#include <kservice.h>
#include <tqmap.h>
class QGroupBox;
class QListViewItem;

class KAboutData;
class KComboBox;

namespace Komposer {

  class PrefsModule : public KPrefsModule
  {
    Q_OBJECT
  public:
    PrefsModule( TQWidget *parent=0, const char *name=0 );
    virtual const KAboutData *aboutData() const;
  };

  class EditorSelection : public KPrefsWid
  {
    Q_OBJECT

  public:
    EditorSelection( const TQString &text, TQString &reference, TQWidget *parent );
    ~EditorSelection();

    void readConfig();
    void writeConfig();

    TQGroupBox *groupBox() const;

  private slots:
    void slotActivated( const TQString & );

  private:
    void setItem( const TQString & );
  private:
    TQString &m_reference;

    TQGroupBox *m_box;
    KComboBox *m_editorsCombo;
    TQMap<TQString, KService::Ptr> m_services;
  };
}

#endif
