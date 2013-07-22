/* Copyright (C) 2013 Laurent Montel <montel@kde.org>
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

#ifndef SIEVEEDITORTEXTMODEWIDGET_H
#define SIEVEEDITORTEXTMODEWIDGET_H

#include "sieveeditorabstractwidget.h"

class KTextEdit;
class QLineEdit;
class QPushButton;
class QSplitter;

namespace KSieveUi {
class SieveFindBar;
class SieveInfoWidget;
class SieveTextEdit;
class SieveEditorTextModeWidget : public SieveEditorAbstractWidget
{
    Q_OBJECT
public:
    explicit SieveEditorTextModeWidget(QWidget *parent=0);
    ~SieveEditorTextModeWidget();


    void setSieveCapabilities( const QStringList &capabilities );

    void resultDone();

    QString script() const;
    void setScript( const QString &script );

    void setDebugColor( const QColor &col );
    void setDebugScript( const QString &debug );
    void setScriptName( const QString &name );

Q_SIGNALS:
    void checkSyntax();
    void enableButtonOk( bool );

private slots:
    void slotTextChanged();
    void slotImport();
    void slotSaveAs();
    void slotFind();
    void slotAutoGenerateScripts();
    void slotCheckSyntax();
    void slotGenerateXml();

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

    SieveInfoWidget *mSieveInfo;
    QSplitter *mMainSplitter;
    QSplitter *mExtraSplitter;
    QSplitter *mTemplateSplitter;
};

}

#endif // SIEVEEDITORTEXTMODEWIDGET_H
