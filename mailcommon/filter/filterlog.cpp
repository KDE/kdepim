/*
  Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "filterlog.h"

#include "messageviewer/utils/util.h"

#include <QDebug>

#include <QFile>
#include <QTime>
#include <QTextDocument>

#include <sys/stat.h>

using namespace MailCommon;

class FilterLog::Private
{
public:
    Private(FilterLog *qq)
        : q(qq),
          mLogging(false),
          mMaxLogSize(512 * 1024),
          mCurrentLogSize(0),
          mAllowedTypes(FilterLog::Meta |
                        FilterLog::PatternDescription |
                        FilterLog::RuleResult |
                        FilterLog::PatternResult |
                        FilterLog::AppliedAction)
    {
    }

    static FilterLog *mSelf;

    FilterLog *q;
    QStringList mLogEntries;
    bool mLogging;
    long mMaxLogSize;
    long mCurrentLogSize;
    int mAllowedTypes;

    void checkLogSize();
};

void FilterLog::Private::checkLogSize()
{
    if (mCurrentLogSize > mMaxLogSize && mMaxLogSize > -1) {
        qDebug() << "Filter log: memory limit reached, starting to discard old items, size ="
                 << QString::number(mCurrentLogSize);

        // avoid some kind of hysteresis, shrink the log to 90% of its maximum
        while (mCurrentLogSize > (mMaxLogSize * 0.9)) {
            QStringList::Iterator it = mLogEntries.begin();
            if (it != mLogEntries.end()) {
                mCurrentLogSize -= (*it).length();
                mLogEntries.erase(it);
                qDebug() << "Filter log: new size =" << QString::number(mCurrentLogSize);
            } else {
                qDebug() << "Filter log: size reduction disaster!";
                q->clear();
            }
        }

        emit q->logShrinked();
    }
}

FilterLog *FilterLog::Private::mSelf = 0;

FilterLog::FilterLog()
    : d(new Private(this))
{
}

FilterLog::~FilterLog()
{
    delete d;
}

FilterLog *FilterLog::instance()
{
    if (!FilterLog::Private::mSelf) {
        FilterLog::Private::mSelf = new FilterLog();
    }

    return FilterLog::Private::mSelf;
}

bool FilterLog::isLogging() const
{
    return d->mLogging;
}

void FilterLog::setLogging(bool active)
{
    d->mLogging = active;
    emit logStateChanged();
}

void FilterLog::setMaxLogSize(long size)
{
    if (size < -1) {
        size = -1;
    }

    // do not allow less than 1 KByte except unlimited (-1)
    if (size >= 0 && size < 1024) {
        size = 1024;
    }

    d->mMaxLogSize = size;
    emit logStateChanged();
    d->checkLogSize();
}

long FilterLog::maxLogSize() const
{
    return d->mMaxLogSize;
}

void FilterLog::setContentTypeEnabled(ContentType contentType, bool enable)
{
    if (enable) {
        d->mAllowedTypes |= contentType;
    } else {
        d->mAllowedTypes &= ~contentType;
    }

    emit logStateChanged();
}

bool FilterLog::isContentTypeEnabled(ContentType contentType) const
{
    return (d->mAllowedTypes & contentType);
}

void FilterLog::add(const QString &logEntry, ContentType contentType)
{
    if (isLogging() && (d->mAllowedTypes & contentType)) {
        QString timedLog = QLatin1Char('[') + QTime::currentTime().toString() + QLatin1String("] ");
        if (contentType & ~Meta) {
            timedLog += logEntry;
        } else {
            timedLog = logEntry;
        }

        d->mLogEntries.append(timedLog);
        emit logEntryAdded(timedLog);
        d->mCurrentLogSize += timedLog.length();
        d->checkLogSize();
    }
}

void FilterLog::addSeparator()
{
    add(QLatin1String("------------------------------"), Meta);
}

void FilterLog::clear()
{
    d->mLogEntries.clear();
    d->mCurrentLogSize = 0;
}

QStringList FilterLog::logEntries() const
{
    return d->mLogEntries;
}

void FilterLog::dump()
{
#ifndef NDEBUG
    qDebug() << "----- starting filter log -----";
    foreach (const QString &entry, d->mLogEntries) {
        qDebug() << entry;
    }
    qDebug() << "------ end of filter log ------";
#endif
}

bool FilterLog::saveToFile(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    fchmod(file.handle(), MessageViewer::Util::getWritePermissions());

    file.write("<html>\n<body>\n");
    file.write("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
    foreach (const QString &entry, d->mLogEntries) {
        const QString line = QLatin1String("<p>") + entry + QLatin1String("</p>") + QLatin1Char('\n');
        file.write(line.toLocal8Bit());
    }
    file.write("</body>\n</html>\n");
    file.close();
    return true;
}

QString FilterLog::recode(const QString &plain)
{
    return plain.toHtmlEscaped();
}

