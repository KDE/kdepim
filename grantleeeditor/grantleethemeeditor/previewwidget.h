/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef GRANTLEEPREVIEWWIDGET_H
#define GRANTLEEPREVIEWWIDGET_H
#include <QWidget>
#include "grantleethemeeditor_export.h"

namespace GrantleeThemeEditor
{
class GRANTLEETHEMEEDITOR_EXPORT PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();

    virtual void updateViewer();

    virtual void loadConfig();
    virtual void createScreenShot(const QStringList &lstFileName);
    virtual void setThemePath(const QString &projectDirectory, const QString &mainPageFileName);

    void setPrinting(bool printMode);
    bool printing() const;

public Q_SLOTS:
    virtual void slotMainFileNameChanged(const QString &);
    virtual void slotExtraHeaderDisplayChanged(const QStringList &headers);

Q_SIGNALS:
    void needUpdateViewer();

protected:
    bool mPrinting;
};
}

#endif // GRANTLEEPREVIEWWIDGET_H
