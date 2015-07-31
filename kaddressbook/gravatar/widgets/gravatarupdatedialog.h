/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef GRAVATARUPDATEDIALOG_H
#define GRAVATARUPDATEDIALOG_H

#include <QDialog>
namespace KABGravatar
{
class GravatarUpdateWidget;
class GravatarUpdateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GravatarUpdateDialog(QWidget *parent = Q_NULLPTR);
    ~GravatarUpdateDialog();

    void setEmail(const QString &email);

    QPixmap pixmap() const;
    void setOriginalPixmap(const QPixmap &pix);

    void setOriginalUrl(const QString &url);

    QUrl resolvedUrl() const;

    bool saveUrl() const;
private Q_SLOTS:
    void slotAccepted();    
    void slotSaveImage();
    void slotSaveUrl();

    void slotActivateButton();
private:
    void readConfig();
    void writeConfig();
    GravatarUpdateWidget *mGravatarUpdateWidget;
    QPushButton *mSaveImageButton;
    QPushButton *mSaveUrlButton;
    bool mSaveUrl;
};
}
#endif // GRAVATARUPDATEDIALOG_H
