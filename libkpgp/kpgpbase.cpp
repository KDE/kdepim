/*
    kpgpbase.cpp

    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <config-libkpgp.h> /* HAVE_SYS_POLL_H */
#include "kpgp_debug.h"
#include "kpgpbase.h"
#include "kpgp.h"
#include "kpgpblock.h"

#include <qdebug.h>
//#include <kdefakes.h> /* setenv, unsetenv */

#include <QApplication>
#include <QByteArray>

#include <stdlib.h> /* setenv, unsetenv */
#include <unistd.h> /* pipe, close, fork, dup2, execl, _exit, write, read */
#include <errno.h>

#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>  /* poll, etc. */
#endif

#include <sys/types.h> /* pid_t */
#include <sys/wait.h> /* waitpid */

namespace Kpgp {

Base::Base()
  : input(), output(), error(), errMsg(), status(OK)
{
}


Base::~Base()
{
}


void
Base::clear()
{
  input = QByteArray();
  output = QByteArray();
  error = QByteArray();
  errMsg.clear();
  status = OK;
}


int
Base::run( const char *cmd, const char *passphrase, bool onlyReadFromPGP )
{
#ifdef HAVE_SYS_POLL_H
  /* the pipe ppass is used for to pass the password to
   * pgp. passing the password together with the normal input through
   * stdin doesn't seem to work as expected (at least for pgp5.0)
   */
  char str[1025] = "\0";
  int pin[2], pout[2], perr[2], ppass[2];
  int len, len2;
  FILE *pass;
  pid_t child_pid;
  int childExitStatus;
  struct pollfd pollin, pollout, pollerr;
  int pollstatus;

  if(passphrase)
  {
    pipe(ppass);

    pass = fdopen(ppass[1], "w");
    fwrite(passphrase, sizeof(char), strlen(passphrase), pass);
    fwrite("\n", sizeof(char), 1, pass);
    fclose(pass);
    close(ppass[1]);

    // tell pgp which fd to use for the passphrase
    QString tmp;
    tmp.sprintf("%d",ppass[0]);
    ::setenv("PGPPASSFD",tmp.toUtf8()/*.data()*/,1);

    //Uncomment these lines for testing only! Doing so will decrease security!
    //qCDebug(KPGP_LOG) <<"pgp PGPPASSFD =" << tmp;
    //qCDebug(KPGP_LOG) <<"pgp pass =" << passphrase;
  }
  else
    ::unsetenv("PGPPASSFD");

  //Uncomment these lines for testing only! Doing so will decrease security!
  qCDebug(KPGP_LOG) <<"pgp cmd =" << cmd;
  //qCDebug(KPGP_LOG) <<"pgp input =" << QString(input)
  //          << "input length =" << input.length();

  error = "";
  output = "";

  pipe(pin);
  pipe(pout);
  pipe(perr);

  QApplication::flush();
  if(!(child_pid = fork()))
  {
    /*We're the child.*/
    close(pin[1]);
    dup2(pin[0], 0);
    close(pin[0]);

    close(pout[0]);
    dup2(pout[1], 1);
    close(pout[1]);

    close(perr[0]);
    dup2(perr[1], 2);
    close(perr[1]);

    execl("/bin/sh", "sh", "-c", cmd,  (void *)0);
    _exit(127);
  }

  /*Only get here if we're the parent.*/
  close(pin[0]);
  close(pout[1]);
  close(perr[1]);

  // poll for "There is data to read."
  pollout.fd = pout[0];
  pollout.events = POLLIN;
  pollout.revents = 0; // init with 0, just in case
  pollerr.fd = perr[0];
  pollerr.events = POLLIN;
  pollerr.revents = 0; // init with 0, just in case

  // poll for "Writing now will not block."
  pollin.fd = pin[1];
  pollin.events = POLLOUT;
  pollin.revents = 0; // init with 0, just in case

  if (!onlyReadFromPGP) {
    if (!input.isEmpty()) {
      // write to pin[1] one line after the other to prevent dead lock
      int input_length = input.length();
      for (int i=0; i<input_length; i+=len2) {
        len2 = 0;

        // check if writing now to pin[1] will not block (5 ms timeout)
        //qCDebug(KPGP_LOG) <<"Polling pin[1]...";
        pollstatus = poll(&pollin, 1, 5);
        if (pollstatus == 1) {
          //qCDebug(KPGP_LOG) <<"Status for polling pin[1]:" << pollin.revents;
          if (pollin.revents & POLLERR) {
            qCDebug(KPGP_LOG) <<"PGP seems to have hung up";
            break;
          }
          else if (pollin.revents & POLLOUT) {
            // search end of next line
            if ((len2 = input.indexOf('\n', i)) == -1)
              len2 = input_length - i;
            else
              len2 = len2 - i + 1;

            //qCDebug(KPGP_LOG) <<"Trying to write" << len2 <<" bytes to pin[1] ...";
            len2 = write(pin[1], input.data() + i, len2);
            //qCDebug(KPGP_LOG) <<"Wrote" << len2 <<" bytes to pin[1] ...";
          }
        }
        else if (!pollstatus) {
          //qCDebug(KPGP_LOG) <<"Timeout while polling pin[1]:"
          //              << pollin.revents;
        }
        else if (pollstatus == -1) {
          qCDebug(KPGP_LOG) <<"Error while polling pin[1]:"
                        << pollin.revents;
        }

        if (pout[0] >= 0) {
          do {
            // check if there is data to read from pout[0]
            //qCDebug(KPGP_LOG) <<"Polling pout[0]...";
            pollstatus = poll(&pollout, 1, 0);
            if (pollstatus == 1) {
              //qCDebug(KPGP_LOG) <<"Status for polling pout[0]:" << pollout.revents;
              if (pollout.revents & POLLIN) {
                //qCDebug(KPGP_LOG) <<"Trying to read" << 1024 <<" bytes from pout[0]";
                if ((len = read(pout[0],str,1024))>0) {
                  //qCDebug(KPGP_LOG) <<"Read" << len <<" bytes from pout[0]";
                  str[len] ='\0';
                  output += str;
                }
                else
                  break;
              }
            }
            else if (pollstatus == -1) {
              qCDebug(KPGP_LOG) <<"Error while polling pout[0]:"
                            << pollout.revents;
            }
          } while ((pollstatus == 1) && (pollout.revents & POLLIN));
        }

        if (perr[0] >= 0) {
          do {
            // check if there is data to read from perr[0]
            //qCDebug(KPGP_LOG) <<"Polling perr[0]...";
            pollstatus = poll(&pollerr, 1, 0);
            if (pollstatus == 1) {
              //qCDebug(KPGP_LOG) <<"Status for polling perr[0]:" << pollerr.revents;
              if (pollerr.revents & POLLIN) {
                //qCDebug(KPGP_LOG) <<"Trying to read" << 1024 <<" bytes from perr[0]";
                if ((len = read(perr[0],str,1024))>0) {
                  //qCDebug(KPGP_LOG) <<"Read" << len <<" bytes from perr[0]";
                  str[len] ='\0';
                  error += str;
                }
                else
                  break;
              }
            }
            else if (pollstatus == -1) {
              qCDebug(KPGP_LOG) <<"Error while polling perr[0]:"
                            << pollerr.revents;
            }
          } while ((pollstatus == 1) && (pollerr.revents & POLLIN));
        }

        // abort writing to PGP if PGP hung up
        if ((pollstatus == 1) &&
            ((pollout.revents & POLLHUP) || (pollerr.revents & POLLHUP))) {
          qCDebug(KPGP_LOG) <<"PGP hung up";
          break;
        }
      }
    }
    else // if input.isEmpty()
      write(pin[1], "\n", 1);
    //qCDebug(KPGP_LOG) <<"All input was written to pin[1]";
  }
  close(pin[1]);

  pid_t waitpidRetVal;

  do {
    //qCDebug(KPGP_LOG) <<"Checking if PGP is still running...";
    childExitStatus = 0;
    waitpidRetVal = waitpid(child_pid, &childExitStatus, WNOHANG);
    //qCDebug(KPGP_LOG) <<"waitpid returned" << waitpidRetVal;
    if (pout[0] >= 0) {
      do {
        // check if there is data to read from pout[0]
        //qCDebug(KPGP_LOG) <<"Polling pout[0]...";
        pollstatus = poll(&pollout, 1, 0);
        if (pollstatus == 1) {
          //qCDebug(KPGP_LOG) <<"Status for polling pout[0]:" << pollout.revents;
          if (pollout.revents & POLLIN) {
            //qCDebug(KPGP_LOG) <<"Trying to read" << 1024 <<" bytes from pout[0]";
            if ((len = read(pout[0],str,1024))>0) {
              //qCDebug(KPGP_LOG) <<"Read" << len <<" bytes from pout[0]";
              str[len] ='\0';
              output += str;
            } else {
              /*
               * Apparently, on NetBSD when the child dies, the pipe begins
               * receiving empty data packets *before* waitpid() has signaled
               * that the child has died.  Also, notice that this happens
               * without any error bit being set in pollfd.revents (is this a
               * NetBSD bug??? ).  Notice that these anomalous packets exist
               * according to poll(), but have length 0 according to read().
               * Thus, kde can remain stuck inside this loop.
               *
               * A solution to this problem is to get out of the inner loop
               * when read() returns <=0.  In this way, kde has another chance
               * to call waitpid() to check if the child has died -- and this
               * time the call should succeed.
               *
               * Setting POLLHUP in pollfd.revents is not necessary, but I just
               * like the idea of signaling that something strange has
               * happened.
               */
              pollout.revents |= POLLHUP;
              break;
            }
          }
        }
        else if (pollstatus == -1) {
          qCDebug(KPGP_LOG) <<"Error while polling pout[0]:"
                        << pollout.revents;
        }
      } while ((pollstatus == 1) && (pollout.revents & POLLIN));
    }

    if (perr[0] >= 0) {
      do {
        // check if there is data to read from perr[0]
        //qCDebug(KPGP_LOG) <<"Polling perr[0]...";
        pollstatus = poll(&pollerr, 1, 0);
        if (pollstatus == 1) {
          //qCDebug(KPGP_LOG) <<"Status for polling perr[0]:" << pollerr.revents;
          if (pollerr.revents & POLLIN) {
            //qCDebug(KPGP_LOG) <<"Trying to read" << 1024 <<" bytes from perr[0]";
            if ((len = read(perr[0],str,1024))>0) {
              //qCDebug(KPGP_LOG) <<"Read" << len <<" bytes from perr[0]";
              str[len] ='\0';
              error += str;
            } else {
              /*
               * Apparently, on NetBSD when the child dies, the pipe begins
               * receiving empty data packets *before* waitpid() has signaled
               * that the child has died.  Also, notice that this happens
               * without any error bit being set in pollfd.revents (is this a
               * NetBSD bug??? ).  Notice that these anomalous packets exist
               * according to poll(), but have length 0 according to read().
               * Thus, kde can remain stuck inside this loop.
               *
               * A solution to this problem is to get out of the inner loop
               * when read() returns <=0.  In this way, kde has another chance
               * to call waitpid() to check if the child has died -- and this
               * time the call should succeed.
               *
               * Setting POLLHUP in pollfd.revents is not necessary, but I just
               * like the idea of signaling that something strange has
               * happened.
               */
              pollerr.revents |= POLLHUP;
              break;
            }
          }
        }
        else if (pollstatus == -1) {
          qCDebug(KPGP_LOG) <<"Error while polling perr[0]:"
                        << pollerr.revents;
        }
      } while ((pollstatus == 1) && (pollerr.revents & POLLIN));
    }
  } while (waitpidRetVal == 0);

  close(pout[0]);
  close(perr[0]);

  unsetenv("PGPPASSFD");
  if(passphrase)
    close(ppass[0]);

  // Did the child exit normally?
  if (WIFEXITED(childExitStatus) != 0) {
    // Get the return code of the child
    childExitStatus = WEXITSTATUS(childExitStatus);
    qCDebug(KPGP_LOG) <<"PGP exited with exit status" << childExitStatus;
  }
  else {
    childExitStatus = -1;
    qCDebug(KPGP_LOG) <<"PGP exited abnormally!";
  }

  //Uncomment these lines for testing only! Doing so will decrease security!
  //qCDebug(KPGP_LOG) <<"pgp output =" << QString(output);
  //qCDebug(KPGP_LOG) <<"pgp error =" << error;

  /* Make the information visible, so that a user can
   * get to know what's going on during the pgp calls.
   */
  qCDebug(KPGP_LOG) << error;

  return childExitStatus;
#else // HAVE_SYS_POLL_H
#ifdef __GNUC__
#warning WIN32 libkpgp: PGP support not ported to win32!
#endif
  return 1;
#endif
}


int
Base::runGpg( const char *cmd, const char *passphrase, bool onlyReadFromGnuPG )
{
#ifdef HAVE_SYS_POLL_H
  /* the pipe ppass is used for to pass the password to
   * pgp. passing the password together with the normal input through
   * stdin doesn't seem to work as expected (at least for pgp5.0)
   */
  char str[1025] = "\0";
  int pin[2], pout[2], perr[2], ppass[2];
  int len, len2;
  FILE *pass;
  pid_t child_pid;
  int childExitStatus;
  char gpgcmd[1024] = "\0";
  struct pollfd poller[3];
  int num_pollers = 0;
  const int STD_OUT = 0;
  const int STD_ERR = 1;
  const int STD_IN = 2;
  int pollstatus;

  if(passphrase)
  {
    pipe(ppass);

    pass = fdopen(ppass[1], "w");
    fwrite(passphrase, sizeof(char), strlen(passphrase), pass);
    fwrite("\n", sizeof(char), 1, pass);
    fclose(pass);
    close(ppass[1]);

    //Uncomment these lines for testing only! Doing so will decrease security!
    //qCDebug(KPGP_LOG) <<"pass =" << passphrase;
  }

  //Uncomment these lines for testing only! Doing so will decrease security!
  //qCDebug(KPGP_LOG) <<"pgp cmd =" << cmd;
  //qCDebug(KPGP_LOG) <<"pgp input =" << QString(input)
  //          << "input length =" << input.length();

  error = "";
  output = "";

  pipe(pin);
  pipe(pout);
  pipe(perr);

  if( passphrase ) {
    if( mVersion >= "1.0.7" ) {
      // GnuPG >= 1.0.7 supports the gpg-agent, so we look for it.
      if( 0 == getenv("GPG_AGENT_INFO") ) {
        // gpg-agent not found, so we tell gpg not to use the agent
        snprintf( gpgcmd, 1023,
                  "LANGUAGE=C gpg --no-use-agent --passphrase-fd %d %s",
                  ppass[0], cmd );
      }
      else {
        // gpg-agent seems to be running, so we tell gpg to use the agent
        snprintf( gpgcmd, 1023,
                  "LANGUAGE=C gpg --use-agent %s",
                  cmd );
      }
    }
    else {
      // GnuPG < 1.0.7 doesn't know anything about the gpg-agent
      snprintf( gpgcmd, 1023,
                "LANGUAGE=C gpg --passphrase-fd %d %s",
                ppass[0], cmd );
    }
  }
  else {
    snprintf(gpgcmd, 1023, "LANGUAGE=C gpg %s",cmd);
  }

  QApplication::flush();
  if(!(child_pid = fork()))
  {
    /*We're the child.*/
    close(pin[1]);
    dup2(pin[0], 0);
    close(pin[0]);

    close(pout[0]);
    dup2(pout[1], 1);
    close(pout[1]);

    close(perr[0]);
    dup2(perr[1], 2);
    close(perr[1]);

    //#warning FIXME: there has to be a better way to do this
     /* this is nasty nasty nasty (but it works) */
    if( passphrase ) {
      if( mVersion >= "1.0.7" ) {
        // GnuPG >= 1.0.7 supports the gpg-agent, so we look for it.
        if( 0 == getenv("GPG_AGENT_INFO") ) {
          // gpg-agent not found, so we tell gpg not to use the agent
          snprintf( gpgcmd, 1023,
                    "LANGUAGE=C gpg --no-use-agent --passphrase-fd %d %s",
                    ppass[0], cmd );
        }
        else {
          // gpg-agent seems to be running, so we tell gpg to use the agent
          snprintf( gpgcmd, 1023,
                    "LANGUAGE=C gpg --use-agent %s",
                    cmd );
        }
      }
      else {
        // GnuPG < 1.0.7 doesn't know anything about the gpg-agent
        snprintf( gpgcmd, 1023,
                  "LANGUAGE=C gpg --passphrase-fd %d %s",
                  ppass[0], cmd );
      }
    }
    else {
      snprintf(gpgcmd, 1023, "LANGUAGE=C gpg %s",cmd);
    }

    qCDebug(KPGP_LOG) <<"pgp cmd =" << gpgcmd;

    execl("/bin/sh", "sh", "-c", gpgcmd,  (void *)0);
    _exit(127);
  }

  // Only get here if we're the parent.

  close(pin[0]);
  close(pout[1]);
  close(perr[1]);

  // poll for "There is data to read."
  poller[STD_OUT].fd = pout[0];
  poller[STD_OUT].events = POLLIN;
  poller[STD_ERR].fd = perr[0];
  poller[STD_ERR].events = POLLIN;
  num_pollers = 2;

  if (!onlyReadFromGnuPG) {
    // poll for "Writing now will not block."
    poller[STD_IN].fd = pin[1];
    poller[STD_IN].events = POLLOUT;
    num_pollers = 3;
  } else {
    close (pin[1]);
    pin[1] = -1;
  }

  pid_t waitpidRetVal;
  int input_pos = 0;
  int input_length = input.length();

  do {
    //qCDebug(KPGP_LOG) <<"Checking if GnuPG is still running...";
    childExitStatus = 0;
    waitpidRetVal = waitpid(child_pid, &childExitStatus, WNOHANG);
    //qCDebug(KPGP_LOG) <<"waitpid returned" << waitpidRetVal;
    do {
      // poll the pipes
      pollstatus = poll(poller, num_pollers, 10);
      if( 0 < pollstatus ) {
        // Check stdout.
        if (poller[STD_OUT].revents & POLLIN) {
          //qCDebug(KPGP_LOG) <<"Trying to read" << 1024 <<" bytes from pout[0]";
          if ((len = read(pout[0],str,1024))>0) {
            //qCDebug(KPGP_LOG) <<"Read" << len <<" bytes from pout[0]";
            str[len] ='\0';
            output += str;
          }
          else {
            // FreeBSD/NetBSD workaround
            //
            // Apparently, on Free/NetBSD when the child dies, the pipe begins
            // receiving empty data packets *before* waitpid() has signaled
            // that the child has died.  Also, notice that this happens
            // without any error bit being set in pollfd.revents (is this a
            // Free/NetBSD bug??? ).  Notice that these anomalous packets exist
            // according to poll(), but have length 0 according to read().
            // Thus, we can remain stuck inside this loop.
            //
            // A solution to this problem is to get out of the inner loop
            // when read() returns <=0.  In this way, we have another chance
            // to call waitpid() to check if the child has died -- and this
            // time the call should succeed.
            //
            // Set POLLHUP in pollfd.revents to signal that something strange
            // has happened and disable polling of stdout.
            poller[STD_OUT].revents |= POLLHUP;
            poller[STD_OUT].events = 0;
          }
        } else if (poller[STD_OUT].revents & POLLHUP) {
          // disable polling of stdout
          poller[STD_OUT].events = 0;
        }

        // Check stderr.
        if (poller[STD_ERR].revents & POLLIN) {
          //qCDebug(KPGP_LOG) <<"Trying to read" << 1024 <<" bytes from perr[0]";
          if ((len = read(poller[STD_ERR].fd,str,1024))>0) {
            //qCDebug(KPGP_LOG) <<"Read" << len <<" bytes from perr[0]";
            str[len] ='\0';
            error += str;
          }
          else {
            // FreeBSD/NetBSD workaround (for details see above)
            poller[STD_ERR].revents |= POLLHUP;
            poller[STD_ERR].events = 0;
          }
        } else if (poller[STD_ERR].revents & POLLHUP) {
          // disable polling of stderr
          poller[STD_ERR].events = 0;
        }

        if (num_pollers > 2) {
          if (poller[STD_IN].revents & ( POLLERR | POLLHUP ) ) {
            qCDebug(KPGP_LOG) <<"GnuPG seems to have hung up";
            close (pin[1]);
            pin[1] = -1;
            --num_pollers;
          }
          else if (poller[STD_IN].revents & POLLOUT) {
            if (!input.isEmpty()) {
              // search end of next line
              if ((len2 = input.indexOf('\n', input_pos)) == -1)
                len2 = input_length - input_pos;
              else
                len2 = len2 - input_pos + 1;

              //qCDebug(KPGP_LOG) <<"Trying to write" << len2 <<" bytes to pin[1] ...";
              len2 = write(pin[1], input.data() + input_pos, len2 );
              //qCDebug(KPGP_LOG) <<"Wrote" << len2 <<" bytes to pin[1] ...";
              input_pos += len2;

              // We are done.
              if (input_pos >= input_length) {
                //qCDebug(KPGP_LOG) <<"All input was written to pin[1]";
                close (pin[1]);
                pin[1] = -1;
                --num_pollers;
              }
            }
            else { // if input.isEmpty()
              write(pin[1], "\n", 1);
              //qCDebug(KPGP_LOG) <<"All input was written to pin[1]";
              close (pin[1]);
              pin[1] = -1;
              --num_pollers;
            }
          }
        }
      }
    } while ( (pollstatus > 0) && ( (num_pollers > 2)
                                    || (poller[STD_OUT].events != 0)
                                    || (poller[STD_ERR].events != 0) ) );

    if (pollstatus == -1) {
      qCDebug(KPGP_LOG) <<"GnuPG poll failed, errno:" << errno;
    }

  } while(waitpidRetVal == 0);

  if( 0 <= pin[1] )
    close (pin[1]);
  close(pout[0]);
  close(perr[0]);

  if(passphrase)
    close(ppass[0]);

  // Did the child exit normally?
  if (WIFEXITED(childExitStatus) != 0) {
    // Get the return code of the child
    childExitStatus = WEXITSTATUS(childExitStatus);
    qCDebug(KPGP_LOG) <<"GnuPG exited with exit status" << childExitStatus;
  }
  else {
    childExitStatus = -1;
    qCDebug(KPGP_LOG) <<"GnuPG exited abnormally!";
  }

  //Uncomment these lines for testing only! Doing so will decrease security!
  //qCDebug(KPGP_LOG) <<"gpg stdout:" << QString(output);

  // Make the information visible, so that a user can
  // get to know what's going on during the gpg calls.
  qCDebug(KPGP_LOG) <<"gpg stderr:" << error;

  return childExitStatus;
#else // HAVE_SYS_POLL_H
#ifdef __GNUC__
#warning WIN32 libkpgp: GnuPG support not ported to WIN32!
#endif
  return 1;
#endif
}


QByteArray
Base::addUserId()
{
  QByteArray cmd;
  QByteArray pgpUser = Module::getKpgp()->user();

  if(!pgpUser.isEmpty())
  {
    cmd += " -u 0x";
    cmd += pgpUser;
    return cmd;
  }
  return QByteArray();
}


} // namespace Kpgp
