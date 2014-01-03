/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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


#ifndef NOTENETWORKCONFIG_H
#define NOTENETWORKCONFIG_H
#include "noteshared_export.h"
#include <KCModule>
class KLineEdit;
class QCheckBox;
class KComponentData;
class KIntNumInput;
namespace NoteShared {

class NOTESHARED_EXPORT NoteNetworkConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoteNetworkConfigWidget( QWidget *parent = 0);
    ~NoteNetworkConfigWidget();

    void save();
    void load();
private:
    QCheckBox *mTmpChkB;
    KLineEdit *kcfg_SenderID;
    KIntNumInput *kcfg_Port;
};


class NOTESHARED_EXPORT NoteNetworkConfig : public KCModule
{
    Q_OBJECT
public:
    NoteNetworkConfig( const KComponentData &inst, QWidget *parent );
    /** Reimplemented from KCModule. */
    virtual void load();
    virtual void save();
};
}

#endif // NOTENETWORKCONFIG_H
