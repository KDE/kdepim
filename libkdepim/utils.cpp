/**
 * utils.cpp
 *
 * Copyright (C)  2007 Laurent Montel <montel@kde.org>
 * Copyright (C) 2008 Jaroslaw Staniek <js@iidea.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "utils.h"

using namespace KPIM;

#if 0

#include <windows.h>
#include <comdef.h> // (bstr_t)
#include <winperf.h>
#include <psapi.h>
#include <signal.h>
#include <unistd.h>

#include <qlist.h>

#include <kdebug.h>

static PPERF_OBJECT_TYPE FirstObject( PPERF_DATA_BLOCK PerfData )
{
  return (PPERF_OBJECT_TYPE)((PBYTE)PerfData + PerfData->HeaderLength);
}

static PPERF_INSTANCE_DEFINITION FirstInstance( PPERF_OBJECT_TYPE PerfObj )
{
  return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj + PerfObj->DefinitionLength);
}

static PPERF_OBJECT_TYPE NextObject( PPERF_OBJECT_TYPE PerfObj )
{
  return (PPERF_OBJECT_TYPE)((PBYTE)PerfObj + PerfObj->TotalByteLength);
}

static PPERF_COUNTER_DEFINITION FirstCounter( PPERF_OBJECT_TYPE PerfObj )
{
  return (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj + PerfObj->HeaderLength);
}

static PPERF_INSTANCE_DEFINITION NextInstance( PPERF_INSTANCE_DEFINITION PerfInst )
{
  PPERF_COUNTER_BLOCK PerfCntrBlk 
    = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst + PerfInst->ByteLength);
  return (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfCntrBlk + PerfCntrBlk->ByteLength);
}

static PPERF_COUNTER_DEFINITION NextCounter( PPERF_COUNTER_DEFINITION PerfCntr )
{
  return (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr + PerfCntr->ByteLength);
}

static PPERF_COUNTER_BLOCK CounterBlock(PPERF_INSTANCE_DEFINITION PerfInst)
{
  return (PPERF_COUNTER_BLOCK) ((LPBYTE) PerfInst + PerfInst->ByteLength);
}

#define GETPID_TOTAL 64 * 1024
#define GETPID_BYTEINCREMENT 1024
#define GETPID_PROCESS_OBJECT_INDEX 230
#define GETPID_PROC_ID_COUNTER 784

void Utils::getProcessesIdForName( const QString& processName, QList<int>& pids )
{
  LPCTSTR pProcessName = (LPCTSTR)processName.utf16();
  PPERF_OBJECT_TYPE perfObject;
  PPERF_INSTANCE_DEFINITION perfInstance;
  PPERF_COUNTER_DEFINITION perfCounter, curCounter;
  PPERF_COUNTER_BLOCK counterPtr;
  DWORD bufSize = GETPID_TOTAL;
  PPERF_DATA_BLOCK perfData = (PPERF_DATA_BLOCK) malloc( bufSize );

  char key[64];
  sprintf(key,"%d %d", GETPID_PROCESS_OBJECT_INDEX, GETPID_PROC_ID_COUNTER);
  LONG lRes;
  while( (lRes = RegQueryValueExA( HKEY_PERFORMANCE_DATA,
                               key,
                               NULL,
                               NULL,
                               (LPBYTE) perfData,
                               &bufSize )) == ERROR_MORE_DATA )
  {
    // get a buffer that is big enough
    bufSize += GETPID_BYTEINCREMENT;
    perfData = (PPERF_DATA_BLOCK) realloc( perfData, bufSize );
  }

  // Get the first object type.
  perfObject = FirstObject( perfData );

  // Process all objects.
  for( uint i = 0; i < perfData->NumObjectTypes; i++ ) {
    if (perfObject->ObjectNameTitleIndex != GETPID_PROCESS_OBJECT_INDEX) {
      perfObject = NextObject( perfObject );
      continue;
    }
    pids.clear();
    perfCounter = FirstCounter( perfObject );
    perfInstance = FirstInstance( perfObject );
    _bstr_t bstrProcessName,bstrInput;
    // retrieve the instances
    for( int instance = 0; instance < perfObject->NumInstances; instance++ ) {
      curCounter = perfCounter;
      bstrInput = pProcessName;
      bstrProcessName = (wchar_t *)((PBYTE)perfInstance + perfInstance->NameOffset);
      if (!_wcsicmp((LPCWSTR)bstrProcessName, (LPCWSTR) bstrInput)) {
        // retrieve the counters
        for( uint counter = 0; counter < perfObject->NumCounters; counter++ ) {
          if (curCounter->CounterNameTitleIndex == GETPID_PROC_ID_COUNTER) {
            counterPtr = CounterBlock(perfInstance);
            DWORD *value = (DWORD*)((LPBYTE) counterPtr + curCounter->CounterOffset);
            pids.append( int( *value ) );
            break;
          }
          curCounter = NextCounter( curCounter );
        }
      }
      perfInstance = NextInstance( perfInstance );
    }
  }
  free(perfData);
  RegCloseKey(HKEY_PERFORMANCE_DATA);
}

bool Utils::otherProcessesExist( const QString& processName )
{
  QList<int> pids;
  getProcessesIdForName( processName, pids );
  int myPid = getpid();
  foreach ( int pid, pids ) {
    if (myPid != pid) {
      kDebug() << "Process ID is " << pid;
      return true;
    }
  }
  return false;
}

bool Utils::killProcesses( const QString& processName )
{
  QList<int> pids;
  getProcessesIdForName( processName, pids );
  if ( pids.empty() )
    return true;
  kWarning() << "Killing process \"" << processName << " (pid=" << pids[0] << ")..";
  int overallResult = 0;
  foreach( int pid, pids ) {
    int result = kill( pid, SIGTERM );
    if ( result == 0 )
      continue;
    result = kill( pid, SIGKILL );
    if ( result != 0 )
      overallResult = result;
  }
  return overallResult == 0;
}

#endif // Q_WS_WIN

QString Utils::rot13(const QString &s)
{
  QString r(s);

  for (int i=0; i<r.length(); i++) {
    if ( r[i] >= QChar('A') && r[i] <= QChar('M') ||
         r[i] >= QChar('a') && r[i] <= QChar('m') )
      r[i] = (char)((int)QChar(r[i]).toLatin1() + 13);
    else
      if  ( r[i] >= QChar('N') && r[i] <= QChar('Z') ||
            r[i] >= QChar('n') && r[i] <= QChar('z') )
        r[i] = (char)((int)QChar(r[i]).toLatin1() - 13);
  }

  return r;
}
