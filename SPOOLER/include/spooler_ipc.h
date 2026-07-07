/*
 * spooler_ipc.h - Internal IPC definitions shared between PMSPL.DLL,
 *                 SPL1B.DLL, SPOOLCP.DLL and SPOOL.EXE
 *
 * Architecture:
 *   SPOOL.EXE  - daemon process, owns the queue database and job files
 *   PMSPL.DLL  - client API; communicates with SPOOL.EXE via named pipe
 *   SPL1B.DLL  - backend library loaded by SPOOL.EXE for job scheduling
 *   SPOOLCP.DLL- PM control-panel dialog loaded by the WPS printer object
 *
 * Named pipe used for IPC:
 *   \PIPE\SPOOLER  (instance per request, NMPWAIT_WAIT_FOREVER)
 *
 * All strings are null-terminated, max CCHMAXPATH or the constant below.
 */

#ifndef SPOOLER_IPC_H
#define SPOOLER_IPC_H

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS
#define INCL_SPLFSE
#include <os2.h>
#include <pmspl.h>

/* ------------------------------------------------------------------ */
/* Named pipe and spool-file paths                                     */
/* ------------------------------------------------------------------ */
#define SPL_PIPE_NAME       "\\PIPE\\SPOOLER"
#define SPL_SPOOL_DIR       "\\SPOOL\\"          /* under boot drive   */
#define SPL_MAX_QUEUES      64
#define SPL_MAX_JOBS        256
#define SPL_MAX_DEVICES     32

/* ------------------------------------------------------------------ */
/* IPC request codes                                                   */
/* ------------------------------------------------------------------ */
#define SPLREQ_QM_OPEN          0x0001
#define SPLREQ_QM_STARTDOC      0x0002
#define SPLREQ_QM_WRITE         0x0003
#define SPLREQ_QM_ENDDOC        0x0004
#define SPLREQ_QM_CLOSE         0x0005
#define SPLREQ_QM_ABORT         0x0006
#define SPLREQ_QM_ABORTDOC      0x0007
#define SPLREQ_QM_NEWPAGE       0x0008
#define SPLREQ_QM_GETJOBID      0x0009

#define SPLREQ_CREATE_QUEUE     0x0101
#define SPLREQ_DELETE_QUEUE     0x0102
#define SPLREQ_HOLD_QUEUE       0x0103
#define SPLREQ_RELEASE_QUEUE    0x0104
#define SPLREQ_PURGE_QUEUE      0x0105
#define SPLREQ_QUERY_QUEUE      0x0106
#define SPLREQ_SET_QUEUE        0x0107
#define SPLREQ_ENUM_QUEUE       0x0108

#define SPLREQ_DELETE_JOB       0x0201
#define SPLREQ_HOLD_JOB         0x0202
#define SPLREQ_RELEASE_JOB      0x0203
#define SPLREQ_QUERY_JOB        0x0204
#define SPLREQ_SET_JOB          0x0205
#define SPLREQ_ENUM_JOB         0x0206
#define SPLREQ_COPY_JOB         0x0207

#define SPLREQ_CREATE_DEVICE    0x0301
#define SPLREQ_DELETE_DEVICE    0x0302
#define SPLREQ_CONTROL_DEVICE   0x0303
#define SPLREQ_QUERY_DEVICE     0x0304
#define SPLREQ_SET_DEVICE       0x0305
#define SPLREQ_ENUM_DEVICE      0x0306

#define SPLREQ_ENUM_DRIVER      0x0401
#define SPLREQ_QUERY_DRIVER     0x0402
#define SPLREQ_SET_DRIVER       0x0403

#define SPLREQ_ENUM_QP          0x0501

#define SPLREQ_ENUM_PRINTER     0x0601

#define SPLREQ_CREATE_PORT      0x0701
#define SPLREQ_DELETE_PORT      0x0702
#define SPLREQ_ENUM_PORT        0x0703
#define SPLREQ_QUERY_PORT       0x0704
#define SPLREQ_SET_PORT         0x0705

/* ------------------------------------------------------------------ */
/* IPC message header                                                  */
/* ------------------------------------------------------------------ */
#pragma pack(1)

typedef struct _SPLMSGHDR {
    USHORT  usCode;         /* SPLREQ_* code           */
    USHORT  cbPayload;      /* bytes following header  */
    ULONG   ulReserved;
} SPLMSGHDR;
typedef SPLMSGHDR *PSPLMSGHDR;

/* Generic reply header */
typedef struct _SPLREPLYHDR {
    SPLERR  rc;             /* 0 = success             */
    USHORT  cbData;         /* bytes of data following */
} SPLREPLYHDR;
typedef SPLREPLYHDR *PSPLREPLYHDR;

/* SplQmOpen request payload */
typedef struct _SPLREQ_QMOPEN {
    CHAR    szToken[CCHMAXPATH];
    LONG    lCount;
    /* pqmdopData strings follow as null-terminated array */
} SPLREQ_QMOPEN;

/* SplQmStartDoc payload */
typedef struct _SPLREQ_STARTDOC {
    ULONG   ulHandle;
    CHAR    szDocName[CCHMAXPATH];
} SPLREQ_STARTDOC;

/* SplQmWrite payload header (actual data bytes follow) */
typedef struct _SPLREQ_WRITE {
    ULONG   ulHandle;
    LONG    lCount;
} SPLREQ_WRITE;

/* Single-handle request (Close, Abort, AbortDoc, EndDoc) */
typedef struct _SPLREQ_HANDLE {
    ULONG   ulHandle;
} SPLREQ_HANDLE;

/* SplQmNewPage */
typedef struct _SPLREQ_NEWPAGE {
    ULONG   ulHandle;
    ULONG   ulPageNumber;
} SPLREQ_NEWPAGE;

/* SplQmGetJobID reply data */
typedef struct _SPLREPLY_JOBID {
    ULONG   ulJobID;
    CHAR    szComputerName[CNLEN+1];
    CHAR    szQueueName[QNLEN+1];
} SPLREPLY_JOBID;

/* Named device/port request: level + name, optional parmnum */
typedef struct _SPLREQ_NAMED {
    ULONG   ulLevel;
    ULONG   ulParmNum;          /* used by Set* calls; 0 for Query/Delete */
    CHAR    szName[PDLEN+1];    /* device or port name */
    /* For Set*: serialised pBuf bytes follow immediately */
} SPLREQ_NAMED;
typedef SPLREQ_NAMED *PSPLREQ_NAMED;

/* Device control request */
typedef struct _SPLREQ_DEVCTL {
    ULONG   ulControl;          /* PRD_PAUSE_DEV / PRD_CONT_DEV etc. */
    CHAR    szPortName[PDLEN+1];
} SPLREQ_DEVCTL;
typedef SPLREQ_DEVCTL *PSPLREQ_DEVCTL;

#pragma pack()

/* ------------------------------------------------------------------ */
/* Internal job record stored by SPL1B.DLL / SPOOL.EXE               */
/* ------------------------------------------------------------------ */
typedef struct _SPLJOB {
    USHORT  uJobId;
    USHORT  uPriority;
    USHORT  fsStatus;
    ULONG   ulSubmitted;
    ULONG   ulSize;
    ULONG   ulPagesSpooled;
    ULONG   ulPagesSent;
    ULONG   ulPagesPrinted;
    ULONG   ulTimePrinted;
    ULONG   ulStartPage;
    ULONG   ulEndPage;
    CHAR    szUserName[UNLEN+1];
    CHAR    szQueueName[QNLEN+1];
    CHAR    szDocName[CCHMAXPATH];
    CHAR    szDataType[DTLEN+1];
    CHAR    szDriverName[DRIV_NAME_SIZE+1+DRIV_DEVICENAME_SIZE+1];
    CHAR    szSpoolFile[CCHMAXPATH];
    CHAR    szComment[MAXCOMMENTSZ+1];
    CHAR    szProcParams[CCHMAXPATH];
    CHAR    szPortName[PDLEN+1];
} SPLJOB;
typedef SPLJOB *PSPLJOB;

/* ------------------------------------------------------------------ */
/* Internal queue record                                               */
/* ------------------------------------------------------------------ */
typedef struct _SPLQUEUE {
    CHAR    szName[QNLEN+1];
    USHORT  uPriority;
    USHORT  uStartTime;
    USHORT  uUntilTime;
    USHORT  fsType;
    USHORT  fsStatus;
    USHORT  cJobs;
    CHAR    szPrinters[PRINTERNAME_SIZE+1];
    CHAR    szDriverName[DRIV_NAME_SIZE+1+DRIV_DEVICENAME_SIZE+1];
    CHAR    szPrProc[QNLEN+1];
    CHAR    szParms[CCHMAXPATH];
    CHAR    szComment[MAXCOMMENTSZ+1];
    CHAR    szSepFile[CCHMAXPATH];
} SPLQUEUE;
typedef SPLQUEUE *PSPLQUEUE;

/* ------------------------------------------------------------------ */
/* Internal device record                                              */
/* ------------------------------------------------------------------ */
typedef struct _SPLDEVICE {
    CHAR    szName[PDLEN+1];
    CHAR    szLogAddr[CCHMAXPATH];
    CHAR    szComment[MAXCOMMENTSZ+1];
    CHAR    szDrivers[DRIV_NAME_SIZE+1+DRIV_DEVICENAME_SIZE+1];
    USHORT  uJobId;
    USHORT  fsStatus;
    USHORT  usTimeOut;
} SPLDEVICE;
typedef SPLDEVICE *PSPLDEVICE;

/* ------------------------------------------------------------------ */
/* Helper: open the spooler pipe (used by PMSPL.DLL client side)      */
/* ------------------------------------------------------------------ */
HPIPE SplOpenPipe(VOID);
VOID  SplClosePipe(HPIPE hPipe);
SPLERR SplTransact(USHORT usCode, PVOID pReq, USHORT cbReq,
                   PVOID pReply, USHORT cbReply, PUSHORT pcbActual);

#endif /* SPOOLER_IPC_H */
