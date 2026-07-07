/*
 * LIBUNI.C -- cWarp clone of OS/2 LIBUNI.DLL (Unicode / ULS functions).
 *
 * Original: IBM 14.093 "Unicode Functions" (78 exports, imports uconv+DOSCALLS).
 * Implements the PURE UCS-2 string functions for real (per Toolkit unidef.h) and
 * stubs the character-attribute / locale / conversion functions (which need Unicode
 * data tables), keeping full ORDINAL PARITY so it is a loadable drop-in.
 * CALLCONV = _System (caller-cleanup) -> zero-arg stubs are stack-safe.
 * Build: wcc386 + wlink (no forwarders -> no LINK386 needed).
 */

typedef unsigned short UniChar;

/* ULS return code (ulserrno.h): ULS_API_ERROR(12) = 0x00020400|12 */
#define ULS_NOTIMPLEMENTED 0x0002040CL

unsigned long _System _DLL_InitTerm(unsigned long hmod,unsigned long flag);
#pragma aux _DLL_InitTerm "_DLL_InitTerm";
unsigned long _System _DLL_InitTerm(unsigned long hmod,unsigned long flag)
{ (void)hmod;(void)flag; return 1UL; }

static int uni_in(const UniChar *set,UniChar c)
{ while(*set){ if(*set==c) return 1; set++; } return 0; }

/* ================= real UCS-2 string functions ================= */
unsigned long _System UniStrlen(const UniChar *s)
{ const UniChar *p=s; while(*p) p++; return (unsigned long)(p-s); }
#pragma aux UniStrlen "UniStrlen";

UniChar * _System UniStrcpy(UniChar *d,const UniChar *s)
{ UniChar *r=d; while((*d++=*s++)!=0){;} return r; }
#pragma aux UniStrcpy "UniStrcpy";

UniChar * _System UniStrncpy(UniChar *d,const UniChar *s,unsigned long n)
{ UniChar *r=d; while(n&&(*d=*s)){d++;s++;n--;} while(n){*d++=0;n--;} return r; }
#pragma aux UniStrncpy "UniStrncpy";

UniChar * _System UniStrcat(UniChar *d,const UniChar *s)
{ UniChar *r=d; while(*d)d++; while((*d++=*s++)!=0){;} return r; }
#pragma aux UniStrcat "UniStrcat";

UniChar * _System UniStrncat(UniChar *d,const UniChar *s,unsigned long n)
{ UniChar *r=d; while(*d)d++; while(n&&*s){*d++=*s++;n--;} *d=0; return r; }
#pragma aux UniStrncat "UniStrncat";

int _System UniStrcmp(const UniChar *a,const UniChar *b)
{ while(*a&&*a==*b){a++;b++;} return (int)*a-(int)*b; }
#pragma aux UniStrcmp "UniStrcmp";

int _System UniStrncmp(const UniChar *a,const UniChar *b,unsigned long n)
{ while(n&&*a&&*a==*b){a++;b++;n--;} if(!n)return 0; return (int)*a-(int)*b; }
#pragma aux UniStrncmp "UniStrncmp";

UniChar * _System UniStrchr(const UniChar *s,UniChar c)
{ for(;;s++){ if(*s==c)return (UniChar*)s; if(!*s)return (UniChar*)0; } }
#pragma aux UniStrchr "UniStrchr";

UniChar * _System UniStrrchr(const UniChar *s,UniChar c)
{ const UniChar *last=(const UniChar*)0; for(;;s++){ if(*s==c)last=s; if(!*s)break; } return (UniChar*)last; }
#pragma aux UniStrrchr "UniStrrchr";

unsigned long _System UniStrspn(const UniChar *s,const UniChar *set)
{ const UniChar *p=s; while(*p&&uni_in(set,*p))p++; return (unsigned long)(p-s); }
#pragma aux UniStrspn "UniStrspn";

unsigned long _System UniStrcspn(const UniChar *s,const UniChar *set)
{ const UniChar *p=s; while(*p&&!uni_in(set,*p))p++; return (unsigned long)(p-s); }
#pragma aux UniStrcspn "UniStrcspn";

UniChar * _System UniStrpbrk(const UniChar *s,const UniChar *set)
{ for(;*s;s++) if(uni_in(set,*s)) return (UniChar*)s; return (UniChar*)0; }
#pragma aux UniStrpbrk "UniStrpbrk";

UniChar * _System UniStrstr(const UniChar *hay,const UniChar *nee)
{ if(!*nee)return (UniChar*)hay; for(;*hay;hay++){ const UniChar *h=hay; const UniChar *n=nee; while(*h&&*n&&*h==*n){h++;n++;} if(!*n)return (UniChar*)hay; } return (UniChar*)0; }
#pragma aux UniStrstr "UniStrstr";

UniChar * _System UniStrtok(UniChar *s,const UniChar *dl)
{ static UniChar *save=(UniChar*)0; UniChar *tok; if(!s)s=save; if(!s)return (UniChar*)0; while(*s&&uni_in(dl,*s))s++; if(!*s){save=(UniChar*)0;return (UniChar*)0;} tok=s; while(*s&&!uni_in(dl,*s))s++; if(*s){*s=0;save=s+1;}else save=(UniChar*)0; return tok; }
#pragma aux UniStrtok "UniStrtok";

UniChar _System UniToupper(UniChar c){ return (UniChar)((c>=0x61&&c<=0x7A)?(c-0x20):c); }
#pragma aux UniToupper "UniToupper";

UniChar _System UniTolower(UniChar c){ return (UniChar)((c>=0x41&&c<=0x5A)?(c+0x20):c); }
#pragma aux UniTolower "UniTolower";

UniChar * _System UniStrupr(UniChar *s){ UniChar *r=s; for(;*s;s++) if(*s>=0x61&&*s<=0x7A)*s=(UniChar)(*s-0x20); return r; }
#pragma aux UniStrupr "UniStrupr";

UniChar * _System UniStrlwr(UniChar *s){ UniChar *r=s; for(;*s;s++) if(*s>=0x41&&*s<=0x5A)*s=(UniChar)(*s+0x20); return r; }
#pragma aux UniStrlwr "UniStrlwr";

int _System UniQueryAlnum(const void *loc,UniChar c){ (void)loc; return (c>=0x30&&c<=0x39)||(c>=0x41&&c<=0x5A)||(c>=0x61&&c<=0x7A); }
#pragma aux UniQueryAlnum "UniQueryAlnum";

int _System UniQueryAlpha(const void *loc,UniChar c){ (void)loc; return (c>=0x41&&c<=0x5A)||(c>=0x61&&c<=0x7A); }
#pragma aux UniQueryAlpha "UniQueryAlpha";

int _System UniQueryBlank(const void *loc,UniChar c){ (void)loc; return c==0x20||c==0x09; }
#pragma aux UniQueryBlank "UniQueryBlank";

int _System UniQueryCntrl(const void *loc,UniChar c){ (void)loc; return c<0x20||c==0x7F; }
#pragma aux UniQueryCntrl "UniQueryCntrl";

int _System UniQueryDigit(const void *loc,UniChar c){ (void)loc; return c>=0x30&&c<=0x39; }
#pragma aux UniQueryDigit "UniQueryDigit";

int _System UniQueryGraph(const void *loc,UniChar c){ (void)loc; return c>0x20&&c<0x7F; }
#pragma aux UniQueryGraph "UniQueryGraph";

int _System UniQueryLower(const void *loc,UniChar c){ (void)loc; return c>=0x61&&c<=0x7A; }
#pragma aux UniQueryLower "UniQueryLower";

int _System UniQueryPrint(const void *loc,UniChar c){ (void)loc; return c>=0x20&&c<0x7F; }
#pragma aux UniQueryPrint "UniQueryPrint";

int _System UniQueryPunct(const void *loc,UniChar c){ (void)loc; return (c>0x20&&c<0x7F)&&!((c>=0x30&&c<=0x39)||(c>=0x41&&c<=0x5A)||(c>=0x61&&c<=0x7A)); }
#pragma aux UniQueryPunct "UniQueryPunct";

int _System UniQuerySpace(const void *loc,UniChar c){ (void)loc; return c==0x20||(c>=0x09&&c<=0x0D); }
#pragma aux UniQuerySpace "UniQuerySpace";

int _System UniQueryUpper(const void *loc,UniChar c){ (void)loc; return c>=0x41&&c<=0x5A; }
#pragma aux UniQueryUpper "UniQueryUpper";

int _System UniQueryXdigit(const void *loc,UniChar c){ (void)loc; return (c>=0x30&&c<=0x39)||(c>=0x41&&c<=0x46)||(c>=0x61&&c<=0x66); }
#pragma aux UniQueryXdigit "UniQueryXdigit";

/* ================= ordinal-parity stubs ================= */
int _System UniFreeAttrObject(void){ return 0; }
#pragma aux UniFreeAttrObject "UniFreeAttrObject";
int _System UniQueryCharAttr(void){ return 0; }
#pragma aux UniQueryCharAttr "UniQueryCharAttr";
int _System UniScanForAttr(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniScanForAttr "UniScanForAttr";
int _System UniCreateAttrObject(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniCreateAttrObject "UniCreateAttrObject";
int _System UniCreateTransformObject(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniCreateTransformObject "UniCreateTransformObject";
int _System UniFreeTransformObject(void){ return 0; }
#pragma aux UniFreeTransformObject "UniFreeTransformObject";
int _System UniQueryLocaleObject(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniQueryLocaleObject "UniQueryLocaleObject";
int _System UniCreateLocaleObject(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniCreateLocaleObject "UniCreateLocaleObject";
int _System UniFreeLocaleObject(void){ return 0; }
#pragma aux UniFreeLocaleObject "UniFreeLocaleObject";
int _System UniFreeMem(void){ return 0; }
#pragma aux UniFreeMem "UniFreeMem";
int _System UniFreeLocaleInfo(void){ return 0; }
#pragma aux UniFreeLocaleInfo "UniFreeLocaleInfo";
int _System UniQueryLocaleInfo(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniQueryLocaleInfo "UniQueryLocaleInfo";
int _System UniQueryLocaleItem(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniQueryLocaleItem "UniQueryLocaleItem";
int _System UniStrcmpi(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniStrcmpi "UniStrcmpi";
int _System UniStrcoll(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniStrcoll "UniStrcoll";
int _System UniStrfmon(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniStrfmon "UniStrfmon";
int _System UniStrftime(void){ return 0; }
#pragma aux UniStrftime "UniStrftime";
int _System UniStrncmpi(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniStrncmpi "UniStrncmpi";
int _System UniStrptime(void){ return 0; }
#pragma aux UniStrptime "UniStrptime";
int _System UniStrtod(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniStrtod "UniStrtod";
int _System UniStrtol(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniStrtol "UniStrtol";
int _System UniStrtoul(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniStrtoul "UniStrtoul";
int _System UniStrxfrm(void){ return 0; }
#pragma aux UniStrxfrm "UniStrxfrm";
int _System UniLocaleStrToToken(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniLocaleStrToToken "UniLocaleStrToToken";
int _System UniLocaleTokenToStr(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniLocaleTokenToStr "UniLocaleTokenToStr";
int _System UniTransformStr(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniTransformStr "UniTransformStr";
int _System UniTransLower(void){ return 0; }
#pragma aux UniTransLower "UniTransLower";
int _System UniTransUpper(void){ return 0; }
#pragma aux UniTransUpper "UniTransUpper";
int _System UniMapCtryToLocale(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniMapCtryToLocale "UniMapCtryToLocale";
int _System UniMakeKey(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniMakeKey "UniMakeKey";
int _System UniQueryChar(void){ return 0; }
#pragma aux UniQueryChar "UniQueryChar";
int _System UniGetOverride(void){ return 0; }
#pragma aux UniGetOverride "UniGetOverride";
int _System UniGetColval(void){ return 0; }
#pragma aux UniGetColval "UniGetColval";
int _System UniQueryAttr(void){ return 0; }
#pragma aux UniQueryAttr "UniQueryAttr";
int _System UniQueryStringType(void){ return 0; }
#pragma aux UniQueryStringType "UniQueryStringType";
int _System UniQueryCharType(void){ return 0; }
#pragma aux UniQueryCharType "UniQueryCharType";
int _System UniQueryNumericValue(void){ return 0; }
#pragma aux UniQueryNumericValue "UniQueryNumericValue";
int _System UniQueryCharTypeTable(void){ return 0; }
#pragma aux UniQueryCharTypeTable "UniQueryCharTypeTable";
int _System UniProcessUconv(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniProcessUconv "UniProcessUconv";
int _System Locale(void){ return 0; }
#pragma aux Locale "Locale";
int _System UniMakeUserLocale(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniMakeUserLocale "UniMakeUserLocale";
int _System UniSetUserLocaleItem(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniSetUserLocaleItem "UniSetUserLocaleItem";
int _System UniDeleteUserLocale(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniDeleteUserLocale "UniDeleteUserLocale";
int _System UniCompleteUserLocale(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniCompleteUserLocale "UniCompleteUserLocale";
int _System UniQueryLocaleValue(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniQueryLocaleValue "UniQueryLocaleValue";
int _System UniQueryLocaleList(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniQueryLocaleList "UniQueryLocaleList";
int _System UniQueryLanguageName(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniQueryLanguageName "UniQueryLanguageName";
int _System UniQueryCountryName(void){ return (int)ULS_NOTIMPLEMENTED; }
#pragma aux UniQueryCountryName "UniQueryCountryName";
