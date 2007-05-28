/***************************************************************************
   Copyright (C) 2006
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "p2kwrapper.h"
#include <kurl.h>
#include <kdebug.h>

#include <iostream>
#include <unistd.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3ValueList>
#define EXITDEBUG(msg, retval) { kDebug() << msg << endl; return retval; }

using namespace std;

P2KWrapper::P2KWrapper(int vendor, int at_prodID, int p2k_prodID, const QString &acmDevice, QObject *parent, const char *name)
    : QObject(parent, name)
{
    i_vendor=vendor;
    i_at_prodID=at_prodID;
    i_p2k_prodID=p2k_prodID;
    b_connected=false;
    s_acmDevice=acmDevice;

    p2k_init();
    p2k_setACMdevice( acmDevice.latin1() );
    p2k_setATconfig( vendor, at_prodID );
    p2k_setP2Kconfig( vendor, p2k_prodID );
    kDebug() << "Initialized p2k lib\n";
    n_files=0;
}

P2KWrapper::P2KWrapper(QObject *parent, const char *name)
    : QObject(parent, name)
{
    b_connected=false;
    p2k_init();

    kDebug() << "Initialized p2k lib\n";
    n_files=0;
}

void P2KWrapper::setATConfig(int v, int p)
{
    p2k_init();
    i_vendor=v;
    i_at_prodID=p;
    p2k_setATconfig(i_vendor, i_at_prodID);
}

void P2KWrapper::setP2KConfig(int v, int p)
{
    kDebug() << "Set p2k config: vendor=" << v << "; prodID=" << p << endl;
    p2k_init();
    i_vendor=v;
    i_p2k_prodID=p;
    p2k_setP2Kconfig(v,p);
}

bool P2KWrapper::findDevice(const QString &acmDevice, const QString &rawVendor, const QString &rawProduct)
{
    p2k_init();
    kDebug() << "P2KWrapper::findDevice(" << acmDevice << "," << rawVendor << "," << rawProduct << ")\n";
    p2k_setACMdevice( acmDevice.latin1() );
    p2k_devInfo* devList=p2k_getDevList();
    int i=0;
    while( devList[i].vendor >=0 ){
        kDebug() << "Current device infos: " << QString::number(devList[i].vendor,16)
                << "::" << QString::number(devList[i].product,16) << endl;
        if ( ! (devList[i].vendor==0x22b8) )
        {
            kDebug() << "Vendor not found\n";
            i++; continue;
        }
        kDebug() << "Found phone from vendor " << rawVendor << endl;
        QString model=QStringList::split( "MODEL=", rawProduct ).last();
        kDebug() << "Phone model from p2k:" << devList[i].productStr << ";\n";
        if ( QString( devList[i].productStr ).contains( model ) )
        {
            p2k_setACMdevice( acmDevice.latin1() );
            if ( devList[i].product%2 )
            {
                kDebug() << "Setting AT to " << QString::number(devList[i].product+1,16) << " and p2k to "
                        << QString::number(devList[i].product,16) << endl;
                p2k_setATconfig( devList[i].vendor, devList[i].product+1 );
                p2k_setP2Kconfig( devList[i].vendor, devList[i].product );
            } else
            {
                kDebug() << "Setting AT to " << QString::number(devList[i].product,16) << " and p2k to "
                        << QString::number(devList[i].product-1,16) << endl;
                p2k_setATconfig( devList[i].vendor, devList[i].product );
                p2k_setP2Kconfig( devList[i].vendor, devList[i].product -1);
            }
            return true;
        }
        i++;
    }
    return false;
}

static uint state;
static const char throbber[]={'|', '/', '-', '\\'};

extern "C" { Q3ValueList<p2k_fileInfo> kiop2k_fileList; }

extern "C"
        void onFile( p2k_fileInfo file)
{
//     kDebug() << "onFile " << throbber[state%4] << " :: " << state++ << endl;
    kiop2k_fileList+=(file);
}


P2KWrapper::~P2KWrapper()
{
    kDebug() << "P2KWrapper::~P2KWrapper()\n";
    kDebug() << "P2KWrapper::ClosingPhone\n";
    p2k_closePhone();
}


#include "p2kwrapper.moc"


/*!
    \fn P2KWrapper::connect()
 */
bool P2KWrapper::connectPhone()
{
    if (b_connected) EXITDEBUG("phone already connected", true);
    kDebug() << " P2KWrapper::connectPhone()\n";
    int err=p2k_findPhone();
    kDebug() << "err==" << err << endl;
    if(err==P2K_PHONE_NONE)
        EXITDEBUG("ERROR! Phone not found", false);
    if(err==P2K_PHONE_AT)
    {
        kDebug() << "Phone found in AT mode\n";
        if(p2k_setP2Kmode(1000) )
            EXITDEBUG("Could not set phone to p2k mode", false);
    }
    for( int i=0; i<10 ; i++ ){
        err=p2k_openPhone(100000);
        if( err ) kDebug() << "Can't connect to the phone (try " << i << ", error number: " << err << ")\n";
        else break;
        sleep (4);
    }
    if (err)
        EXITDEBUG("Couldn't connect to phone", false);
    kDebug() << "Phone connected in p2k mode\n";
    b_connected=true;
    int i_files=p2k_fileCount();
//     if (i_files != n_files) fetchFileList(false);
    return b_connected;
}

void P2KWrapper::closePhone()
{
    kDebug() << "P2KWrapper::closePhone()\n";
    if (  p2k_closePhone() )
    {
        kDebug() << "!!!!!!!!!!!!!!!! Error while closing phone\n";
        return;
    }
    b_connected=false;
    kDebug() << "**************** Phone disconnected\n";
}

/*!
    \fn P2KWrapper::getRoot()
 */
QStringList P2KWrapper::getRoot()
{
    kDebug() << "P2KWrapper::getRoot()\n";
    if (!connectPhone() ) return QStringList(QString() );
    char *buffer=new char[256];
    int err=p2k_FSAC_getVolumes( buffer );
    if( err )
    {
        delete [] buffer;
        return QStringList(QString() );
    }
    QString drives( (char*) buffer);
    delete [] buffer;
    kDebug() << drives << endl;
    if(drives=="FLASH")
    {
        return QStringList("/");
    }
    drives=drives.replace('/', "");
    return QStringList::split(" ", drives);
}

void P2KWrapper::fetchFileList()
{
    ::state=0;
    kDebug() << "P2KWrapper::fetchFileList()\n";
    if (!connectPhone() ) 
    {
        kDebug() << "***** ERROR: Couldn't connect to the phone\n";
        return;
    }
    files.clear();
    dirs.clear();
    ::kiop2k_fileList.clear();
    p2k_FSAC_fileList( onFile );
    kDebug() << "\nkio_p2k: ************************************* Finished listing files **********************************\n";
    kDebug() << "Files count: " << files.count() << ", while reported " << countFiles() << endl;
    files=::kiop2k_fileList;
//     dirs.clear();
    Q3ValueListIterator<p2k_fileInfo> it;
    QString filename;
    KUrl p_url;
    for( it=files.begin(); it!=files.end(); ++it)
    {
        p_url.setPath((*it).name);
//         kDebug() << "found filename: " << (*it).name << "; parent dir=" << p_url.directory() << endl;
//         filename=QString( (*it).name);
//         filename=filename.left( filename.findRev('/') ) ;
        if( dirs.findIndex( p_url.directory() ) <0 ) dirs.append( p_url.directory() );
//         for(int i=0; i<filename.contains('/'); i++)
//             if( dirs.findIndex( filename.left(filename.find('/',i) ) ) <0 ) dirs.append( filename.left(filename.find('/',i) ) );
    }
    QStringList tempdir;
    uint sep=0;
    for(QStringList::Iterator dit=dirs.begin(); dit!=dirs.end(); ++dit)
    {
        sep=(*dit).contains(QDir::separator());
        for(int j=1; j<=sep; j++)
            if(dirs.contains((*dit).section(QDir::separator(), 0, j))==0) dirs+=(*dit).section(QDir::separator(), 0, j);
    }
    n_files=files.count();
}

int P2KWrapper::countFiles()
{
    if(!connectPhone() ) return 0;
    return p2k_fileCount();
}

// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#define READ_BUF_SIZE 0x400


/**
 * @brief Get a file
 * @param name the name of the file.
 * @param buffer a pointer to the buffer, or NULL if you need only to discover the file size.
 * @return File size if successful, 0 otherwise.
 */
int P2KWrapper::getFile(const QString &name, char* buffer)
{
    kDebug() << "P2KWrapper::getFile(" << name << ",...)\n";
    if( ! connectPhone()  || getFiles().count() == 0)
    {
        kDebug() << "\\-/ <<----- Couldn't connect to phone\n";
        return 0;
    } else kDebug() << "\\-/ <<----- Connected to phone\n";
    p2k_FSAC_close();
//     QByteArray retval=0;
    p2k_fileInfo file;
    bool found=false;
    Q3ValueListIterator<p2k_fileInfo> files_it;
    for(files_it=files.begin(); files_it!=files.end(); ++files_it)
    {
        if( name==(*files_it).name )
        {
            file=(*files_it);
            found=true;
            break;
        }
    }
    if(!found) { return 0; }
    if( ! buffer ) return file.size;
    kDebug() << "kio_p2k: ******* trying to open " << file.name << endl;
    if( p2k_FSAC_open( file.name, file.attr ) ) {
        kDebug() << "******\\\\\\\\\\////// <<<<<<<<<<<>>>>>>>> openFile failed\n";
        return 0;
    }
    kDebug() << "kio_p2k: Open done, now reading to uchar[" << file.size << "]\n";
/*    if( ! p2k_FSAC_read( (uchar*) buffer, file.size) )
    {
        p2k_FSAC_close();
        return file.size;
    }*/
//     while( pt<file.size ){
//         kDebug() << "Seek success #" << pt << ":" << p2k_FSAC_seek( pt, P2K_SEEK_BEGIN ) << endl;
//         if(file.size-pt >= 0x400 )
//             p2k_FSAC_read( (uchar*) &buffer[pt], 0x400 );
//         else p2k_FSAC_read( (uchar*) &buffer[pt], file.size % 0x400 );
// 
//         pt+=0x400;
//     }
    kDebug() << "File opened, now reading\n";
    uchar *buf=new uchar[file.size];
    uchar *buf2=buf;
    int b_read=0, toread=0;
    while( b_read<file.size )
    {
        if( (file.size-b_read)>=READ_BUF_SIZE) toread=READ_BUF_SIZE; else
        {
            kDebug() << "Last record, remaining size=" << toread << endl;
            toread=(file.size-b_read);
        }
        if(p2k_FSAC_read(buf2, toread)!=0 )
        {
            p2k_FSAC_close();
            return 0;
        }
        buf2+=toread; b_read+=toread;
    }
//     int tmpfile=::open("/tmp/tmpfile", O_WRONLY | O_CREAT |O_TRUNC );
//     write(tmpfile, buf, file.size);
//     close(tmpfile);

//     for (int i=0; i<file.size; i++ )
//     {
//         cout << (int) cbuffer [ i ] << "   ";
//         if(! i%10) cout << endl;
//     }
    p2k_FSAC_close();
    memcpy(buffer, buf, file.size);
    delete [] buf;
    return file.size;
}

bool P2KWrapper::putFile( const QString &name, char* buffer, int size)
{
    kDebug() << "P2KWrapper::putFile(" << name << ",.......,......)\n";
    if( ! connectPhone() ) return false;

    kDebug() << "Writing " << name << " to the phone, size: " << size << "bytes.\n";
    kDebug() << "Opening connection: " << p2k_FSAC_open( (char*) name.latin1(), '\0' ) << endl;
    int pt=0;
    while ( pt<size )
    {
        kDebug() << "Seek to trunk #" << pt << ": " << p2k_FSAC_seek( pt, P2K_SEEK_BEGIN ) << endl;
        if(size>=0x400) p2k_FSAC_write( (uchar*) &buffer[pt], 0x400 );
        else p2k_FSAC_write( (uchar*) &buffer[pt], size%0x400 );
        pt+=0x400;
    }
    p2k_FSAC_close();
    return true;
}

bool P2KWrapper::deleteFile( const QString &name )
{
    kDebug() << "P2KWrapper::deleteFile(" << name << ")\n";
    if( ! connectPhone() ) return false;
    int result=p2k_FSAC_delete( (char*) name.latin1() );
    kDebug() << "Deleting " << name << ": " << result << endl;
//     p2k_FSAC_close();
    return !result;
}
