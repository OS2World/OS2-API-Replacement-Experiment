/*
 * PMSPL.C -- OS/2 PM Spooler Dynalink stub
 *
 * Replacement for PMSPL.DLL (LX, 280283 bytes, IBM v14.103)
 * Module description: 'PM Spooler Dynalink'
 *
 * Build (32-bit):
 *   wcc386 -bt=OS2 -bm -wx -s -fo=PMSPL.OBJ PMSPL.C
 *   wlink @PMSPL.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Format:  LX, 7 objects
 *   Obj1: 16-bit CODE   26KB  -- 16-bit thunk layer (SPLQM* entry points)
 *   Obj2: 16-bit CODE    4KB  -- 16-bit thunk table (DOSPRINT*, WPRINT*)
 *   Obj3: 32-bit CODE  265KB  -- full spooler implementation
 *   Obj4: 32-bit RW     0.3KB -- instance data
 *   Obj5: 32-bit RO SHR 0.6KB -- message strings, error table
 *   Obj6: 32-bit RW SHR 9KB   -- shared spooler state, config strings
 *   Obj7: 32-bit RO SHR 1.5KB -- user-facing error message strings
 *
 * Imports: SPL1B(14), DOSCALLS(75), PMGRE(11), PMGPI(24), PMWIN(24),
 *          KBDCALLS(5), VIOCALLS(4), NLS(15), PMSHAPI(10), SESMGR(12), MSG(11)
 *
 * Key strings (Obj6): 'DOSPFSINIT', 'PM_Q_RAW', 'C:\\OS2\\POOL.EXE',
 *   'DEFAULT.SEP', 'MAX_FILEHANDLES', 'NOBYPASS', 'oso001.MSG'
 * Key strings (Obj7): user-visible spooler error messages for print jobs
 *
 * Exports (233 functions) in groups:
 *   SPLQM*  (ords 1-36)    -- Spooler Queue Manager
 *   SPLSTD* (ords 50-56)   -- Standard spool device interface
 *   PRT*    (ords 70-79)   -- 16-bit printer interface (thunks)
 *   DOSPRINT* (ords 80-108) -- DOS Print API (LAN Manager compatible)
 *   WPRINT*  (ords 110-138) -- Win Print API
 *   SPL32*  (ords 168, 301-356) -- 32-bit spooler entry points
 *   NET*SPL* (ords 200-206) -- Network spooler
 *   SPL*    (ords 380-449)  -- SPL management API (queues, devices, jobs)
 *   SPLTHK* (ords 480-510)  -- Thunk versions of SPL* for 16-bit callers
 *   PRT*    (ords 600-608)  -- 32-bit PRT extensions
 *   SPL*    (ords 609-625)  -- SPL registration, control panel, printer API
 * ============================================================
 */

#define INCL_DOS
#define INCL_WIN
#define INCL_DEV
#include <os2.h>

/* SPLERR is ULONG in OS/2 Toolkit pmspl.h -- define it here since
 * pmspl.h conflicts with some Toolkit versions under OpenWatcom.
 * HQUEUE is a queue handle (ULONG). PHQUEUE is a pointer to one. */
#ifndef SPLERR
typedef ULONG SPLERR;
#endif
#ifndef HQUEUE
typedef ULONG HQUEUE;
typedef HQUEUE *PHQUEUE;
#endif
#ifndef PHFILE
typedef HFILE *PHFILE;
#endif

/* All stubs return PMSPL_ERROR (non-zero) to signal spooler absent */
#define PMSPL_ERROR  0xFFFFUL

/* ================================================================== */
/* SPLQM* -- Spooler Queue Manager (ords 1-36)                        */
/* ================================================================== */

SPLERR APIENTRY SPLQMOPEN(PSZ pszComputerName, PSZ pszQueueName, PHQUEUE phQueue)
{ (void)pszComputerName;(void)pszQueueName;(void)phQueue; return PMSPL_ERROR; }
#pragma aux SPLQMOPEN "SPLQMOPEN"

SPLERR APIENTRY SPLQMSTARTDOC(HQUEUE hQueue, PSZ pszDocName, PHFILE phFile)
{ (void)hQueue;(void)pszDocName;(void)phFile; return PMSPL_ERROR; }
#pragma aux SPLQMSTARTDOC "SPLQMSTARTDOC"

SPLERR APIENTRY SPLQMENDDOC(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMENDDOC "SPLQMENDDOC"

SPLERR APIENTRY SPLQMWRITE(HQUEUE hQueue, ULONG cbBuf, PVOID pBuf)
{ (void)hQueue;(void)cbBuf;(void)pBuf; return PMSPL_ERROR; }
#pragma aux SPLQMWRITE "SPLQMWRITE"

SPLERR APIENTRY SPLQMABORT(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMABORT "SPLQMABORT"

SPLERR APIENTRY SPLQMCLOSE(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMCLOSE "SPLQMCLOSE"

ULONG APIENTRY SPLMESSAGEBOX(PSZ pszText, ULONG idMsg, PSZ pszTitle,
                               ULONG flStyle, ULONG fBeep)
{ (void)pszText;(void)idMsg;(void)pszTitle;(void)flStyle;(void)fBeep;
  return PMSPL_ERROR; }
#pragma aux SPLMESSAGEBOX "SPLMESSAGEBOX"

SPLERR APIENTRY SPLQMABORTDOC(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMABORTDOC "SPLQMABORTDOC"

BOOL APIENTRY SPLQMSPOOLERPRESENT(PSZ pszComputerName)
{ (void)pszComputerName; return FALSE; }
#pragma aux SPLQMSPOOLERPRESENT "SPLQMSPOOLERPRESENT"

SPLERR APIENTRY SPLQMCHOOSELOGADDR(PULONG pulLogAddr)
{ (void)pulLogAddr; return PMSPL_ERROR; }
#pragma aux SPLQMCHOOSELOGADDR "SPLQMCHOOSELOGADDR"

SPLERR APIENTRY SPLQMINITIALIZE(PSZ pszComputerName)
{ (void)pszComputerName; return PMSPL_ERROR; }
#pragma aux SPLQMINITIALIZE "SPLQMINITIALIZE"

SPLERR APIENTRY SPLQMTERMINATE(PSZ pszComputerName)
{ (void)pszComputerName; return PMSPL_ERROR; }
#pragma aux SPLQMTERMINATE "SPLQMTERMINATE"

SPLERR APIENTRY SPLPRMSPOOL(PVOID pData, ULONG cbData)
{ (void)pData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPLPRMSPOOL "SPLPRMSPOOL"

SPLERR APIENTRY SPLQMCANCELWAITINGJOBS(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMCANCELWAITINGJOBS "SPLQMCANCELWAITINGJOBS"

SPLERR APIENTRY SPLQMCANCELPRINTINGJOB(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMCANCELPRINTINGJOB "SPLQMCANCELPRINTINGJOB"

SPLERR APIENTRY SPLVQIFINIT(PVOID pVQIFData, ULONG cbData)
{ (void)pVQIFData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPLVQIFINIT "SPLVQIFINIT"

SPLERR APIENTRY SPLINIT(VOID)
{ return PMSPL_ERROR; }
#pragma aux SPLINIT "SPLINIT"

SPLERR APIENTRY SPLQMLISTQUEUES(PSZ pszComputerName, PULONG pcQueues,
                                   PVOID pBuf, PULONG pcbBuf, ULONG ulLevel)
{ (void)pszComputerName;(void)pcQueues;(void)pBuf;(void)pcbBuf;(void)ulLevel;
  return PMSPL_ERROR; }
#pragma aux SPLQMLISTQUEUES "SPLQMLISTQUEUES"

SPLERR APIENTRY SPLQMQUERYQINFO(PSZ pszComputerName, PSZ pszQueueName,
                                   PVOID pBuf, PULONG pcbBuf, ULONG ulLevel)
{ (void)pszComputerName;(void)pszQueueName;(void)pBuf;(void)pcbBuf;(void)ulLevel;
  return PMSPL_ERROR; }
#pragma aux SPLQMQUERYQINFO "SPLQMQUERYQINFO"

SPLERR APIENTRY SPLQMLISTJOBS(HQUEUE hQueue, PULONG pcJobs,
                                 PVOID pBuf, PULONG pcbBuf, ULONG ulLevel)
{ (void)hQueue;(void)pcJobs;(void)pBuf;(void)pcbBuf;(void)ulLevel;
  return PMSPL_ERROR; }
#pragma aux SPLQMLISTJOBS "SPLQMLISTJOBS"

SPLERR APIENTRY SPLQMQUERYJINFO(HQUEUE hQueue, ULONG ulJobId,
                                   PVOID pBuf, PULONG pcbBuf, ULONG ulLevel)
{ (void)hQueue;(void)ulJobId;(void)pBuf;(void)pcbBuf;(void)ulLevel;
  return PMSPL_ERROR; }
#pragma aux SPLQMQUERYJINFO "SPLQMQUERYJINFO"

SPLERR APIENTRY SPLQMHOLDQUEUE(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMHOLDQUEUE "SPLQMHOLDQUEUE"

SPLERR APIENTRY SPLQMRELEASEQUEUE(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMRELEASEQUEUE "SPLQMRELEASEQUEUE"

SPLERR APIENTRY SPLQMHOLDJOB(HQUEUE hQueue, ULONG ulJobId)
{ (void)hQueue;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLQMHOLDJOB "SPLQMHOLDJOB"

SPLERR APIENTRY SPLQMRELEASEJOB(HQUEUE hQueue, ULONG ulJobId)
{ (void)hQueue;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLQMRELEASEJOB "SPLQMRELEASEJOB"

SPLERR APIENTRY SPLQMCANCELJOB(HQUEUE hQueue, ULONG ulJobId)
{ (void)hQueue;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLQMCANCELJOB "SPLQMCANCELJOB"

SPLERR APIENTRY SPLQMRESTARTJOB(HQUEUE hQueue, ULONG ulJobId)
{ (void)hQueue;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLQMRESTARTJOB "SPLQMRESTARTJOB"

SPLERR APIENTRY SPLQMREPEATJOB(HQUEUE hQueue, ULONG ulJobId)
{ (void)hQueue;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLQMREPEATJOB "SPLQMREPEATJOB"

SPLERR APIENTRY SPLQMPRINTJOBNEXT(HQUEUE hQueue, ULONG ulJobId)
{ (void)hQueue;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLQMPRINTJOBNEXT "SPLQMPRINTJOBNEXT"

SPLERR APIENTRY SPLQMQUERYPINFO(HQUEUE hQueue, PVOID pBuf,
                                   PULONG pcbBuf, ULONG ulLevel)
{ (void)hQueue;(void)pBuf;(void)pcbBuf;(void)ulLevel; return PMSPL_ERROR; }
#pragma aux SPLQMQUERYPINFO "SPLQMQUERYPINFO"

SPLERR APIENTRY SPLQMCHANGEJOBPRTY(HQUEUE hQueue, ULONG ulJobId, ULONG ulPriority)
{ (void)hQueue;(void)ulJobId;(void)ulPriority; return PMSPL_ERROR; }
#pragma aux SPLQMCHANGEJOBPRTY "SPLQMCHANGEJOBPRTY"

SPLERR APIENTRY SPLQMWRITEFILE(HQUEUE hQueue, PSZ pszFileName)
{ (void)hQueue;(void)pszFileName; return PMSPL_ERROR; }
#pragma aux SPLQMWRITEFILE "SPLQMWRITEFILE"

SPLERR APIENTRY SPLQMSETSTATUS(HQUEUE hQueue, ULONG ulJobId, ULONG ulStatus)
{ (void)hQueue;(void)ulJobId;(void)ulStatus; return PMSPL_ERROR; }
#pragma aux SPLQMSETSTATUS "SPLQMSETSTATUS"

/* ================================================================== */
/* SPLSTD* -- Standard spool device (ords 50-56)                      */
/* ================================================================== */

SPLERR APIENTRY SPLSTDOPEN(PSZ pszDeviceName, PHFILE phFile)
{ (void)pszDeviceName;(void)phFile; return PMSPL_ERROR; }
#pragma aux SPLSTDOPEN "SPLSTDOPEN"

SPLERR APIENTRY SPLSTDCLOSE(HFILE hFile)
{ (void)hFile; return PMSPL_ERROR; }
#pragma aux SPLSTDCLOSE "SPLSTDCLOSE"

SPLERR APIENTRY SPLSTDSTART(HFILE hFile, PSZ pszJobName)
{ (void)hFile;(void)pszJobName; return PMSPL_ERROR; }
#pragma aux SPLSTDSTART "SPLSTDSTART"

SPLERR APIENTRY SPLSTDSTOP(HFILE hFile)
{ (void)hFile; return PMSPL_ERROR; }
#pragma aux SPLSTDSTOP "SPLSTDSTOP"

SPLERR APIENTRY SPLSTDDELETE(HFILE hFile)
{ (void)hFile; return PMSPL_ERROR; }
#pragma aux SPLSTDDELETE "SPLSTDDELETE"

SPLERR APIENTRY SPLSTDGETBITS(HFILE hFile, PVOID pBuf, ULONG cbBuf,
                                 PULONG pcbActual)
{ (void)hFile;(void)pBuf;(void)cbBuf;(void)pcbActual; return PMSPL_ERROR; }
#pragma aux SPLSTDGETBITS "SPLSTDGETBITS"

SPLERR APIENTRY SPLSTDQUERYLENGTH(HFILE hFile, PULONG pcbLength)
{ (void)hFile;(void)pcbLength; return PMSPL_ERROR; }
#pragma aux SPLSTDQUERYLENGTH "SPLSTDQUERYLENGTH"

/* ================================================================== */
/* PRT* -- 16-bit printer interface thunks (ords 70-79)               */
/* ================================================================== */

SPLERR APIENTRY PRTOPEN(PSZ pszDeviceName, PULONG phPrinter)
{ (void)pszDeviceName;(void)phPrinter; return PMSPL_ERROR; }
#pragma aux PRTOPEN "PRTOPEN"

SPLERR APIENTRY PRTWRITE(ULONG hPrinter, PVOID pBuf, ULONG cbBuf,
                           PULONG pcbActual)
{ (void)hPrinter;(void)pBuf;(void)cbBuf;(void)pcbActual; return PMSPL_ERROR; }
#pragma aux PRTWRITE "PRTWRITE"

SPLERR APIENTRY PRTDEVIOCTL(ULONG hPrinter, ULONG ulCategory, ULONG ulFunction,
                               PVOID pParams, ULONG cbParams,
                               PVOID pData, ULONG cbData)
{ (void)hPrinter;(void)ulCategory;(void)ulFunction;(void)pParams;
  (void)cbParams;(void)pData;(void)cbData; return PMSPL_ERROR; }
#pragma aux PRTDEVIOCTL "PRTDEVIOCTL"

SPLERR APIENTRY PRTCLOSE(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRTCLOSE "PRTCLOSE"

SPLERR APIENTRY PRTABORT(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRTABORT "PRTABORT"

SPLERR APIENTRY PRT16QUERY(ULONG hPrinter, PVOID pBuf, ULONG cbBuf)
{ (void)hPrinter;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux PRT16QUERY "PRT16QUERY"

SPLERR APIENTRY PRT16SET(ULONG hPrinter, PVOID pBuf, ULONG cbBuf)
{ (void)hPrinter;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux PRT16SET "PRT16SET"

SPLERR APIENTRY PRT16ABORTDOC(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRT16ABORTDOC "PRT16ABORTDOC"

SPLERR APIENTRY PRT16RESETABORT(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRT16RESETABORT "PRT16RESETABORT"

/* ================================================================== */
/* DOSPRINT* -- DOS Print API / LAN Manager compat (ords 80-108)      */
/* ================================================================== */

SPLERR APIENTRY DOSPRINTDESTCONTROL(PSZ pszServer, PSZ pszDevName,
                                      ULONG ulControl)
{ (void)pszServer;(void)pszDevName;(void)ulControl; return PMSPL_ERROR; }
#pragma aux DOSPRINTDESTCONTROL "DOSPRINTDESTCONTROL"

SPLERR APIENTRY DOSPRINTDESTGETINFO(PSZ pszServer, PSZ pszName,
                                      ULONG ulLevel, PVOID pBuf,
                                      ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux DOSPRINTDESTGETINFO "DOSPRINTDESTGETINFO"

SPLERR APIENTRY DOSPRINTDESTENUM(PSZ pszServer, ULONG ulLevel,
                                    PVOID pBuf, ULONG cbBuf,
                                    PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux DOSPRINTDESTENUM "DOSPRINTDESTENUM"

SPLERR APIENTRY DOSPRINTJOBCONTINUE(PSZ pszServer, ULONG ulJobId)
{ (void)pszServer;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBCONTINUE "DOSPRINTJOBCONTINUE"

SPLERR APIENTRY DOSPRINTJOBPAUSE(PSZ pszServer, ULONG ulJobId)
{ (void)pszServer;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBPAUSE "DOSPRINTJOBPAUSE"

SPLERR APIENTRY DOSPRINTJOBDEL(PSZ pszServer, ULONG ulJobId)
{ (void)pszServer;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBDEL "DOSPRINTJOBDEL"

SPLERR APIENTRY DOSPRINTJOBSCHEDULE(PSZ pszServer, PSZ pszQueueName,
                                      ULONG ulJobId)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBSCHEDULE "DOSPRINTJOBSCHEDULE"

SPLERR APIENTRY DOSPRINTJOBADD(PSZ pszServer, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBADD "DOSPRINTJOBADD"

SPLERR APIENTRY DOSPRINTJOBADD2(PSZ pszServer, ULONG ulLevel,
                                   PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBADD2 "DOSPRINTJOBADD2"

SPLERR APIENTRY DOSPRINTJOBGETINFO(PSZ pszServer, ULONG ulJobId,
                                     ULONG ulLevel, PVOID pBuf,
                                     ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)ulJobId;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBGETINFO "DOSPRINTJOBGETINFO"

SPLERR APIENTRY DOSPRINTJOBSETINFO(PSZ pszServer, ULONG ulJobId,
                                     ULONG ulLevel, PVOID pBuf,
                                     ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)ulJobId;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBSETINFO "DOSPRINTJOBSETINFO"

SPLERR APIENTRY DOSPRINTJOBENUM(PSZ pszServer, PSZ pszQueueName,
                                   ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                                   PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBENUM "DOSPRINTJOBENUM"

SPLERR APIENTRY DOSPRINTQADD(PSZ pszServer, ULONG ulLevel,
                                PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux DOSPRINTQADD "DOSPRINTQADD"

SPLERR APIENTRY DOSPRINTQPAUSE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux DOSPRINTQPAUSE "DOSPRINTQPAUSE"

SPLERR APIENTRY DOSPRINTQCONTINUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux DOSPRINTQCONTINUE "DOSPRINTQCONTINUE"

SPLERR APIENTRY DOSPRINTQDEL(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux DOSPRINTQDEL "DOSPRINTQDEL"

SPLERR APIENTRY DOSPRINTQGETINFO(PSZ pszServer, PSZ pszQueueName,
                                    ULONG ulLevel, PVOID pBuf,
                                    ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux DOSPRINTQGETINFO "DOSPRINTQGETINFO"

SPLERR APIENTRY DOSPRINTQSETINFO(PSZ pszServer, PSZ pszQueueName,
                                    ULONG ulLevel, PVOID pBuf,
                                    ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux DOSPRINTQSETINFO "DOSPRINTQSETINFO"

SPLERR APIENTRY DOSPRINTQENUM(PSZ pszServer, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf,
                                 PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux DOSPRINTQENUM "DOSPRINTQENUM"

SPLERR APIENTRY DOSPRINTDESTADD(PSZ pszServer, ULONG ulLevel,
                                   PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux DOSPRINTDESTADD "DOSPRINTDESTADD"

SPLERR APIENTRY DOSPRINTDESTSETINFO(PSZ pszServer, PSZ pszName,
                                      ULONG ulLevel, PVOID pBuf,
                                      ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux DOSPRINTDESTSETINFO "DOSPRINTDESTSETINFO"

SPLERR APIENTRY DOSPRINTDESTDEL(PSZ pszServer, PSZ pszName)
{ (void)pszServer;(void)pszName; return PMSPL_ERROR; }
#pragma aux DOSPRINTDESTDEL "DOSPRINTDESTDEL"

SPLERR APIENTRY DOSPRINTQPURGE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux DOSPRINTQPURGE "DOSPRINTQPURGE"

SPLERR APIENTRY DOSPRINTJOBGETID(HFILE hFile, PVOID pInfo)
{ (void)hFile;(void)pInfo; return PMSPL_ERROR; }
#pragma aux DOSPRINTJOBGETID "DOSPRINTJOBGETID"

SPLERR APIENTRY DOSPRINTDRIVERENUM(PSZ pszServer, ULONG ulLevel,
                                     PVOID pBuf, ULONG cbBuf,
                                     PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux DOSPRINTDRIVERENUM "DOSPRINTDRIVERENUM"

SPLERR APIENTRY DOSPRINTQPROCESSORENUM(PSZ pszServer, ULONG ulLevel,
                                         PVOID pBuf, ULONG cbBuf,
                                         PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux DOSPRINTQPROCESSORENUM "DOSPRINTQPROCESSORENUM"

SPLERR APIENTRY DOSPRINTPORTENUM(PSZ pszServer, ULONG ulLevel,
                                    PVOID pBuf, ULONG cbBuf,
                                    PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux DOSPRINTPORTENUM "DOSPRINTPORTENUM"

/* ================================================================== */
/* WPRINT* -- Win Print API (ords 110-138)                            */
/* ================================================================== */

SPLERR APIENTRY WPRINTDESTCONTROL(PSZ pszServer, PSZ pszDevName, ULONG ulControl)
{ (void)pszServer;(void)pszDevName;(void)ulControl; return PMSPL_ERROR; }
#pragma aux WPRINTDESTCONTROL "WPRINTDESTCONTROL"

SPLERR APIENTRY WPRINTDESTGETINFO(PSZ pszServer, PSZ pszName, ULONG ulLevel,
                                    PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux WPRINTDESTGETINFO "WPRINTDESTGETINFO"

SPLERR APIENTRY WPRINTDESTENUM(PSZ pszServer, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf,
                                  PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux WPRINTDESTENUM "WPRINTDESTENUM"

SPLERR APIENTRY WPRINTJOBCONTINUE(PSZ pszServer, ULONG ulJobId)
{ (void)pszServer;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux WPRINTJOBCONTINUE "WPRINTJOBCONTINUE"

SPLERR APIENTRY WPRINTJOBPAUSE(PSZ pszServer, ULONG ulJobId)
{ (void)pszServer;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux WPRINTJOBPAUSE "WPRINTJOBPAUSE"

SPLERR APIENTRY WPRINTJOBDEL(PSZ pszServer, ULONG ulJobId)
{ (void)pszServer;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux WPRINTJOBDEL "WPRINTJOBDEL"

SPLERR APIENTRY WPRINTJOBGETINFO(PSZ pszServer, ULONG ulJobId, ULONG ulLevel,
                                    PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)ulJobId;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux WPRINTJOBGETINFO "WPRINTJOBGETINFO"

SPLERR APIENTRY WPRINTJOBSETINFO(PSZ pszServer, ULONG ulJobId, ULONG ulLevel,
                                    PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)ulJobId;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux WPRINTJOBSETINFO "WPRINTJOBSETINFO"

SPLERR APIENTRY WPRINTJOBENUM(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf,
                                 PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux WPRINTJOBENUM "WPRINTJOBENUM"

SPLERR APIENTRY WPRINTQADD(PSZ pszServer, ULONG ulLevel, PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux WPRINTQADD "WPRINTQADD"

SPLERR APIENTRY WPRINTQPAUSE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux WPRINTQPAUSE "WPRINTQPAUSE"

SPLERR APIENTRY WPRINTQCONTINUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux WPRINTQCONTINUE "WPRINTQCONTINUE"

SPLERR APIENTRY WPRINTQDEL(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux WPRINTQDEL "WPRINTQDEL"

SPLERR APIENTRY WPRINTQGETINFO(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux WPRINTQGETINFO "WPRINTQGETINFO"

SPLERR APIENTRY WPRINTQSETINFO(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux WPRINTQSETINFO "WPRINTQSETINFO"

SPLERR APIENTRY WPRINTQENUM(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                               ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux WPRINTQENUM "WPRINTQENUM"

SPLERR APIENTRY WPRINTDESTADD(PSZ pszServer, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux WPRINTDESTADD "WPRINTDESTADD"

SPLERR APIENTRY WPRINTDESTSETINFO(PSZ pszServer, PSZ pszName, ULONG ulLevel,
                                     PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux WPRINTDESTSETINFO "WPRINTDESTSETINFO"

SPLERR APIENTRY WPRINTDESTDEL(PSZ pszServer, PSZ pszName)
{ (void)pszServer;(void)pszName; return PMSPL_ERROR; }
#pragma aux WPRINTDESTDEL "WPRINTDESTDEL"

SPLERR APIENTRY WPRINTQPURGE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux WPRINTQPURGE "WPRINTQPURGE"

SPLERR APIENTRY WPRINTDRIVERENUM(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                    ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux WPRINTDRIVERENUM "WPRINTDRIVERENUM"

SPLERR APIENTRY WPRINTQPROCESSORENUM(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                        ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux WPRINTQPROCESSORENUM "WPRINTQPROCESSORENUM"

SPLERR APIENTRY WPRINTPORTENUM(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                  ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux WPRINTPORTENUM "WPRINTPORTENUM"

SPLERR APIENTRY WPRINTJOBCOPY(PSZ pszServer, ULONG ulJobId, PSZ pszQueueName)
{ (void)pszServer;(void)ulJobId;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux WPRINTJOBCOPY "WPRINTJOBCOPY"

SPLERR APIENTRY WPRINTDRIVERGETINFO(PSZ pszServer, PSZ pszDriverName,
                                      ULONG ulLevel, PVOID pBuf,
                                      ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszDriverName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux WPRINTDRIVERGETINFO "WPRINTDRIVERGETINFO"

SPLERR APIENTRY WPRINTDRIVERSETINFO(PSZ pszServer, PSZ pszDriverName,
                                      ULONG ulLevel, PVOID pBuf,
                                      ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszDriverName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux WPRINTDRIVERSETINFO "WPRINTDRIVERSETINFO"

SPLERR APIENTRY WPRINTJOBMOVE(PSZ pszServer, ULONG ulJobId, PSZ pszQueueName)
{ (void)pszServer;(void)ulJobId;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux WPRINTJOBMOVE "WPRINTJOBMOVE"

SPLERR APIENTRY WPRINTJOBMOVEALL(PSZ pszServer, PSZ pszSrcQueue,
                                    PSZ pszDstQueue)
{ (void)pszServer;(void)pszSrcQueue;(void)pszDstQueue; return PMSPL_ERROR; }
#pragma aux WPRINTJOBMOVEALL "WPRINTJOBMOVEALL"

SPLERR APIENTRY WPRT16SET(ULONG hPrinter, PVOID pBuf, ULONG cbBuf)
{ (void)hPrinter;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux WPRT16SET "WPRT16SET"

/* ================================================================== */
/* SPL32* ords 168, 301-320, 350-356                                  */
/* ================================================================== */

SPLERR APIENTRY SPL32PROFILEWRITECHECK(PSZ pszProfile, PSZ pszSection,
                                          PSZ pszKey, PSZ pszValue)
{ (void)pszProfile;(void)pszSection;(void)pszKey;(void)pszValue;
  return PMSPL_ERROR; }
#pragma aux SPL32PROFILEWRITECHECK "SPL32PROFILEWRITECHECK"

SPLERR APIENTRY SPL32QMOPEN(PSZ pszComputerName, PSZ pszQueueName,
                               PHQUEUE phQueue)
{ (void)pszComputerName;(void)pszQueueName;(void)phQueue; return PMSPL_ERROR; }
#pragma aux SPL32QMOPEN "SPL32QMOPEN"

SPLERR APIENTRY SPL32QMSTARTDOC(HQUEUE hQueue, PSZ pszDocName, PHFILE phFile)
{ (void)hQueue;(void)pszDocName;(void)phFile; return PMSPL_ERROR; }
#pragma aux SPL32QMSTARTDOC "SPL32QMSTARTDOC"

SPLERR APIENTRY SPL32QMENDDOC(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPL32QMENDDOC "SPL32QMENDDOC"

SPLERR APIENTRY SPL32QMWRITE(HQUEUE hQueue, ULONG cbBuf, PVOID pBuf)
{ (void)hQueue;(void)cbBuf;(void)pBuf; return PMSPL_ERROR; }
#pragma aux SPL32QMWRITE "SPL32QMWRITE"

SPLERR APIENTRY SPL32QMABORT(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPL32QMABORT "SPL32QMABORT"

SPLERR APIENTRY SPL32QMCLOSE(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPL32QMCLOSE "SPL32QMCLOSE"

ULONG APIENTRY SPL32MESSAGEBOX(PSZ pszText, ULONG idMsg, PSZ pszTitle,
                                  ULONG flStyle, ULONG fBeep)
{ (void)pszText;(void)idMsg;(void)pszTitle;(void)flStyle;(void)fBeep;
  return PMSPL_ERROR; }
#pragma aux SPL32MESSAGEBOX "SPL32MESSAGEBOX"

SPLERR APIENTRY SPL32QMABORTDOC(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPL32QMABORTDOC "SPL32QMABORTDOC"

BOOL APIENTRY SPL32QMSPOOLERPRESENT(PSZ pszComputerName)
{ (void)pszComputerName; return FALSE; }
#pragma aux SPL32QMSPOOLERPRESENT "SPL32QMSPOOLERPRESENT"

SPLERR APIENTRY SPL32QMCHOOSELOGADDR(PULONG pulLogAddr)
{ (void)pulLogAddr; return PMSPL_ERROR; }
#pragma aux SPL32QMCHOOSELOGADDR "SPL32QMCHOOSELOGADDR"

SPLERR APIENTRY SPL32QMINITIALIZE(PSZ pszComputerName)
{ (void)pszComputerName; return PMSPL_ERROR; }
#pragma aux SPL32QMINITIALIZE "SPL32QMINITIALIZE"

SPLERR APIENTRY SPL32QMTERMINATE(PSZ pszComputerName)
{ (void)pszComputerName; return PMSPL_ERROR; }
#pragma aux SPL32QMTERMINATE "SPL32QMTERMINATE"

SPLERR APIENTRY SPL32PRMSPOOL(PVOID pData, ULONG cbData)
{ (void)pData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPL32PRMSPOOL "SPL32PRMSPOOL"

SPLERR APIENTRY SPL32QMCANCELWAITINGJOBS(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPL32QMCANCELWAITINGJOBS "SPL32QMCANCELWAITINGJOBS"

SPLERR APIENTRY SPL32QMCANCELPRINTINGJOB(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPL32QMCANCELPRINTINGJOB "SPL32QMCANCELPRINTINGJOB"

SPLERR APIENTRY SPL32VQIFINIT(PVOID pVQIFData, ULONG cbData)
{ (void)pVQIFData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPL32VQIFINIT "SPL32VQIFINIT"

SPLERR APIENTRY SPL32INIT(VOID)
{ return PMSPL_ERROR; }
#pragma aux SPL32INIT "SPL32INIT"

SPLERR APIENTRY SPL32STDOPEN(PSZ pszDeviceName, PHFILE phFile)
{ (void)pszDeviceName;(void)phFile; return PMSPL_ERROR; }
#pragma aux SPL32STDOPEN "SPL32STDOPEN"

SPLERR APIENTRY SPL32STDCLOSE(HFILE hFile)
{ (void)hFile; return PMSPL_ERROR; }
#pragma aux SPL32STDCLOSE "SPL32STDCLOSE"

SPLERR APIENTRY SPL32STDSTART(HFILE hFile, PSZ pszJobName)
{ (void)hFile;(void)pszJobName; return PMSPL_ERROR; }
#pragma aux SPL32STDSTART "SPL32STDSTART"

SPLERR APIENTRY SPL32STDSTOP(HFILE hFile)
{ (void)hFile; return PMSPL_ERROR; }
#pragma aux SPL32STDSTOP "SPL32STDSTOP"

SPLERR APIENTRY SPL32STDDELETE(HFILE hFile)
{ (void)hFile; return PMSPL_ERROR; }
#pragma aux SPL32STDDELETE "SPL32STDDELETE"

SPLERR APIENTRY SPL32STDGETBITS(HFILE hFile, PVOID pBuf, ULONG cbBuf,
                                   PULONG pcbActual)
{ (void)hFile;(void)pBuf;(void)cbBuf;(void)pcbActual; return PMSPL_ERROR; }
#pragma aux SPL32STDGETBITS "SPL32STDGETBITS"

SPLERR APIENTRY SPL32STDQUERYLENGTH(HFILE hFile, PULONG pcbLength)
{ (void)hFile;(void)pcbLength; return PMSPL_ERROR; }
#pragma aux SPL32STDQUERYLENGTH "SPL32STDQUERYLENGTH"

/* ================================================================== */
/* PRT32* -- 32-bit printer interface (ords 370-375)                  */
/* ================================================================== */

SPLERR APIENTRY PRT32OPEN(PSZ pszDeviceName, PULONG phPrinter)
{ (void)pszDeviceName;(void)phPrinter; return PMSPL_ERROR; }
#pragma aux PRT32OPEN "PRT32OPEN"

SPLERR APIENTRY PRT32WRITE(ULONG hPrinter, PVOID pBuf, ULONG cbBuf,
                              PULONG pcbActual)
{ (void)hPrinter;(void)pBuf;(void)cbBuf;(void)pcbActual; return PMSPL_ERROR; }
#pragma aux PRT32WRITE "PRT32WRITE"

SPLERR APIENTRY PRT32DEVIOCTL(ULONG hPrinter, ULONG ulCategory,
                                ULONG ulFunction, PVOID pParams, ULONG cbParams,
                                PVOID pData, ULONG cbData)
{ (void)hPrinter;(void)ulCategory;(void)ulFunction;(void)pParams;
  (void)cbParams;(void)pData;(void)cbData; return PMSPL_ERROR; }
#pragma aux PRT32DEVIOCTL "PRT32DEVIOCTL"

SPLERR APIENTRY PRT32CLOSE(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRT32CLOSE "PRT32CLOSE"

SPLERR APIENTRY PRT32ABORT(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRT32ABORT "PRT32ABORT"

/* ================================================================== */
/* SPL management API (ords 380-449)                                  */
/* ================================================================== */

SPLERR APIENTRY SPLCONTROLDEVICE(PSZ pszServer, PSZ pszDevName, ULONG ulControl)
{ (void)pszServer;(void)pszDevName;(void)ulControl; return PMSPL_ERROR; }
#pragma aux SPLCONTROLDEVICE "SPLCONTROLDEVICE"

SPLERR APIENTRY SPLQUERYDEVICE(PSZ pszServer, PSZ pszDevName, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszDevName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLQUERYDEVICE "SPLQUERYDEVICE"

SPLERR APIENTRY SPLENUMDEVICE(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                 ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLENUMDEVICE "SPLENUMDEVICE"

SPLERR APIENTRY SPLRELEASEJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLRELEASEJOB "SPLRELEASEJOB"

SPLERR APIENTRY SPLHOLDJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLHOLDJOB "SPLHOLDJOB"

SPLERR APIENTRY SPLDELETEJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLDELETEJOB "SPLDELETEJOB"

SPLERR APIENTRY SPLQUERYJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId,
                               ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                               PULONG pcbNeeded)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId;(void)ulLevel;
  (void)pBuf;(void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLQUERYJOB "SPLQUERYJOB"

SPLERR APIENTRY SPLSETJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId,
                             ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                             ULONG ulParmNum)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId;(void)ulLevel;
  (void)pBuf;(void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLSETJOB "SPLSETJOB"

SPLERR APIENTRY SPLENUMJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                              PVOID pBuf, ULONG cbBuf,
                              PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLENUMJOB "SPLENUMJOB"

SPLERR APIENTRY SPLCREATEQUEUE(PSZ pszServer, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux SPLCREATEQUEUE "SPLCREATEQUEUE"

SPLERR APIENTRY SPLHOLDQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLHOLDQUEUE "SPLHOLDQUEUE"

SPLERR APIENTRY SPLRELEASEQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLRELEASEQUEUE "SPLRELEASEQUEUE"

SPLERR APIENTRY SPLDELETEQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLDELETEQUEUE "SPLDELETEQUEUE"

SPLERR APIENTRY SPLQUERYQUEUE(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLQUERYQUEUE "SPLQUERYQUEUE"

SPLERR APIENTRY SPLSETQUEUE(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                               PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLSETQUEUE "SPLSETQUEUE"

SPLERR APIENTRY SPLENUMQUEUE(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLENUMQUEUE "SPLENUMQUEUE"

SPLERR APIENTRY SPLCREATEDEVICE(PSZ pszServer, ULONG ulLevel,
                                   PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux SPLCREATEDEVICE "SPLCREATEDEVICE"

SPLERR APIENTRY SPLSETDEVICE(PSZ pszServer, PSZ pszDevName, ULONG ulLevel,
                                PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszDevName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLSETDEVICE "SPLSETDEVICE"

SPLERR APIENTRY SPLDELETEDEVICE(PSZ pszServer, PSZ pszDevName)
{ (void)pszServer;(void)pszDevName; return PMSPL_ERROR; }
#pragma aux SPLDELETEDEVICE "SPLDELETEDEVICE"

SPLERR APIENTRY SPLPURGEQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLPURGEQUEUE "SPLPURGEQUEUE"

SPLERR APIENTRY SPLENUMDRIVER(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                 ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLENUMDRIVER "SPLENUMDRIVER"

SPLERR APIENTRY SPLENUMQUEUEPROCESSOR(PSZ pszServer, ULONG ulLevel,
                                         PVOID pBuf, ULONG cbBuf,
                                         PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLENUMQUEUEPROCESSOR "SPLENUMQUEUEPROCESSOR"

SPLERR APIENTRY SPLENUMPORT(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                               ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLENUMPORT "SPLENUMPORT"

SPLERR APIENTRY SPLQUERYPORT(PSZ pszServer, PSZ pszPortName, ULONG ulLevel,
                                PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszPortName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLQUERYPORT "SPLQUERYPORT"

SPLERR APIENTRY SPLSETPORT(PSZ pszServer, PSZ pszPortName, ULONG ulLevel,
                              PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszPortName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLSETPORT "SPLSETPORT"

SPLERR APIENTRY SPLQUERYPATH(PSZ pszServer, PULONG pulType,
                                PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pulType;(void)pBuf;(void)cbBuf;(void)pcbNeeded;
  return PMSPL_ERROR; }
#pragma aux SPLQUERYPATH "SPLQUERYPATH"

SPLERR APIENTRY SPLCREATEPORT(PSZ pszServer, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux SPLCREATEPORT "SPLCREATEPORT"

SPLERR APIENTRY SPLDELETEPORT(PSZ pszServer, PSZ pszPortName)
{ (void)pszServer;(void)pszPortName; return PMSPL_ERROR; }
#pragma aux SPLDELETEPORT "SPLDELETEPORT"

SPLERR APIENTRY SPL32QMQUERYPRINTRES(PVOID pData, ULONG cbData)
{ (void)pData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPL32QMQUERYPRINTRES "SPL32QMQUERYPRINTRES"

SPLERR APIENTRY SPLENUMPRINTER(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                  ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLENUMPRINTER "SPLENUMPRINTER"

SPLERR APIENTRY SPLCOPYJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId,
                              PSZ pszDstQueue)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId;(void)pszDstQueue;
  return PMSPL_ERROR; }
#pragma aux SPLCOPYJOB "SPLCOPYJOB"

SPLERR APIENTRY SPLQMSETUP(PVOID pSetupData, ULONG cbData)
{ (void)pSetupData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPLQMSETUP "SPLQMSETUP"

SPLERR APIENTRY SPLQUERYJOBFILE(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId,
                                   PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLQUERYJOBFILE "SPLQUERYJOBFILE"

SPLERR APIENTRY SPLRENAMEQUEUE(PSZ pszServer, PSZ pszOldName, PSZ pszNewName)
{ (void)pszServer;(void)pszOldName;(void)pszNewName; return PMSPL_ERROR; }
#pragma aux SPLRENAMEQUEUE "SPLRENAMEQUEUE"

SPLERR APIENTRY SPLQUERYDRIVER(PSZ pszServer, PSZ pszDriverName, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszDriverName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLQUERYDRIVER "SPLQUERYDRIVER"

SPLERR APIENTRY SPLSETDRIVER(PSZ pszServer, PSZ pszDriverName, ULONG ulLevel,
                                PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszDriverName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLSETDRIVER "SPLSETDRIVER"

SPLERR APIENTRY SPLMOVEJOB(PSZ pszServer, PSZ pszSrcQueue, ULONG ulJobId,
                              PSZ pszDstQueue)
{ (void)pszServer;(void)pszSrcQueue;(void)ulJobId;(void)pszDstQueue;
  return PMSPL_ERROR; }
#pragma aux SPLMOVEJOB "SPLMOVEJOB"

SPLERR APIENTRY SPLMOVEALLJOBS(PSZ pszServer, PSZ pszSrcQueue, PSZ pszDstQueue)
{ (void)pszServer;(void)pszSrcQueue;(void)pszDstQueue; return PMSPL_ERROR; }
#pragma aux SPLMOVEALLJOBS "SPLMOVEALLJOBS"

/* ================================================================== */
/* SPLTHK* -- Thunk versions of SPL* (ords 480-510)                  */
/* ================================================================== */

SPLERR APIENTRY SPLTHKCONTROLDEVICE(PSZ pszServer, PSZ pszDevName, ULONG ulCtl)
{ (void)pszServer;(void)pszDevName;(void)ulCtl; return PMSPL_ERROR; }
#pragma aux SPLTHKCONTROLDEVICE "SPLTHKCONTROLDEVICE"

SPLERR APIENTRY SPLTHKQUERYDEVICE(PSZ pszServer, PSZ pszDevName, ULONG ulLevel,
                                     PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszDevName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLTHKQUERYDEVICE "SPLTHKQUERYDEVICE"

SPLERR APIENTRY SPLTHKENUMDEVICE(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                    ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLTHKENUMDEVICE "SPLTHKENUMDEVICE"

SPLERR APIENTRY SPLTHKRELEASEJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLTHKRELEASEJOB "SPLTHKRELEASEJOB"

SPLERR APIENTRY SPLTHKHOLDJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLTHKHOLDJOB "SPLTHKHOLDJOB"

SPLERR APIENTRY SPLTHKDELETEJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId; return PMSPL_ERROR; }
#pragma aux SPLTHKDELETEJOB "SPLTHKDELETEJOB"

SPLERR APIENTRY SPLTHKQUERYJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId,
                                  ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                                  PULONG pcbNeeded)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId;(void)ulLevel;
  (void)pBuf;(void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLTHKQUERYJOB "SPLTHKQUERYJOB"

SPLERR APIENTRY SPLTHKSETJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId,
                                ULONG ulLevel, PVOID pBuf, ULONG cbBuf,
                                ULONG ulParmNum)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId;(void)ulLevel;
  (void)pBuf;(void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLTHKSETJOB "SPLTHKSETJOB"

SPLERR APIENTRY SPLTHKENUMJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                                 PVOID pBuf, ULONG cbBuf,
                                 PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLTHKENUMJOB "SPLTHKENUMJOB"

SPLERR APIENTRY SPLTHKCREATEQUEUE(PSZ pszServer, ULONG ulLevel,
                                     PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux SPLTHKCREATEQUEUE "SPLTHKCREATEQUEUE"

SPLERR APIENTRY SPLTHKHOLDQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLTHKHOLDQUEUE "SPLTHKHOLDQUEUE"

SPLERR APIENTRY SPLTHKRELEASEQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLTHKRELEASEQUEUE "SPLTHKRELEASEQUEUE"

SPLERR APIENTRY SPLTHKDELETEQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLTHKDELETEQUEUE "SPLTHKDELETEQUEUE"

SPLERR APIENTRY SPLTHKQUERYQUEUE(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                                    PVOID pBuf, ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLTHKQUERYQUEUE "SPLTHKQUERYQUEUE"

SPLERR APIENTRY SPLTHKSETQUEUE(PSZ pszServer, PSZ pszQueueName, ULONG ulLevel,
                                  PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszQueueName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLTHKSETQUEUE "SPLTHKSETQUEUE"

SPLERR APIENTRY SPLTHKENUMQUEUE(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                   ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLTHKENUMQUEUE "SPLTHKENUMQUEUE"

SPLERR APIENTRY SPLTHKCREATEDEVICE(PSZ pszServer, ULONG ulLevel,
                                      PVOID pBuf, ULONG cbBuf)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux SPLTHKCREATEDEVICE "SPLTHKCREATEDEVICE"

SPLERR APIENTRY SPLTHKSETDEVICE(PSZ pszServer, PSZ pszDevName, ULONG ulLevel,
                                   PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszDevName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLTHKSETDEVICE "SPLTHKSETDEVICE"

SPLERR APIENTRY SPLTHKDELETEDEVICE(PSZ pszServer, PSZ pszDevName)
{ (void)pszServer;(void)pszDevName; return PMSPL_ERROR; }
#pragma aux SPLTHKDELETEDEVICE "SPLTHKDELETEDEVICE"

SPLERR APIENTRY SPLTHKPURGEQUEUE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLTHKPURGEQUEUE "SPLTHKPURGEQUEUE"

SPLERR APIENTRY SPLTHKENUMDRIVER(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                    ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLTHKENUMDRIVER "SPLTHKENUMDRIVER"

SPLERR APIENTRY SPLTHKENUMQUEUEPROCESSOR(PSZ pszServer, ULONG ulLevel,
                                            PVOID pBuf, ULONG cbBuf,
                                            PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLTHKENUMQUEUEPROCESSOR "SPLTHKENUMQUEUEPROCESSOR"

SPLERR APIENTRY SPLTHKENUMPORT(PSZ pszServer, ULONG ulLevel, PVOID pBuf,
                                  ULONG cbBuf, PULONG pcReturned, PULONG pcTotal)
{ (void)pszServer;(void)ulLevel;(void)pBuf;(void)cbBuf;
  (void)pcReturned;(void)pcTotal; return PMSPL_ERROR; }
#pragma aux SPLTHKENUMPORT "SPLTHKENUMPORT"

SPLERR APIENTRY SPLTHKCOPYJOB(PSZ pszServer, PSZ pszQueueName, ULONG ulJobId,
                                 PSZ pszDstQueue)
{ (void)pszServer;(void)pszQueueName;(void)ulJobId;(void)pszDstQueue;
  return PMSPL_ERROR; }
#pragma aux SPLTHKCOPYJOB "SPLTHKCOPYJOB"

SPLERR APIENTRY SPLTHKQUERYDRIVER(PSZ pszServer, PSZ pszDriverName,
                                     ULONG ulLevel, PVOID pBuf,
                                     ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszServer;(void)pszDriverName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLTHKQUERYDRIVER "SPLTHKQUERYDRIVER"

SPLERR APIENTRY SPLTHKSETDRIVER(PSZ pszServer, PSZ pszDriverName, ULONG ulLevel,
                                   PVOID pBuf, ULONG cbBuf, ULONG ulParmNum)
{ (void)pszServer;(void)pszDriverName;(void)ulLevel;(void)pBuf;
  (void)cbBuf;(void)ulParmNum; return PMSPL_ERROR; }
#pragma aux SPLTHKSETDRIVER "SPLTHKSETDRIVER"

SPLERR APIENTRY SPLTHKMOVEJOB(PSZ pszServer, PSZ pszSrcQueue, ULONG ulJobId,
                                 PSZ pszDstQueue)
{ (void)pszServer;(void)pszSrcQueue;(void)ulJobId;(void)pszDstQueue;
  return PMSPL_ERROR; }
#pragma aux SPLTHKMOVEJOB "SPLTHKMOVEJOB"

SPLERR APIENTRY SPLTHKMOVEALLJOBS(PSZ pszServer, PSZ pszSrcQueue,
                                     PSZ pszDstQueue)
{ (void)pszServer;(void)pszSrcQueue;(void)pszDstQueue; return PMSPL_ERROR; }
#pragma aux SPLTHKMOVEALLJOBS "SPLTHKMOVEALLJOBS"

/* ================================================================== */
/* NETSPL* -- Network spool API (ords 200-206)                        */
/* ================================================================== */

SPLERR APIENTRY NETSPLQMOPEN(PSZ pszComputerName, PSZ pszQueueName,
                                PHQUEUE phQueue)
{ (void)pszComputerName;(void)pszQueueName;(void)phQueue; return PMSPL_ERROR; }
#pragma aux NETSPLQMOPEN "NETSPLQMOPEN"

SPLERR APIENTRY NETSPLQMSTARTDOC(HQUEUE hQueue, PSZ pszDocName, PHFILE phFile)
{ (void)hQueue;(void)pszDocName;(void)phFile; return PMSPL_ERROR; }
#pragma aux NETSPLQMSTARTDOC "NETSPLQMSTARTDOC"

SPLERR APIENTRY NETSPLQMWRITE(HQUEUE hQueue, ULONG cbBuf, PVOID pBuf)
{ (void)hQueue;(void)cbBuf;(void)pBuf; return PMSPL_ERROR; }
#pragma aux NETSPLQMWRITE "NETSPLQMWRITE"

SPLERR APIENTRY NETSPLQMENDDOC(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux NETSPLQMENDDOC "NETSPLQMENDDOC"

SPLERR APIENTRY NETSPLQMCLOSE(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux NETSPLQMCLOSE "NETSPLQMCLOSE"

SPLERR APIENTRY NETSPLQMABORT(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux NETSPLQMABORT "NETSPLQMABORT"

SPLERR APIENTRY NETSPLQMABORTDOC(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux NETSPLQMABORTDOC "NETSPLQMABORTDOC"

/* ================================================================== */
/* PRT/SPL extensions (ords 600-625)                                  */
/* ================================================================== */

SPLERR APIENTRY PRTRESETABORT(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRTRESETABORT "PRTRESETABORT"

SPLERR APIENTRY PRTABORTDOC(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRTABORTDOC "PRTABORTDOC"

SPLERR APIENTRY PRTNEWPAGE(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux PRTNEWPAGE "PRTNEWPAGE"

SPLERR APIENTRY PRTQUERY(ULONG hPrinter, PVOID pBuf, ULONG cbBuf)
{ (void)hPrinter;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux PRTQUERY "PRTQUERY"

SPLERR APIENTRY PRTSET(ULONG hPrinter, PVOID pBuf, ULONG cbBuf)
{ (void)hPrinter;(void)pBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux PRTSET "PRTSET"

SPLERR APIENTRY SPLPROTSENDCMD(PVOID pCmd, ULONG cbCmd)
{ (void)pCmd;(void)cbCmd; return PMSPL_ERROR; }
#pragma aux SPLPROTSENDCMD "SPLPROTSENDCMD"

SPLERR APIENTRY SPLPROTXLATECMD(PVOID pIn, ULONG cbIn,
                                   PVOID pOut, PULONG pcbOut)
{ (void)pIn;(void)cbIn;(void)pOut;(void)pcbOut; return PMSPL_ERROR; }
#pragma aux SPLPROTXLATECMD "SPLPROTXLATECMD"

SPLERR APIENTRY SPLQMNEWPAGE(HQUEUE hQueue)
{ (void)hQueue; return PMSPL_ERROR; }
#pragma aux SPLQMNEWPAGE "SPLQMNEWPAGE"

SPLERR APIENTRY SPLQMGETJOBID(HQUEUE hQueue, PULONG pulJobId)
{ (void)hQueue;(void)pulJobId; return PMSPL_ERROR; }
#pragma aux SPLQMGETJOBID "SPLQMGETJOBID"

SPLERR APIENTRY SPLALERT(ULONG ulAlertType, PSZ pszServer,
                            PSZ pszDevice, PVOID pData)
{ (void)ulAlertType;(void)pszServer;(void)pszDevice;(void)pData;
  return PMSPL_ERROR; }
#pragma aux SPLALERT "SPLALERT"

SPLERR APIENTRY SPLDISABLE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLDISABLE "SPLDISABLE"

SPLERR APIENTRY SPLENABLE(PSZ pszServer, PSZ pszQueueName)
{ (void)pszServer;(void)pszQueueName; return PMSPL_ERROR; }
#pragma aux SPLENABLE "SPLENABLE"

SPLERR APIENTRY SPLREGISTER(PSZ pszServer, PVOID pRegData, ULONG cbData)
{ (void)pszServer;(void)pRegData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPLREGISTER "SPLREGISTER"

SPLERR APIENTRY SPLUNREGISTER(PSZ pszServer, PSZ pszAppName)
{ (void)pszServer;(void)pszAppName; return PMSPL_ERROR; }
#pragma aux SPLUNREGISTER "SPLUNREGISTER"

SPLERR APIENTRY SPLREGISTERCONTROLPANEL(PVOID pCPData, ULONG cbData)
{ (void)pCPData;(void)cbData; return PMSPL_ERROR; }
#pragma aux SPLREGISTERCONTROLPANEL "SPLREGISTERCONTROLPANEL"

SPLERR APIENTRY SPLQUERYCONTROLPANEL(PSZ pszQueueName, PVOID pBuf,
                                        ULONG cbBuf, PULONG pcbNeeded)
{ (void)pszQueueName;(void)pBuf;(void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLQUERYCONTROLPANEL "SPLQUERYCONTROLPANEL"

SPLERR APIENTRY SPLDISPLAYCONTROLPANEL(PSZ pszQueueName, HWND hwndOwner)
{ (void)pszQueueName;(void)hwndOwner; return PMSPL_ERROR; }
#pragma aux SPLDISPLAYCONTROLPANEL "SPLDISPLAYCONTROLPANEL"

SPLERR APIENTRY SPLGETPORTFROMQ(PSZ pszQueueName, PSZ pszPortBuf, ULONG cbBuf)
{ (void)pszQueueName;(void)pszPortBuf;(void)cbBuf; return PMSPL_ERROR; }
#pragma aux SPLGETPORTFROMQ "SPLGETPORTFROMQ"

SPLERR APIENTRY SPLGETCONTROLPANELLIST(PVOID pBuf, ULONG cbBuf,
                                          PULONG pcbNeeded)
{ (void)pBuf;(void)cbBuf;(void)pcbNeeded; return PMSPL_ERROR; }
#pragma aux SPLGETCONTROLPANELLIST "SPLGETCONTROLPANELLIST"

SPLERR APIENTRY SPLPRINTEROPEN(PSZ pszPrinterName, PULONG phPrinter)
{ (void)pszPrinterName;(void)phPrinter; return PMSPL_ERROR; }
#pragma aux SPLPRINTEROPEN "SPLPRINTEROPEN"

SPLERR APIENTRY SPLNOTIFYLOGON(PSZ pszUserName, ULONG ulLogonType)
{ (void)pszUserName;(void)ulLogonType; return PMSPL_ERROR; }
#pragma aux SPLNOTIFYLOGON "SPLNOTIFYLOGON"

SPLERR APIENTRY SPLPRINTERCLOSE(ULONG hPrinter)
{ (void)hPrinter; return PMSPL_ERROR; }
#pragma aux SPLPRINTERCLOSE "SPLPRINTERCLOSE"

SPLERR APIENTRY SPLPRINTERWRITE(ULONG hPrinter, PVOID pBuf, ULONG cbBuf,
                                   PULONG pcbActual)
{ (void)hPrinter;(void)pBuf;(void)cbBuf;(void)pcbActual; return PMSPL_ERROR; }
#pragma aux SPLPRINTERWRITE "SPLPRINTERWRITE"

SPLERR APIENTRY SPLPRINTERPRINTFILE(ULONG hPrinter, PSZ pszFileName)
{ (void)hPrinter;(void)pszFileName; return PMSPL_ERROR; }
#pragma aux SPLPRINTERPRINTFILE "SPLPRINTERPRINTFILE"

SPLERR APIENTRY SPLSYNCHDRIVERS(PSZ pszServer)
{ (void)pszServer; return PMSPL_ERROR; }
#pragma aux SPLSYNCHDRIVERS "SPLSYNCHDRIVERS"
