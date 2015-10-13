#ifndef __LIBKLEOPATRACLIENT_TESTS_TEST_UTIL_H__
#define __LIBKLEOPATRACLIENT_TESTS_TEST_UTIL_H__

#include <QStringList>
#include <QFileInfo>
#include <QFile>

static QStringList filePathsFromArgs(int argc, char *argv[])
{
    QStringList result;
    for (int i = 1; i < argc; ++i) {
        result.push_back(QFileInfo(QFile::decodeName(argv[i])).absoluteFilePath());
    }
    return result;
}

#endif /* __LIBKLEOPATRACLIENT_TESTS_TEST_UTIL_H__ */
