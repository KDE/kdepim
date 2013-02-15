/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#ifndef IMAGESCALINGSELECTFORMAT_H
#define IMAGESCALINGSELECTFORMAT_H

#include <KDialog>

class KLineEdit;
class KPushButton;
class QListWidget;

namespace MessageComposer {

class ImageScalingSelectFormatDialog : public KDialog
{
    Q_OBJECT
public:
    explicit ImageScalingSelectFormatDialog(QWidget *parent);
    ~ImageScalingSelectFormatDialog();
    void setFormat(const QString &format);
    QString format() const;

private:
    void initialize();
    QListWidget *mListWidget;
};

class ImageScalingSelectFormat : public QWidget
{
    Q_OBJECT
public:
    explicit ImageScalingSelectFormat(QWidget *parent);
    ~ImageScalingSelectFormat();

    void setFormat(const QString &format);
    QString format() const;

private Q_SLOTS:
    void slotSelectFormat();

private:
    KLineEdit *mFormat;
    KPushButton *mSelectFormat;
};
}

#endif // IMAGESCALINGSELECTFORMAT_H
