/*
 * DDNSRES.C -- OS/2 TCP/IP Dynamic DNS DLL stub
 *
 * Replacement for ddnsres.dll (LX, 289606 bytes, IBM TCP/IP 4.3)
 * Module description: 'TCP/IP for OS/2: DYNAMIC DNS DLL'
 * Module name: 'DDNSRES'
 *
 * Build (32-bit):
 *   wcc386 -bt=OS2 -bm -wx -s -fo=DDNSRES.OBJ DDNSRES.C
 *   wlink @DDNSRES.LNK
 *
 * ============================================================
 * Binary Analysis Summary
 * ============================================================
 * Format:  LX, 4 objects
 *   Obj1: 32-bit CODE  192KB  -- DNS resolver + DDNS update engine
 *   Obj2: 32-bit RO SHR 2KB  -- shared data/strings
 *   Obj3: 32-bit RW    0KB   -- uninitialised data
 *   Obj4: 32-bit RW   77KB   -- _res structure + DNS state
 *
 * Imports: TCPIP32, DOSCALLS, NLS, MSG
 *
 * Exports (57 functions) in groups:
 *   BSD resolver (ords 5,11-17,28-29,31-32): inet_addr, gethostbyname,
 *     gethostbyaddr, getnetbyname/addr, setnetent, endnetent, getnetent,
 *     sethostent, endhostent, dn_expand, dn_comp
 *   Internal resolver (ords 35-43): _getshort, _getlong, __putshort,
 *     __putlong, res_init, res_mkquery, res_send, _res (data)
 *   Host lookup (ords 46-47): _gethtbyname, _gethtbyaddr
 *   Resolver API (ords 52-54): res_search, res_query, res_querydomain
 *   Dynamic DNS (ords 55-65,80-81,87,99): DDNSGetRRs, DDNSInitUpdate,
 *     DDNSUpdate_A/CNAME/PTR/KEY/HINFO/MX/TXT, DDNSSignUpdate,
 *     DDNSFinalizeUpdate, DDNSSendUpdate, DDNSFreeUpdate, DDNSDumpUpdate
 *   Utility (ords 66-79,82-84,86): __hostalias, _res_opcodes,
 *     __dn_skipname, inet_nsap_addr/ntoa, _res_resultcodes,
 *     __p_time/__p_type/__p_class, herror, __fp_query, __res_nameinquery,
 *     Hexdump, __fp_nquery, res_randomid, inet_aton, __p_fqname
 *   Send update (ord 87): res_send_update
 * ============================================================
 */

#define INCL_DOS
#include <os2.h>

/* Standard socket/DNS types without pulling in socket headers */
typedef unsigned long  in_addr_t;
typedef unsigned short in_port_t;
typedef unsigned long  ULONG32;

struct in_addr { unsigned long s_addr; };
struct hostent {
    char  *h_name;
    char **h_aliases;
    int    h_addrtype;
    int    h_length;
    char **h_addr_list;
};
struct netent {
    char  *n_name;
    char **n_aliases;
    int    n_addrtype;
    unsigned long n_net;
};

/* _res structure placeholder (77KB in original -- we export a small stub) */
static unsigned char _res_data[512] = {0};

/* Error indicator */
#define DNS_ERROR  -1
#define DNS_FALSE   0

/* ================================================================ */
/* BSD resolver functions (ords 5, 11-17, 28-29)                   */
/* ================================================================ */

unsigned long APIENTRY inet_addr(const char *cp)
{ (void)cp; return 0xFFFFFFFF; /* INADDR_NONE */ }
#pragma aux inet_addr "inet_addr"

struct hostent * APIENTRY gethostbyname(const char *name)
{ (void)name; return (struct hostent *)0; }
#pragma aux gethostbyname "gethostbyname"

struct hostent * APIENTRY gethostbyaddr(const char *addr, int len, int type)
{ (void)addr;(void)len;(void)type; return (struct hostent *)0; }
#pragma aux gethostbyaddr "gethostbyaddr"

struct netent * APIENTRY getnetbyname(const char *name)
{ (void)name; return (struct netent *)0; }
#pragma aux getnetbyname "getnetbyname"

struct netent * APIENTRY getnetbyaddr(long net, int type)
{ (void)net;(void)type; return (struct netent *)0; }
#pragma aux getnetbyaddr "getnetbyaddr"

void APIENTRY setnetent(int stayopen)
{ (void)stayopen; }
#pragma aux setnetent "setnetent"

void APIENTRY endnetent(void)
{ }
#pragma aux endnetent "endnetent"

struct netent * APIENTRY getnetent(void)
{ return (struct netent *)0; }
#pragma aux getnetent "getnetent"

void APIENTRY sethostent(int stayopen)
{ (void)stayopen; }
#pragma aux sethostent "sethostent"

void APIENTRY endhostent(void)
{ }
#pragma aux endhostent "endhostent"

/* ================================================================ */
/* DNS compression/expansion (ords 31-32)                          */
/* ================================================================ */

int APIENTRY dn_expand(const unsigned char *msg, const unsigned char *eom,
                        const unsigned char *comp_dn, char *exp_dn, int length)
{ (void)msg;(void)eom;(void)comp_dn;(void)exp_dn;(void)length;
  return DNS_ERROR; }
#pragma aux dn_expand "dn_expand"

int APIENTRY dn_comp(const char *exp_dn, unsigned char *comp_dn, int length,
                      unsigned char **dnptrs, unsigned char **lastdnptr)
{ (void)exp_dn;(void)comp_dn;(void)length;(void)dnptrs;(void)lastdnptr;
  return DNS_ERROR; }
#pragma aux dn_comp "dn_comp"

/* ================================================================ */
/* Internal resolver wire-format helpers (ords 35-38)              */
/* ================================================================ */

unsigned short APIENTRY _getshort(const unsigned char *msgp)
{ (void)msgp; return 0; }
#pragma aux _getshort "_getshort"

unsigned long APIENTRY _getlong(const unsigned char *msgp)
{ (void)msgp; return 0; }
#pragma aux _getlong "_getlong"

void APIENTRY __putshort(unsigned short s, unsigned char *msgp)
{ (void)s;(void)msgp; }
#pragma aux __putshort "__putshort"

void APIENTRY __putlong(unsigned long l, unsigned char *msgp)
{ (void)l;(void)msgp; }
#pragma aux __putlong "__putlong"

/* ================================================================ */
/* Resolver API (ords 39-41, 52-54)                                */
/* ================================================================ */

int APIENTRY res_init(void)
{ return DNS_ERROR; }
#pragma aux res_init "res_init"

int APIENTRY res_mkquery(int op, const char *dname, int class_, int type,
                           const unsigned char *data, int datalen,
                           const unsigned char *newrr, unsigned char *buf,
                           int buflen)
{ (void)op;(void)dname;(void)class_;(void)type;(void)data;(void)datalen;
  (void)newrr;(void)buf;(void)buflen; return DNS_ERROR; }
#pragma aux res_mkquery "res_mkquery"

int APIENTRY res_send(const unsigned char *msg, int msglen,
                       unsigned char *answer, int anslen)
{ (void)msg;(void)msglen;(void)answer;(void)anslen; return DNS_ERROR; }
#pragma aux res_send "res_send"

int APIENTRY res_search(const char *dname, int class_, int type,
                          unsigned char *answer, int anslen)
{ (void)dname;(void)class_;(void)type;(void)answer;(void)anslen;
  return DNS_ERROR; }
#pragma aux res_search "res_search"

int APIENTRY res_query(const char *dname, int class_, int type,
                         unsigned char *answer, int anslen)
{ (void)dname;(void)class_;(void)type;(void)answer;(void)anslen;
  return DNS_ERROR; }
#pragma aux res_query "res_query"

int APIENTRY res_querydomain(const char *name, const char *domain,
                               int class_, int type, unsigned char *answer,
                               int anslen)
{ (void)name;(void)domain;(void)class_;(void)type;(void)answer;(void)anslen;
  return DNS_ERROR; }
#pragma aux res_querydomain "res_querydomain"

/* ================================================================ */
/* Host table lookup (ords 46-47)                                  */
/* ================================================================ */

struct hostent * APIENTRY _gethtbyname(const char *name)
{ (void)name; return (struct hostent *)0; }
#pragma aux _gethtbyname "_gethtbyname"

struct hostent * APIENTRY _gethtbyaddr(const char *addr, int len, int type)
{ (void)addr;(void)len;(void)type; return (struct hostent *)0; }
#pragma aux _gethtbyaddr "_gethtbyaddr"

/* ================================================================ */
/* Dynamic DNS update API (ords 55-65, 80-81, 87, 99)             */
/* ================================================================ */

int APIENTRY DDNSGetRRs(void *pCtx, const char *name, int type,
                          void **ppRR)
{ (void)pCtx;(void)name;(void)type;(void)ppRR; return DNS_ERROR; }
#pragma aux DDNSGetRRs "DDNSGetRRs"

int APIENTRY DDNSInitUpdate(void **ppCtx, const char *server,
                              const char *zone)
{ (void)ppCtx;(void)server;(void)zone; return DNS_ERROR; }
#pragma aux DDNSInitUpdate "DDNSInitUpdate"

int APIENTRY DDNSUpdate_A(void *pCtx, const char *name,
                            unsigned long ttl, struct in_addr addr)
{ (void)pCtx;(void)name;(void)ttl;(void)addr; return DNS_ERROR; }
#pragma aux DDNSUpdate_A "DDNSUpdate_A"

int APIENTRY DDNSUpdate_CNAME(void *pCtx, const char *name,
                                unsigned long ttl, const char *cname)
{ (void)pCtx;(void)name;(void)ttl;(void)cname; return DNS_ERROR; }
#pragma aux DDNSUpdate_CNAME "DDNSUpdate_CNAME"

int APIENTRY DDNSUpdate_PTR(void *pCtx, const char *name,
                              unsigned long ttl, const char *ptr)
{ (void)pCtx;(void)name;(void)ttl;(void)ptr; return DNS_ERROR; }
#pragma aux DDNSUpdate_PTR "DDNSUpdate_PTR"

int APIENTRY DDNSSignUpdate(void *pCtx, const char *keyname,
                              const char *key, unsigned long keylen)
{ (void)pCtx;(void)keyname;(void)key;(void)keylen; return DNS_ERROR; }
#pragma aux DDNSSignUpdate "DDNSSignUpdate"

int APIENTRY DDNSFinalizeUpdate(void *pCtx)
{ (void)pCtx; return DNS_ERROR; }
#pragma aux DDNSFinalizeUpdate "DDNSFinalizeUpdate"

int APIENTRY DDNSSendUpdate(void *pCtx)
{ (void)pCtx; return DNS_ERROR; }
#pragma aux DDNSSendUpdate "DDNSSendUpdate"

void APIENTRY DDNSFreeUpdate(void *pCtx)
{ (void)pCtx; }
#pragma aux DDNSFreeUpdate "DDNSFreeUpdate"

void APIENTRY DDNSDumpUpdate(void *pCtx)
{ (void)pCtx; }
#pragma aux DDNSDumpUpdate "DDNSDumpUpdate"

int APIENTRY DDNSUpdate_KEY(void *pCtx, const char *name,
                              unsigned long ttl, const char *key,
                              unsigned long keylen)
{ (void)pCtx;(void)name;(void)ttl;(void)key;(void)keylen; return DNS_ERROR; }
#pragma aux DDNSUpdate_KEY "DDNSUpdate_KEY"

int APIENTRY DDNSUpdate_HINFO(void *pCtx, const char *name,
                                unsigned long ttl, const char *cpu,
                                const char *os)
{ (void)pCtx;(void)name;(void)ttl;(void)cpu;(void)os; return DNS_ERROR; }
#pragma aux DDNSUpdate_HINFO "DDNSUpdate_HINFO"

int APIENTRY DDNSUpdate_MX(void *pCtx, const char *name, unsigned long ttl,
                             unsigned short pref, const char *exchange)
{ (void)pCtx;(void)name;(void)ttl;(void)pref;(void)exchange;
  return DNS_ERROR; }
#pragma aux DDNSUpdate_MX "DDNSUpdate_MX"

int APIENTRY res_send_update(const unsigned char *msg, int msglen,
                               unsigned char *answer, int anslen)
{ (void)msg;(void)msglen;(void)answer;(void)anslen; return DNS_ERROR; }
#pragma aux res_send_update "res_send_update"

int APIENTRY DDNSUpdate_TXT(void *pCtx, const char *name, unsigned long ttl,
                              const char *txt)
{ (void)pCtx;(void)name;(void)ttl;(void)txt; return DNS_ERROR; }
#pragma aux DDNSUpdate_TXT "DDNSUpdate_TXT"

/* ================================================================ */
/* Utility / internal functions (ords 66-79, 82-84, 86)            */
/* ================================================================ */

char * APIENTRY __hostalias(const char *name)
{ (void)name; return (char *)0; }
#pragma aux __hostalias "__hostalias"

void APIENTRY __dn_skipname(void)
{ }
#pragma aux __dn_skipname "__dn_skipname"

int APIENTRY inet_nsap_addr(const char *ascii, unsigned char *binary,
                              int maxlen)
{ (void)ascii;(void)binary;(void)maxlen; return 0; }
#pragma aux inet_nsap_addr "inet_nsap_addr"

char * APIENTRY inet_nsap_ntoa(int binlen, const unsigned char *binary,
                                 char *ascii)
{ (void)binlen;(void)binary;(void)ascii; return (char *)0; }
#pragma aux inet_nsap_ntoa "inet_nsap_ntoa"

void APIENTRY herror(const char *s)
{ (void)s; }
#pragma aux herror "herror"

void APIENTRY __fp_query(const unsigned char *msg, void *file)
{ (void)msg;(void)file; }
#pragma aux __fp_query "__fp_query"

int APIENTRY __res_nameinquery(const char *name, int type, int class_,
                                  const unsigned char *buf,
                                  const unsigned char *eom)
{ (void)name;(void)type;(void)class_;(void)buf;(void)eom; return 0; }
#pragma aux __res_nameinquery "__res_nameinquery"

void APIENTRY Hexdump(const unsigned char *buf, int len)
{ (void)buf;(void)len; }
#pragma aux Hexdump "Hexdump"

void APIENTRY __fp_nquery(const unsigned char *msg, int len, void *file)
{ (void)msg;(void)len;(void)file; }
#pragma aux __fp_nquery "__fp_nquery"

unsigned short APIENTRY res_randomid(void)
{ return 0; }
#pragma aux res_randomid "res_randomid"

int APIENTRY inet_aton(const char *cp, struct in_addr *addr)
{ (void)cp;(void)addr; return 0; }
#pragma aux inet_aton "inet_aton"

char * APIENTRY __p_fqname(const unsigned char *cp, const unsigned char *msg,
                              void *file)
{ (void)cp;(void)msg;(void)file; return (char *)0; }
#pragma aux __p_fqname "__p_fqname"

/* Data exports -- exported as arrays */
const char * _res_opcodes_data[] = {
    "QUERY","IQUERY","CQUERYM","CQUERYU","4","5","6","7",
    "8","9","10","11","12","13","IQUERY","15", NULL
};
const char **_res_opcodes = _res_opcodes_data;
#pragma aux _res_opcodes "_res_opcodes"

const char * _res_resultcodes_data[] = {
    "NOERROR","FORMERR","SERVFAIL","NXDOMAIN","NOTIMP","REFUSED",
    "6","7","8","9","10","11","12","13","14","NOCHANGE", NULL
};
const char **_res_resultcodes = _res_resultcodes_data;
#pragma aux _res_resultcodes "_res_resultcodes"

/* __p_time, __p_type, __p_class -- return static empty strings */
char * APIENTRY __p_time(unsigned long t)
{ (void)t; return ""; }
#pragma aux __p_time "__p_time"

char * APIENTRY __p_type(int type)
{ (void)type; return ""; }
#pragma aux __p_type "__p_type"

char * APIENTRY __p_class(int class_)
{ (void)class_; return ""; }
#pragma aux __p_class "__p_class"

/* _res -- exported data structure (stub: zeroed) */
unsigned char _res[512] = {0};
#pragma aux _res "_res"
