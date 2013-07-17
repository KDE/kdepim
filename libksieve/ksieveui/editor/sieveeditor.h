/* Copyright (C) 2011-2012-2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSIEVE_KSIEVEUI_SIEVEEDITOR_H
#define KSIEVE_KSIEVEUI_SIEVEEDITOR_H

#include "ksieveui_export.h"
#include "sievetextedit.h"

#include <kdialog.h>

class QLineEdit;
class KTextEdit;
class QSplitter;

namespace KSieveUi {

class SieveFindBar;
class SieveInfoWidget;

class KSIEVEUI_EXPORT SieveEditor : public KDialog
{
    Q_OBJECT
    Q_PROPERTY( QString script READ script WRITE setScript )

public:
    explicit SieveEditor( QWidget * parent=0 );
    ~SieveEditor();

    QString script() const;
    QString originalScript() const;
    void setScript( const QString &script );
    void setDebugColor( const QColor &col );
    void setDebugScript( const QString &debug );
    void setScriptName( const QString &name );

    void resultDone();

    void setSieveCapabilities( const QStringList &capabilities );

Q_SIGNALS:
    void checkSyntax();

private slots:
    void slotTextChanged();
    void slotImport();
    void slotSaveAs();
    void slotFind();
    void slotAutoGenerateScripts();
    void slotCheckSyntax();

private:
    void readConfig();
    void writeConfig();
    bool saveToFile( const QString &filename );
    bool loadFromFile( const QString &filename );
    QString mOriginalScript;
    QStringList mSieveCapabilities;
    SieveTextEdit * mTextEdit;
    KTextEdit *mDebugTextEdit;
    QLineEdit *mScriptName;
    SieveFindBar *mFindBar;
    QAction *mCheckSyntax;

    QPushButton *mOkButton;
    SieveInfoWidget *mSieveInfo;
    QSplitter *mMainSplitter;
    QSplitter *mExtraSplitter;
    QSplitter *mTemplateSplitter;
};

}

#endif

