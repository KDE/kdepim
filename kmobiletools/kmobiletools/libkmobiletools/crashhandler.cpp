/***************************************************************************
*   Copyright (C) 2007 Sérgio Gomes <sergiomdgomes@gmail.com>             *
*   Original work by Max Howell <max.howell@methylblue.com> in Amarok     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "crashhandler.h"
#include "aboutdata.h"

#include <kdebug.h>       //kBacktrace()
#include <kdeversion.h>
#include <klocale.h>
#include <ktemporaryfile.h>

#include <qfile.h>
#include <qregexp.h>
#include <q3textstream.h>
#include <qglobal.h> //qVersion()
//Added by qt3to4:
#include <Q3CString>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <cstdio>         //popen, fread
#include <iostream>
#include <sys/types.h>    //pid_t
#include <sys/wait.h>     //waitpid
#include <unistd.h>       //write, getpid
#include <ktoolinvocation.h>

namespace KMobileTools
{
    static QString runCommand( const Q3CString &command )
    {
        static const uint SIZE = 40960; //40 KiB
        static char stdoutBuf[ SIZE ] = {0};

        std::cout << "Running: " << command.constData() << std::endl;

        FILE *process = ::popen( command, "r" );
        if ( process )
        {
            stdoutBuf[ std::fread( static_cast<void*>( stdoutBuf ), sizeof(char), SIZE-1, process ) ] = '\0';
                ::pclose( process );
        }
        return QString::fromLocal8Bit( stdoutBuf );
    }

    void Crash::crashHandler( int /*signal*/ )
    {
        // we need to fork to be able to get a
        // semi-decent bt - I dunno why
        const pid_t pid = ::fork();

        if( pid < 0 )
        {
            std::cout << "forking crash reporter failed\n";
            // continuing now can't do no good
            _exit( 1 );
    }
    else if ( pid == 0 )
    {
            // we are the child process (the result of the fork)
        std::cout << "KMobileTools is crashing...\n";

        QString subject = KMOBILETOOLS_VERSION " ";
        QString body = i18n(
                "KMobileTools has crashed! We are terribly sorry about this :(\n\n"
                "But, all is not lost! You could potentially help us fix the crash. "
                "Information describing the crash is below, so just click send, "
                "or if you have time, write a brief description of how the crash happened first.\n\n"
                "If you want, you can also add as attachment KMobileTools log files. You can find them in %1\n\n"
                "Many thanks.\n\n", KGlobal::dirs()->saveLocation("tmp", "kmobiletools", true) );
        body += i18n( "\n\n\n\n\n\n"
                "The information below is to help the developers identify the problem, "
                "please do not modify it.\n\n\n\n" );


        body += "======== DEBUG INFORMATION  =======\n"
                "Version:    " KMOBILETOOLS_VERSION "\n"
                "Build date: " __DATE__ "\n"
                "CC version: " __VERSION__ "\n" //assuming we're using GCC
                "KDElibs:    " KDE_VERSION_STRING "\n"
                "Qt:         %1\n"
                "CPU count:  %2\n";

        QString cpucount = "unknown";
#ifdef __linux__
            QString line;
            uint cpuCount = 0;
            QFile cpuinfo( "/proc/cpuinfo" );
            if ( cpuinfo.open( QIODevice::ReadOnly ) ) {
                while ( cpuinfo.readLine( line, 20000 ) != -1 ) {
                    if ( line.startsWith( "processor" ) ) {
                        ++cpuCount;
                    }
                }
            }
            cpucount = QString::number( cpuCount );
#endif


            body = body.arg( qVersion() )
                    .arg( cpucount );

#ifdef NDEBUG
            body += "NDEBUG:     true";
#endif
            body += '\n';

            /// obtain the backtrace with gdb

            KTemporaryFile temp;
            temp.setAutoRemove( true );

            const int handle = temp.handle();

//             QCString gdb_command_string =
//                     "file amarokapp\n"
//                     "attach " + QCString().setNum( ::getppid() ) + "\n"
//                     "bt\n" "echo \\n\n"
//                     "thread apply all bt\n";

            const Q3CString gdb_batch =
                    "bt\n"
                    "echo \\n\\n\n"
                    "bt full\n"
                    "echo \\n\\n\n"
                    "echo ==== (gdb) thread apply all bt ====\\n\n"
                    "thread apply all bt\n";

            ::write( handle, gdb_batch, gdb_batch.length() );
            ::fsync( handle );

            // so we can read stderr too
            ::dup2( fileno( stdout ), fileno( stderr ) );


            Q3CString gdb;
            gdb  = "gdb --nw -n --batch -x ";
            gdb += temp.name().latin1();
            gdb += " kmobiletools ";
            gdb += Q3CString().setNum( ::getppid() );

            QString bt = runCommand( gdb );

            /// clean up
            bt.remove( "(no debugging symbols found)..." );
            bt.remove( "(no debugging symbols found)\n" );
            bt.replace( QRegExp("\n{2,}"), "\n" ); //clean up multiple \n characters
            bt.trimmed();

            /// analyze usefulness
            bool useful = true;
            const QString fileCommandOutput = runCommand( "file `which kmobiletools`" );

            if( fileCommandOutput.find( "not stripped", false ) == -1 )
                subject += "[___stripped]"; //same length as below
            else
                subject += "[NOTstripped]";

            if( !bt.isEmpty() ) {
                const int invalidFrames = bt.count( QRegExp("\n#[0-9]+\\s+0x[0-9A-Fa-f]+ in \\?\\?") );
                const int validFrames = bt.count( QRegExp("\n#[0-9]+\\s+0x[0-9A-Fa-f]+ in [^?]") );
                const int totalFrames = invalidFrames + validFrames;
		const int sourceFrames = bt.count( QRegExp("at[\\s]+[\\w]+\\.(cpp|h):[\\d]+") );
		
	        body += QString("Total frames: %1, invalid: %2, valid: %3, with source: %4\n\n")
		.arg(totalFrames).arg(invalidFrames).arg(validFrames).arg(sourceFrames);

                if( totalFrames > 0 ) {
                    const double validity = (double(validFrames) / totalFrames) * sourceFrames;
                    subject += QString("[validity: %1]").arg( validity, 0, 'f', 2 );
                    if( validity <= 0.5 || sourceFrames==0 ) useful = false;
                }
                subject += QString("[frames: %1]").arg( totalFrames, 3 /*padding*/ );

                if( bt.find( QRegExp(" at \\w*\\.cpp:\\d+\n") ) >= 0 )
                    subject += "[line numbers]";
            }
            else
                useful = false;

            std::cout << subject.latin1() << std::endl;


            //TODO -fomit-frame-pointer buggers up the backtrace, so detect it
            //TODO -O optimization can rearrange execution and stuff so show a warning for the developer
            //TODO pass the CXXFLAGS used with the email

            if( useful ) {
                body += "==== file `which kmobiletools` =======\n";
                body += fileCommandOutput + "\n\n";
                body += "==== (gdb) bt =====================\n";
                body += bt + "\n\n";
                body += "==== kBacktrace() ================\n";
                body += kBacktrace();

                //TODO startup notification
                KToolInvocation::invokeMailer(
                        /*to*/          "bugs@kmobiletools.org",
                        /*cc*/          QString(),
                        /*bcc*/         QString(),
                        /*subject*/     subject,
                        /*body*/        body,
                        /*messageFile*/ QString(),
                        /*attachURLs*/  QStringList(),
                        /*startup_id*/  "" );
            }
            else {
                std::cout << qPrintable( i18n( "\nKMobileTools has crashed! We are terribly sorry about this :(\n\n"
                        "But, all is not lost! Perhaps an upgrade is already available "
                        "which fixes the problem. Please check your distribution's software repository.\n" ) );
            }

            //_exit() exits immediately, otherwise this
            //function is called repeatedly ad finitum
            ::_exit( 255 );
    }

    else {
            // we are the process that crashed

            ::alarm( 0 );

            // wait for child to exit
            ::waitpid( pid, NULL, 0 );
            ::_exit( 253 );
    }
    }
}

