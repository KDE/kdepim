/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef ATTACHMENTENCRYPTWITHCHIASMUSJOB_H
#define ATTACHMENTENCRYPTWITHCHIASMUSJOB_H

#include <QObject>
namespace KMime
{
class Content;
}
namespace Kleo
{
class SpecialJob;
}
namespace GpgME
{
class Error;
}
class KJob;
namespace MessageViewer
{
class AttachmentEncryptWithChiasmusJob : public QObject
{
    Q_OBJECT
public:
    explicit AttachmentEncryptWithChiasmusJob(QObject *parent = Q_NULLPTR);
    ~AttachmentEncryptWithChiasmusJob();
    void start();

    void setContent(KMime::Content *content);
    void setCurrentFileName(const QString &currentFileName);

    void setMainWindow(QWidget *mainWindow);

private Q_SLOTS:
    void slotAtmDecryptWithChiasmusResult(const GpgME::Error &err, const QVariant &result);
    void slotAtmDecryptWithChiasmusUploadResult(KJob *job);
private:
    QString mCurrentFileName;
    Kleo::SpecialJob *mJob;
    QWidget *mMainWindow;
    KMime::Content *mContent;
};
}
#endif // ATTACHMENTENCRYPTWITHCHIASMUSJOB_H
