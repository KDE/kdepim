/* -*- mode: c++; c-basic-offset:4 -*-
    tests/test_uiserver.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

//
// Usage: test_uiserver <socket> --verify-detached <signed data> <signature>
//

#include <config-kleopatra.h>

#include <kleo-assuan.h>
#include <gpg-error.h>

#include <kleo/exception.h>

#include "../utils/wsastarter.h"
#include "../utils/hex.h"

#ifndef Q_OS_WIN32
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
#endif
#include <assert.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace Kleo;

#ifdef Q_OS_WIN32
static const bool HAVE_FD_PASSING = false;
#else
static const bool HAVE_FD_PASSING = true;
#endif

static const unsigned int ASSUAN_CONNECT_FLAGS = HAVE_FD_PASSING ? 1 : 0 ;

static std::vector<int> inFDs, outFDs, msgFDs;
static std::vector<std::string> inFiles, outFiles, msgFiles;
static std::map<std::string, std::string> inquireData;

static void usage(const std::string &msg = std::string())
{
    std::cerr << msg << std::endl <<
              "\n"
              "Usage: test_uiserver <socket> [<io>] [<options>] [<inquire>] command [<args>]\n"
              "where:\n"
#ifdef Q_OS_WIN32
              "      <io>: [--input[-fd] <file>] [--output[-fd] <file>] [--message[-fd] <file>]\n"
#else
              "      <io>: [--input <file>] [--output <file>] [--message <file>]\n"
#endif
              " <options>: *[--option name=value]\n"
              " <inquire>: [--inquire keyword=<file>]\n";
    exit(1);
}

#ifndef HAVE_ASSUAN2
static assuan_error_t data(void *void_ctx, const void *buffer, size_t len)
{
#else
static gpg_error_t data(void *void_ctx, const void *buffer, size_t len)
{
#endif
    (void)void_ctx; (void)buffer; (void)len;
    return 0; // ### implement me
}

#ifndef HAVE_ASSUAN2
static assuan_error_t status(void *void_ctx, const char *line)
{
#else
static gpg_error_t status(void *void_ctx, const char *line)
{
#endif
    (void)void_ctx; (void)line;
    return 0;
}

#ifndef HAVE_ASSUAN2
static assuan_error_t inquire(void *void_ctx, const char *keyword)
{
#else
static gpg_error_t inquire(void *void_ctx, const char *keyword)
{
#endif
    assuan_context_t ctx = (assuan_context_t)void_ctx;
    assert(ctx);
    const std::map<std::string, std::string>::const_iterator it = inquireData.find(keyword);
    if (it == inquireData.end()) {
        return gpg_error(GPG_ERR_UNKNOWN_COMMAND);
    }

    if (!it->second.empty() && it->second[0] == '@') {
        return gpg_error(GPG_ERR_NOT_IMPLEMENTED);
    }

    if (const gpg_error_t err = assuan_send_data(ctx, it->second.c_str(), it->second.size())) {
        qDebug("assuan_write_data: %s", gpg_strerror(err));
        return err;
    }

    return 0;
}

int main(int argc, char *argv[])
{

    const Kleo::WSAStarter _wsastarter;

#ifndef HAVE_ASSUAN2
    assuan_set_assuan_err_source(GPG_ERR_SOURCE_DEFAULT);
#else
    assuan_set_gpg_err_source(GPG_ERR_SOURCE_DEFAULT);
#endif

    if (argc < 3) {
        usage();    // need socket and command, at least
    }

    const char *socket = argv[1];

    std::vector<const char *> options;

    std::string command;
    for (int optind = 2 ; optind < argc ; ++optind) {
        const char *const arg = argv[optind];
        if (qstrcmp(arg, "--input") == 0) {
            const std::string file = argv[++optind];
            inFiles.push_back(file);
        } else if (qstrcmp(arg, "--output") == 0) {
            const std::string file = argv[++optind];
            outFiles.push_back(file);
        } else if (qstrcmp(arg, "--message") == 0) {
            const std::string file = argv[++optind];
            msgFiles.push_back(file);
#ifndef Q_OS_WIN32
        } else if (qstrcmp(arg, "--input-fd") == 0) {
            int inFD;
            if ((inFD = open(argv[++optind], O_RDONLY)) == -1) {
                perror("--input-fd open()");
                return 1;
            }
            inFDs.push_back(inFD);
        } else if (qstrcmp(arg, "--output-fd") == 0) {
            int outFD;
            if ((outFD = open(argv[++optind], O_WRONLY | O_CREAT, 0666)) ==  -1) {
                perror("--output-fd open()");
                return 1;
            }
            outFDs.push_back(outFD);
        } else if (qstrcmp(arg, "--message-fd") == 0) {
            int msgFD;
            if ((msgFD = open(argv[++optind], O_RDONLY)) ==  -1) {
                perror("--message-fd open()");
                return 1;
            }
            msgFDs.push_back(msgFD);
#endif
        } else if (qstrcmp(arg, "--option") == 0) {
            options.push_back(argv[++optind]);
        } else if (qstrcmp(arg, "--inquire") == 0) {
            const std::string inqval = argv[++optind];
            const size_t pos = inqval.find('=');
            // ### implement indirection with "@file"...
            inquireData[inqval.substr(0, pos)] = inqval.substr(pos + 1);
        } else {
            while (optind < argc) {
                if (!command.empty()) {
                    command += ' ';
                }
                command += argv[optind++];
            }
        }
    }
    if (command.empty()) {
        usage("Command expected, but only options found");
    }

    assuan_context_t ctx = 0;

#ifndef HAVE_ASSUAN2
    if (const gpg_error_t err = assuan_socket_connect_ext(&ctx, socket, -1, ASSUAN_CONNECT_FLAGS)) {
        qDebug("%s", Exception(err, "assuan_socket_connect_ext").what());
#else
    if (const gpg_error_t err = assuan_new(&ctx)) {
        qDebug("%s", Exception(err, "assuan_new").what());
        return 1;
    }

    if (const gpg_error_t err = assuan_socket_connect(ctx, socket, -1, ASSUAN_CONNECT_FLAGS)) {
        qDebug("%s", Exception(err, "assuan_socket_connect").what());
#endif
        return 1;
    }

    assuan_set_log_stream(ctx, stderr);

#ifndef Q_OS_WIN32
    for (std::vector<int>::const_iterator it = inFDs.begin(), end = inFDs.end() ; it != end ; ++it) {
        if (const gpg_error_t err = assuan_sendfd(ctx, *it)) {
            qDebug("%s", Exception(err, "assuan_sendfd( inFD )").what());
            return 1;
        }

        if (const gpg_error_t err = assuan_transact(ctx, "INPUT FD", 0, 0, 0, 0, 0, 0)) {
            qDebug("%s", Exception(err, "INPUT FD").what());
            return 1;
        }
    }

    for (std::vector<int>::const_iterator it = msgFDs.begin(), end = msgFDs.end() ; it != end ; ++it) {
        if (const gpg_error_t err = assuan_sendfd(ctx, *it)) {
            qDebug("%s", Exception(err, "assuan_sendfd( msgFD )").what());
            return 1;
        }

        if (const gpg_error_t err = assuan_transact(ctx, "MESSAGE FD", 0, 0, 0, 0, 0, 0)) {
            qDebug("%s", Exception(err, "MESSAGE FD").what());
            return 1;
        }
    }

    for (std::vector<int>::const_iterator it = outFDs.begin(), end = outFDs.end() ; it != end ; ++it) {
        if (const gpg_error_t err = assuan_sendfd(ctx, *it)) {
            qDebug("%s", Exception(err, "assuan_sendfd( outFD )").what());
            return 1;
        }

        if (const gpg_error_t err = assuan_transact(ctx, "OUTPUT FD", 0, 0, 0, 0, 0, 0)) {
            qDebug("%s", Exception(err, "OUTPUT FD").what());
            return 1;
        }
    }
#endif

    for (std::vector<std::string>::const_iterator it = inFiles.begin(), end = inFiles.end() ; it != end ; ++it) {
        char buffer[1024];
        sprintf(buffer, "INPUT FILE=%s", hexencode(*it).c_str());

        if (const gpg_error_t err = assuan_transact(ctx, buffer, 0, 0, 0, 0, 0, 0)) {
            qDebug("%s", Exception(err, buffer).what());
            return 1;
        }
    }

    for (std::vector<std::string>::const_iterator it = msgFiles.begin(), end = msgFiles.end() ; it != end ; ++it) {
        char buffer[1024];
        sprintf(buffer, "MESSAGE FILE=%s", hexencode(*it).c_str());

        if (const gpg_error_t err = assuan_transact(ctx, buffer, 0, 0, 0, 0, 0, 0)) {
            qDebug("%s", Exception(err, buffer).what());
            return 1;
        }
    }

    for (std::vector<std::string>::const_iterator it = outFiles.begin(), end = outFiles.end() ; it != end ; ++it) {
        char buffer[1024];
        sprintf(buffer, "OUTPUT FILE=%s", hexencode(*it).c_str());

        if (const gpg_error_t err = assuan_transact(ctx, buffer, 0, 0, 0, 0, 0, 0)) {
            qDebug("%s", Exception(err, buffer).what());
            return 1;
        }
    }

    Q_FOREACH (const char *opt, options) {
        std::string line = "OPTION ";
        line += opt;
        if (const gpg_error_t err = assuan_transact(ctx, line.c_str(), 0, 0, 0, 0, 0, 0)) {
            qDebug("%s", Exception(err, line).what());
            return 1;
        }
    }

    if (const gpg_error_t err = assuan_transact(ctx, command.c_str(), data, ctx, inquire, ctx, status, ctx)) {
        qDebug("%s", Exception(err, command).what());
        return 1;
    }

#ifndef HAVE_ASSUAN2
    assuan_disconnect(ctx);
#else
    assuan_release(ctx);
#endif

    return 0;
}
