
// Last updated: 19 Aug 2016, 21.42 Arizona Time (UTC-7)

/*  TODO:
 * - 
 * - 
 * - #if  statements  (will need a preprocessor of some kind?)
 * - 
 * - 
 * - 
 */

#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

//#define DEBUG_ON

#ifdef DEBUG_ON
  #define GIVE_ERROR_LINES   // indicate what NBC.CPP line is giving the error (for use in debugging)
#endif

#define LFs2CRLFs          // Change LFs (\n) to CRLF (\n\r)

// this is called a "variadic" macro when using ellipsis in the macro too.
// the ## eliminates the ending comma if no va_args are used.
#ifdef GIVE_ERROR_LINES
  #define  ERROR(i, b, ...)  error(__LINE__, i, b, ##__VA_ARGS__)
#else
  #define  ERROR(i, b, ...)  error(i, b, ##__VA_ARGS__)
#endif

#include "nbc.h"

// all data
bool verbose = FALSE;
 int warning_level = 1;  // 0 = no warnings, 1 = less severe, 2 = somewhat severe, 3 = most severe, 4 = all warnings
 int warn_cnt = 0;
 int error_cnt = 0;
bool dump_tables = FALSE; // set to TRUE from command line, then dump macro/ident/symbol tables if errors found

// assembler flags
// these flags are used to output certain items to the .asm file.
// different assemblers need different outputs
// this is defaulted to NBASM
bool use_offset = TRUE;
bool use_nbasm_out = TRUE;
bool use_fasm_out = FALSE;
bool use_efi = FALSE;


// prep.c data
 int TokenValueInt = 0;
char TokenIdentName[MAX_IDENT_LEN + 1];
 int TokenIdentNameLen = 0;

char TokenValueString[MAX_STRING_LEN + 1];
 int TokenStringLen = 0;

 int LineNo = 1;
 int LinePos = 1;
char CharQueue[MAX_CHAR_QUEUE_LEN];
 int CharQueueLen = 0;

// Macro Table
struct S_MACRO_TABLE *MacroTablePtr = NULL,    // memory block pointer
                     *MacroTable = NULL;       // current position in memory block

// String Table
struct S_STRING_TABLE *StringTablePtr = NULL,  // memory block pointer
                      *StringTable = NULL;     // current position in memory block

// Ident Table
struct S_IDENT_PREFIX *IdentTablePtr = NULL,   // memory block pointer
                      *LastIdent = NULL;       // pointer to last valid Ident
int SizeOfIdentTable = 0;

// Goto Table
#define MAX_GOTO_LABELS 64
struct S_GOTO_LABELS gotoLabels[MAX_GOTO_LABELS];
int gotoLabCnt = 0;

// Data structures to support #include
char NbcLibPath[MAX_SEARCH_PATH] = "";
 int cur_file = -1;
char SysSearchPaths[MAX_SEARCH_PATH];
 int SysSearchPathsLen = 0;
char SearchPaths[MAX_SEARCH_PATH];
 int SearchPathsLen = 0;
FILE *targ_fp = NULL;

struct S_FILES {
  char filename[MAX_FILE_NAME_LEN + 1];
  FILE *fp;
  char CharQueue[3];
   int LineNo;
   int LinePos;
} files[MAX_INCLUDES + 1];

// Data structures to support #ifdef/#ifndef,#else,#endif
int PrepDontSkipTokens = PREP_STACK_TRUE;
int PrepStack[PREP_STACK_SIZE];
int PrepSp = 0;

// Data structures to support #pragma pack(...)
#define PPACK_STACK_SIZE 16
int PragmaPackValue;
int PragmaPackValues[PPACK_STACK_SIZE];
int PragmaPackSp = 0;
int ExprLevel = 0;

int OutputFormat = FORMATFLAT;
bool GenExterns = TRUE;
int WhatSection = -1;

// Name of the function to call in main()'s prolog to construct C++ objects/init data.
char *MainPrologCtorFxn = NULL;
bool do_start_code = FALSE;
bool is_library_code = FALSE;

// Names of C functions and variables are usually prefixed with an underscore.
// One notable exception is the ELF format used by gcc in Linux.
// Global C identifiers in the ELF format should not be prefixed with an underscore.
bool UseLeadingUnderscores = TRUE;

char *FileHeader = "";
char *FileFooter = "";
char *CodeHeader = "";
char *CodeFooter = "";
char *DataHeader = "";
char *DataFooter = "";

bool CharIsSigned = TRUE;
int  SizeOfWord = 2; // in chars (char can be a multiple of octets); ints and pointers are of word size
int  SizeOfEIP  = 2; // in real mode, ip is only 2 bytes, in pmode, it is 4 bytes
bool allow_32bit = FALSE;

// flag to allow C99 Code
bool allowC99code = TRUE;

// TBD??? implement a function to allocate N labels with overflow checks
int LabelCnt = 1; // label counter for jumps
int StructCpyLabel16 = 0; // label of the function to copy structures/unions (in 16-bit code)
int StructCpyLabel32 = 0; // label of the function to copy structures/unions (in 32-bit code)
int StructPushLabel = 0; // label of the function to push structures/unions onto the stack

// call stack (from higher to lower addresses):
//   param n
//   ...
//   param 1
//   return address
//   saved xbp register
//   local var 1
//   ...
//   local var n
int CurFxnSyntaxPtr = 0;
int CurFxnParamCnt = 0;
int CurFxnParamCntMin = 0;
int CurFxnParamCntMax = 0;
int CurFxnLocalOfs = 0; // negative
int CurFxnMinLocalOfs = 0; // negative

int CurFxnReturnExprTypeSynPtr = 0;
int CurFxnEpilogLabel = 0;

char *CurFxnName = NULL;
int CurFxnNameLabel = 0;
bool isMain;

int ParseLevel = 0; // Parse level/scope (file:0, fxn:1+)
int ParamLevel = 0; // 1+ if parsing params, 0 otherwise
bool NakedFlag = FALSE;  // if set, don't do epolog, etc.

#define SYNTAX_STACK_MAX 65536
struct S_SYNTAX_STACK *SyntaxStack = NULL;
int SyntaxStackCnt = 0;

#define STACK_SIZE 256
struct S_SYNTAX_STACK stack[STACK_SIZE];
int sp = 0;
struct S_SYNTAX_STACK opstack[STACK_SIZE];
int opsp = 0;

// all code
unsigned truncUint(unsigned n) {
  // Truncate n to SizeOfWord * 8 bits
  if (SizeOfWord == 2)
    n &= ~(~0u << 8 << 8);
  else if (SizeOfWord == 4)
    n &= ~(~0u << 8 << 12 << 12);
  return n;
}

int truncInt(int n) {
  // Truncate n to SizeOfWord * 8 bits and then sign-extend it
  unsigned un = n;
  if (SizeOfWord == 2) {
    un &= ~(~0u << 8 << 8);
    un |= (((un >> 8 >> 7) & 1) * ~0u) << 8 << 8;
  } else if (SizeOfWord == 4) {
    un &= ~(~0u << 8 << 12 << 12);
    un |= (((un >> 8 >> 12 >> 11) & 1) * ~0u) << 8 << 12 << 12;
  }
  return (int) un;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macro functions
struct S_MACRO_TABLE *FindMacro(const char *name) {
  struct S_MACRO_TABLE *m = MacroTablePtr;

  while (m < MacroTable) {
    if (!strcmp(MACRO_TABLE_ID(m), name))
      return m;
    m = MACRO_TABLE_NEXT(m);
  }
  
  return NULL;
}

bool UndefineMacro(const char *name) {
  struct S_MACRO_TABLE *m = FindMacro(name);
  
  if (m) {
    // move current position back the length of the one being removed
    MacroTable = (struct S_MACRO_TABLE *) ((bit8u *) MacroTable - m->length);
    // move next and remaining macros to the position of the one being undefined
    memmove(m, MACRO_TABLE_NEXT(m), m->length);
    return TRUE;
  }
  
  return FALSE;
}

void AddMacroIdent(const char *name) {
  int len = strlen(name);
  
  if ((((bit8u *) MacroTable - (bit8u *) MacroTablePtr) + (sizeof(struct S_MACRO_TABLE) + len + 1 + 1)) > MAX_MACRO_TABLE_LEN)
    ERROR(0x0002, FALSE);
  
  MacroTable->length = sizeof(struct S_MACRO_TABLE) + len + 1 + MACRO_DEFAULT_EX_LEN + 1;
  MacroTable->idlen = len + 1;
  MacroTable->exlen = 0;
  strcpy(MACRO_TABLE_ID(MacroTable), name);
  *MACRO_TABLE_EX(MacroTable) = '\0';
  MacroTable = MACRO_TABLE_NEXT(MacroTable);
}

// Append a character (e) to the end of ex[]
void AddMacroExpansionChar(const char *name, const char e) {
  struct S_MACRO_TABLE *m = FindMacro(name);
  
  if (m) {
    // if we have exceeded the default length for the expansion string
    if ((sizeof(struct S_MACRO_TABLE) + m->idlen + m->exlen + 1 + 1) > m->length) {
      if ((((bit8u *) MacroTable - (bit8u *) MacroTablePtr) + MACRO_DEFAULT_EX_LEN + 1) > MAX_MACRO_TABLE_LEN)
        ERROR(0x0002, FALSE);
      memmove((bit8u *) MACRO_TABLE_NEXT(m) + MACRO_DEFAULT_EX_LEN + 1, MACRO_TABLE_NEXT(m), (bit8u *) MacroTable - (bit8u *) MACRO_TABLE_NEXT(m));
      m->length += MACRO_DEFAULT_EX_LEN + 1;
      MacroTable = (struct S_MACRO_TABLE *) ((bit8u *) MacroTable + MACRO_DEFAULT_EX_LEN + 1);
    }
    *(MACRO_TABLE_EX(m) + m->exlen) = e;
    if (e == '\0') {
      // right trim the string
      while (m->exlen && strchr(" \t", *(MACRO_TABLE_EX(m) + m->exlen - 1)))
        m->exlen--;
    } else
      m->exlen++;
    // null terminate the string
    *(MACRO_TABLE_EX(m) + m->exlen) = '\0';
  }    
}

void DefineMacro(const char *name, const char *expansion) {
  AddMacroIdent(name);
  do {
    AddMacroExpansionChar(name, *expansion);
  } while (*expansion++ != '\0');
}

////////////////////////////////////////////////////////////////
//// Dump routines
void DumpMacroTable(void) {
  struct S_MACRO_TABLE *m = MacroTablePtr;
  
  printf_out("; Macro table:\n");
  while (m < MacroTable) {
    if (m->exlen)
      printf_out(";  %s = %s\n", MACRO_TABLE_ID(m), MACRO_TABLE_EX(m));
    else
      printf_out(";  %s\n", MACRO_TABLE_ID(m));
    m = MACRO_TABLE_NEXT(m);
  }
  printf_out("; Bytes used: %i of %i\n\n", (int) ((bit8u *) MacroTable - (bit8u *) MacroTablePtr), MAX_MACRO_TABLE_LEN);
}

void DumpIdentTable(void) {
  struct S_IDENT_PREFIX *p;
  int i, len = 0, c;
  
  printf_out("\n; Identifier table:            slen    A U L (Assigned, Used, Local)\n");
  if (SizeOfIdentTable > 0) {
    // first get the length of the longest identifier
    p = IdentTablePtr;
    c = SizeOfIdentTable;
    do {
      if (p->slen > len)
        len = p->slen;
      c -= (sizeof(struct S_IDENT_PREFIX) + p->slen);
      p = (struct S_IDENT_PREFIX *) ((bit8u *) p + sizeof(struct S_IDENT_PREFIX) + p->slen);
    } while(c > 0);
    
    // be realistic
    if (len > 30) len = 30;
    
    p = IdentTablePtr;
    c = SizeOfIdentTable;
    do {
      printf_out(";  %s", IDENT_STR(p));
      for (i=p->slen - 1; i<len; i++)
        printf_out(" ");
      printf_out(" (% 3i)   %c %c %c\n", p->slen, 
        (p->flags & IDENT_FLAGS_ASSIGNED) ? 'A' : '.',
        (p->flags & IDENT_FLAGS_USED)     ? 'U' : '.',
        (p->flags & IDENT_FLAGS_ISLOCAL)  ? 'L' : '.');
      c -= (sizeof(struct S_IDENT_PREFIX) + p->slen);
      p = (struct S_IDENT_PREFIX *) ((bit8u *) p + sizeof(struct S_IDENT_PREFIX) + p->slen);
    } while(c > 0);
  }
  printf_out("; Bytes used: %i/%i\n\n", SizeOfIdentTable, MAX_IDENT_TABLE_LEN);
}

void DumpSynDecls(void) {
  int i;
  int used = SyntaxStackCnt * sizeof(struct S_SYNTAX_STACK);
  int total = SYNTAX_STACK_MAX * sizeof(struct S_SYNTAX_STACK);
  printf_out("\n; Syntax/declaration table/stack:\n");
  printf_out(";  tok  param  flags\n");
  for (i=0; i<SyntaxStackCnt; i++)
    printf_out("; % 5i % 5i % 5i\n", SyntaxStack[i].tok, SyntaxStack[i].param, SyntaxStack[i].flags);
  printf_out("; Bytes used: %d/%d\n\n", used, total);
}

#define PURGE_MAX_LEN  80
void PurgeStringTable(void) {
  char *p;
  int len, cnt;
  bool quot;
  
  SwitchSection(SECTION_IS_DATA);
  
  struct S_STRING_TABLE *st = StringTablePtr;
  while (st < StringTable) {
    p = (char *) st + sizeof(struct S_STRING_TABLE);
    len = st->len;
    GenNumLabel(st->label);
    
    do {
      GenStartAsciiString((st->flags & STRING_IS_WIDE) > 0);
      quot = FALSE;
      cnt = 0;
      while (len--) {
        // quote ASCII chars for better readability
        if ((*p >= 0x20) && (*p <= 0x7E) && (*p != '\"')) {
          if (!quot) {
            quot = TRUE;
            printf_out("\"");
          }
          printf_out("%c", *p);
        } else {
          if (quot) {
            quot = FALSE;
            printf_out("\",");
          }
          printf_out("%u", *p);
          if (len && (cnt <= PURGE_MAX_LEN))
            printf_out(",");
        }
        p++;
        if (cnt++ > PURGE_MAX_LEN)
          break;
      }
      if (quot)
        printf_out("\"");
      puts_out("");
    } while (len > 0);
    
    st = STRING_TABLE_NEXT(st, st->len);
  }
}

// We add a string to a string table to be able to dump at the end of the compile.
int AddString(const int label, const char *str, const int len, unsigned int flags) {
  // First see if the string is already in the table.  If it is, just return that label
  struct S_STRING_TABLE *st = StringTablePtr;
  while (st < StringTable) {
    if (st->len == len) {
      char *p = (char *) st + sizeof(struct S_STRING_TABLE);
      if (!memcmp(str, p, len))
        return st->label;
    }
    st = STRING_TABLE_NEXT(st, st->len);
  }
  
  // there was no match already in the string table, so add one.
  if ((((bit8u *) StringTable - (bit8u *) StringTablePtr) + (sizeof(struct S_STRING_TABLE) + len)) > MAX_STRING_TABLE_LEN)
    ERROR(0x0003, FALSE);
  
  StringTable->label = label;
  StringTable->len = len;
  StringTable->flags = flags;
  memcpy((bit8u *) StringTable + sizeof(struct S_STRING_TABLE), str, len);
  StringTable = STRING_TABLE_NEXT(StringTable, len);
  
  return label;
}

struct S_STRING_TABLE *FindString(const int label) {
  struct S_STRING_TABLE *st = StringTablePtr;
  
  while (st < StringTable) {
    if (st->label == label)
      return st;
    st = STRING_TABLE_NEXT(st, st->len);
  }
  
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// always start at the top so we get the last declared Ident...
//  LastIdent will point to the last ident in the table
struct S_IDENT_PREFIX *FindIdent(const char *name) {
  struct S_IDENT_PREFIX *p = LastIdent;
  if (SizeOfIdentTable > 0)
    do {
      if (!strcmp(IDENT_STR(p), name))
        return p;
      p = p->prev;
    } while (p);
  
  return NULL;
}

// add an Ident to the list, returning the offset of the ident if
//  it is already in the list, or returning the offset to the newly added.
struct S_IDENT_PREFIX *AddIdent(const char *name, const bit32u ident_flags) {
  struct S_IDENT_PREFIX *p;
  int len;
  
  p = FindIdent(name);
  if (p != NULL)
    return p;
  
  len = strlen(name);
  
  if (len >= MAX_IDENT_NAME_LEN)
    ERROR(0x0100, FALSE, name);
  
  if ((SizeOfIdentTable + sizeof(struct S_IDENT_PREFIX) + len) > MAX_IDENT_TABLE_LEN)
    ERROR(0x0004, FALSE);
  
  p = LastIdent;
  if (LastIdent == NULL)
    LastIdent = IdentTablePtr;
  else
    LastIdent = (struct S_IDENT_PREFIX *) ((bit8u *) LastIdent + sizeof(struct S_IDENT_PREFIX) + LastIdent->slen);
  LastIdent->prev = p;
  LastIdent->slen = len + 1;
  LastIdent->flags = ident_flags;
  LastIdent->line_no = LineNo;
  strcpy(IDENT_STR(LastIdent), name);
  SizeOfIdentTable += sizeof(struct S_IDENT_PREFIX) + len + 1;
  
  return LastIdent;
}

struct S_IDENT_PREFIX *AddNumericIdent(int n) {
  char s[1 + (2 + CHAR_BIT * sizeof n) / 3];
  char *p = s + sizeof(s);
  *--p = '\0';
  p = lab2str(p, n);
  return AddIdent(p, 0);
}

int AddGotoLabel(char *name, int label) {
  int i;
  for (i = 0; i < gotoLabCnt; i++) {
    if (!strcmp(IDENT_STR(gotoLabels[i].ident), name)) {
      if (gotoLabels[i].stat & label)
        ERROR(0x0101, FALSE, name);
      gotoLabels[i].stat |= 2 * !label + label;
      return gotoLabels[i].label_num;
    }
  }
  if (gotoLabCnt >= MAX_GOTO_LABELS)
    ERROR(0x0005, FALSE);
  gotoLabels[gotoLabCnt].ident = AddIdent(name, 0);
  gotoLabels[gotoLabCnt].label_num = LabelCnt++;
  gotoLabels[gotoLabCnt].stat = 2 *! label + label;
  return gotoLabels[gotoLabCnt++].label_num;
}

void UndoNonLabelIdents(struct S_IDENT_PREFIX *ident, const bool doGoTos) {
  char temp[MAX_IDENT_NAME_LEN + 1]; 
  
  // as we go back to the last save place, check each Ident to
  //  see if it was a local and if it was not used.  If it was
  //  not used, give a warning.
  struct S_IDENT_PREFIX *p = LastIdent;
  while (LastIdent != ident) {
    if ((LastIdent->flags & (IDENT_FLAGS_ISLOCAL | IDENT_FLAGS_USED)) == IDENT_FLAGS_ISLOCAL)
      warning(0x0101, LastIdent->line_no, IDENT_STR(LastIdent));
    LastIdent = LastIdent->prev;
  }
  
  if (doGoTos) {
    for (int i = 0; i < gotoLabCnt; i++)
      if ((bit8u *) gotoLabels[i].ident >= (bit8u *) ident) {
        strcpy(temp, IDENT_STR(gotoLabels[i].ident));
        gotoLabels[i].ident = AddIdent(temp, 0);
      }
  }
  
  SizeOfIdentTable = ((bit8u *) LastIdent + sizeof(struct S_IDENT_PREFIX) + LastIdent->slen) - (bit8u *) IdentTablePtr;
}

#define MAX_CASES 128
struct S_CASES Cases[MAX_CASES];
int CasesCnt = 0;

void AddCase(int val, int label) {
  if (CasesCnt >= MAX_CASES)
    ERROR(0x0141, FALSE);
  
  Cases[CasesCnt].Value = val;
  Cases[CasesCnt++].Label = label;
}

char *rws[] = {
  "break", "case", "char", "wchar_t", "continue", "default", "do", "else",
  "extern", "for", "if", "int", "return", "signed", "sizeof",
  "static", "switch", "unsigned", "farC", "farD", "farE", "farF", "farG", 
  "void", "while", "_asm", "auto", "const", "double", "enum", 
  "float", "goto", "_inline", "long", "register", "restrict", 
  "short", "struct", "typedef", "union", "volatile", "_Bool", 
  "_Complex", "_Imaginary", "__interrupt", "_naked"
};

unsigned rwtk[] = {
  tokBreak, tokCase, tokChar, tokWChar, tokCont, tokDefault, tokDo, tokElse,
  tokExtern, tokFor, tokIf, tokInt, tokReturn, tokSigned, tokSizeof,
  tokStatic, tokSwitch, tokUnsigned, tokFarC, tokFarD, tokFarE, tokFarF, tokFarG,
  tokVoid, tokWhile, tok_Asm, tokAuto, tokConst, tokDouble, tokEnum, 
  tokFloat, tokGoto, tokInline, tokLong, tokRegister, tokRestrict, 
  tokShort, tokStruct, tokTypedef, tokUnion, tokVolatile, tok_Bool,
  tok_Complex, tok_Imagin, tokIntr, tokNaked
};

int GetTokenByWord(const char *word) {
  unsigned i;
  
  for (i = 0; i < sizeof(rws) / sizeof(rws[0]); i++)
    if (!strcmp(rws[i], word))
      return rwtk[i];
  
  return tokIdent;
}

unsigned char tktk[] = {
  tokEof,
  // Single-character operators and punctuators:
  '+', '-', '~', '*', '/', '%', '&', '|', '^', '!',
  '<', '>', '(', ')', '[', ']',
  '{', '}', '=', ',', ';', ':', '.', '?',
  // Multi-character operators and punctuators:
  tokLShift, tokLogAnd, tokEQ, tokLEQ, tokInc, tokArrow, tokAssignMul,
  tokAssignMod, tokAssignSub, tokAssignRSh, tokAssignXor,
  tokRShift, tokLogOr, tokNEQ, tokGEQ, tokDec, tokEllipsis,
  tokAssignDiv, tokAssignAdd, tokAssignLSh, tokAssignAnd, tokAssignOr,
  // Some of the above tokens get converted into these in the process:
  tokUnaryAnd, tokUnaryPlus, tokPostInc, tokPostAdd,
  tokULess, tokULEQ, tokURShift, tokUDiv, tokUMod, tokComma,
  tokUnaryStar, tokUnaryMinus, tokPostDec, tokPostSub,
  tokUGreater, tokUGEQ, tokAssignURSh, tokAssignUDiv, tokAssignUMod,
  // Helper (pseudo-)tokens:
  tokNumInt, tokLitStr, tokLitWStr, tokLocalOfs, tokNumUint, tokIdent, tokShortCirc,
  tokSChar, tokShort, tokLong, tokUChar, tokUShort, tokULong,
};

char *tks[] = {
  "<EOF>",
  // Single-character operators and punctuators:
  "+", "-", "~", "*", "/", "%", "&", "|", "^", "!",
  "<", ">", "(", ")", "[", "]",
  "{", "}", "=", ",", ";", ":", ".", "?",
  // Multi-character operators and punctuators:
  "<<", "&&", "==", "<=", "++", "->", "*=",
  "%=", "-=", ">>=", "^=",
  ">>", "||", "!=", ">=", "--", "...",
  "/=", "+=", "<<=", "&=", "|=",
  // Some of the above tokens get converted into these in the process:
  "&u", "+u", "++p", "+=p",
  "<u", "<=u", ">>u", "/u", "%u", ",b",
  "*u", "-u", "--p", "-=p",
  ">u", ">=u", ">>=u", "/=u", "%=u",
  // Helper (pseudo-)tokens:
  "<NumInt>",  "<LitStr>",  "<LitWStr>", "<LocalOfs>", "<NumUint>", "<Ident>", "<ShortCirc>",
  "signed char", "short", "long", "unsigned char", "unsigned short", "unsigned long",
};

char *GetTokenName(int token) {
  unsigned i;
  
  /* +-~* /% &|^! << >> && || < <= > >= == !=  () *[] ++ -- = += -= ~= *= /= %= &= |= ^= <<= >>= {} ,;: -> ... */
  
  // Tokens other than reserved keywords:
  for (i = 0; i < sizeof tktk / sizeof tktk[0]; i++)
    if (tktk[i] == token)
      return tks[i];
  
  // Reserved keywords:
  for (i = 0; i < sizeof rws / sizeof rws[0]; i++)
    if (rwtk[i] == token)
      return rws[i];
  
  ERROR(0xE000, FALSE, token);
  return "";
}

// a temp char to hold the current source line
#define PRINT_LINE  0
#if (PRINT_LINE == TRUE)
  #define MAX_LINE_LEN 1024
  char printline[MAX_LINE_LEN];
  int ppos = 0;
#endif

int GetNextChar(void) {
  int ch = EOF;
  
  if (files[cur_file].fp) {
    if ((ch = fgetc(files[cur_file].fp)) == EOF) {
      fclose(files[cur_file].fp);
      files[cur_file].fp = NULL;
      
      // store the last line/pos, they may still be needed later
      files[cur_file].LineNo = LineNo;
      files[cur_file].LinePos = LinePos;
      
      // don't drop the file record just yet
    }
  }
  
#if (PRINT_LINE == TRUE)
  // print the current source line
  if (ppos < (MAX_LINE_LEN - 1)) {
    if (isprint(ch)) {
      printline[ppos] = (char) ch;
      printline[++ppos] = 0;
    }
  }
  if (ch == '\n') {
    if (ppos > 1)
      printf_out("; %s\n", printline);
    ppos = 0;
  }
#endif
    
  return ch;
}

void ShiftChar(void) {
  if (CharQueueLen)
    memmove(CharQueue, CharQueue + 1, --CharQueueLen);
  
  // make sure there always are at least 3 chars in the queue
  while (CharQueueLen < 3) {
    int ch = GetNextChar();
    if (ch == EOF)
      ch = '\0';
    CharQueue[CharQueueLen++] = ch;
  }
}

void ShiftCharN(int n) {
  while (n-- > 0) {
    ShiftChar();
    LinePos++;
  }
}

void IncludeFile(char quot) {
  int nlen = strlen(TokenValueString);
  
  if (CharQueueLen != 3)
    ERROR(0xE001, FALSE);
  
  if ((cur_file + 1) > MAX_INCLUDES)
    ERROR(0x0020, FALSE, MAX_INCLUDES);
  
  // store the current file's position and buffered chars
  files[cur_file].LineNo = LineNo;
  files[cur_file].LinePos = LinePos;
  memcpy(files[cur_file].CharQueue, CharQueue, CharQueueLen);
  
  if (nlen > MAX_FILE_NAME_LEN)
    ERROR(0x0021, FALSE);
  
  // open the included file
  // First, try opening "file" in the current directory
  // (Open Watcom C/C++ 1.9, Turbo C++ 1.01 use the current directory,
  // unlike gcc, which uses the same directory as the current file)
  cur_file++;
  if (quot == '"') {
    strcpy(files[cur_file].filename, TokenValueString);
    files[cur_file].fp = fopen(files[cur_file].filename, "r");
  } else
    files[cur_file].fp = NULL;
  
  // Next, iterate the search paths trying to open "file" or <file>.
  // "file" is first searched using the list provided by the -I option.
  // "file" is then searched using the list provided by the -SI option.
  // <file> is searched using the list provided by the -SI option.
  if (files[cur_file].fp == NULL) {
    int i;
    char *paths = SearchPaths;
    int pl = SearchPathsLen;
    for (;;) {
      if (quot == '<') {
        paths = SysSearchPaths;
        pl = SysSearchPathsLen;
      }
      for (i = 0; i < pl; ) {
        int plen = strlen(paths + i);
        if (plen + 1 + nlen < MAX_FILE_NAME_LEN) {
          strcpy(files[cur_file].filename, paths + i);
          strcpy(files[cur_file].filename + plen + 1, TokenValueString);
          // Use '/' as a separator, typical for Linux/Unix,
          // but also supported by file APIs in DOS/Windows just as '\\'
          files[cur_file].filename[plen] = '/';
          if ((files[cur_file].fp = fopen(files[cur_file].filename, "r")) != NULL)
            break;
        }
        i += plen + 1;
      }
      if (files[cur_file].fp || quot == '<')
        break;
      quot = '<';
    }
  }
  
  if (files[cur_file].fp != NULL) {
    // reset line/pos and empty the char queue
    CharQueueLen = 0;
    LineNo = LinePos = 1;
    // fill the char queue with file data
    ShiftChar();
  } else {  
    cur_file--; // back up to last one
    ERROR(0x0022, TRUE, TokenValueString);
  }
}

bool EndOfFiles(void) {
  // if there are no including files, we're done
  if (cur_file == 0)
    return TRUE;

  // restore the including file's position and buffered chars
  cur_file--;
  LineNo = files[cur_file].LineNo;
  LinePos = files[cur_file].LinePos;
  CharQueueLen = 3;
  memcpy(CharQueue, files[cur_file].CharQueue, CharQueueLen);
  
  return FALSE;
}

void SkipSpace(const bool SkipNewLines) {
  char *p = CharQueue;
  
  while (*p != '\0') {
    if (strchr(" \t\f\v", *p)) {
      ShiftCharN(1);
      continue;
    }
    
    if (strchr("\r\n", *p)) {
      if (!SkipNewLines)
        return;
      
      if (*p == '\r' && p[1] == '\n')
        ShiftChar();
      
      ShiftChar();
      LineNo++;
      LinePos = 1;
      continue;
    }
    
    if (*p == '/') {
      if (p[1] == '/')  {
        // // comment
        ShiftCharN(2);
        while (!strchr("\r\n", *p))
          ShiftCharN(1);
        continue;
      }
      
      else if (p[1] == '*') {
        // /**/ comment
        ShiftCharN(2);
        while (*p != '\0' && !(*p == '*' && p[1] == '/')) {
          if (strchr("\r\n", *p)) {
            if (!SkipNewLines)
              ERROR(0x0300, FALSE);
            
            if (*p == '\r' && p[1] == '\n')
              ShiftChar();
            
            ShiftChar();
            LineNo++;
            LinePos = 1;
          } else
            ShiftCharN(1);
        }
        if (*p == '\0')
          ERROR(0x0300, FALSE);
        ShiftCharN(2);
        continue;
      }
    } // endof if (*p == '/')
    
    break;
  } // endof while (*p != '\0')
}

void SkipLine(void) {
  char *p = CharQueue;
  
  while (*p != '\0') {
    if (strchr("\r\n", *p)) {
      if (*p == '\r' && p[1] == '\n')
        ShiftChar();

      ShiftChar();
      LineNo++;
      LinePos = 1;
      break;
    } else
      ShiftCharN(1);
  }
}

void GetIdent(bool allow_digit) {
  char *p = CharQueue;
  
//  if (*p != '_' && !isalpha(*p))
  if ((*p != '_') && !isalpha(*p) && (allow_digit && !isdigit(*p)))
    ERROR(0x0102, FALSE);
  
  if ((*p == 'L') && (p[1] == '\'' || p[1] == '"')) {
    ERROR(0x0402, FALSE);
    return;
  }
  
  TokenIdentNameLen = 0;
  TokenIdentName[TokenIdentNameLen++] = *p;
  TokenIdentName[TokenIdentNameLen] = '\0';
  ShiftCharN(1);
  
  while ((*p == '_') || (isalnum(*p))) {
    if (TokenIdentNameLen == MAX_IDENT_LEN)
      ERROR(0x0100, FALSE, TokenIdentName);
    TokenIdentName[TokenIdentNameLen++] = *p;
    TokenIdentName[TokenIdentNameLen] = '\0';
    ShiftCharN(1);
  }
}

void GetString(char terminator, const int option) {
  char *p = CharQueue;
  int ch = '\0';
#ifdef LFs2CRLFs
  int last = '\0';
#endif
  
  TokenStringLen = 0;
  TokenValueString[TokenStringLen] = '\0';
  
  ShiftCharN(1);
  while (!(*p == terminator || strchr("\n\r", *p))) {
    ch = *p;
    if (ch == '\\') {
      ShiftCharN(1);
      ch = *p;
      if (strchr("\n\r", ch))
        break;
      switch (ch) {
        case 'a': ch = '\a'; ShiftCharN(1); break;
        case 'b': ch = '\b'; ShiftCharN(1); break;
        case 'f': ch = '\f'; ShiftCharN(1); break;
        case 'n': ch = '\n'; ShiftCharN(1); break;
        case 'r': ch = '\r'; ShiftCharN(1); break;
        case 't': ch = '\t'; ShiftCharN(1); break;
        case 'v': ch = '\v'; ShiftCharN(1); break;
        // DONE: \nnn, \xnn
        case 'x': {
          // hexadecimal character codes \xN+
          int cnt = 0;
          ch = 0;
          ShiftCharN(1);
          while (*p != '\0' && (isdigit(*p & 0xFFu) || strchr("abcdefABCDEF", *p))) {
            ch = (ch * 16) & 0xFF;
            if (*p >= 'a') ch += *p - 'a' + 10;
            else if (*p >= 'A') ch += *p - 'A' + 10;
            else ch += *p - '0';
            ShiftCharN(1);
            cnt++;
          }
          if (!cnt)
            ERROR(0x0400, FALSE);
          ch -= (ch >= 0x80 && CHAR_MIN < 0) * 0x100;
        } break;
        default:
          if (*p >= '0' && *p <= '7') {
            // octal character codes \N+
            int cnt = 0;
            ch = 0;
            while (*p >= '0' && *p <= '7') {
              ch = (ch * 8) & 0xFF;
              ch += *p - '0';
              ShiftCharN(1);
              // octal escape sequence is terminated after three octal digits
              if (++cnt == 3)
                break;
            }
            ch -= (ch >= 0x80 && CHAR_MIN < 0) * 0x100;
          } else
            ShiftCharN(1);
          break;
      } // endof switch (ch)
    } // endof if (ch == '\\')
    else
      ShiftCharN(1);
    
    switch (option) {
      case '#': // string literal (with file name) for #line and #include
        if (TokenStringLen == MAX_STRING_LEN)
          ERROR(0x0401, FALSE);
#ifdef LFs2CRLFs
        if ((ch == '\n') && (last != '\r'))
          TokenValueString[TokenStringLen++] = '\r';
#endif
        TokenValueString[TokenStringLen++] = ch;
        TokenValueString[TokenStringLen] = '\0';
        break;
      case 'a': // string literal for asm()
        printf_out("%c", ch);
        break;
      default: // skipped string literal (we may still need the size)
        if (TokenStringLen++ == UINT_MAX)
          ERROR(0x0401, FALSE);
        break;
    }
#ifdef LFs2CRLFs
    last = ch;
#endif
  } // endof while (!(*p == '\0' || *p == terminator || strchr("\n\r", *p)))
  
  if (*p != terminator)
    ERROR(0x0400, FALSE);
  
  if (terminator == '\'') {
    if (TokenStringLen == 0)
      ERROR(0x0400, FALSE);
    
// I do not beleive we need these two lines    
//    TokenValueInt = ch & 0xFFu;
//    TokenValueInt -= (CharIsSigned && TokenValueInt >= 0x80) * 0x100;
  }
  
  ShiftCharN(1);
  
  SkipSpace(option != '#');
}

void pushPrep(const int NoSkip) {
  if (PrepSp >= PREP_STACK_SIZE)
    ERROR(0x0310, FALSE);
  PrepStack[PrepSp++] = PrepDontSkipTokens;
  PrepDontSkipTokens = NoSkip;
}

void popPrep(void) {
  if (PrepSp < 1)
    ERROR(0x0311, FALSE);
  PrepDontSkipTokens = PrepStack[--PrepSp];
}

int GetNumber(void) {
  char *p = CharQueue;
  int ch = *p;
  unsigned n = 0;
  int type = 0;
  bool uSuffix = FALSE;
  bool lSuffix = FALSE;
    
  if (ch == '0') {
    // this is either an octal or a hex constant
    type = 'o';
    ShiftCharN(1);
    if (((ch = *p) == 'x') || (ch == 'X')) {
      // this is a hex constant
      int cnt = 0;
      ShiftCharN(1);
      while (((ch = *p) != '\0') && (isdigit(ch & 0xFFu) || strchr("abcdefABCDEF", ch))) {
        if (ch >= 'a') ch -= 'a' - 10;
        else if (ch >= 'A') ch -= 'A' - 10;
        else ch -= '0';
        if ((PrepDontSkipTokens == PREP_STACK_TRUE) && (n * 16 / 16 != n || n * 16 + ch < n * 16))
          ERROR(0x0330, FALSE);
        n = n * 16 + ch;
        ShiftCharN(1);
        cnt++;
      }
      if (!cnt)
        ERROR(0x0331, FALSE);
      type = 'h';
    }
    // this is an octal constant
    else while ((ch = *p) >= '0' && ch <= '7') {
      ch -= '0';
      if ((PrepDontSkipTokens == PREP_STACK_TRUE) && (((n * 8 / 8) != n) || ((n * 8 + ch) < (n * 8))))
        ERROR(0x0330, FALSE);
      n = n * 8 + ch;
      ShiftCharN(1);
    }
  }
  // this is a decimal constant
  else {
    type = 'd';
    while ((ch = *p) >= '0' && ch <= '9') {
      ch -= '0';
      if ((PrepDontSkipTokens == PREP_STACK_TRUE) && (n * 10 / 10 != n || n * 10 + ch < n * 10))
        ERROR(0x0330, FALSE);
      n = (n * 10) + ch;
      ShiftCharN(1);
    }
  }
  
  // possible combinations of suffixes:
  //   none
  //   U
  //   UL
  //   L
  //   LU
  if (((ch = *p) == 'u') || (ch == 'U')) {
    uSuffix = TRUE;
    ShiftCharN(1);
  }
  if (((ch = *p) == 'l') || (ch == 'L')) {
    lSuffix = TRUE;
    ShiftCharN(1);
    if (!uSuffix && (((ch = *p) == 'u') || (ch == 'U'))) {
      uSuffix = TRUE;
      ShiftCharN(1);
    }
  }
  
  if (PrepDontSkipTokens < PREP_STACK_TRUE) {
    // Don't fail on big constants when skipping tokens under #if
    TokenValueInt = 0;
    return tokNumInt;
  }
  
  // This all depends on the type of the operand we are storing the number to
  // Ensure the constant fits into 16/32/64 bits
  if (
      ((SizeOfWord == 2) && (n >> 8 >> 8)) // equiv. to SizeOfWord == 2 && n > 0xFFFF
      || ((SizeOfWord == 2) && lSuffix) // long (which must have at least 32 bits) isn't supported in 16-bit models
      || ((SizeOfWord == 4) && (n >> 8 >> 12 >> 12)) // equiv. to SizeOfWord == 4 && n > 0xFFFFFFFF
     )
    ERROR(0x0332, FALSE, SizeOfWord * 8);
  
  TokenValueInt = (int) n;
  
  // Unsuffixed (with 'u') integer constants (octal, decimal, hex)
  // fitting into 15(31) out of 16(32) bits are signed ints
  if (!uSuffix &&
      (
       ((SizeOfWord == 2) && !(n >> 15)) // equiv. to SizeOfWord == 2 && n <= 0x7FFF
       || ((SizeOfWord == 4) && !(n >> 8 >> 12 >> 11)) // equiv. to SizeOfWord == 4 && n <= 0x7FFFFFFF
      )
     )
    return tokNumInt;
  
  // Unlike octal and hex constants, decimal constants are always
  // a signed type. Error out when a decimal constant doesn't fit
  // into an int since currently there's no next bigger signed type
  // (e.g. long) to use instead of int.
  if (!uSuffix && (type == 'd'))
    ERROR(0x0333, FALSE, SizeOfWord * 8);
  
  return tokNumUint;
}

int GetTokenInner(void) {
  char *p = CharQueue;
  int ch = *p;
  char ismodified = '\0';
  
  // these single-character tokens/operators need no further processing
  if (strchr(",;:()[]{}~?", ch)) {
    ShiftCharN(1);
    return ch;
  }
  
  // parse multi-character tokens/operators
  
  // other assignment operators
  switch (ch) {
    case '+':
      if (p[1] == '+') { ShiftCharN(2); return tokInc; }
      if (p[1] == '=') { ShiftCharN(2); return tokAssignAdd; }
      ShiftCharN(1); return ch;
    case '-':
      if (p[1] == '-') { ShiftCharN(2); return tokDec; }
      if (p[1] == '=') { ShiftCharN(2); return tokAssignSub; }
      if (p[1] == '>') { ShiftCharN(2); return tokArrow; }
      ShiftCharN(1); return ch;
    case '!':
      if (p[1] == '=') { ShiftCharN(2); return tokNEQ; }
      ShiftCharN(1); return ch;
    case '=':
      if (p[1] == '=') { ShiftCharN(2); return tokEQ; }
      ShiftCharN(1); return ch;
    case '<':
      if (p[1] == '=') { ShiftCharN(2); return tokLEQ; }
      if (p[1] == '<') { ShiftCharN(2); if (p[0] != '=') return tokLShift; ShiftCharN(1); return tokAssignLSh; }
      ShiftCharN(1); return ch;
    case '>':
      if (p[1] == '=') { ShiftCharN(2); return tokGEQ; }
      if (p[1] == '>') { ShiftCharN(2); if (p[0] != '=') return tokRShift; ShiftCharN(1); return tokAssignRSh; }
      ShiftCharN(1); return ch;
    case '&':
      if (p[1] == '&') { ShiftCharN(2); return tokLogAnd; }
      if (p[1] == '=') { ShiftCharN(2); return tokAssignAnd; }
      ShiftCharN(1); return ch;
    case '|':
      if (p[1] == '|') { ShiftCharN(2); return tokLogOr; }
      if (p[1] == '=') { ShiftCharN(2); return tokAssignOr; }
      ShiftCharN(1); return ch;
    case '^':
      if (p[1] == '=') { ShiftCharN(2); return tokAssignXor; }
      ShiftCharN(1); return ch;
    case '.':
      if (p[1] == '.' && p[2] == '.') { ShiftCharN(3); return tokEllipsis; }
      ShiftCharN(1); return ch;
    case '*':
      if (p[1] == '=') { ShiftCharN(2); return tokAssignMul; }
      ShiftCharN(1); return ch;
    case '%':
      if (p[1] == '=') { ShiftCharN(2); return tokAssignMod; }
      ShiftCharN(1); return ch;
    case '/':
      if (p[1] == '=') { ShiftCharN(2); return tokAssignDiv; }
      // if (p[1] == '/' || p[1] == '*') { SkipSpace(TRUE); continue; } // already taken care of
      ShiftCharN(1); return ch;
  }
  
  // hex and octal constants
  if (isdigit(ch & 0xFFu))
    return GetNumber();
  
  // wchar_t *s2 =  L"hello"; // const wchar_t*
  //   or
  // puts(L"hello");
  // https://msdn.microsoft.com/en-us/library/69ze775t.aspx
  // we currently don't support the "uUR" parts...
  if (strchr("LuUR", ch) && ((p[1] == '"') || (p[1] == '\''))) {
    ismodified = ch;
    ch = p[1];
    ShiftCharN(1);
  }
  
  // parse character and string constants
  if (ch == '\'') {
    GetString(ch, '#');
    
    // was it a 'ABCD' style literal string?
    if (ch == '\'') {
      // allow up to 4 characters
      if (TokenStringLen > 4)
        ERROR(0x0400, FALSE);
      // if 1 character, do we 'sign' it
      if (TokenStringLen == 1) {
        TokenValueInt = TokenValueString[0];
        TokenValueInt -= (CharIsSigned && (TokenValueInt >= 0x80)) * 0x100;
        if (ismodified == 'L')
          return tokWChar;
      } else {
        // else, convert to little-endian
        //  'ABCD' = 0xAABBCCDD
        // This also allows a character string of zero length,
        //  returning a value of zero.
        // i = '' will return i = 0;
        // 'ABCD'  = 0x41424344
        TokenValueInt = 0;
        for (ch=0; ch<TokenStringLen; ch++)
          TokenValueInt = (TokenValueInt << 8) | TokenValueString[ch];
      }
      
      return tokNumInt;
    }
    
    return tokLitStr;
  } // endof if (ch == '\'')
  
  else if (ch == '"') {
    // The caller of GetTokenInner()/GetToken() will call GetString('"', 'd')
    // to complete string literal parsing and storing as appropriate
    switch (ismodified) {
      case 'L':
        return tokLitWStr;
      default:
        return tokLitStr;
    }
  }
  
  return tokEof;
}

void Reserve4Expansion(char *name, int len) {
  if ((MAX_CHAR_QUEUE_LEN - CharQueueLen) < (len + 1))
    ERROR(0x0010, FALSE, name);
  
  memmove(CharQueue + len + 1, CharQueue, CharQueueLen);
  
  CharQueue[len] = ' '; // space to avoid concatenation
  
  CharQueueLen += len + 1;
}

// TBD??? implement file I/O for input source code and output code (use fxn ptrs/wrappers to make librarization possible)
// support string literals
int GetToken(void) {
  struct S_MACRO_TABLE *macro = NULL;
  char *p = CharQueue;
  int ch, tok;
  
  for (;;) {
/* +-~* /% &|^! << >> && || < <= > >= == !=  () *[] ++ -- = += -= ~= *= /= %= &= |= ^= <<= >>= {} ,;: -> ... */
   
    // skip white space and comments
    SkipSpace(TRUE);
    
    if ((ch = *p) == '\0') {
      // done with the current file, drop its record,
      // pick up the including files (if any) or terminate
      if (EndOfFiles())
        break;
      continue;
    }
    
    if ((tok = GetTokenInner()) != tokEof) {
      if (PrepDontSkipTokens == PREP_STACK_TRUE)
        return tok;
      if ((tok == tokLitStr) || (tok == tokLitWStr))
        GetString('"', 0);
      continue;
    }
    
    // parse identifiers and reserved keywords
    if ((ch == '_') || isalpha(ch & 0xFF)) {
      GetIdent(FALSE);
      
      if (PrepDontSkipTokens < PREP_STACK_TRUE)
        continue;
      
      tok = GetTokenByWord(TokenIdentName);
      
      // if _naked is given, set flag for this "block" only
      if (tok == tokNaked) {
        NakedFlag = TRUE;
        continue;
      }
      
      // TBD!!! think of expanding macros in the context of concatenating string literals,
      // maybe factor out this piece of code
      if (!strcmp(TokenIdentName, "__FILE__")) {
        char *p = files[cur_file].filename;           // cur_file
        int len = strlen(p);
        Reserve4Expansion(TokenIdentName, len + 2);
        *CharQueue = '"';
        memcpy(CharQueue + 1, p, len);
        CharQueue[len + 1] = '"';
        continue;
      }
      else if (!strcmp(TokenIdentName, "__LINE__")) {
        char s[(2 + CHAR_BIT * sizeof(LineNo)) / 3];
        char *p = lab2str(s + sizeof(s), LineNo);
        int len = s + sizeof(s) - p;
        Reserve4Expansion(TokenIdentName, len);
        memcpy(CharQueue, p, len);
        continue;
      }
      else if (macro = FindMacro(TokenIdentName)) {
        // this is a macro identifier, need to expand it
        Reserve4Expansion(TokenIdentName, MACRO_TABLE_EX_LEN(macro));
        memcpy(CharQueue, MACRO_TABLE_EX(macro), MACRO_TABLE_EX_LEN(macro));
        continue;
      }
      
      return tok;
    } // endof if (ch == '_' || isalpha(ch))
    
    // parse preprocessor directives
    if (ch == '#') {
      int line = 0;
      
      ShiftCharN(1);
      
      // Skip space
      SkipSpace(FALSE);
      
      // Allow # not followed by a directive
      if (strchr("\r\n", *p))
        continue;
      
      // Get preprocessor directive
      if (isdigit(*p & 0xFF)) {
        // gcc-style #line directive without "line"
        line = 1;
      } else {
        GetIdent(FALSE);
        if (!strcmp(TokenIdentName, "line")) {
          // C89-style #line directive
          SkipSpace(FALSE);
          if (!isdigit(*p & 0xFFu))
            ERROR(0x0011, FALSE);
          line = 1;
        }
      }
      
      if (line) {
        // Support for external, gcc-like, preprocessor output:
        //   # linenum filename flags
        //
        // no flags, flag = 1 -- start of a file
        //           flag = 2 -- return to a file after #include
        //        other flags -- uninteresting

        // DONE: should also support the following C89 form:
        // # line linenum filename-opt
        
        if (GetNumber() != tokNumInt)
          ERROR(0x0012, FALSE);
        line = TokenValueInt;
        
        SkipSpace(FALSE);
        
        if ((*p == '"') || (*p == '<')) {
          if (*p == '"')
            GetString('"', '#');
          else
            GetString('>', '#');
          
          if (strlen(TokenValueString) > MAX_FILE_NAME_LEN)
            ERROR(0x0021, FALSE);
          strcpy(files[cur_file].filename, TokenValueString);
        }
        
        // Ignore gcc-style #line's flags, if any
        while (!strchr("\r\n", *p))
          ShiftCharN(1);
        
        LineNo = line - 1; // "line" is the number of the next line
        LinePos = 1;
        
        continue;
      } // endof if (line)
      
      if (!strcmp(TokenIdentName, "pragma")) {
        // TBD??? fail if inside a structure declaration
        if (PrepDontSkipTokens < PREP_STACK_TRUE) {
          while (!strchr("\r\n", *p))
            ShiftCharN(1);
          continue;
        }
        SkipSpace(FALSE);
        GetIdent(FALSE);
        
        SkipSpace(FALSE);
        if (*p == '(')
          ShiftCharN(1);
        SkipSpace(FALSE);
        
        // #pragma optimizer(off|on)
        if (!strcmp(TokenIdentName, "optimizer")) {
          if (*p == 'o') {
            GetIdent(FALSE);
            if (!strcmp(TokenIdentName, "on"))
              if (use_nbasm_out) puts_out(".opton\n");
            else if (!strcmp(TokenIdentName, "off"))
              if (use_nbasm_out) puts_out(".optoff\n");
          }
        }
        
        // #pragma codetype(tiny|small)
        else if (!strcmp(TokenIdentName, "codetype")) {
          if ((*p == 't') || (*p == 's')) {
            GetIdent(FALSE);
            if (!strcmp(TokenIdentName, "tiny"))
              if (use_nbasm_out) puts_out(".model tiny\n");
            else if (!strcmp(TokenIdentName, "small"))
              if (use_nbasm_out) puts_out(".model small\n");
            else
              ERROR(0x0011, FALSE);
          } else
            ERROR(0x0011, FALSE);
        }
        
        // #pragma ptype(rmode|pmode)
        else if (!strcmp(TokenIdentName, "ptype")) {
          if ((*p == 'r') || (*p == 'p')) {
            GetIdent(FALSE);
            if (!strcmp(TokenIdentName, "rmode")) {
              if (use_nbasm_out) 
                puts_out(".rmode\n");  // eip is 2 bytes (ip)
              else if (use_fasm_out)
                puts_out("use16\n");  // eip is 2 bytes (ip)
              SizeOfEIP = 2;
            } else if (!strcmp(TokenIdentName, "pmode")) {
              if (use_nbasm_out) 
                puts_out(".pmode\n");  // eip is 4 bytes (eip)
              else if (use_fasm_out)
                puts_out("use32\n");  // eip is 4 bytes (eip)
              SizeOfEIP = 4;
            } else
              ERROR(0x0011, FALSE);
          } else
            ERROR(0x0011, FALSE);
        }
        
        // #pragma processor(.186|etc|short|long)
        else if (!strcmp(TokenIdentName, "proc")) {
          GetIdent(TRUE);
          if (!strcmp(TokenIdentName, "8086")) {
            if (use_nbasm_out) puts_out(".8086\n");
            allow_32bit = FALSE;
          } else if (!strcmp(TokenIdentName, "186")) {
            if (use_nbasm_out) puts_out(".186\n");
            allow_32bit = FALSE;
          } else if (!strcmp(TokenIdentName, "286")) {
            if (use_nbasm_out) puts_out(".286\n");
            allow_32bit = FALSE;
          } else if (!strcmp(TokenIdentName, "386")) {
            if (use_nbasm_out) puts_out(".386\n");
            allow_32bit = TRUE;
          } else if (!strcmp(TokenIdentName, "386P")) {
            if (use_nbasm_out) puts_out(".386P\n");
            allow_32bit = TRUE;
          } else if (!strcmp(TokenIdentName, "486")) {
            if (use_nbasm_out) puts_out(".486\n");
            allow_32bit = TRUE;
          } else if (!strcmp(TokenIdentName, "x86")) {
            if (use_nbasm_out) puts_out(".x86\n");
            allow_32bit = TRUE;
          } else if (!strcmp(TokenIdentName, "short")) { // do 16-bit offsets and registers
            SizeOfWord = 2;
          } else if (!strcmp(TokenIdentName, "long")) {  // do 32-bit offsets and registers
            SizeOfWord = 4;
            if (!allow_32bit)
              ERROR(0x001A, FALSE);
          } else
            ERROR(0x0011, FALSE);
        }
        
        // #pragma org(value)
        else if (!strcmp(TokenIdentName, "org")) {
          if (isxdigit(*p)) {
            GetNumber();
            printf_out("            org %08Xh\n", TokenValueInt);
          } else
            ERROR(0x0011, FALSE);
        }
        
        // #pragma diag(off|on)
        else if (!strcmp(TokenIdentName, "diag")) {
          if (*p == 'o') {
            GetIdent(FALSE);
            if (!strcmp(TokenIdentName, "on"))
              if (use_nbasm_out) puts_out(".diag 1     ; turn on diagnostic reporting\n");
            else if (!strcmp(TokenIdentName, "off"))
              if (use_nbasm_out) puts_out(".diag 0     ; turn off diagnostic reporting\n");
          }
        }
        
        // #pragma stack(value)
        else if (!strcmp(TokenIdentName, "stack")) {
          if (isdigit(*p)) {
            GetNumber();
            printf_out(".stack %i\n", TokenValueInt);
          } else
            ERROR(0x0011, FALSE);
        }
        
        // #pragma start()
        else if (!strcmp(TokenIdentName, "start")) {
          do_start_code = TRUE;
          // this resizes the memory and calls the argc/argv[] stuff
          puts_out(
            "\n  .start                "
            "\n  jmp   _nbc_start      "
          );
        }
        
        // #pragma pack(push, n)
        // #pragma pack(pop)
        else if (!strcmp(TokenIdentName, "pack")) {
          bool canHaveNumber = TRUE, hadNumber = FALSE;
          
          if (*p == 'p') {
            GetIdent(FALSE);
            if (!strcmp(TokenIdentName, "push")) {
              SkipSpace(FALSE);
              if (*p == ',') {
                ShiftCharN(1);
                SkipSpace(FALSE);
                if (!isdigit(*p & 0xFF) || GetNumber() != tokNumInt)
                  ERROR(0x0011, FALSE);
                hadNumber = TRUE;
              }
              if (PragmaPackSp >= PPACK_STACK_SIZE)
                ERROR(0x0013, FALSE);
              PragmaPackValues[PragmaPackSp++] = PragmaPackValue;
            }
            else if (!strcmp(TokenIdentName, "pop")) {
              if (PragmaPackSp <= 0)
                ERROR(0x0014, FALSE);
              PragmaPackValue = PragmaPackValues[--PragmaPackSp];
            } else
              ERROR(0x0011, FALSE);
            SkipSpace(FALSE);
            canHaveNumber = FALSE;
          }
          
          if (canHaveNumber && isdigit(*p & 0xFF)) {
            if (GetNumber() != tokNumInt)
              ERROR(0x0011, FALSE);
            hadNumber = TRUE;
            SkipSpace(FALSE);
          }
          
          if (hadNumber) {
            PragmaPackValue = TokenValueInt;
            if ((PragmaPackValue <= 0) ||
                (PragmaPackValue > SizeOfWord) ||
                (PragmaPackValue & (PragmaPackValue - 1)))
              ERROR(0x0015, FALSE);
          } else if (canHaveNumber)
            PragmaPackValue = SizeOfWord;
        } else
          ERROR(0x0011, FALSE);
        
        if (*p != ')')
          ERROR(0x0011, FALSE);
        ShiftCharN(1);
        
        SkipSpace(FALSE);
        if (!strchr("\r\n", *p))
          ERROR(0x0011, FALSE);
        continue;
      }
      
      // if #define
      if (!strcmp(TokenIdentName, "define")) {
        // Skip space and get macro name
        SkipSpace(FALSE);
        GetIdent(FALSE);
        
        if (PrepDontSkipTokens < PREP_STACK_TRUE) {
          SkipSpace(FALSE);
          while (!strchr("\r\n", *p))
            ShiftCharN(1);
          continue;
        }
        
        if (FindMacro(TokenIdentName))
          ERROR(0x0016, FALSE, TokenIdentName);
        if (*p == '(')
          ERROR(0x0017, FALSE, TokenIdentName);
        
        AddMacroIdent(TokenIdentName);
        
        SkipSpace(FALSE);
        
        // accumulate the macro expansion text
        while (!strchr("\r\n", *p)) {
          AddMacroExpansionChar(TokenIdentName, *p);
          ShiftCharN(1);
          if ((*p != '\0') && (strchr(" \t", *p) || (*p == '/' && (p[1] == '/' || p[1] == '*')))) {
            SkipSpace(FALSE);
            AddMacroExpansionChar(TokenIdentName, ' ');
          }
        }
        AddMacroExpansionChar(TokenIdentName, '\0');
        
        continue;
      }
      
      else if (!strcmp(TokenIdentName, "undef")) {
        // Skip space and get macro name
        SkipSpace(FALSE);
        GetIdent(FALSE);
        
        if (PrepDontSkipTokens == PREP_STACK_TRUE)
          UndefineMacro(TokenIdentName);
        
        SkipSpace(FALSE);
        if (!strchr("\r\n", *p))
          ERROR(0x0011, FALSE);
        continue;
      }
      
      // if #MACRO, then change to a string of that macro
      else if (macro = FindMacro(TokenIdentName)) {
        int len = MACRO_TABLE_EX_LEN(macro);
        Reserve4Expansion(TokenIdentName, len + 2); // +2 for the ""'s
        *CharQueue = '"';
        memcpy(CharQueue + 1, MACRO_TABLE_EX(macro), len);
        CharQueue[len + 1] = '"';
        continue;
      }
      
      else if (!strcmp(TokenIdentName, "error")) {
        GetString('\n', '#');
        ERROR(0xFFFE, TRUE, TokenValueString);
        continue;
      }
      
      else if (!strcmp(TokenIdentName, "include")) {
        char quot;
        
        // Skip space and get file name
        SkipSpace(FALSE);
        
        quot = *p;
        if (*p == '"')
          GetString('"', '#');
        else if (*p == '<')
          GetString('>', '#');
        else
          ERROR(0x0021, FALSE);

        SkipSpace(FALSE);
        if (!strchr("\r\n", *p))
          ERROR(0x0011, FALSE);
        
        if (PrepDontSkipTokens == PREP_STACK_TRUE)
          IncludeFile(quot);
        
        continue;
      }
      
      else if (!strcmp(TokenIdentName, "outfile")) {
        // Skip space and get file name
        SkipSpace(FALSE);
        
        if (*p == '"')
          GetString('"', '#');
        else
          ERROR(0x0021, FALSE);
        
        SkipSpace(FALSE);
        if (!strchr("\r\n", *p))
          ERROR(0x0011, FALSE);
        
        if (PrepDontSkipTokens == PREP_STACK_TRUE)
          printf_out("outfile '%s'   ; target filename\n", TokenValueString);
        
        continue;
      }
      
      // Handle the #if/#ifdef/#else/#endif/etc stuff
      if (DoIfElse(TokenIdentName, p))
        continue;
      
      if (PrepDontSkipTokens < PREP_STACK_TRUE) {
        SkipLine();
        continue;
      }
      
      ERROR(0x0011, FALSE);
    } // endof if (ch == '#')
    
    ERROR(0x0333, FALSE, p[0]);
  } // endof for (;;)
  
  return tokEof;
}

// This is the start of a mimimul #if preproccessor...
//  This needs a lot of work
//
bool DoIfElse(char *TokenIdentName, char *p) {
  int def, tok, tempSkip;
  bool elif = !strcmp(TokenIdentName, "elif");
  bool Not, close;
  
  if (!strcmp(TokenIdentName, "if") || elif) {
    // must set this or it will not tokenize the parameter(s) of #elif
    tempSkip = PrepDontSkipTokens;
    PrepDontSkipTokens = PREP_STACK_TRUE;
    tok = GetToken();
    if (tok == '!') {
      Not = TRUE;
      tok = GetToken();
    //} else if (tok == '(') {
    //  close = TRUE;  // we need this twice????? (loop it or something)
    //  tok = GetToken();
    }
    PrepDontSkipTokens = tempSkip;
    if ((tok == tokIdent) && !strcmp(TokenIdentName, "defined")) {
      SkipSpace(FALSE);
      if (*p == '(') {
        GetToken();
        close = TRUE;
      }
      GetIdent(FALSE);
      def = (FindMacro(TokenIdentName) != NULL);
      if (close)
        if (GetToken() != ')')
          ERROR(0x0111, FALSE, "");
      SkipSpace(FALSE);
      if (!strchr("\r\n", *p))
        ERROR(0x0011, FALSE);
    }
    else if ((tok == tokNumInt) || (tok == tokNumUint)) {
      def = (TokenValueInt != 0);
    }
    
    else {
      printf("\n ************* %i [%s]", tok, TokenIdentName);
    }
    
    //if (close)
    //  if (GetToken() != ')')
    //    ERROR(0x0111, FALSE, "");
    
    if (Not)
      def = !def;
    
    if (elif) {
      if (PrepSp == 0)
        ERROR(0x0311, FALSE);
      if (PrepDontSkipTokens == PREP_STACK_FALSE)
        PrepDontSkipTokens = def;
      else
        PrepDontSkipTokens = PREP_STACK_ELIF;
    } else
      pushPrep(def);
    
    SkipSpace(FALSE);
    if (!strchr("\r\n", *p))
      ERROR(0x0011, FALSE);
    return TRUE;
  }
  
  else if (!strcmp(TokenIdentName, "ifdef")) {
    // Skip space and get macro name
    SkipSpace(FALSE);
    GetIdent(FALSE);
    def = (FindMacro(TokenIdentName) != NULL);
    SkipSpace(FALSE);
    if (!strchr("\r\n", *p))
      ERROR(0x0011, FALSE);
    pushPrep(def);
    return TRUE;
  }
  
  else if (!strcmp(TokenIdentName, "ifndef")) {
    // Skip space and get macro name
    SkipSpace(FALSE);
    GetIdent(FALSE);
    def = (FindMacro(TokenIdentName) != NULL);
    SkipSpace(FALSE);
    if (!strchr("\r\n", *p))
      ERROR(0x0011, FALSE);
    pushPrep(!def);
    return TRUE;
  }
  
  else if (!strcmp(TokenIdentName, "else")) {
    SkipSpace(FALSE);
    if (!strchr("\r\n", *p))
      ERROR(0x0011, FALSE);
    if (PrepDontSkipTokens == PREP_STACK_FALSE)
      PrepDontSkipTokens = PREP_STACK_TRUE;
    else
      PrepDontSkipTokens = PREP_STACK_FALSE;
    return TRUE;
  }
  
  else if (!strcmp(TokenIdentName, "endif")) {
    SkipSpace(FALSE);
    if (!strchr("\r\n", *p))
      ERROR(0x0011, FALSE);
    popPrep();
    return TRUE;
  }
  
  else
    return FALSE;
}

#include "cgx86.c"

void push3(int t, bit32u v, bit32u f) {
  if (sp >= STACK_SIZE)
    ERROR(0x0103, FALSE);
  stack[sp].tok = t;
  stack[sp].param = v;
  stack[sp].flags = f;
  sp++;
}

void push2(int t, bit32u v) {
  push3(t, v, 0);
}

void push(int t) {
  push2(t, 0);
}

int stacktop() {
  if (sp == 0)
    ERROR(0xE002, FALSE);
  return stack[sp - 1].tok;
}

int pop3(int *v, bit32u *f) {
  int t = stacktop();
  sp--;
  *v = (int) stack[sp].param;
  *f = stack[sp].flags;
  return t;
}

int pop2(int *v) {
  bit32u f;
  return pop3(v, &f);
}

int pop() {
  int v;
  return pop2(&v);
}

void ins3(int pos, int t, bit32u v, bit32u f) {
  if (sp >= STACK_SIZE)
    ERROR(0x0103, FALSE);
  memmove(&stack[pos + 1], &stack[pos], sizeof(struct S_SYNTAX_STACK) * (sp - pos));
  stack[pos].tok = t;
  stack[pos].param = (bit32u) v;
  stack[pos].flags = f;
  sp++;
}

void ins2(int pos, int t, bit32u v) {
  ins3(pos, t, v, 0);
}

void ins(int pos, int t) {
  ins2(pos, t, 0);
}

void del(int pos, int cnt) {
  memmove(&stack[pos],
          &stack[pos + cnt],
          sizeof(struct S_SYNTAX_STACK) * (sp - (pos + cnt)));
  sp -= cnt;
}

void pushop3(int t, bit32u v, bit32u f) {
  if (opsp >= STACK_SIZE)
    ERROR(0x0103, FALSE);
  opstack[opsp].tok = t;
  opstack[opsp].param = v;
  opstack[opsp].flags = f;
  opsp++;
}

void pushop2(int t, bit32u v) {
  pushop3(t, v, 0);
}

void pushop(int t) {
  pushop2(t, 0);
}

int opstacktop() {
  if (opsp == 0)
    ERROR(0xE003, FALSE);
  return opstack[opsp - 1].tok;
}

int popop3(int *v, bit32u *f) {
  int t = opstacktop();
  opsp--;
  *v = (int) opstack[opsp].param;
  *f = opstack[opsp].flags;
  return t;
}

int popop2(int *v) {
  bit32u f;
  return popop3(v, &f);
}

int popop() {
  int v;
  return popop2(&v);
}

int isop(int tok) {
  unsigned i;
  static unsigned char toks[] = {
    '!',
    '~',
    '&',
    '*',
    '/', '%',
    '+', '-',
    '|', '^',
    '<', '>',
    '=',
    tokLogOr, tokLogAnd,
    tokEQ, tokNEQ,
    tokLEQ, tokGEQ,
    tokLShift, tokRShift,
    tokInc, tokDec,
    tokSizeof,
    tokAssignMul, tokAssignDiv, tokAssignMod,
    tokAssignAdd, tokAssignSub,
    tokAssignLSh, tokAssignRSh,
    tokAssignAnd, tokAssignXor, tokAssignOr,
    tokComma,
    '?'
  };
  
  for (i = 0; i < sizeof(toks) / sizeof(toks[0]); i++)
    if (toks[i] == tok)
      return 1;
  return 0;
}

int isunary(int tok) {
  return (tok == '!') | (tok == '~') | (tok == tokInc) | (tok == tokDec) | (tok == tokSizeof);
}

int preced(int tok) {
  switch (tok) {
    case '*': case '/': case '%': return 13;
    case '+': case '-': return 12;
    case tokLShift: case tokRShift: return 11;
    case '<': case '>': case tokLEQ: case tokGEQ: return 10;
    case tokEQ: case tokNEQ: return 9;
    case '&': return 8;
    case '^': return 7;
    case '|': return 6;
    case tokLogAnd: return 5;
    case tokLogOr: return 4;
    case '?': case ':': return 3;
    case '=':
    case tokAssignMul: case tokAssignDiv: case tokAssignMod:
    case tokAssignAdd: case tokAssignSub:
    case tokAssignLSh: case tokAssignRSh:
    case tokAssignAnd: case tokAssignXor: case tokAssignOr:
      return 2;
    case tokComma:
      return 1;
  }
  return 0;
}

int precedGEQ(int lfttok, int rhttok) {
  int pl = preced(lfttok);
  int pr = preced(rhttok);
  // ternary/conditional operator ?: is right-associative
  if (pl == 3 && pr >= 3)
    pl = 0;
  // assignment is right-associative
  if (pl == 2 && pr >= 2)
    pl = 0;
  return pl >= pr;
}

char *lab2str(char *p, int n) {
  do {
    *--p = '0' + (n % 10);
    n /= 10;
  } while (n);
  
  return p;
}

int exprUnary(int tok, int *gotUnary, int commaSeparator, int argOfSizeOf) {
  int decl = 0;
  *gotUnary = 0;
  static int sizeofLevel = 0;
  
  if (isop(tok) && (isunary(tok) || strchr("&*+-", tok))) {
    int lastTok = tok;
    int sizeofLevelInc = lastTok == tokSizeof;
    
    sizeofLevel += sizeofLevelInc;
    tok = exprUnary(GetToken(), gotUnary, commaSeparator, sizeofLevelInc);
    sizeofLevel -= sizeofLevelInc;

    if (!*gotUnary)
      ERROR(0x0110, FALSE, GetTokenName(lastTok));
    switch (lastTok) {
      // DONE: remove all collapsing of all unary operators.
      // It's wrong because type checking must occur before any optimizations.
      // WRONG: DONE: collapse alternating & and * (e.g. "*&*&x" "&*&*x")
      // WRONGISH: DONE: replace prefix ++/-- with +=1/-=1
      case '&':
        push(tokUnaryAnd);
        break;
      case '*':
        push(tokUnaryStar);
        break;
      case '+':
        push(tokUnaryPlus);
        break;
      case '-':
        push(tokUnaryMinus);
        break;
      case '!':
        // replace "!" with "== 0"
        push(tokNumInt);
        push(tokEQ);
        break;
      default:
        push(lastTok);
        break;
    }
  } else {
    int inspos = sp;
    
    if (tok == tokNumInt || tok == tokNumUint) {
      push2(tok, TokenValueInt);
      *gotUnary = 1;
      tok = GetToken();
    } 
    
    else if ((tok == tokLitStr) || (tok == tokLitWStr)) {
      int lbl = LabelCnt++;
      unsigned len = 0, tsize = 1024;
      struct S_IDENT_PREFIX *ident;
      int wastok = tok;
      char *tempstr = (char *) calloc(tsize, 1);
      
      do {
        GetString('"', sizeofLevel ? 0 : '#'); // throw away string data inside sizeof, e.g. sizeof "a" or sizeof("a" + 1)
        if (len + TokenStringLen < len || len + TokenStringLen >= truncUint(-1))
          ERROR(0x0401, FALSE);
        tok = GetToken();
        if ((len + TokenStringLen) > tsize)
          tempstr = (char *) realloc(tempstr, tsize = (len + TokenStringLen + 1024));
        strcat(tempstr, TokenValueString);
        len += TokenStringLen;
      } while (tok == wastok); // concatenate adjacent string literals
      
      lbl = AddString(lbl, tempstr, len += 1, (wastok == tokLitWStr) ? STRING_IS_WIDE : 0);
      free(tempstr);
      
      // DONE: can this break incomplete yet declarations???, e.g.: int x[sizeof("az")][5];
      PushSyntax2(tokIdent, (bit32u) (ident = AddNumericIdent(lbl)));
      PushSyntax('[');
      PushSyntax2(tokNumUint, len);
      PushSyntax(']');
      PushSyntax(tokChar);
      
      push2(tokIdent, (bit32u) ident);
      *gotUnary = 1;
    }
    
    else if (tok == tokIdent) {
      push2(tok, (bit32u) AddIdent(TokenIdentName, 0));
      *gotUnary = 1;
      tok = GetToken();
    }
    
    else if (tok == '(') {
      tok = GetToken();
      decl = TokenStartsDeclaration(tok, 1);
      
      if (decl) {
        int synPtr;
        int lbl = LabelCnt++;
        char s[1 + (2 + CHAR_BIT * sizeof lbl) / 3 + sizeof "<something>" - 1];
        char *p = s + sizeof(s);
        
        tok = ParseDecl(tok, NULL, !argOfSizeOf, 0);
        if (tok != ')')
          ERROR(0x0111, FALSE, GetTokenName(tok));
        synPtr = FindSymbol("<something>");
        
        // Rename "<something>" to "<something#>", where # is lbl.
        // This makes the nameless declaration uniquely identifiable by name.
        *--p = '\0';
        *--p = ")>"[argOfSizeOf]; // differentiate casts (something#) from not casts <something#>
        
        p = lab2str(p, lbl);
        
        p -= sizeof("<something>") - 2 - 1;
        memcpy(p, "something", sizeof("something") - 1);
        
        *--p = "(<"[argOfSizeOf]; // differentiate casts (something#) from not casts <something#>
        
        SyntaxStack[synPtr].param = (bit32u) AddIdent(p, 0);
        
        tok = GetToken();
        if (argOfSizeOf) {
          // expression: sizeof(type)
          *gotUnary = 1;
        } else {
          // unary type cast operator: (type)
          decl = 0;
          tok = exprUnary(tok, gotUnary, commaSeparator, 0);
          if (!*gotUnary)
            ERROR(0x0112, FALSE);
        }
        push2(tokIdent, SyntaxStack[synPtr].param);
      }
      
      else {
        tok = expr(tok, gotUnary, 0);
        if (tok != ')')
          ERROR(0x0111, FALSE, GetTokenName(tok));
        if (!*gotUnary)
          ERROR(0x0113, FALSE);
        tok = GetToken();
      }
    }
    
    while (*gotUnary && !decl) {
      // is this start of a 'funcname()'
      if (tok == '(') {
        int acnt = 0;
        ins(inspos, '(');
        for (;;) {
          int pos2 = sp;
          
          tok = GetToken();
          tok = expr(tok, gotUnary, 1);
          
          // Reverse the order of argument evaluation, which is important for
          // variadic functions like printf():
          // we want 1st arg to be the closest to the stack top.
          // This also reverses the order of evaluation of all groups of
          // arguments.
          int last_tok = 0;  // to check if the last token was the '&', meaning the address of
          while (pos2 < sp) {
            // TBD??? not quite efficient
            int t, v; bit32u f;
            t = pop3(&v, &f);
            
            // if we pass a pointer of a local variable as a parameter to a function,
            //  we need to assume that this variable is not initialized, so mark it as so
            if ((t == tokIdent) && (last_tok == tokUnaryAnd)) {
              struct S_IDENT_PREFIX *p = (struct S_IDENT_PREFIX *) v;
              if (p->flags & IDENT_FLAGS_ISLOCAL)
                p->flags |= IDENT_FLAGS_ASSIGNED;
            }
            
            ins3(inspos + 1, t, v, f);
            pos2++;
            last_tok = t;
          }
          
          if (tok == ',') {
            if (!*gotUnary)
              ERROR(0x0114, FALSE);
            acnt++;
            ins(inspos + 1, ','); // helper argument separator (hint for expression evaluator)
            continue; // off to next arg
          }
          if (tok == ')') {
            if (acnt && !*gotUnary)
              ERROR(0x0115, FALSE);
            *gotUnary = 1; // don't fail for 0 args in ()
            break; // end of args
          }
          // DONE: think of inserting special arg pseudo tokens for verification purposes
          ERROR(0x0116, FALSE, GetTokenName(tok));
        } // endof for(;;) for fxn args
        push(')');
      }
      
      else if (tok == '[') {
        tok = GetToken();
        tok = expr(tok, gotUnary, 0);
        if (!*gotUnary)
          ERROR(0x0117, FALSE);
        if (tok != ']')
          ERROR(0x0118, FALSE, GetTokenName(tok));
        // TBD??? add implicit casts to size_t of array indicies.
        // E1[E2] -> *(E1 + E2)
        // push('[');
        push('+');
        push(tokUnaryStar);
      }
      // WRONG: DONE: replace postfix ++/-- with (+=1)-1/(-=1)+1
      else if (tok == tokInc) {
        push(tokPostInc);
      }
      
      else if (tok == tokDec) {
        push(tokPostDec);
      }
      
      else if (tok == '.' || tok == tokArrow) {
        // transform a.b into (&a)->b
        if (tok == '.')
          push(tokUnaryAnd);
        tok = GetToken();
        if (tok != tokIdent)
          ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
        push2(tok, (bit32u) AddIdent(TokenIdentName, 0));
        // "->" in "a->b" will function as "+" in "*(type_of_b*)((char*)a + offset_of_b_in_a)"
        push(tokArrow);
        push(tokUnaryStar);
      }
      
      else
        break;
      
      tok = GetToken();
    } // endof while (*gotUnary)
  }
  
  if (tok == ',' && !commaSeparator)
    tok = tokComma;
  
  return tok;
}

int expr(int tok, int *gotUnary, int commaSeparator) {
  *gotUnary = 0;
  
  pushop(tokEof);
  
  tok = exprUnary(tok, gotUnary, commaSeparator, 0);
  
  while (tok != tokEof && strchr(",;:)]}", tok) == NULL && *gotUnary) {
    if (isop(tok) && !isunary(tok)) {
      while (precedGEQ(opstacktop(), tok)) {
        int t, v; bit32u f;
        int c = 0;
        // move ?expr: as a whole to the expression stack as "expr?"
        do {
          t = popop3(&v, &f);
          if (t != ':')
            push3(t, v, f);
          c += (t == ':') - (t == '?');
        } while (c);
      }
      
      // here: preced(postacktop()) < preced(tok)
      pushop(tok);
      
      // treat the ternary/conditional operator ?expr: as a pseudo binary operator
      if (tok == '?') {
        int ssp = sp;
        
        tok = expr(GetToken(), gotUnary, 0);
        if (!*gotUnary || tok != ':')
          ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
        
        // move ?expr: as a whole to the operator stack
        // this is beautiful and ugly at the same time
        while (sp > ssp) {
          int t, v; bit32u f;
          t = pop3(&v, &f);
          pushop3(t, v, f);
        }
        
        pushop(tok);
      }
      
      tok = exprUnary(GetToken(), gotUnary, commaSeparator, 0);
      // DONE: figure out a check to see if exprUnary() fails to add a rhs operand
      if (!*gotUnary)
        ERROR(0x0110, FALSE, GetTokenName(tok));
      
      continue;
    }
    
    ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  }
  
  while (opstacktop() != tokEof) {
    int t, v; bit32u f;
    t = popop3(&v, &f);
    if (t != ':')
      push3(t, v, f);
  }
  
  popop();
  
  return tok;
}

void decayArray(int *ExprTypeSynPtr, int arithmetic) {
  // Dacay arrays to pointers to their first elements in
  // binary + and - operators
  if ((*ExprTypeSynPtr >= 0) && (SyntaxStack[*ExprTypeSynPtr].tok == '[')) {
    while (SyntaxStack[*ExprTypeSynPtr].tok != ']')
      ++*ExprTypeSynPtr;
    ++*ExprTypeSynPtr;
    *ExprTypeSynPtr = -*ExprTypeSynPtr;
  }
  // Also, to simplify code, return all other pointers as
  // negative expression stack syntax indices/pointers
  else if ((*ExprTypeSynPtr >= 0) && (SyntaxStack[*ExprTypeSynPtr].tok == '*')) {
    ++*ExprTypeSynPtr;
    *ExprTypeSynPtr = -*ExprTypeSynPtr;
  }
  
  // disallow arithmetic on pointers to void
  // disallow function pointers
  if (arithmetic) {
    if (*ExprTypeSynPtr < 0) {
      if (SyntaxStack[-*ExprTypeSynPtr].tok == tokVoid)
        ERROR(0x0211, FALSE);
      if (SyntaxStack[-*ExprTypeSynPtr].tok == '(' || !GetDeclSize(-*ExprTypeSynPtr, 0))
        ERROR(0x0200, FALSE);
    } else {
      if (SyntaxStack[*ExprTypeSynPtr].tok == '(')
        ERROR(0x0201, FALSE);
    }
  }
}

bool nonVoidTypeCheck(int ExprTypeSynPtr) {
  if (ExprTypeSynPtr >= 0 && SyntaxStack[ExprTypeSynPtr].tok == tokVoid) {
    ERROR(0x0212, FALSE, GetTokenName(SyntaxStack[ExprTypeSynPtr].tok));
    return TRUE;
  } else
    return FALSE;
}

bool scalarTypeCheck(int ExprTypeSynPtr) {
  nonVoidTypeCheck(ExprTypeSynPtr);
  return ((ExprTypeSynPtr >= 0) && (SyntaxStack[ExprTypeSynPtr].tok == tokStructPtr));
}

void numericTypeCheck(int ExprTypeSynPtr, int tok) {
  if (ExprTypeSynPtr >= 0 &&
      (SyntaxStack[ExprTypeSynPtr].tok == tokChar ||
       SyntaxStack[ExprTypeSynPtr].tok == tokWChar ||
       SyntaxStack[ExprTypeSynPtr].tok == tokSChar ||
       SyntaxStack[ExprTypeSynPtr].tok == tokUChar ||
       SyntaxStack[ExprTypeSynPtr].tok == tokShort ||
       SyntaxStack[ExprTypeSynPtr].tok == tokUShort ||
       SyntaxStack[ExprTypeSynPtr].tok == tokInt ||
       SyntaxStack[ExprTypeSynPtr].tok == tokUnsigned))
    return;
  else
    ERROR(0x0202, FALSE, GetTokenName(tok));
}

void compatCheck(int *ExprTypeSynPtr, int TheOtherExprTypeSynPtr, int ConstExpr[2], int lidx, int ridx) {
  int exprTypeSynPtr = *ExprTypeSynPtr;
  int c = 0;
  int lptr, rptr, lnum, rnum;

  // convert functions to pointers to functions
  if (exprTypeSynPtr >= 0 && SyntaxStack[exprTypeSynPtr].tok == '(')
    *ExprTypeSynPtr = exprTypeSynPtr = -exprTypeSynPtr;
  if (TheOtherExprTypeSynPtr >= 0 && SyntaxStack[TheOtherExprTypeSynPtr].tok == '(')
    TheOtherExprTypeSynPtr = -TheOtherExprTypeSynPtr;

  lptr = exprTypeSynPtr < 0;
  rptr = TheOtherExprTypeSynPtr < 0;
  lnum = !lptr && (SyntaxStack[exprTypeSynPtr].tok == tokInt ||
                   SyntaxStack[exprTypeSynPtr].tok == tokUnsigned);
  rnum = !rptr && (SyntaxStack[TheOtherExprTypeSynPtr].tok == tokInt ||
                   SyntaxStack[TheOtherExprTypeSynPtr].tok == tokUnsigned);

  // both operands have arithmetic type
  // (arithmetic operands have been already promoted):
  if (lnum && rnum)
    return;
  
  // both operands have void type:
  if (!lptr && SyntaxStack[exprTypeSynPtr].tok == tokVoid &&
      !rptr && SyntaxStack[TheOtherExprTypeSynPtr].tok == tokVoid)
    return;

  // TBD??? check for exact 0?
  // one operand is a pointer and the other is NULL constant
  // ((void*)0 is also a valid null pointer constant),
  // the type of the expression is that of the pointer:
  if (lptr &&
      ((rnum && ConstExpr[1]) ||
       (rptr && SyntaxStack[-TheOtherExprTypeSynPtr].tok == tokVoid &&
        stack[ridx].tok == tokUnaryPlus && // "(type*)constant" appears as "constant +(unary)"
        (stack[ridx - 1].tok == tokNumInt || stack[ridx - 1].tok == tokNumUint))))
    return;
  if (rptr &&
      ((lnum && ConstExpr[0]) ||
       (lptr && SyntaxStack[-exprTypeSynPtr].tok == tokVoid &&
        stack[lidx].tok == tokUnaryPlus && // "(type*)constant" appears as "constant +(unary)"
        (stack[lidx - 1].tok == tokNumInt || stack[lidx - 1].tok == tokNumUint))))
  {
    *ExprTypeSynPtr = TheOtherExprTypeSynPtr;
    return;
  }

  // not expecting non-pointers beyond this point
  if (!(lptr && rptr))
    ERROR(0x0203, FALSE);

  // one operand is a pointer and the other is a pointer to void
  // (except (void*)0, which is different from other pointers to void),
  // the type of the expression is pointer to void:
  if (SyntaxStack[-exprTypeSynPtr].tok == tokVoid)
    return;
  if (SyntaxStack[-TheOtherExprTypeSynPtr].tok == tokVoid) {
    *ExprTypeSynPtr = TheOtherExprTypeSynPtr;
    return;
  }
  
  // both operands are pointers to compatible types:
  if (exprTypeSynPtr == TheOtherExprTypeSynPtr)
    return;

  exprTypeSynPtr = -exprTypeSynPtr;
  TheOtherExprTypeSynPtr = -TheOtherExprTypeSynPtr;

  for (;;) {
    int tok = SyntaxStack[exprTypeSynPtr].tok;
    if (tok != SyntaxStack[TheOtherExprTypeSynPtr].tok)
      ERROR(0x0203, FALSE);

    if (tok != tokIdent &&
      SyntaxStack[exprTypeSynPtr].param != SyntaxStack[TheOtherExprTypeSynPtr].param)
      ERROR(0x0203, FALSE);

    c += (tok == '(') - (tok == ')') + (tok == '[') - (tok == ']');

    if (!c) {
      switch (tok) {
        case tokVoid:
        case tokChar: case tokSChar: case tokUChar: case tokWChar:
        case tokShort: case tokUShort:
        case tokInt: case tokUnsigned:
        case tokStructPtr:
          return;
      }
    }

    exprTypeSynPtr++;
    TheOtherExprTypeSynPtr++;
  }
}

void shiftCountCheck(int *psr, int idx, int ExprTypeSynPtr) {
  int sr = *psr;
  // can't shift by a negative count and by a count exceeding
  // the number of bits in int
  if ((SyntaxStack[ExprTypeSynPtr].tok != tokUnsigned && sr < 0) ||
      (unsigned) sr >= CHAR_BIT * sizeof(int) ||
      (unsigned) sr >= 8u * SizeOfWord) {
    //ERROR(0, FALSE, "exprval(): Invalid shift count\n");
    warning(0x0001, 0);
    // truncate the count, so the assembler doesn't get an invalid count
    sr &= SizeOfWord * 8 - 1;
    *psr = sr;
    stack[idx].param = (bit32u) sr;
  }
}

int divCheckAndCalc(int tok, int *psl, int sr, int Unsigned, int ConstExpr[2]) {
  int div0 = 0;
  int sl = *psl;

  if (!ConstExpr[1])
    return !div0;

  if (Unsigned) {
    sl = (int) truncUint(sl);
    sr = (int) truncUint(sr);
  } else {
    sl = truncInt(sl);
    sr = truncInt(sr);
  }

  if (sr == 0) {
    div0 = 1;
  } else if (!ConstExpr[0]) {
    return !div0;
  } else if (!Unsigned && ((sl == INT_MIN && sr == -1) || sl / sr != truncInt(sl / sr))) {
    div0 = 1;
  } else {
    if (Unsigned) {
      if (tok == '/')
        sl = (int)((unsigned)sl / sr);
      else
        sl = (int)((unsigned)sl % sr);
    } else {
      // TBD!!! C89 gives freedom in how exactly division of negative integers
      // can be implemented w.r.t. rounding and w.r.t. the sign of the remainder.
      // A stricter, C99-conforming implementation, non-dependent on the
      // compiler used to compile Smaller C is needed.
      if (tok == '/')
        sl /= sr;
      else
        sl %= sr;
    }
    *psl = sl;
  }
  
  if (div0)
    warning(0x0002, 0);
  
  return !div0;
}

void promoteType(int *ExprTypeSynPtr, int *TheOtherExprTypeSynPtr)
{
  // chars must be promoted to ints in expressions as the very first thing
  if (*ExprTypeSynPtr >= 0 &&
      (SyntaxStack[*ExprTypeSynPtr].tok == tokChar ||
       SyntaxStack[*ExprTypeSynPtr].tok == tokWChar ||
       SyntaxStack[*ExprTypeSynPtr].tok == tokShort ||
       SyntaxStack[*ExprTypeSynPtr].tok == tokUShort ||
       SyntaxStack[*ExprTypeSynPtr].tok == tokSChar ||
       SyntaxStack[*ExprTypeSynPtr].tok == tokUChar))
    *ExprTypeSynPtr = SymIntSynPtr;

  // ints must be converted to unsigned ints if they are used in binary
  // operators whose other operand is unsigned int (except <<,>>,<<=,>>=)
  if (*ExprTypeSynPtr >= 0 && SyntaxStack[*ExprTypeSynPtr].tok == tokInt &&
      *TheOtherExprTypeSynPtr >= 0 && SyntaxStack[*TheOtherExprTypeSynPtr].tok == tokUnsigned)
    *ExprTypeSynPtr = SymUintSynPtr;
}

int GetFxnInfo(int ExprTypeSynPtr, int *MinParams, int *MaxParams, int *ReturnExprTypeSynPtr, int *FirstParamSynPtr) {
  int ptr = 0;

  *MaxParams = *MinParams = 0;

  if (ExprTypeSynPtr < 0) {
    ptr = 1;
    ExprTypeSynPtr = -ExprTypeSynPtr;
  }

  while (SyntaxStack[ExprTypeSynPtr].tok == tokIdent || SyntaxStack[ExprTypeSynPtr].tok == tokLocalOfs)
    ExprTypeSynPtr++;

  if (!(SyntaxStack[ExprTypeSynPtr].tok == '(' ||
        (!ptr && SyntaxStack[ExprTypeSynPtr].tok == '*' && SyntaxStack[ExprTypeSynPtr + 1].tok == '(')))
    return 0;

  // Count params

  while (SyntaxStack[ExprTypeSynPtr].tok != '(')
    ExprTypeSynPtr++;
  ExprTypeSynPtr++;

  if (FirstParamSynPtr)
    *FirstParamSynPtr = ExprTypeSynPtr;
  
  if (SyntaxStack[ExprTypeSynPtr].tok == ')') {
    // "fxn()": unspecified parameters, so, there can be any number of them
    *MaxParams = 32767; // INT_MAX;
    *ReturnExprTypeSynPtr = ExprTypeSynPtr + 1;
    return 1;
  }
  
  if (SyntaxStack[ExprTypeSynPtr + 1].tok == tokVoid) {
    // "fxn(void)": 0 parameters
    *ReturnExprTypeSynPtr = ExprTypeSynPtr + 3;
    return 1;
  }
  
  for (;;) {
    int tok = SyntaxStack[ExprTypeSynPtr].tok;
    if (tok == tokIdent) {
      if (SyntaxStack[ExprTypeSynPtr + 1].tok != tokEllipsis) {
        ++*MinParams;
        ++*MaxParams;
      } else {
        *MaxParams = 32767; // INT_MAX;
      }
    } else if (tok == '(') {
      // skip parameters in parameters
      int c = 1;
      while (c && ExprTypeSynPtr < SyntaxStackCnt) {
        tok = SyntaxStack[++ExprTypeSynPtr].tok;
        c += (tok == '(') - (tok == ')');
      }
    } else if (tok == ')') {
      ExprTypeSynPtr++;
      break;
    }

    ExprTypeSynPtr++;
  }

  // get the function's return type
  *ReturnExprTypeSynPtr = ExprTypeSynPtr;

  return 1;
}

void simplifyConstExpr(int val, int isConst, int *ExprTypeSynPtr, int top, int bottom) {
  if (!isConst || stack[top].tok == tokNumInt || stack[top].tok == tokNumUint)
    return;

  if (SyntaxStack[*ExprTypeSynPtr].tok == tokUnsigned)
    stack[top].tok = tokNumUint;
  else
    stack[top].tok = tokNumInt;
  stack[top].param = (bit32u) val;
  
  del(bottom, top - bottom);
}

int AllocLocal(unsigned size) {
  // Let's calculate variable's relative on-stack location
  int oldOfs = CurFxnLocalOfs;
  
  // Note: local vars are word-aligned on the stack
  CurFxnLocalOfs = (int)((CurFxnLocalOfs - size) & ~(SizeOfWord - 1u));
  if (CurFxnLocalOfs >= oldOfs ||
      CurFxnLocalOfs != truncInt(CurFxnLocalOfs) ||
      CurFxnLocalOfs < -GenMaxLocalsSize())
    ERROR(0x0173, FALSE);
  
  if (CurFxnMinLocalOfs > CurFxnLocalOfs)
    CurFxnMinLocalOfs = CurFxnLocalOfs;
  
  return CurFxnLocalOfs;
}

// DONE: sizeof(type)
// DONE: "sizeof expr"
// DONE: constant expressions
// DONE: collapse constant subexpressions into constants
int exprval(int *idx, int *ExprTypeSynPtr, int *ConstExpr) {
  int tok;
  int s;
  struct S_IDENT_PREFIX *p;
  int RightExprTypeSynPtr;
  int oldIdxRight;
  int oldSpRight;
  int constExpr[3];
  
  if (*idx < 0)
    ERROR(0xE004, FALSE);

  tok = stack[*idx].tok;
  p = (struct S_IDENT_PREFIX *) stack[*idx].param;
  s = stack[*idx].param;
  
  --*idx;
  
  oldIdxRight = *idx;
  oldSpRight = sp;

  switch (tok) {
    // Constants
    case tokNumInt:
      // return the constant's type: int
      *ExprTypeSynPtr = SymIntSynPtr;
      *ConstExpr = 1;
      break;
      
    case tokNumUint:
      // return the constant's type: unsigned int
      *ExprTypeSynPtr = SymUintSynPtr;
      *ConstExpr = 1;
      break;
      
    // Identifiers
    case tokIdent: {
      char *ident = IDENT_STR(p);
      int synPtr, type;
      if (CurFxnName && !strcmp(ident, "__func__")) {
        if (CurFxnNameLabel >= 0)
          CurFxnNameLabel = -CurFxnNameLabel;
        stack[*idx + 1].param = SyntaxStack[SymFuncPtr].param;
        synPtr = SymFuncPtr;
      } else {
        synPtr = FindSymbol(ident);
        // "Rename" static vars in function scope
        if ((synPtr >= 0) && ((synPtr + 1) < SyntaxStackCnt) && (SyntaxStack[synPtr + 1].tok == tokIdent)) {
          p = (struct S_IDENT_PREFIX *) (stack[*idx + 1].param = SyntaxStack[synPtr + 1].param);
          ident = IDENT_STR(p);
          synPtr++;
        }
      }
      
      // This is when the Ident gets used
      // See if it has been assigned first, give warning if not, then mark as being used
      if ((p->flags & (IDENT_FLAGS_ISLOCAL | IDENT_FLAGS_ASSIGNED)) == IDENT_FLAGS_ISLOCAL)
        warning(0x0100, 0, IDENT_STR(p));
      p->flags |= IDENT_FLAGS_USED;
      
      if (synPtr < 0) {
        if ((*idx + 2 >= sp) || (stack[*idx + 2].tok != ')')) {
          ERROR(0x0105, TRUE, ident);
          s = *ExprTypeSynPtr = *ConstExpr = 0;
          break;
        } else {
          warning(0x0003, 0, ident);
          // Implicitly declare "extern int ident();"
          PushSyntax2(tokIdent, (bit32u) p);
          PushSyntax('(');
          PushSyntax(')');
          PushSyntax(tokInt);
          synPtr = FindSymbol(ident);
        }
      }
      
      if (((synPtr + 1) < SyntaxStackCnt) && (SyntaxStack[synPtr + 1].tok == tokNumInt)) {
        // this is an enum constant
        stack[*idx + 1].tok = tokNumInt;
        s = (stack[*idx + 1].param = SyntaxStack[synPtr + 1].param);
        *ExprTypeSynPtr = SymIntSynPtr;
        *ConstExpr = 1;
        break;
      }
      
      // this declaration is actually a type cast
      if (!strncmp(IDENT_STR(SyntaxStack[synPtr].param), "(something", sizeof("(something)") - 1 - 1)) {
        int castSize;
        
        if (SyntaxStack[++synPtr].tok == tokLocalOfs) // TBD!!! is this really needed???
          synPtr++;
        
        s = exprval(idx, ExprTypeSynPtr, ConstExpr);
        
        // can't cast void or structure/union to anything (except void)
        if (*ExprTypeSynPtr >= 0 &&
            (SyntaxStack[*ExprTypeSynPtr].tok == tokVoid ||
             SyntaxStack[*ExprTypeSynPtr].tok == tokStructPtr) &&
            SyntaxStack[synPtr].tok != tokVoid)
          ERROR(0x0203, FALSE);
        
        // can't cast to function, array or structure/union
        if (SyntaxStack[synPtr].tok == '(' ||
            SyntaxStack[synPtr].tok == '[' ||
            SyntaxStack[synPtr].tok == tokStructPtr)
          ERROR(0x0203, FALSE);
        
        // will try to propagate constants through casts
        if (!*ConstExpr &&
            (stack[oldIdxRight - (oldSpRight - sp)].tok == tokNumInt ||
             stack[oldIdxRight - (oldSpRight - sp)].tok == tokNumUint)) {
          s = (int) stack[oldIdxRight - (oldSpRight - sp)].param;
          *ConstExpr = 1;
        }
        
        castSize = GetDeclSize(synPtr, 1);
        
        // insertion of tokUChar, tokSChar and tokUnaryPlus transforms
        // lvalues (values formed by dereferences) into rvalues
        // (by hiding the dereferences), just as casts should do
        switch (castSize) {
          case 1:
            // cast to unsigned char
            stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokUChar;
            s &= 0xFF;
            break;
          case -1:
            // cast to signed char
            stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokSChar;
            if ((s &= 0xFF) >= 0x80)
              s -= 0x100;
            break;
          default:
            if (castSize && castSize != SizeOfWord && -castSize != SizeOfWord) {
              if (castSize == 2) {
                // cast to unsigned short
                stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokUShort;
                s &= 0xFFFF;
              } else {
                // cast to signed short
                stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokShort;
                if ((s &= 0xFFFF) >= 0x8000)
                  s -= 0x10000;
              }
            } else {
              // cast to int/unsigned/pointer
              if (stack[oldIdxRight - (oldSpRight - sp)].tok == tokUnaryStar)
                // hide the dereference
                stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokUnaryPlus;
              else
                del(oldIdxRight + 1 - (oldSpRight - sp), 1);
            }
            break;
          }
          
          switch (SyntaxStack[synPtr].tok) {
            case tokChar:
            case tokWChar:
            case tokSChar:
            case tokUChar:
            case tokShort:
            case tokUShort:
            case tokInt:
            case tokUnsigned:
              break;
            default:
              *ConstExpr = 0;
              break;
          }

          *ExprTypeSynPtr = synPtr;
          simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
          break;
        }
        
        type = SymType(synPtr);
        if (type == SymLocalVar || type == SymLocalArr) {
          // replace local variables/arrays with their local addresses
          // (global variables/arrays' addresses are their names)
          stack[*idx + 1].tok = tokLocalOfs;
          stack[*idx + 1].param = SyntaxStack[synPtr + 1].param;
          stack[*idx + 1].flags = SyntaxStack[synPtr + 1].flags;
        }
        if (type == SymLocalVar || type == SymGlobalVar) {
          // add implicit dereferences for local/global variables
          ins3(*idx + 2, tokUnaryStar, GetDeclSize(synPtr, 1), SyntaxStack[synPtr + 1].flags);
        }
        
        // return the identifier's type
        while (SyntaxStack[synPtr].tok == tokIdent || SyntaxStack[synPtr].tok == tokLocalOfs)
          synPtr++;
        *ExprTypeSynPtr = synPtr;
      }
      *ConstExpr = 0;
      break;

    // sizeof operator
    case tokSizeof:
      s = exprval(idx, ExprTypeSynPtr, ConstExpr);

      if (*ExprTypeSynPtr >= 0)
        s = GetDeclSize(*ExprTypeSynPtr, 0);
      else
        s = SizeOfWord;
      if (s == 0)
        ERROR(0x0018, FALSE);

      // replace sizeof with its numeric value
      stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokNumUint;
      stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) s;
      stack[oldIdxRight + 1 - (oldSpRight - sp)].flags = 0;
      
      // remove the sizeof's subexpression
      del(*idx + 1, oldIdxRight - (oldSpRight - sp) - *idx);

      *ExprTypeSynPtr = SymUintSynPtr;
      *ConstExpr = 1;
      break;

    // Address unary operator
    case tokUnaryAnd:
      exprval(idx, ExprTypeSynPtr, ConstExpr);
      
      if (*ExprTypeSynPtr >= 0 && SyntaxStack[*ExprTypeSynPtr].tok == '[') {
        // convert an array into a pointer to the array,
        // remove the reference
        *ExprTypeSynPtr = -*ExprTypeSynPtr;
        del(oldIdxRight + 1 - (oldSpRight - sp), 1);
      } else if (*ExprTypeSynPtr >= 0 && SyntaxStack[*ExprTypeSynPtr].tok == '(') {
        // convert a function into a pointer to the function,
        // remove the reference
        *ExprTypeSynPtr = -*ExprTypeSynPtr;
        del(oldIdxRight + 1 - (oldSpRight - sp), 1);
      } else if (*ExprTypeSynPtr >= 0 &&
               oldIdxRight - (oldSpRight - sp) >= 0 &&
               stack[oldIdxRight - (oldSpRight - sp)].tok == tokUnaryStar) {
        // it's an lvalue (with implicit or explicit dereference),
        // convert it into its address,
        // collapse/remove the reference and the dereference
        *ExprTypeSynPtr = -*ExprTypeSynPtr;
        del(oldIdxRight - (oldSpRight - sp), 2);
      } else
        ERROR(0x0280, FALSE);
      
      *ConstExpr = 0;
      break;

    // Indirection unary operator
    case tokUnaryStar:
      exprval(idx, ExprTypeSynPtr, ConstExpr);
      if ((*ExprTypeSynPtr < 0) || (SyntaxStack[*ExprTypeSynPtr].tok == '*')) {
        // type is a pointer to something,
        // transform it into that something
        if (*ExprTypeSynPtr < 0)
          *ExprTypeSynPtr = -*ExprTypeSynPtr;
        else
          ++*ExprTypeSynPtr;
        nonVoidTypeCheck(*ExprTypeSynPtr);
        if ((SyntaxStack[*ExprTypeSynPtr].tok == tokStructPtr) && !GetDeclSize(*ExprTypeSynPtr, 0))
          // incomplete structure/union type
          ERROR(0x0203, FALSE);
        // remove the dereference if that something is an array or a function
        if ((SyntaxStack[*ExprTypeSynPtr].tok == '[') || (SyntaxStack[*ExprTypeSynPtr].tok == '('))
          del(oldIdxRight + 1 - (oldSpRight - sp), 1);
        // else add dereference size in bytes
        else {
          stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) GetDeclSize(*ExprTypeSynPtr, 1);
          stack[oldIdxRight + 1 - (oldSpRight - sp)].flags = SyntaxStack[*ExprTypeSynPtr].flags;
        }
      } else if (SyntaxStack[*ExprTypeSynPtr].tok == '[') {
        // type is an array,
        // transform it into the array's first element
        // (a subarray, if type is a multidimensional array)
        while (SyntaxStack[*ExprTypeSynPtr].tok != ']')
          ++*ExprTypeSynPtr;
        ++*ExprTypeSynPtr;
        // remove the dereference if that element is an array
        if (SyntaxStack[*ExprTypeSynPtr].tok == '[')
          del(oldIdxRight + 1 - (oldSpRight - sp), 1);
        // else add dereference size in bytes
        else {
          stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) GetDeclSize(*ExprTypeSynPtr, 1);
          stack[oldIdxRight + 1 - (oldSpRight - sp)].flags = SyntaxStack[*ExprTypeSynPtr].flags;
        }
      } else
        ERROR(0x0204, FALSE);
      
      *ConstExpr = 0;
      break;

    // Additive binary operators
    case '+':
    case '-': {
    // WRONGISH: DONE: replace prefix ++/-- with +=1/-=1
    // WRONG: DONE: replace postfix ++/-- with (+=1)-1/(-=1)+1
      int ptrmask;
      int oldIdxLeft, oldSpLeft;
      int sl, sr;
      int incSize;
      sr = exprval(idx, &RightExprTypeSynPtr, &constExpr[1]);
      oldIdxLeft = *idx;
      oldSpLeft = sp;
      sl = exprval(idx, ExprTypeSynPtr, &constExpr[0]);

      if (tok == '+')
        s = (int)((unsigned)sl + sr);
      else
        s = (int)((unsigned)sl - sr);
      
      if (scalarTypeCheck(RightExprTypeSynPtr)) ERROR(0x0203, FALSE);
      if (scalarTypeCheck(*ExprTypeSynPtr)) ERROR(0x0203, FALSE);
      
      // Decay arrays to pointers to their first elements
      decayArray(&RightExprTypeSynPtr, 1);
      decayArray(ExprTypeSynPtr, 1);
      
      ptrmask = (RightExprTypeSynPtr < 0) + (*ExprTypeSynPtr < 0) * 2;
      
      // index/subscript scaling
      if (ptrmask == 1 && tok == '+') { // pointer in right-hand expression
        incSize = GetDeclSize(-RightExprTypeSynPtr, 0);
        
        if (constExpr[0]) { // integer constant in left-hand expression
          s = (int)((unsigned)sl * incSize);
          stack[oldIdxLeft - (oldSpLeft - sp)].param = (bit32u) s;
          // optimize a little if possible
          {
            int i = oldIdxRight - (oldSpRight - sp);
            // Skip any type cast markers
            while (stack[i].tok == tokUnaryPlus || stack[i].tok == '+')
              i--;
            // See if the pointer is an integer constant or a local variable offset
            // and if it is, adjust it here instead of generating code for
            // addition/subtraction
            if (stack[i].tok == tokNumInt || stack[i].tok == tokNumUint || stack[i].tok == tokLocalOfs) {
              s = (int)((unsigned) stack[i].param + s);
              stack[i].param = (bit32u) s; // TBD!!! need extra truncation?
              del(oldIdxLeft - (oldSpLeft - sp), 1);
              del(oldIdxRight - (oldSpRight - sp) + 1, 1);
            }
          }
        } else if (incSize != 1) {
          ins2(oldIdxLeft + 1 - (oldSpLeft - sp), tokNumInt, incSize);
          ins(oldIdxLeft + 1 - (oldSpLeft - sp), '*');
        }
        
        *ExprTypeSynPtr = RightExprTypeSynPtr;
      } else if (ptrmask == 2) { // pointer in left-hand expression
        incSize = GetDeclSize(-*ExprTypeSynPtr, 0);
        if (constExpr[1]) { // integer constant in right-hand expression
          s = (int) ((unsigned) sr * incSize);
          stack[oldIdxRight - (oldSpRight - sp)].param = (bit32u) s;
          // optimize a little if possible
          {
            int i = oldIdxLeft - (oldSpLeft - sp);
            // Skip any type cast markers
            while (stack[i].tok == tokUnaryPlus || stack[i].tok == '+')
              i--;
            // See if the pointer is an integer constant or a local variable offset
            // and if it is, adjust it here instead of generating code for
            // addition/subtraction
            if ((stack[i].tok == tokNumInt) || (stack[i].tok == tokNumUint) || (stack[i].tok == tokLocalOfs)) {
              if (tok == '-')
                s = (int) ~(s - 1u);
              s = (int) ((unsigned) stack[i].param + s);
              stack[i].param = (bit32u) s; // TBD!!! need extra truncation?
              del(oldIdxRight - (oldSpRight - sp), 2);
            }
          }
        } else if (incSize != 1) {
          ins2(oldIdxRight + 1 - (oldSpRight - sp), tokNumInt, incSize);
          ins(oldIdxRight + 1 - (oldSpRight - sp), '*');
        }
      } else if (ptrmask == 3 && tok == '-') { // pointers in both expressions
        incSize = GetDeclSize(-*ExprTypeSynPtr, 0);
        // TBD!!! "ptr1-ptr2": better pointer type compatibility test needed, like compatCheck()?
        if (incSize != GetDeclSize(-RightExprTypeSynPtr, 0))
          ERROR(0x0205, FALSE);
        if (incSize != 1) {
          ins2(oldIdxRight + 2 - (oldSpRight - sp), tokNumInt, incSize);
          ins(oldIdxRight + 2 - (oldSpRight - sp), '/');
        }
        *ExprTypeSynPtr = SymIntSynPtr;
      } else if (ptrmask)
        ERROR(0x0206, FALSE);
      
      // Promote the result from char to int (and from int to unsigned) if necessary
      promoteType(ExprTypeSynPtr, &RightExprTypeSynPtr);
      
      *ConstExpr = constExpr[0] && constExpr[1];
      simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
    } break;

    // Prefix/postfix increment/decrement unary operators
    case tokInc:
    case tokDec:
    case tokPostInc:
    case tokPostDec: {
      int incSize = 1;
      int inc = tok == tokInc || tok == tokPostInc;
      int post = tok == tokPostInc || tok == tokPostDec;
      int opSize;
      bit32u flags = 0;

      exprval(idx, ExprTypeSynPtr, ConstExpr);

      if (scalarTypeCheck(*ExprTypeSynPtr)) ERROR(0x0203, FALSE);

      decayArray(ExprTypeSynPtr, 1);

      // lvalue check for ++, --
      if (!(((oldIdxRight - (oldSpRight - sp)) >= 0) && (stack[oldIdxRight - (oldSpRight - sp)].tok == tokUnaryStar)))
        ERROR(0x0281, FALSE);
      
      // "remove" the lvalue dereference as we don't need
      // to read the value and forget its location. We need to
      // keep the lvalue location.
      // Remember the operand size.
      opSize = (int) stack[oldIdxRight - (oldSpRight - sp)].param;
      flags = stack[oldIdxRight - (oldSpRight - sp)].flags;
      del(oldIdxRight - (oldSpRight - sp), 1);
      
      if (*ExprTypeSynPtr < 0)
        incSize = GetDeclSize(-*ExprTypeSynPtr, 0);

      if (incSize == 1) {
        // store the operand size in the operator
        stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) opSize;
        stack[oldIdxRight + 1 - (oldSpRight - sp)].flags = flags;
      } else {
        // replace ++/-- with "postfix" +=/-= incSize when incSize != 1
        if (inc) {
          if (post) stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokPostAdd;
          else      stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokAssignAdd;
        } else {
          if (post) stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokPostSub;
          else      stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokAssignSub;
        }
        // store the operand size in the operator
        stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) opSize;
        stack[oldIdxRight + 1 - (oldSpRight - sp)].flags = flags;
        ins2(oldIdxRight + 1 - (oldSpRight - sp), tokNumInt, incSize);
      }
      
      *ConstExpr = 0;
    } break;
    
    // Simple assignment binary operator
    case '=': {
      int oldIdxLeft, oldSpLeft;
      int opSize;
      int structs;
      bit32u flags;
      struct S_IDENT_PREFIX *p;
      
      exprval(idx, &RightExprTypeSynPtr, ConstExpr);
      
      // mark the Ident as being initialized
      p = (struct S_IDENT_PREFIX *) stack[*idx].param;
      if (p && (stack[*idx].tok == tokIdent)) {
      //  if (p->flags & IDENT_FLAGS_ISCONST)
      //    ERROR(0x016C, TRUE, IDENT_STR(p));
      //  else
          p->flags |= IDENT_FLAGS_ASSIGNED;
      }
      
      oldIdxLeft = *idx;
      oldSpLeft = sp;
      exprval(idx, ExprTypeSynPtr, ConstExpr);
      
      nonVoidTypeCheck(RightExprTypeSynPtr);
      nonVoidTypeCheck(*ExprTypeSynPtr);
      
      decayArray(&RightExprTypeSynPtr, 0);
      decayArray(ExprTypeSynPtr, 0);
      
      // is the param an lvalue?
      if (!(((oldIdxLeft - (oldSpLeft - sp)) >= 0) && (stack[oldIdxLeft - (oldSpLeft - sp)].tok == tokUnaryStar)))
        ERROR(0x0282, FALSE);
      
      structs = ((RightExprTypeSynPtr >= 0) && (SyntaxStack[RightExprTypeSynPtr].tok == tokStructPtr)) +
              ((*ExprTypeSynPtr >= 0) && (SyntaxStack[*ExprTypeSynPtr].tok == tokStructPtr)) * 2;
      if (structs) {
        int sz;
        
        if ((structs != 3) || (SyntaxStack[RightExprTypeSynPtr].param != SyntaxStack[*ExprTypeSynPtr].param))
          ERROR(0x0203, FALSE);
        
        // TBD??? (a = b) should be an rvalue and so &(a = b) and (&(a = b))->c shouldn't be
        // allowed, while (a = b).c should be allowed.
        
        // transform "*psleft = *psright" into "*fxn(sizeof *psright, psright, psleft)"
/*      
        if (stack[oldIdxLeft - (oldSpLeft - sp)].tok != tokUnaryStar ||
            stack[oldIdxRight - (oldSpRight - sp)].tok != tokUnaryStar)
          ERROR(0xE005, FALSE);
*/      
        stack[oldIdxLeft - (oldSpLeft - sp)].tok = ','; // replace '*' with ','
        stack[oldIdxRight - (oldSpRight - sp)].tok = ','; // replace '*' with ','
        
        sz = GetDeclSize(RightExprTypeSynPtr, 0);
        
        stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokNumUint; // replace '=' with "sizeof *psright"
        stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) sz;
        ins(oldIdxRight + 2 - (oldSpRight - sp), ',');
        
        if (SizeOfWord == 2) {
          if (!StructCpyLabel16)
            StructCpyLabel16 = LabelCnt++;
          ins2(oldIdxRight + 2 - (oldSpRight - sp), tokIdent, (bit32u) AddNumericIdent(StructCpyLabel16));
        } else {
          if (!StructCpyLabel32)
            StructCpyLabel32 = LabelCnt++;
          ins2(oldIdxRight + 2 - (oldSpRight - sp), tokIdent, (bit32u) AddNumericIdent(StructCpyLabel32));
        }
        
        ins2(oldIdxRight + 2 - (oldSpRight - sp), ')', SizeOfWord * 3);
        ins2(oldIdxRight + 2 - (oldSpRight - sp), tokUnaryStar, 0); // use 0 deref size to drop meaningless dereferences
        
        ins2(*idx + 1, '(', SizeOfWord * 3);
      } else {
        // "remove" the lvalue dereference as we don't need
        // to read the value and forget its location. We need to
        // keep the lvalue location.
        opSize = (int) stack[oldIdxLeft - (oldSpLeft - sp)].param;
        flags = stack[oldIdxLeft - (oldSpLeft - sp)].flags;
        
        // store the operand size in the operator
        stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) opSize;
        stack[oldIdxRight + 1 - (oldSpRight - sp)].flags = flags;
        del(oldIdxLeft - (oldSpLeft - sp), 1);
      }
      
      *ConstExpr = 0;
    } break;

    // other assignment operators
    
    // Arithmetic and bitwise unary operators
    case '~':
    case tokUnaryPlus:
    case tokUnaryMinus:
      s = exprval(idx, ExprTypeSynPtr, ConstExpr);
      if (scalarTypeCheck(*ExprTypeSynPtr)) ERROR(0x0203, FALSE);
      numericTypeCheck(*ExprTypeSynPtr, tok);
      switch (tok) {
      case '~':           s = ~s; break;
      case tokUnaryPlus:  s = +s; break;
      case tokUnaryMinus: s = (int) ~(s - 1u); break;
      }
      promoteType(ExprTypeSynPtr, ExprTypeSynPtr);
      simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
      break;

    // Arithmetic and bitwise binary operators
    case '*':
    case '/':
    case '%':
    case tokLShift:
    case tokRShift:
    case '&':
    case '^':
    case '|':
      {
        // int oldIdxLeft, oldSpLeft;
        int sr, sl;
        int Unsigned;
        sr = exprval(idx, &RightExprTypeSynPtr, &constExpr[1]);
        // oldIdxLeft = *idx;
        // oldSpLeft = sp;
        sl = exprval(idx, ExprTypeSynPtr, &constExpr[0]);

        if (scalarTypeCheck(RightExprTypeSynPtr)) ERROR(0x0203, FALSE);
        if (scalarTypeCheck(*ExprTypeSynPtr)) ERROR(0x0203, FALSE);

        numericTypeCheck(RightExprTypeSynPtr, tok);
        numericTypeCheck(*ExprTypeSynPtr, tok);

        *ConstExpr = constExpr[0] && constExpr[1];

        Unsigned = SyntaxStack[*ExprTypeSynPtr].tok == tokUnsigned || SyntaxStack[RightExprTypeSynPtr].tok == tokUnsigned;

        switch (tok)
        {
        // DONE: check for division overflows
        case '/':
        case '%':
          *ConstExpr &= divCheckAndCalc(tok, &sl, sr, Unsigned, constExpr);

          if (Unsigned) {
            if (tok == '/')
              stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokUDiv;
            else
              stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokUMod;
          }
          break;

        case '*':
          sl = (int) ((unsigned) sl * sr);
          break;

        case tokLShift:
        case tokRShift:
          if (constExpr[1]) {
            if (SyntaxStack[RightExprTypeSynPtr].tok != tokUnsigned)
              sr = truncInt(sr);
            else
              sr = (int) truncUint(sr);
            shiftCountCheck(&sr, oldIdxRight - (oldSpRight - sp), RightExprTypeSynPtr);
          }
          if (*ConstExpr) {
            if (tok == tokLShift) {
              // left shift is the same for signed and unsigned ints
              sl = (int)((unsigned) sl << sr);
            } else {
              if (SyntaxStack[*ExprTypeSynPtr].tok == tokUnsigned) {
                // right shift for unsigned ints
                sl = (int) (truncUint(sl) >> sr);
              } else if (sr) {
                // right shift for signed ints is arithmetic, sign-bit-preserving
                // don't depend on the compiler's implementation, do it "manually"
                sl = truncInt(sl);
                sl = (int) ((truncUint(sl) >> sr) |
                              ((sl < 0) * (~0u << (8 * SizeOfWord - sr))));
              }
            }
          }

          if (SyntaxStack[*ExprTypeSynPtr].tok == tokUnsigned && tok == tokRShift)
              stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokURShift;

          // ignore RightExprTypeSynPtr for the purpose of promotion/conversion of the result of <</>>
          RightExprTypeSynPtr = SymIntSynPtr;
          break;

        case '&': sl &= sr; break;
        case '^': sl ^= sr; break;
        case '|': sl |= sr; break;
        }

        s = sl;
        promoteType(ExprTypeSynPtr, &RightExprTypeSynPtr);
        simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
      }
      break;

    // Relational binary operators
    // DONE: add (sub)tokens for unsigned >, >=, <, <= for pointers
    case '<':
    case '>':
    case tokLEQ:
    case tokGEQ:
    case tokEQ:
    case tokNEQ:
      {
        int ptrmask;
        int sr, sl;
        int Unsigned;
        sr = exprval(idx, &RightExprTypeSynPtr, &constExpr[1]);
        sl = exprval(idx, ExprTypeSynPtr, &constExpr[0]);

        if (scalarTypeCheck(RightExprTypeSynPtr)) ERROR(0x0203, FALSE);
        if (scalarTypeCheck(*ExprTypeSynPtr)) ERROR(0x0203, FALSE);

        decayArray(&RightExprTypeSynPtr, 0);
        decayArray(ExprTypeSynPtr, 0);

        ptrmask = (RightExprTypeSynPtr < 0) + (*ExprTypeSynPtr < 0) * 2;

        // Disallow >, <, >=, <= between a pointer and a number
        if (ptrmask >= 1 && ptrmask <= 2 &&
            tok != tokEQ && tok != tokNEQ)
          ERROR(0x0207, FALSE);

        *ConstExpr = constExpr[0] && constExpr[1];

        Unsigned = !ptrmask &&
          (SyntaxStack[*ExprTypeSynPtr].tok == tokUnsigned || SyntaxStack[RightExprTypeSynPtr].tok == tokUnsigned);

        if (*ConstExpr) {
          if (!Unsigned) {
            sl = truncInt(sl);
            sr = truncInt(sr);
            switch (tok)
            {
            case '<':    sl = sl <  sr; break;
            case '>':    sl = sl >  sr; break;
            case tokLEQ: sl = sl <= sr; break;
            case tokGEQ: sl = sl >= sr; break;
            case tokEQ:  sl = sl == sr; break;
            case tokNEQ: sl = sl != sr; break;
            }
          }
          else
          {
            sl = (int) truncUint(sl);
            sr = (int) truncUint(sr);
            switch (tok)
            {
            case '<':    sl = sl + 0u <  sr + 0u; break;
            case '>':    sl = sl + 0u >  sr + 0u; break;
            case tokLEQ: sl = sl + 0u <= sr + 0u; break;
            case tokGEQ: sl = sl + 0u >= sr + 0u; break;
            case tokEQ:  sl = sl == sr; break;
            case tokNEQ: sl = sl != sr; break;
            }
          }
        }

        if (ptrmask || Unsigned) {
          // Pointer comparison should be unsigned
          int t = tok;
          switch (tok) {
            case '<': t = tokULess; break;
            case '>': t = tokUGreater; break;
            case tokLEQ: t = tokULEQ; break;
            case tokGEQ: t = tokUGEQ; break;
          }
          if (t != tok)
            stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = t;
        }

        s = sl;
        *ExprTypeSynPtr = SymIntSynPtr;
        simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
      }
      break;

    // implicit pseudo-conversion to _Bool of operands of && and ||
    case tok_Bool:
      s = exprval(idx, ExprTypeSynPtr, ConstExpr);
      s = truncInt(s) != 0;
      if (scalarTypeCheck(*ExprTypeSynPtr)) ERROR(0x0203, FALSE);
      decayArray(ExprTypeSynPtr, 0);
      *ExprTypeSynPtr = SymIntSynPtr;
      simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
      break;

    // Logical binary operators
    case tokLogAnd: // DONE: short-circuit
    case tokLogOr: // DONE: short-circuit
      {
        int sr, sl;

        // DONE: think of pushing a special short-circuit (jump-to) token
        // to skip the rhs operand evaluation in && and ||
        // DONE: add implicit "casts to _Bool" of && and || operands,
        // do the same for control statements of if() while() and for(;;).

        int sc = LabelCnt++;
        // tag the logical operator as a numbered short-circuit jump target
        stack[*idx + 1].param = (int) sc;

        // insert "!= 0" for right-hand operand
        switch (stack[*idx].tok)
        {
        case '<':
        case tokULess:
        case '>':
        case tokUGreater:
        case tokLEQ:
        case tokULEQ:
        case tokGEQ:
        case tokUGEQ:
        case tokEQ:
        case tokNEQ:
          break;
        default:
          ins(++*idx, tok_Bool);
          break;
        }

        sr = exprval(idx, &RightExprTypeSynPtr, &constExpr[1]);

        // insert a reference to the short-circuit jump target
        if (tok == tokLogAnd)
          ins2(++*idx, tokShortCirc, sc);
        else
          ins2(++*idx, tokShortCirc, -sc);
        // insert "!= 0" for left-hand operand
        switch (stack[*idx - 1].tok)
        {
        case '<':
        case tokULess:
        case '>':
        case tokUGreater:
        case tokLEQ:
        case tokULEQ:
        case tokGEQ:
        case tokUGEQ:
        case tokEQ:
        case tokNEQ:
          --*idx;
          break;
        default:
          ins(*idx, tok_Bool);
          break;
        }

        sl = exprval(idx, ExprTypeSynPtr, &constExpr[0]);

        if (tok == tokLogAnd)
          s = sl && sr;
        else
          s = sl || sr;

        *ExprTypeSynPtr = SymIntSynPtr;
        *ConstExpr = constExpr[0] && constExpr[1];
        if (constExpr[0]) {
          if (tok == tokLogAnd) {
            if (!sl)
              *ConstExpr = 1, s = 0;
            // TBD??? else can drop LHS expression
          }
          else
          {
            if (sl)
              *ConstExpr = s = 1;
            // TBD??? else can drop LHS expression
          }
        }
        simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
      }
      break;

    // Function call
    case ')': {
        int tmpSynPtr, c;
        int minParams, maxParams;
        int firstParamSynPtr;
        int oldIdx, oldSp;
        unsigned structSize = 0;
        int retStruct = 0;
        int retOfs = 0;
        
        exprval(idx, ExprTypeSynPtr, ConstExpr);
        
        if (!GetFxnInfo(*ExprTypeSynPtr, &minParams, &maxParams, ExprTypeSynPtr, &firstParamSynPtr))
          ERROR(0x0208, FALSE);

        // DONE: validate the number of function parameters
        // TBD??? warnings/errors on int<->pointer substitution in params
        
        // If a structure is returned, allocate space for it on the stack
        // and pass its location as the first (implicit) argument.
        if (ParseLevel &&
            *ExprTypeSynPtr >= 0 &&
            SyntaxStack[*ExprTypeSynPtr].tok == tokStructPtr) {
          unsigned sz = GetDeclSize(*ExprTypeSynPtr, 0);
          // Make sure the return structure type is complete
          if (!sz)
            ERROR(0x0203, FALSE);
          retOfs = AllocLocal(sz);
          // Transform fxn(args) into fxn(pretval, args)
          ins(*idx + 1, ',');
          ins2(*idx + 1, tokLocalOfs, retOfs);
          retStruct = 1;
        }
        
        // evaluate function parameters
        c = 0;
        while (stack[*idx].tok != '(') {
          int gotStructs;
          // add a comma after the first (last to be pushed) parameter,
          // so all parameters can be pushed whenever a comma is encountered
          if (!c)
            ins(*idx + 1, ',');

          oldIdx = *idx;
          oldSp = sp;
          
          exprval(idx, &tmpSynPtr, ConstExpr);
          if (nonVoidTypeCheck(tmpSynPtr)) ERROR(0x020A, FALSE);
          // If the argument is a structure, push it by calling a dedicated function
          gotStructs = tmpSynPtr >= 0 && SyntaxStack[tmpSynPtr].tok == tokStructPtr;
          if (gotStructs) {
            unsigned sz = GetDeclSize(tmpSynPtr, 0);
            int i = oldIdx - (oldSp - sp);
            stack[i].tok = ')';
            stack[i].param = SizeOfWord * 2;
            stack[i].flags = 0;
            
            if (!StructPushLabel)
              StructPushLabel = LabelCnt++;
            
            // The code generator expects functions to return values.
            // If a function argument is a value produced by another function,
            // as is the case here, the code generator will naturally
            // want/need to push something of the size of the machine word.
            // This works perfectly with non-structures.
            // But we only want to push the structure without pushing any other words.
            // In order to avoid involving changes in the code generator,
            // we make the function that pushes structures onto the stack
            // push all words but the first one. The dedicated function will
            // return this word and the code generator will push it.
            // This is ugly.
            
            ins2(i, tokIdent, (bit32u) AddNumericIdent(StructPushLabel));
            ins(i, ',');
            i = *idx + 1;
            ins(i, ',');
            ins2(i, tokNumUint, (int)sz);
            ins2(i, '(', SizeOfWord * 2);
            
            if (sz > (unsigned) GenMaxLocalsSize())
              ERROR(0x02A0, FALSE);
            // Structures will be padded to machine word boundary when pushed
            sz = (sz + SizeOfWord - 1) & ~(SizeOfWord - 1u);
            // Count the cumulative size of the pushed structures, excluding
            // the first words that will be pushed by the code generator
            if (structSize + sz < structSize)
              ERROR(0x02A0, FALSE);
            structSize += sz - SizeOfWord;
            if (structSize > (unsigned)GenMaxLocalsSize())
              ERROR(0x02A0, FALSE);
            // TBD??? complete overflow checks (an expression may contain more than one call)?
          }
          
          if (++c > maxParams)
            ERROR(0x02A0, FALSE);
          
          // Issue a warning if the argument has to be a pointer but isn't and vice versa.
          // DONE: struct type compat checks
          // TBD??? Compare pointer types deeply as in compatCheck()???
          // TBD??? Issue a similar warning for return values and initializers???
          if (c <= minParams) {
            int t;
            int gotPtr = tmpSynPtr < 0;
            int needPtr;
            if (!gotPtr) {
              t = SyntaxStack[tmpSynPtr].tok;
              gotPtr = (t == '*') | (t == '[') | (t == '('); // arrays and functions decay to pointers
            }
            // Find the type of the formal parameter in the function declaration
            while ((t = SyntaxStack[firstParamSynPtr].tok) != tokIdent) {
              if (t == '(') {
                // skip parameters in parameters
                int c = 1;
                while (c) {
                  t = SyntaxStack[++firstParamSynPtr].tok;
                  c += (t == '(') - (t == ')');
                }
              }
              firstParamSynPtr++;
            }
            firstParamSynPtr++;
            gotStructs += (SyntaxStack[firstParamSynPtr].tok == tokStructPtr) * 2;
            if (gotStructs) {
              // Structures must be of the same type
              if (gotStructs != 3 || SyntaxStack[tmpSynPtr].param != SyntaxStack[firstParamSynPtr].param)
                ERROR(0x0203, FALSE);
            }
            needPtr = SyntaxStack[firstParamSynPtr].tok == '*';
            if (needPtr != gotPtr &&
                // Make an exception for integer constants equal to 0, treat them as NULL pointers
                !(
                   needPtr &&
                   *ConstExpr &&
                   !stack[*idx + 1].param
                 )
               )
              warning(0x0179, 0, (const int) (needPtr ? "" : "non-"), c);
          }
          
          if (stack[*idx].tok == ',')
            --*idx;
        }
        --*idx;

        if (c < minParams)
          ERROR(0x02A1, FALSE);
        
        // store the cumulative parameter size in the function call operators
        //stack[1 + *idx].param = stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) (c * SizeOfWord);
        {
          int i = oldIdxRight + 1 - (oldSpRight - sp);
          // Count the implicit param/arg for returned structure
          c += retStruct;
          // Correct the value by which the stack pointer
          // will be incremented after the call
          c += structSize / SizeOfWord;
          stack[1 + *idx].param = stack[i].param = c * SizeOfWord;
          // If a structure is returned, transform
          // fxn(pretval, args) into *(fxn(pretval, args), pretval)
          if (retStruct) {
            ins(i + 1, tokUnaryStar);
            ins(i + 1, tokComma);
            ins2(i + 1, tokLocalOfs, retOfs);
            ins(i + 1, tokVoid);
          }
        }
        
        *ConstExpr = 0;
      }
      break;

    // Binary comma operator
    case tokComma: {
        int oldIdxLeft, oldSpLeft;
        int retStruct = 0;
        s = exprval(idx, &RightExprTypeSynPtr, &constExpr[1]);
        oldIdxLeft = *idx;
        oldSpLeft = sp;

        // Signify uselessness of the result of the left operand's value
        ins(*idx + 1, tokVoid);

        exprval(idx, ExprTypeSynPtr, &constExpr[0]);
        *ConstExpr = constExpr[0] && constExpr[1];
        *ExprTypeSynPtr = RightExprTypeSynPtr;
        retStruct = RightExprTypeSynPtr >= 0 && SyntaxStack[RightExprTypeSynPtr].tok == tokStructPtr;
        if (*ConstExpr) {
          // both subexprs are const, remove both and comma
          simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
        } else if (constExpr[0]) {
          // only left subexpr is const, remove it and comma
          del(*idx + 1, oldIdxLeft - (oldSpLeft - sp) - *idx);
          if (!retStruct)
            // Ensure non-lvalue-ness of the result by changing comma to unary plus
            // and thus hiding dereference, if any
            stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = tokUnaryPlus;
          else
            // However, (something, struct).member should still be allowed,
            // so, comma needs to produce lvalue
            del(oldIdxRight + 1 - (oldSpRight - sp), 1);
        } else if (retStruct) {
          // However, (something, struct).member should still be allowed,
          // so, comma needs to produce lvalue. Swap comma and structure dereference.
          int i = oldIdxRight + 1 - (oldSpRight - sp);
          stack[i].tok = tokUnaryStar;
          stack[i].param = stack[i - 1].param;
          stack[i - 1].tok = tokComma;
        }
      }
      break;

    // Compound assignment operators
    case tokAssignMul: case tokAssignDiv: case tokAssignMod:
    case tokAssignAdd: case tokAssignSub:
    case tokAssignLSh: case tokAssignRSh:
    case tokAssignAnd: case tokAssignXor: case tokAssignOr: {
        int ptrmask;
        int oldIdxLeft, oldSpLeft;
        int incSize, opSize, Unsigned;
        int sr = exprval(idx, &RightExprTypeSynPtr, &constExpr[1]);
        bit32u flags = 0;
        
        oldIdxLeft = *idx;
        oldSpLeft = sp;
        exprval(idx, ExprTypeSynPtr, &constExpr[0]);
        
        if (scalarTypeCheck(RightExprTypeSynPtr)) ERROR(0x0203, FALSE);
        if (scalarTypeCheck(*ExprTypeSynPtr)) ERROR(0x0203, FALSE);
        
        decayArray(&RightExprTypeSynPtr, 1);
        decayArray(ExprTypeSynPtr, 1);
        
        if (!(oldIdxLeft - (oldSpLeft - sp) >= 0 && stack[oldIdxLeft - (oldSpLeft - sp)].tok == tokUnaryStar))
          ERROR(0x0283, FALSE, GetTokenName(tok));
        
        // "remove" the lvalue dereference as we don't need
        // to read the value and forget its location. We need to
        // keep the lvalue location.
        opSize = (int) stack[oldIdxLeft - (oldSpLeft - sp)].param;
        flags = stack[oldIdxLeft - (oldSpLeft - sp)].flags;
        
        // store the operand size in the operator
        stack[oldIdxRight + 1 - (oldSpRight - sp)].param = (bit32u) opSize;
        stack[oldIdxRight + 1 - (oldSpRight - sp)].flags = flags;
        del(oldIdxLeft - (oldSpLeft - sp), 1);
        
        ptrmask = (RightExprTypeSynPtr < 0) + (*ExprTypeSynPtr < 0) * 2;
        
        Unsigned = !ptrmask &&
          (SyntaxStack[*ExprTypeSynPtr].tok == tokUnsigned || SyntaxStack[RightExprTypeSynPtr].tok == tokUnsigned);
        
        if (tok != tokAssignAdd && tok != tokAssignSub) {
          if (ptrmask)
            ERROR(0x0209, FALSE, GetTokenName(tok));
        } else {
          // No pointer to the right of += and -=
          if (ptrmask & 1)
            ERROR(0x0209, FALSE, GetTokenName(tok));
        }
        
        if (tok == tokAssignLSh || tok == tokAssignRSh) {
          if (constExpr[1]) {
            if (SyntaxStack[RightExprTypeSynPtr].tok != tokUnsigned)
              sr = truncInt(sr);
            else
              sr = (int) truncUint(sr);
            shiftCountCheck(&sr, oldIdxRight - (oldSpRight - sp), RightExprTypeSynPtr);
          }
        }
        
        if (tok == tokAssignDiv || tok == tokAssignMod) {
          int t, sl = 0;
          if (tok == tokAssignDiv)
            t = '/';
          else
            t = '%';
          divCheckAndCalc(t, &sl, sr, 1, constExpr);
        }
        
        // TBD??? replace +=/-= with prefix ++/-- if incSize == 1
        if (ptrmask == 2) { // left-hand expression
          incSize = GetDeclSize(-*ExprTypeSynPtr, 0);
          if (constExpr[1]) {
            int t = (int) (stack[oldIdxRight - (oldSpRight - sp)].param * (unsigned) incSize);
            stack[oldIdxRight - (oldSpRight - sp)].param = (bit32u) t;
          } else if (incSize != 1) {
            ins2(oldIdxRight + 1 - (oldSpRight - sp), tokNumInt, incSize);
            ins(oldIdxRight + 1 - (oldSpRight - sp), '*');
          }
        } else if (Unsigned) {
          int t = tok;
          switch (tok) {
            case tokAssignDiv: t = tokAssignUDiv; break;
            case tokAssignMod: t = tokAssignUMod; break;
            case tokAssignRSh:
              if (SyntaxStack[*ExprTypeSynPtr].tok == tokUnsigned)
                t = tokAssignURSh;
              break;
          }
          if (t != tok)
            stack[oldIdxRight + 1 - (oldSpRight - sp)].tok = t;
        }

        *ConstExpr = 0;
      }
      break;

    // Ternary/conditional operator
    case '?': {
      int oldIdxLeft, oldSpLeft;
      int sr, sl, smid;
      int condTypeSynPtr;
      int sc = (LabelCnt += 2) - 2;
      int structs;

      // "exprL ? exprMID : exprR" appears on the stack as
      // "exprL exprR exprMID ?"

      stack[*idx + 1].tok = tokLogAnd; // piggyback on && for CG (ugly, but simple)
      stack[*idx + 1].param = (bit32u) (sc + 1);
      smid = exprval(idx, ExprTypeSynPtr, &constExpr[1]);

      ins2(*idx + 1, tokLogAnd, sc); // piggyback on && for CG (ugly, but simple)

      ins2(*idx + 1, tokGoto, sc + 1); // jump to end of ?:
      oldIdxLeft = *idx;
      oldSpLeft = sp;
      sr = exprval(idx, &RightExprTypeSynPtr, &constExpr[2]);

      ins2(*idx + 1, tokShortCirc, -sc); // jump to mid if left is non-zero
      sl = exprval(idx, &condTypeSynPtr, &constExpr[0]);

      if (scalarTypeCheck(condTypeSynPtr)) ERROR(0x0203, FALSE);

      decayArray(&RightExprTypeSynPtr, 0);
      decayArray(ExprTypeSynPtr, 0);
      promoteType(&RightExprTypeSynPtr, ExprTypeSynPtr);
      promoteType(ExprTypeSynPtr, &RightExprTypeSynPtr);

      // TBD??? move struct/union-related checks into compatChecks()

      structs = (RightExprTypeSynPtr >= 0 && SyntaxStack[RightExprTypeSynPtr].tok == tokStructPtr) +
              (*ExprTypeSynPtr >= 0 && SyntaxStack[*ExprTypeSynPtr].tok == tokStructPtr) * 2;
      if (structs) {
        if (structs != 3 ||
            SyntaxStack[RightExprTypeSynPtr].param != SyntaxStack[*ExprTypeSynPtr].param)
          ERROR(0x0203, FALSE);

        // transform "cond ? a : b" into "*(cond ? &a : &b)"
/*
        if (stack[oldIdxLeft - (oldSpLeft - sp)].tok != tokUnaryStar ||
            stack[oldIdxRight - (oldSpRight - sp)].tok != tokUnaryStar)
          ERROR(0xE006, FALSE);
*/
        del(oldIdxLeft - (oldSpLeft - sp), 1); // delete '*'
        del(oldIdxRight - (oldSpRight - sp), 1); // delete '*'
        ins2(oldIdxRight + 2 - (oldSpRight - sp), tokUnaryStar, 0); // use 0 deref size to drop meaningless dereferences
      } else {
        compatCheck(ExprTypeSynPtr,
                    RightExprTypeSynPtr,
                    &constExpr[1],
                    oldIdxRight - (oldSpRight - sp),
                    oldIdxLeft - (oldSpLeft - sp));
      }

      *ConstExpr = s = 0;

      if (constExpr[0]) {
        if (truncUint(sl)) {
          if (constExpr[1])
            *ConstExpr = 1, s = smid;
          // TBD??? else can drop LHS and RHS expressions
        } else {
          if (constExpr[2])
            *ConstExpr = 1, s = sr;
          // TBD??? else can drop LHS and MID expressions
        }
      }
      simplifyConstExpr(s, *ConstExpr, ExprTypeSynPtr, oldIdxRight + 1 - (oldSpRight - sp), *idx + 1);
    } break;
    
    // Postfix indirect structure/union member selection operator
    case tokArrow: {
      int i, j = 0, c = 1, ofs = 0;
      struct S_IDENT_PREFIX *member;
      bit32u sflags = 0;
      
      stack[*idx + 1].tok = '+'; // replace -> with +
      member = (struct S_IDENT_PREFIX *) stack[*idx].param; // keep the member name, it will be replaced with member offset
      stack[*idx].tok = tokNumInt;
      
      --*idx;
      exprval(idx, ExprTypeSynPtr, ConstExpr);
      
      decayArray(ExprTypeSynPtr, 0);
      
      //if ((*ExprTypeSynPtr >= 0) && (SyntaxStack[*ExprTypeSynPtr].tok == '*'))
      //  *ExprTypeSynPtr = -(*ExprTypeSynPtr + 1); // TBD!!! shouldn't this be done elsewhere?
      
      if ((*ExprTypeSynPtr >= 0) || (SyntaxStack[-*ExprTypeSynPtr].tok != tokStructPtr))
        ERROR(0x013A, FALSE);
      
      i = (int) SyntaxStack[-*ExprTypeSynPtr].param;
      if (((i + 2) > SyntaxStackCnt) ||
          ((SyntaxStack[i].tok != tokStruct) && (SyntaxStack[i].tok != tokUnion)) ||
           (SyntaxStack[i + 1].tok != tokTag))
        ERROR(0xE007, FALSE);
      
      if (!GetDeclSize(i, 0))
        // incomplete structure/union type
        ERROR(0x0203, FALSE);
      
      // save the .flags member of the main struct ident
      sflags = SyntaxStack[-*ExprTypeSynPtr].flags;
      i += 5; // step inside the {} body of the struct/union
      while (c) {
        int t = SyntaxStack[i].tok;
        c += (t == '(') - (t == ')') + (t == '{') - (t == '}');
        if ((c == 1) &&
            (t == tokMemberIdent) && (SyntaxStack[i].param == (bit32u) member) &&
            (SyntaxStack[i + 1].tok == tokLocalOfs)) {
          j = i;
          ofs = (int) SyntaxStack[i + 1].param;
          //break;  // ???????
        }
        i++;
      }
      if (!j)
        ERROR(0x013B, FALSE, IDENT_STR(member));
      
      j += 2;
      *ExprTypeSynPtr = -j; // type: pointer to member's type
      
      SyntaxStack[-*ExprTypeSynPtr].flags = sflags;
      
      stack[oldIdxRight - (oldSpRight - sp)].param = (bit32u) ofs; // member offset within structure/union
      
      // optimize a little, if possible
      {
        int i = oldIdxRight - (oldSpRight - sp) - 1;
        // Skip any type cast markers
        while (stack[i].tok == tokUnaryPlus)
          i--;
        // See if the pointer is an integer constant or a local variable offset
        // and if it is, adjust it here instead of generating code for
        // addition/subtraction
        if (stack[i].tok == tokNumInt || stack[i].tok == tokNumUint || stack[i].tok == tokLocalOfs) {
          stack[i].param = ((unsigned) stack[i].param + ofs); // TBD!!! need extra truncation?
          del(oldIdxRight - (oldSpRight - sp), 2);
        }
      }
            
      *ConstExpr = 0;
    } break;

    default:
      ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  }
  
  return s;
}

int ParseExpr(int tok, int *GotUnary, int *ExprTypeSynPtr, int *ConstExpr, int *ConstVal, int option, int option2) {
  bool identFirst = (tok == tokIdent);
  int oldOfs = CurFxnLocalOfs;
  *ConstVal = *ConstExpr = 0;
  *ExprTypeSynPtr = SymVoidSynPtr;
  
  if (!ExprLevel++)
    opsp = sp = 0;
  
  if (option == '=')
    push2(tokIdent, option2);
  
  tok = expr(tok, GotUnary, option == ',' || option == '=');
  
  if (tok == tokEof || strchr(",;:)]}", tok) == NULL)
    ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  
  if (option == '=') {
    push('=');
  } else if (option == tokGotoLabel && identFirst && tok == ':' && *GotUnary && sp == 1 && stack[sp - 1].tok == tokIdent) {
    // This is a label.
    ExprLevel--;
    return tokGotoLabel;
  }
  
  if (*GotUnary) {
    int j;
    // Do this twice so we can see the stack before
    // and after manipulations
    for (j = 0; j < 2; j++) {
      if (!j) {
        int idx = sp - 1;
        *ConstVal = exprval(&idx, ExprTypeSynPtr, ConstExpr);
        // remove the unneeded unary +'s that have served their cast-substitute purpose
        // also remove dereferences of size 0 (dereferences of pointers to structures)
        for (idx = sp - 1; idx >= 0; idx--)
          if ((stack[idx].tok == tokUnaryPlus) || ((stack[idx].tok == tokUnaryStar) && !stack[idx].param))
            del(idx, 1);
      }
    }
  }
  
  ExprLevel--;
  
  return tok;
}

// smc.c code

// Equivalent to puts() but outputs to targ_fp
// if it's not NULL.
int puts_out(const char *s) {
  int res;
  // Turbo C++ 1.01's fputs() returns EOF if s is empty, which is wrong.
  // Hence the workaround.
  if (*s == '\0' || (res = fputs(s, targ_fp)) >= 0)
    // unlike puts(), fputs() doesn't append '\n', so append it manually
    res = fputc('\n', targ_fp);
  return res;
}

// Equivalent to printf() but outputs to targ_fp
// if it's not NULL.
int printf_out(const char *format, ...) {
  int res;

  va_list vl;
  va_start(vl, format);
  
  res = vfprintf(targ_fp, format, vl);
  
  va_end(vl);
  
  return res;
}

// include the list of warnings and errors
#include "errorlist.h"

#ifdef GIVE_ERROR_LINES
void error(const int linenum, const int ErrNum, const bool cont, ...) {
#else
void error(const int ErrNum, const bool cont, ...) {
#endif
  
  int err = -1, i = 0, j = cur_file;
  
  va_list vl;
  va_start(vl, cont);
  
  while (ErrorList[i].number != 0xFFFF) {
    if (ErrorList[i].number == ErrNum) {
      // print to the stdout
#ifdef GIVE_ERROR_LINES
      printf("\n%s(%i:%i:[%i]) : error E%04X: ", files[j].filename, LineNo, LinePos, linenum, ErrorList[i].number);
#else
      printf("\n%s(%i:%i) : error E%04X: ", files[j].filename, LineNo, LinePos, ErrorList[i].number);
#endif
      vprintf(ErrorList[i].name, vl);
      // and to the .asm file
#ifdef GIVE_ERROR_LINES
      printf_out("; %s(%i:%i:[%i]) : error E%04X: ", files[j].filename, LineNo, LinePos, linenum, ErrorList[i].number);
#else
      printf_out("; %s(%i:%i) : error E%04X: ", files[j].filename, LineNo, LinePos, ErrorList[i].number);
#endif
      vfprintf(targ_fp, ErrorList[i].name, vl);
      puts_out("");
      // error to return
      err = ErrorList[i].number;
      break;
    }
    i++;
  }
  
  if (err < 0) {
    printf("\n%s(%i:%i) : error EFFFF: Unknown Error Found.", files[j].filename, LineNo, LinePos);
    va_end(vl);
  }
  error_cnt++;
  
  // if fatal error, close files, free memory, and exit
  if (!cont) {
    for (i = 0; i <= cur_file; i++)
      if (files[i].fp)
        fclose(files[i].fp);
    
    if (dump_tables) {
      DumpSynDecls();
      DumpMacroTable();
      DumpIdentTable();
      printf_out("; Next label number: %d\n", LabelCnt);
    }
    
    printf("\n Fatal Error Found.  Halting...\n"
           "     %i Errors Found.\n"
           "     %i Warnings Found.\n", error_cnt, warn_cnt);
    
    // free some memory used (must be after we print the error since 
    //  the string to print may be in part of this memory)
    if (StringTablePtr) free(StringTablePtr);
    if (MacroTablePtr) free(MacroTablePtr);
    if (SyntaxStack) free(SyntaxStack);
    if (IdentTablePtr) free(IdentTablePtr);
    
    if (targ_fp)
      fclose(targ_fp);
    
    exit(err);
  }
}

void warning(const int WarnNum, const int LineOverRide, ...) {
  int warn = -1, i = 0, line = LineNo, pos = LinePos;
  
  // if a line override was used, change to that line number
  if (LineOverRide > 0) {
    line = LineOverRide;
    pos = 1;
  }
  
  va_list vl;
  va_start(vl, LineOverRide);
  
  while (WarningList[i].number != 0xFFFF) {
    if (WarningList[i].number == WarnNum) {
      if (WarningList[i].level <= warning_level) {
        warn_cnt++;
        printf("\n%s(%i:%i) : warning W%04X: ", files[cur_file].filename, line, pos, WarningList[i].number);
        vprintf(WarningList[i].name, vl);
        printf_out("; %s(%i:%i) : warning W%04X: ", files[cur_file].filename, line, pos, WarningList[i].number);
        vfprintf(targ_fp, WarningList[i].name, vl);
        puts_out("");
      }
      warn = WarnNum;
      break;
    }
    i++;
  }
  
  if (warn < 0) {
    printf("\n%s(%i:%i) : warning Wx%04X: Unknown Warning Found.", files[cur_file].filename, LineNo, LinePos, WarnNum);
    va_end(vl);
  }
}

int tsd[] = {
  tokVoid, tokChar, tokWChar, tokInt,
  tokSigned, tokUnsigned, tokShort,
  tokStruct, tokUnion,
  tokConst, tokVolatile, tokRegister,
  tokAuto, tokRestrict, tokInline,
  //tokFarC, tokFarD, tokFarE, tokFarF, tokFarG
};

int TokenStartsDeclaration(int t, int params) {
  bool CurScope;
  unsigned i;
  
  for (i = 0; i < sizeof(tsd) / sizeof(tsd[0]); i++)
    if (tsd[i] == t)
      return 1;
  
  return ((SizeOfWord != 2) && (t == tokLong)) ||
          (t == tokEnum) ||
         ((t == tokIdent) && (FindTypedef(TokenIdentName, &CurScope, TRUE) >= 0)) ||
          (!params && 
         ((t == tokExtern) || (t == tokTypedef) || (t == tokStatic)));
}

void PushSyntax3(int t, bit32u v, bit32u f) {
  if (SyntaxStackCnt >= SYNTAX_STACK_MAX)
    ERROR(0x0006, FALSE);
  SyntaxStack[SyntaxStackCnt].tok = t;
  SyntaxStack[SyntaxStackCnt].param = (bit32u) v;
  SyntaxStack[SyntaxStackCnt++].flags = f;
}

void PushSyntax2(int t, bit32u v) {
  PushSyntax3(t, v, 0);
}

void PushSyntax(int t) {
  PushSyntax2(t, 0);
}

void InsertSyntax3(int pos, int t, bit32u v, int f) {
  if (SyntaxStackCnt >= SYNTAX_STACK_MAX)
    ERROR(0x0006, FALSE);
  memmove(&SyntaxStack[pos + 1],
          &SyntaxStack[pos],
          sizeof(struct S_SYNTAX_STACK) * (SyntaxStackCnt - pos));
  SyntaxStack[pos].tok = t;
  SyntaxStack[pos].param = v;
  SyntaxStack[pos].flags = f;
  SyntaxStackCnt++;
}

void InsertSyntax2(int pos, int t, bit32u v) {
  InsertSyntax3(pos, t, v, 0);
}

void InsertSyntax(int pos, int t) {
  InsertSyntax2(pos, t, 0);
}

void DeleteSyntax(int pos, int cnt) {
  memmove(&SyntaxStack[pos],
          &SyntaxStack[pos + cnt],
          sizeof(struct S_SYNTAX_STACK) * (SyntaxStackCnt - (pos + cnt)));
  SyntaxStackCnt -= cnt;
}

int FindSymbol(char *s) {
  int i;
  
  // TBD!!! return declaration scope number so
  // redeclarations can be reported if occur in the same scope.
  
  // TBD??? Also, I could first use FindIdent() and then just look for the
  // index into IdentTable[] instead of doing strcmp()
  for (i = SyntaxStackCnt - 1; i >= 0; i--) {
    int t = SyntaxStack[i].tok;
    if ((t == tokIdent) && !strcmp(IDENT_STR(SyntaxStack[i].param), s))
      return i;
    
    if (t == ')') {
      // Skip over the function params
      int c = -1;
      while (c) {
        t = SyntaxStack[--i].tok;
        c += (t == '(') - (t == ')');
      }
    }
  }
  
  return -1;
}

int SymType(int SynPtr) {
  bool local = FALSE;
  
  if (SyntaxStack[SynPtr].tok == tokIdent)
    SynPtr++;
  
  if ((local = (SyntaxStack[SynPtr].tok == tokLocalOfs)) != 0)
    SynPtr++;
  
  switch (SyntaxStack[SynPtr].tok) {
    case '(':
      return SymFxn;

    case '[':
      if (local)
        return SymLocalArr;
      return SymGlobalArr;

    default:
      if (local)
        return SymLocalVar;
      return SymGlobalVar;
  }
}

int FindTaggedDecl(char *s, int start, bool *CurScope) {
  int i;

  *CurScope = TRUE;

  for (i = start; i >= 0; i--) {
    int t = SyntaxStack[i].tok;
    if ((t == tokTag) && !strcmp(IDENT_STR(SyntaxStack[i].param), s))
      return i - 1;
    else if (t == ')') {
      // Skip over the function params
      int c = -1;
      while (c) {
        t = SyntaxStack[--i].tok;
        c += (t == '(') - (t == ')');
      }
    } else if (t == '#')
      // the scope has changed to the outer scope
      *CurScope = FALSE;
  }
  
  return -1;
}

// TBD??? rename this fxn? Cleanup/unify search functions?
int FindTypedef(char *s, bool *CurScope, const bool forUse) {
  int i;
  
  *CurScope = TRUE;
  
  for (i = SyntaxStackCnt - 1; i >= 0; i--) {
    int t = SyntaxStack[i].tok;
    
    if ((t == tokTypedef || t == tokIdent) && SyntaxStack[i].param &&
      !strcmp(IDENT_STR(SyntaxStack[i].param), s)) {
      // if the closest declaration isn't from typedef,
      // (i.e. if it's a variable/function declaration),
      // then the type is unknown for the purpose of
      // declaring a variable of this type
      if (forUse && (t == tokIdent))
        return -1;
      return i;
    }
    
    if (t == ')') {
      // Skip over the function params
      int c = -1;
      while (c) {
        t = SyntaxStack[--i].tok;
        c += (t == '(') - (t == ')');
      }
    } else if (t == '#') {
      // the scope has changed to the outer scope
      *CurScope = FALSE;
    }
  }
  
  return -1;
}

int GetDeclSize(int SyntaxPtr, int SizeForDeref) {
  int i;
  unsigned size = 1;
  int arr = 0;
  
  if (SyntaxPtr < 0)
    ERROR(0xE008, FALSE);
  
  for (i = SyntaxPtr; i < SyntaxStackCnt; i++) {
    int tok = SyntaxStack[i].tok;
    switch (tok) {
      case tokIdent: // skip leading identifiers, if any
      case tokLocalOfs: // skip local var offset, too
        break;
      case tokChar:
      case tokSChar:
        if (!arr && ((tok == tokSChar) || CharIsSigned) && SizeForDeref)
          return -1; // 1 byte, needing sign extension when converted to int/unsigned int
        // fallthrough
      case tokUChar:
        return (int) size;
      case tokShort:
        if (!arr && SizeForDeref)
          return -2; // 2 bytes, needing sign extension when converted to int/unsigned int
        // fallthrough
      case tokUShort:
        if (size * 2 / 2 != size)
          ERROR(0x0172, FALSE);
        size *= 2;
        if (size != truncUint(size))
          ERROR(0x0172, FALSE);
        return (int) size;
      case tokInt:
      case tokUnsigned:
      case '*':
      case '(': // size of fxn = size of ptr for now
        if (size * SizeOfWord / SizeOfWord != size)
          ERROR(0x0172, FALSE);
        size *= SizeOfWord;
        if (size != truncUint(size))
          ERROR(0x0172, FALSE);
        return (int) size;
      case tokWChar:
        return size * WIDTH_OF_WIDECHAR;
      case '[':
        if (SyntaxStack[i + 1].tok != tokNumInt && SyntaxStack[i + 1].tok != tokNumUint)
          ERROR(0xE009, FALSE);
        if (SyntaxStack[i + 1].param &&
            size * (int) SyntaxStack[i + 1].param / (int) SyntaxStack[i + 1].param != size)
          ERROR(0x0172, FALSE);
        size *= (int) SyntaxStack[i + 1].param;
        if (size != truncUint(size))
          ERROR(0x0172, FALSE);
        i += 2;
        arr = 1;
        break;
      case tokStructPtr:
        // follow the "type pointer"
        i = (int) SyntaxStack[i].param - 1;
        break;
      case tokStruct:
      case tokUnion:
        if (((i + 2) < SyntaxStackCnt) && (SyntaxStack[i + 2].tok == tokSizeof) && !SizeForDeref) {
          unsigned s = SyntaxStack[i + 2].param;
          if (s && ((size * s / s) != size))
            ERROR(0x0172, FALSE);
          size *= s;
          if (size != truncUint(size))
            ERROR(0x0172, FALSE);
          return (int) size;
        }
        return 0;
      case tokVoid:
        return 0;
      default:
        ERROR(0xE00A, FALSE, tok);
      }
  }
  
  ERROR(0xE00B, FALSE);
  return 0;
}

int GetDeclAlignment(int SyntaxPtr) {
  int i;
  
  if (SyntaxPtr < 0)
    ERROR(0xE00C, FALSE);
  
  for (i = SyntaxPtr; i < SyntaxStackCnt; i++) {
    int tok = SyntaxStack[i].tok;
    switch (tok) {
      case tokIdent: // skip leading identifiers, if any
      case tokLocalOfs: // skip local var offset, too
        break;
      case tokChar:
      case tokSChar:
      case tokUChar:
        return 1;
      case tokShort:
      case tokUShort:
        return 2;
      case tokWChar:
        return WIDTH_OF_WIDECHAR;
      case tokInt:
      case tokUnsigned:
      case '*':
      case '(':
        return SizeOfWord;
      case '[':
        if (SyntaxStack[i + 1].tok != tokNumInt && SyntaxStack[i + 1].tok != tokNumUint)
          ERROR(0xE00D, FALSE);
        i += 2;
        break;
      case tokStructPtr:
        // follow the "type pointer"
        i = (int) SyntaxStack[i].param - 1;
        break;
      case tokStruct:
      case tokUnion:
        if (((i + 3) < SyntaxStackCnt) && (SyntaxStack[i + 2].tok == tokSizeof))
          return (int) SyntaxStack[i + 3].param;
        return 1;
      case tokVoid:
        return 1;
      default:
        ERROR(0xE00E, FALSE);
      }
  }

  ERROR(0xE00F, FALSE);
  return 0;
}

int ParseArrayDimension(int AllowEmptyDimension) {
  int tok;
  int gotUnary, synPtr, constExpr, exprVal;
  unsigned exprValU;
  int oldssp, oldesp;
  struct S_IDENT_PREFIX *undoIdents;
  
  tok = GetToken();
  // DONE: support arbitrary constant expressions
  oldssp = SyntaxStackCnt;
  oldesp = sp;
  undoIdents = LastIdent;
  tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0);
  UndoNonLabelIdents(undoIdents, FALSE); // remove all temporary identifier names
  SyntaxStackCnt = oldssp; // undo any temporary declarations from e.g. "sizeof" in the expression
  sp = oldesp;
  
  if (tok != ']')
    ERROR(0x0119, FALSE, GetTokenName(tok));
  
  if (!gotUnary) {
    if (!AllowEmptyDimension)
      ERROR(0x011A, FALSE);
    // Empty dimension is dimension of 0
    exprVal = 0;
  } else {
    if (!constExpr)
      ERROR(0x0165, FALSE);
    
    exprValU = truncUint(exprVal);
    exprVal = truncInt(exprVal);
    
    promoteType(&synPtr, &synPtr);
    if ((SyntaxStack[synPtr].tok == tokInt && exprVal < 1) || (SyntaxStack[synPtr].tok == tokUnsigned && exprValU < 1))
      ERROR(0x0177, FALSE);
    
    exprVal = (int) exprValU;
  }
  
  PushSyntax2(tokNumUint, exprVal);
  return tok;
}

#define ALLOWED_MASK_NOTHING  (0 << 0)

#define ALLOWED_MASK_CHAR     (1 << 0)
#define ALLOWED_MASK_WCHAR    (1 << 1)
#define ALLOWED_MASK_SHORT    (1 << 2)
#define ALLOWED_MASK_INT      (1 << 3)
#define ALLOWED_MASK_LONG     (1 << 4)
#define ALLOWED_MASK_SIGNED   (1 << 5)
#define ALLOWED_MASK_UNSIGNED (1 << 6)
#define ALLOWED_MASK_VOID     (1 << 7)
#define ALLOWED_MASK_TYPEDEF  (1 << 8)
  #define ALLOWED_STANDARD_MASK ((ALLOWED_MASK_TYPEDEF << 1) - 1)

#define ALLOWED_MASK_STRUCT   (1 << 9)
#define ALLOWED_MASK_ENUM     (1 << 10)
#define ALLOWED_MASK_CONST    (1 << 11)
#define ALLOWED_MASK_VOLATILE (1 << 12)
#define ALLOWED_MASK_REGISTER (1 << 13)
#define ALLOWED_MASK_AUTO     (1 << 14)
#define ALLOWED_MASK_RESTRICT (1 << 15)
#define ALLOWED_MASK_INLINE   (1 << 16)
#define ALLOWED_MASK_FARX     (1 << 17)

#define ALLOWED_MASK_ALL      ((ALLOWED_MASK_FARX << 1) - 1)

int ParseBase(int tok, struct S_SYNTAX_STACK *base, bit32u *ident_flags) {
  int allowedMask = ALLOWED_MASK_ALL;
  int typeMask = 0;
  int tokMask = 0, disallowedMask;

  int structType = -1;
  int typePtr = SyntaxStackCnt;
  int declPtr = -1;
  int ptr = 0;
  bool curScope = FALSE, valid = TRUE, breakit = FALSE,
       gotToken, gotTag = FALSE, empty = TRUE;
  struct S_IDENT_PREFIX *tagIdent = NULL;
  
  base->tok = 0;
  base->param = 0;
  base->flags = 0;
  
  while (1) {
    gotToken = FALSE;
    switch (tok) {
      case tokChar:
        tokMask = ALLOWED_MASK_CHAR;
        disallowedMask = (ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_SHORT | ALLOWED_MASK_VOID |
          ALLOWED_MASK_STRUCT | ALLOWED_MASK_ENUM | ALLOWED_MASK_TYPEDEF);
        break;
      case tokWChar:
        tokMask = ALLOWED_MASK_WCHAR;
        disallowedMask = (ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_SHORT | ALLOWED_MASK_VOID |
          ALLOWED_MASK_STRUCT | ALLOWED_MASK_ENUM | ALLOWED_MASK_TYPEDEF | ALLOWED_MASK_SIGNED | ALLOWED_MASK_UNSIGNED);
        break;
      case tokShort:
        tokMask = ALLOWED_MASK_SHORT;
        disallowedMask = (ALLOWED_MASK_LONG | ALLOWED_MASK_CHAR | ALLOWED_MASK_VOID | ALLOWED_MASK_STRUCT | 
          ALLOWED_MASK_ENUM | ALLOWED_MASK_TYPEDEF);
        break;
      case tokInt:
        tokMask = ALLOWED_MASK_INT;
        disallowedMask = (ALLOWED_MASK_CHAR | ALLOWED_MASK_VOID | ALLOWED_MASK_STRUCT | ALLOWED_MASK_ENUM | 
          ALLOWED_MASK_TYPEDEF);
        break;
      case tokLong:
        tokMask = ALLOWED_MASK_LONG;
        disallowedMask = (ALLOWED_MASK_SHORT | ALLOWED_MASK_CHAR | ALLOWED_MASK_VOID | ALLOWED_MASK_STRUCT |
          ALLOWED_MASK_ENUM | ALLOWED_MASK_TYPEDEF);
        break;
      case tokSigned:
        tokMask = ALLOWED_MASK_SIGNED;
        disallowedMask = (ALLOWED_MASK_UNSIGNED | ALLOWED_MASK_VOID | ALLOWED_MASK_STRUCT | ALLOWED_MASK_ENUM |
          ALLOWED_MASK_TYPEDEF);
        break;
      case tokUnsigned:
        tokMask = ALLOWED_MASK_UNSIGNED;
        disallowedMask = (ALLOWED_MASK_SIGNED | ALLOWED_MASK_VOID | ALLOWED_MASK_STRUCT | ALLOWED_MASK_ENUM |
          ALLOWED_MASK_TYPEDEF);
        break;
      case tokConst:
        *ident_flags |= IDENT_FLAGS_ISCONST; 
        tokMask = ALLOWED_MASK_CONST;
        disallowedMask = ALLOWED_MASK_ENUM;
        break;
      case tokVolatile:
        *ident_flags |= IDENT_FLAGS_ISVOLATILE; 
        tokMask = ALLOWED_MASK_VOLATILE;
        disallowedMask = ALLOWED_MASK_ENUM;
        break;
      case tokRegister:
        *ident_flags |= IDENT_FLAGS_ISREGISTER;
        tokMask = ALLOWED_MASK_REGISTER;
        disallowedMask = ALLOWED_MASK_ENUM;
        break;
      case tokAuto:
        *ident_flags |= IDENT_FLAGS_ISAUTO;
        tokMask = ALLOWED_MASK_AUTO;
        disallowedMask = ALLOWED_MASK_ENUM;
        break;
      case tokRestrict:
        *ident_flags |= IDENT_FLAGS_ISRESTRICT;
        tokMask = ALLOWED_MASK_RESTRICT;
        disallowedMask = ALLOWED_MASK_ENUM;
        break;
      case tokInline:
        *ident_flags |= IDENT_FLAGS_ISINLINE;
        tokMask = ALLOWED_MASK_INLINE;
        disallowedMask = ALLOWED_MASK_ENUM;
        break;
      case tokFarC: case tokFarD: case tokFarE:
      case tokFarF: case tokFarG:
        base->flags |= tok;
        tokMask = ALLOWED_MASK_FARX;
        disallowedMask = ALLOWED_MASK_ENUM;
        break;
      case tokVoid:
        tokMask = ALLOWED_MASK_VOID;
        disallowedMask = (ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_SHORT | ALLOWED_MASK_CHAR |
          ALLOWED_MASK_SIGNED | ALLOWED_MASK_UNSIGNED | ALLOWED_MASK_STRUCT | ALLOWED_MASK_ENUM);
        break;
      case tokEnum:
      case tokStruct:
      case tokUnion:
        if (tok == tokEnum) {
          tokMask = ALLOWED_MASK_ENUM;
          disallowedMask = (ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_SHORT | ALLOWED_MASK_CHAR |
            ALLOWED_MASK_SIGNED | ALLOWED_MASK_UNSIGNED | ALLOWED_MASK_FARX | ALLOWED_MASK_VOID | ALLOWED_MASK_STRUCT | ALLOWED_MASK_TYPEDEF);
        } else {
          tokMask = ALLOWED_MASK_STRUCT;
          disallowedMask = (ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_SHORT | ALLOWED_MASK_CHAR |
            ALLOWED_MASK_SIGNED | ALLOWED_MASK_UNSIGNED | ALLOWED_MASK_VOID | ALLOWED_MASK_ENUM | ALLOWED_MASK_TYPEDEF);
        }
        structType = tok;
        
        tok = GetToken();
        if (tok == tokIdent) {
          // this is a structure/union/enum tag
          gotTag = TRUE;
          declPtr = FindTaggedDecl(TokenIdentName, SyntaxStackCnt - 1, &curScope);
          tagIdent = AddIdent(TokenIdentName, *ident_flags);
          
          if (declPtr >= 0) {
            // Within the same scope we can't declare more than one union, structure or enum
            // with the same tag.
            // There's one common tag namespace for structures, unions and enumerations.
            if (curScope && SyntaxStack[declPtr].tok != structType)
              ERROR(0x0170, FALSE, IDENT_STR(tagIdent));
          } else if (ParamLevel) {
            // new structure/union/enum declarations aren't supported in function parameters
            ERROR(0x0160, FALSE);
          }
        } else {
          // we got the token in tok, so don't get it again below
          gotToken = TRUE;
          
          // structure/union/enum declarations aren't supported in expressions
          if (ExprLevel)
            ERROR(0x0160, FALSE);
          PushSyntax(structType);
          PushSyntax2(tokTag, (bit32u) AddIdent("<something>", *ident_flags));
        }
        break;
        
      case tokIdent:
        if ((ptr = FindTypedef(TokenIdentName, &curScope, TRUE)) >= 0) {
          base->param = (bit32u) ptr;
          tokMask = ALLOWED_MASK_TYPEDEF;
          disallowedMask = (ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_SHORT | ALLOWED_MASK_CHAR |
            ALLOWED_MASK_SIGNED | ALLOWED_MASK_UNSIGNED | ALLOWED_MASK_VOID | ALLOWED_MASK_ENUM | ALLOWED_MASK_STRUCT);
          break;
        } // else 
          // fall through
      default:
        tokMask = disallowedMask = 0;
        breakit = TRUE;
    }
    
    if (breakit)
      break;

    if (allowedMask & tokMask) {
      typeMask |= tokMask;
      allowedMask &= ~(disallowedMask | tokMask);
      if (!gotToken)
        tok = GetToken();
    } else {
      typeMask = 0;
      valid = FALSE;
      break;
    }
  }
  
  switch (typeMask & ALLOWED_STANDARD_MASK) {
    case ALLOWED_MASK_CHAR:
      base->tok = tokChar;
      break;
    case ALLOWED_MASK_SIGNED | ALLOWED_MASK_CHAR:
      base->tok = tokSChar;
      break;
    case ALLOWED_MASK_UNSIGNED | ALLOWED_MASK_CHAR:
      base->tok = tokUChar;
      break;
    case ALLOWED_MASK_WCHAR:
      base->tok = tokWChar;
      break;
    case ALLOWED_MASK_SHORT:
    case ALLOWED_MASK_SHORT | ALLOWED_MASK_SIGNED:
    case ALLOWED_MASK_SHORT | ALLOWED_MASK_INT:
    case ALLOWED_MASK_SHORT | ALLOWED_MASK_INT | ALLOWED_MASK_SIGNED:
      base->tok = tokShort;
      break;
    case ALLOWED_MASK_SHORT | ALLOWED_MASK_UNSIGNED:
    case ALLOWED_MASK_SHORT | ALLOWED_MASK_INT | ALLOWED_MASK_UNSIGNED:
      base->tok = tokUShort;
      break;
    case ALLOWED_MASK_INT:
    case ALLOWED_MASK_SIGNED:
    case ALLOWED_MASK_SIGNED | ALLOWED_MASK_INT:
      base->tok = tokInt;
      break;
    case ALLOWED_MASK_UNSIGNED:
    case ALLOWED_MASK_UNSIGNED | ALLOWED_MASK_INT:
      base->tok = tokUnsigned;
      break;
    case ALLOWED_MASK_LONG:
    case ALLOWED_MASK_LONG | ALLOWED_MASK_SIGNED:
    case ALLOWED_MASK_LONG | ALLOWED_MASK_INT:
    case ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_SIGNED:
      base->tok = tokLong;
      break;
    case ALLOWED_MASK_LONG | ALLOWED_MASK_UNSIGNED:
    case ALLOWED_MASK_LONG | ALLOWED_MASK_INT | ALLOWED_MASK_UNSIGNED:
      base->tok = tokULong;
      break;
    case ALLOWED_MASK_VOID:
      base->tok = tokVoid;
      break;
    case ALLOWED_MASK_TYPEDEF:
      base->tok = tokTypedef;
      break;
    default:
      base->tok = typeMask & ((ALLOWED_MASK_UNSIGNED << 1) - 1); // ?????
  }
  
  if (valid && (structType > 0)) {
    if (tok == '{') {
      unsigned sz, alignment, tmp;
      
      // new structure/union/enum declarations aren't supported in expressions and function parameters
      if (ExprLevel || ParamLevel)
        ERROR(0x0160, FALSE);
      
      if (gotTag) {
        // Cannot redefine a tagged structure/union/enum within the same scope
        if ((declPtr >= 0) && curScope &&
         ((((declPtr + 2) < SyntaxStackCnt) && (SyntaxStack[declPtr + 2].tok == tokSizeof)) || structType == tokEnum))
          ERROR(0x0170, FALSE, IDENT_STR(tagIdent));
        
        PushSyntax(structType);
        PushSyntax2(tokTag, (bit32u) tagIdent);
      }
      
      if (structType == tokEnum) {
        int val = 0;
        
        tok = GetToken();
        while (tok != '}') {
          char* s;
          struct S_IDENT_PREFIX *ident;
          
          if (tok != tokIdent)
            ERROR(0x010F, FALSE, GetTokenName(tok));
          
          s = TokenIdentName;
          if ((FindTypedef(s, &curScope, FALSE) >= 0) && curScope)
            ERROR(0x0171, FALSE, s);
          
          ident = AddIdent(s, *ident_flags);
          
          empty = FALSE;
          
          tok = GetToken();
          if (tok == '=') {
            int gotUnary, synPtr, constExpr;
            int oldssp, oldesp;
            struct S_IDENT_PREFIX *undoIdents;
            
            oldssp = SyntaxStackCnt;
            oldesp = sp;
            undoIdents = LastIdent;
            
            tok = ParseExpr(GetToken(), &gotUnary, &synPtr, &constExpr, &val, ',', 0);
            
            UndoNonLabelIdents(undoIdents, FALSE); // remove all temporary identifier names
            SyntaxStackCnt = oldssp; // undo any temporary declarations from e.g. "sizeof" in the expression
            sp = oldesp;
            
            if (!gotUnary)
              ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
            if (!constExpr)
              ERROR(0x0164, FALSE);
          }
          
          PushSyntax2(tokIdent, (bit32u) ident);
          PushSyntax2(tokNumInt, val);
          val = (int) (val + 1u);
          
          if (tok == ',')
            tok = GetToken();
          else if (tok != '}')
            ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
        }
        
        if (empty)
          ERROR(0x010F, FALSE, GetTokenName('}'));
        
        base->tok = tokEnumPtr;
        base->param = (bit32u) typePtr;
        base->flags = 0;
        
        tok = GetToken();
        return tok;
      } else {
        struct S_STRUCTINFO structInfo;
        structInfo.type = structType;
        structInfo.alignment = 1; // initial member alignment
        structInfo.offset = 0;    // initial member offset
        structInfo.max_size = 0;  // initial max member size (for unions)
        
        PushSyntax(tokSizeof); // 0 = initial structure/union size, to be updated
        PushSyntax2(tokSizeof, 1); // 1 = initial structure/union alignment, to be updated
        
        PushSyntax('{');
        
        tok = GetToken();
        while (tok != '}') {
          if (!TokenStartsDeclaration(tok, 1))
            ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
          tok = ParseDecl(tok, &structInfo, FALSE, 0);
          empty = FALSE;
        }
        
        if (empty)
          ERROR(0x010F, FALSE, GetTokenName('}'));
        
        PushSyntax('}');
        
        // Update structure/union alignment
        alignment = structInfo.alignment;
        SyntaxStack[typePtr + 3].param = (bit32u) alignment;
        
        // Update structure/union size and include trailing padding if needed
        sz = structInfo.offset + structInfo.max_size;
        tmp = sz;
        sz = (sz + alignment - 1) & ~(alignment - 1);
        if (sz < tmp || sz != truncUint(sz))
          ERROR(0x0172, FALSE);
        SyntaxStack[typePtr + 2].param = (int) sz;
        
        tok = GetToken();
      }
    } else {
      if (structType == tokEnum) {
        if (!gotTag || declPtr < 0)
          ERROR(0x0160, FALSE); // TBD!!! different error when enum tag is not found
        
        base->tok = tokEnumPtr;
        base->param = (bit32u) declPtr;
        base->flags = 0;
        return tok;
      }
      
      if (gotTag) {
        if (declPtr >= 0 && SyntaxStack[declPtr].tok == structType) {
          base->tok = tokStructPtr;
          base->param = (bit32u) declPtr;
          return tok;
        }
        
        PushSyntax(structType);
        PushSyntax2(tokTag, (bit32u) tagIdent);
        
        empty = FALSE;
      }
    }
    
    if (empty)
      ERROR(0x0160, FALSE);
    
    base->tok = tokStructPtr;
    base->param = (bit32u) typePtr;
    
    // If we've just defined a structure/union and there are
    // preceding references to this tag within this scope,
    // IOW references to an incomplete type, complete the
    // type in the references
    if (gotTag && SyntaxStack[SyntaxStackCnt - 1].tok == '}') {
      int i;
      for (i = SyntaxStackCnt - 1; i >= 0; i--)
        if (SyntaxStack[i].tok == tokStructPtr) {
          int j = (int) SyntaxStack[i].param;
          if (SyntaxStack[j + 1].param == (bit32u) tagIdent)
            SyntaxStack[i].param = (bit32u) typePtr;
        } else if (SyntaxStack[i].tok == '#') {
          // reached the beginning of the current scope
          break;
        }
    }
  }
  
  if ((SizeOfWord == 2) && ((base->tok == tokLong) || (base->tok == tokULong)))
    valid = FALSE;

  if (SizeOfWord == 4) {
    // to simplify matters, treat long and unsigned long as aliases for int and unsigned int
    // in 32-bit and huge mode(l)s
    if (base->tok == tokLong)
      base->tok = tokInt;
    if (base->tok == tokULong)
      base->tok = tokUnsigned;
  }
  
  if (SizeOfWord == 2) {
    // to simplify matters, treat short and unsigned short as aliases for int and unsigned int
    // in 16-bit mode
    if (base->tok == tokShort)
      base->tok = tokInt;
    if (base->tok == tokUShort)
      base->tok = tokUnsigned;
  }
  
  // TBD!!! review/test this fxn
//  if (!valid || !tok || !(strchr("*([,)", tok) || tok == tokIdent))
  if (!valid || !tok)
    ERROR(0x0214, FALSE);
  
  return tok;
}

/*
  base * name []  ->  name : [] * base
  base *2 (*1 name []1) []2  ->  name : []1 *1 []2 *2 base
  base *3 (*2 (*1 name []1) []2) []3  ->  name : []1 *1 []2 *2 []3 *3 base
*/

int ParseDerived(int tok, bit32u flags, const bit32u ident_flags) {
  int stars = 0;
  int params = 0;
  int isInterrupt = 0;
  
  while (tok == '*') {
    stars++;
    tok = GetToken();
  }
  
  if (tok == tokIntr) {
    // __interrupt is supported in the huge mode(l) only
    ERROR(0x0160, FALSE);
    isInterrupt = 1;
    tok = GetToken();
  }
  
  if (tok == '(') {
    tok = GetToken();
    if (tok != ')' && !TokenStartsDeclaration(tok, 1)) {
      tok = ParseDerived(tok, flags, ident_flags);
      if (tok != ')')
        ERROR(0x0111, FALSE, GetTokenName(tok));
      tok = GetToken();
    } else
      params = 1;
  } else if (tok == tokIdent) {
    PushSyntax3(tok, (bit32u) AddIdent(TokenIdentName, ident_flags), flags);
    tok = GetToken();
  } else
    PushSyntax3(tokIdent, (bit32u) AddIdent("<something>", ident_flags), flags);
  
  if (params || (tok == '(')) {
    int t = SyntaxStack[SyntaxStackCnt - 1].tok;
    if ((t == ')') | (t == ']'))
      ERROR(0x010F, FALSE, GetTokenName('(')); // array of functions or function returning function
    if (!params)
      tok = GetToken();
    else
      PushSyntax2(tokIdent, (bit32u) AddIdent("<something>", ident_flags));
    if (isInterrupt)
      PushSyntax2('(', 1);
    else // fallthrough
      PushSyntax('(');
    
    ParseLevel++;
    ParamLevel++;
    ParseFxnParams(tok);
    ParamLevel--;
    ParseLevel--;
    PushSyntax(')');
    tok = GetToken();
  } else if (tok == '[') {
    // allow the first [] without the dimension in function parameters
    int allowEmptyDimension = 1;
    if (SyntaxStack[SyntaxStackCnt - 1].tok == ')')
      ERROR(0x010F, FALSE, GetTokenName('[')); // function returning array
    while (tok == '[') {
      int oldsp = SyntaxStackCnt;
      PushSyntax(tokVoid); // prevent cases like "int arr[arr];" and "int arr[arr[0]];"
      PushSyntax(tok);
      tok = ParseArrayDimension(allowEmptyDimension);
      if (tok != ']')
        ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
      PushSyntax(']');
      tok = GetToken();
      DeleteSyntax(oldsp, 1);
      allowEmptyDimension = 0;
    }
  }
  
  while (stars--)
    PushSyntax3('*', 0, flags);
  
  if (!tok || !strchr(",;{=)", tok))
    ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  
  return tok;
}

void PushBase(struct S_SYNTAX_STACK *base) {
  if (base->tok == tokTypedef) {
    int ptr = (int) base->param;
    int c = 0;
    bool copying = TRUE;
    
    while (copying) {
      int tok = SyntaxStack[++ptr].tok;
      int t = SyntaxStack[SyntaxStackCnt - 1].tok;
      
      // Cannot have:
      //   function returning function
      //   array of functions
      //   function returning array
      if (((t == ')' || t == ']') && tok == '(') ||
        (t == ')' && tok == '['))
          ERROR(0x0160, FALSE);
      
      PushSyntax3(tok, SyntaxStack[ptr].param, base->flags);
      
      c += (tok == '(') - (tok == ')') + (tok == '[') - (tok == ']');
      
      if (!c) {
        switch (tok) {
          case tokVoid:
          case tokChar: case tokSChar: case tokUChar: case tokWChar:
          case tokShort: case tokUShort:
          case tokInt: case tokUnsigned:
          case tokStructPtr:
            copying = FALSE;
        }
      }
    } // while (copying)
  } else
    PushSyntax3(base->tok, base->param, base->flags);
  
  // Cannot have array of void
  if (SyntaxStack[SyntaxStackCnt - 1].tok == tokVoid &&
      SyntaxStack[SyntaxStackCnt - 2].tok == ']')
    ERROR(0x0210, FALSE);
}

int InitVar(int synPtr, int tok) {
  int p = synPtr, t;
  struct S_IDENT_PREFIX *undoIdents = LastIdent;
  
  while ((SyntaxStack[p].tok == tokIdent) | (SyntaxStack[p].tok == tokLocalOfs))
    p++;
  
  t = SyntaxStack[p].tok;
  if (t == '[') {
    // Initializers for aggregates must be enclosed in braces,
    // except for arrays of char initialized with string literals,
    // in which case braces are optional
    if (tok != '{') {
      t = SyntaxStack[p + 3].tok;
      if (((tok != tokLitStr) && (tok != tokLitWStr)) || ((t != tokChar) && (t != tokUChar) && (t != tokSChar) && (t != tokWChar)))
        ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
    }
    tok = InitArray(p, tok);
  } else if (t == tokStructPtr) {
    if (tok != '{')
      ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
    tok = InitStruct(p, tok);
  } else
    tok = InitScalar(p, tok, 0, 1);
  
  if (!strchr(",;", tok))
    ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  
  UndoNonLabelIdents(undoIdents, FALSE); // remove all temporary identifier names from e.g. "sizeof" or "str"
  
  return tok;
}

int InitScalar(int synPtr, int tok, const unsigned indx, const bool last) {
  unsigned elementSz = GetDeclSize(synPtr, 0);
  int gotUnary, synPtr2, constExpr, exprVal;
  int oldssp = SyntaxStackCnt;
  struct S_IDENT_PREFIX *undoIdents = LastIdent;
  int ttop, braces = 0;
  
  // Initializers for scalars can be optionally enclosed in braces
  if (tok == '{') {
    braces = 1;
    tok = GetToken();
  }
  tok = ParseExpr(tok, &gotUnary, &synPtr2, &constExpr, &exprVal, ',', 0);
  
  if (!gotUnary)
    ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  
  if (braces) {
    if (tok != '}')
      ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
    tok = GetToken();
  }
  
  if (scalarTypeCheck(synPtr2)) ERROR(0x0203, FALSE);
  
  ttop = stack[sp - 1].tok;
  if (ttop == tokNumInt || ttop == tokNumUint) {
    // TBD??? truncate values for types smaller than int (e.g. char and short),
    // so they are always in range?
    
    // print about 10 items per line
    if ((indx % 10) == 0)
      GenIntData(elementSz, (int) stack[0].param);
    else
      printf_out(", %i", (int) stack[0].param);
    if ((((indx + 1) % 10) == 0) || last)
      puts_out("");
//    if (((indx + 1) % 10) == 0)
//      printf_out("(Y %i %i)\n", indx, last);
//    if (indx == last)
//      printf_out("(X %i %i)\n", indx, last);
    
  } else if (elementSz == (unsigned) SizeOfWord) {
    if (ttop == tokIdent)
      GenAddrData(elementSz, stack[sp - 1].param, 0);
    else if (ttop == '+' || ttop == '-') {
      int tleft = stack[sp - 3].tok;
      int tright = stack[sp - 2].tok;
      if (tleft == tokIdent && (tright == tokNumInt || tright == tokNumUint))
        GenAddrData(elementSz, stack[sp - 3].param, (ttop == '+') ? (int) stack[sp - 2].param : -((int) stack[sp - 2].param));
      else if (ttop == '+' &&
               tright == tokIdent &&
               (tleft == tokNumInt || tleft == tokNumUint))
        GenAddrData(elementSz, stack[sp - 2].param, (int) stack[sp - 3].param);
      else
        ERROR(0x0164, FALSE);
    } else
      ERROR(0x0164, FALSE);
    // Defer storage of string literal data (if any) until the end.
    // This will let us generate the contiguous array of pointers to
    // string literals unperturbed by the string literal data
    // (e.g. "char *colors[] = { "red", "green", "blue" };").
  } else
    ERROR(0x0166, FALSE);
  
  UndoNonLabelIdents(undoIdents, FALSE); // remove all temporary identifier names
  SyntaxStackCnt = oldssp; // undo any temporary declarations from e.g. "sizeof" or "str" in the expression
  return tok;
}

int InitArray(int synPtr, int tok) {
  int elementTypePtr = synPtr + 3;
  int elementType = SyntaxStack[elementTypePtr].tok;
  unsigned elementSz = GetDeclSize(elementTypePtr, 0);
  bool braces = FALSE;
  unsigned elementCnt = 0;
  unsigned elementsRequired = SyntaxStack[synPtr + 1].param;
  int extra = 0, remaining = (int) elementsRequired;
  bool arrOfChar = ((elementType == tokChar) || (elementType == tokUChar) || (elementType == tokSChar) || (elementType == tokWChar));
  
  if (tok == '{') {
    braces = TRUE;
    tok = GetToken();
  }
  
  // This allows for wide chars (wchar_t) where the width of the wchar_t is
  //  variable via WIDTH_OF_WIDECHAR at NBC build time
  if (arrOfChar && ((tok == tokLitStr) || (tok == tokLitWStr))) {
    int wastok = tok;
    // this is 'someArray[someCountIfAny] = "some string"' or
    // 'someArray[someCountIfAny] = { "some string" }'
    do {
      GetString('"', '#');
      if (elementCnt + TokenStringLen < elementCnt ||
          elementCnt + TokenStringLen >= truncUint(-1))
        warning(0x0004, 0);
      elementCnt += TokenStringLen;
      remaining -= TokenStringLen;
      extra += GenStrData(TokenValueString, TokenStringLen, elementType);
      tok = GetToken();
    } while (tok == wastok); // concatenate adjacent string literals
    if (remaining > 0) {
      if (elementType == tokWChar)
        GenZeroData(remaining * WIDTH_OF_WIDECHAR);
      else
        GenZeroData(remaining);
    }
    
    if (elementsRequired && ((elementCnt + extra) > elementsRequired))
      warning(0x0004, 0);
    
    if (elementsRequired == 0) {
      if (elementType == tokWChar)
        GenZeroData(WIDTH_OF_WIDECHAR);
      else
        GenZeroData(1);
      elementCnt++;
    }
    
    if (braces) {
      if (tok != '}')
        ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
      tok = GetToken();
    }
  } else {
    while (tok != '}') {
      if (elementType == '[') {
        tok = InitArray(elementTypePtr, tok);
      } else if (elementType == tokStructPtr) {
        tok = InitStruct(elementTypePtr, tok);
      } else
        tok = InitScalar(elementTypePtr, tok, elementCnt, ((elementCnt + 1) >= elementsRequired && elementsRequired));
      
      // Last element?
      if (++elementCnt >= elementsRequired && elementsRequired) {
        if (braces & (tok == ','))
          tok = GetToken();
        break;
      }
      
      if (tok == ',')
        tok = GetToken();
      else if (tok != '}')
        ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
    }
    if (!elementsRequired)
      puts_out("");
    
    if (braces) {
      if (!elementCnt || (tok != '}'))
        ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
      tok = GetToken();
    }
    
    if (elementCnt < elementsRequired) {
      printf_out("\n");
      GenZeroData((elementsRequired - elementCnt) * elementSz);
    }
  }
  
  // Store the element count if it's an incomplete array
  if (!elementsRequired)
    SyntaxStack[synPtr + 1].param = (bit32u) elementCnt;

  return tok;
}

int InitStruct(int synPtr, int tok) {
  int c = 1;
  unsigned size, ofs = 0;
  bool isUnion, braces = FALSE;

  synPtr = (int) SyntaxStack[synPtr].param;
  isUnion = (SyntaxStack[synPtr++].tok == tokUnion);
  size = (int) SyntaxStack[++synPtr].param;
  synPtr += 3; // step inside the {} body of the struct/union

  if (tok == '{') {
    braces = TRUE;
    tok = GetToken();
  }
  
  while (c) {
    int t = SyntaxStack[synPtr].tok;
    c += (t == '(') - (t == ')') + (t == '{') - (t == '}');
    if ((c == 1) && (t == tokMemberIdent))
      break;
    synPtr++;
  }
  
  while (tok != '}') {
    c = 1;
    int elementTypePtr, elementType;
    unsigned elementOfs, elementSz;
    
    elementOfs = (int) SyntaxStack[++synPtr].param;
    elementTypePtr = ++synPtr;
    elementType = SyntaxStack[elementTypePtr].tok;
    elementSz = GetDeclSize(elementTypePtr, 0);
    
    // Alignment
    if (ofs < elementOfs)
      GenZeroData(elementOfs - ofs);

    if (elementType == '[')
      tok = InitArray(elementTypePtr, tok);
    else if (elementType == tokStructPtr)
      tok = InitStruct(elementTypePtr, tok);
    else
      tok = InitScalar(elementTypePtr, tok, 0, 1);
    
    ofs = elementOfs + elementSz;
    
    // Find the next member or the closing brace
    while (c) {
      int t = SyntaxStack[synPtr].tok;
      c += (t == '(') - (t == ')') + (t == '{') - (t == '}');
      if ((c == 1) && (t == tokMemberIdent))
        break;
      synPtr++;
    }

    // Last member?
    // Only one member (first) is initialized in unions explicitly
    if ((!c) | isUnion) {
      if (braces & (tok == ','))
        tok = GetToken();
      break;
    }
    
    if (tok == ',')
      tok = GetToken();
    else if (tok != '}')
      ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  }
  
  if (braces) {
    if (!ofs || (tok != '}'))
      ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
    tok = GetToken();
  }
  
  // Implicit initialization of the rest and trailing padding
  if (ofs < size)
    GenZeroData(size - ofs);

  return tok;
}

// DONE: support extern
// DONE: support static
// DONE: support basic initialization
// DONE: support simple non-array initializations with string literals
// DONE: support basic 1-d array initialization
// DONE: global/static data allocations
int ParseDecl(int tok, struct S_STRUCTINFO *structInfo, const bool cast, int label) {
  struct S_SYNTAX_STACK base;
  int lastSyntaxPtr;
  bool external = (tok == tokExtern);
  bool Static = (tok == tokStatic);
  bool typeDef = (tok == tokTypedef);
  bit32u ident_flags = 0;
  
  if (external || typeDef || Static) {
    tok = GetToken();
    // if extern "C" Ident, don't add the '_' to the name
    if (external && (tok == tokLitStr)) {
      if ((TokenStringLen == 1) && (TokenValueString[0] == 'C'))
        ident_flags |= IDENT_FLAGS_NOUNDERSCR;
      else
        ERROR(0x0019, TRUE, TokenValueString);
      tok = GetToken();
    }
    if (!TokenStartsDeclaration(tok, 1))
      // Implicit int (as in "extern x; static y;") isn't supported
      ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  }
  tok = ParseBase(tok, &base, &ident_flags);
  
  if (label && (tok == ':') && (base.tok == tokTypedef) && !(external | Static | typeDef) && ParseLevel)
    // This is a label.
    return tokGotoLabel;
  
  // check to be sure that 'far' is only used with pointers
  // (for our sake, not C standard sake)
  /// TODO: we also need to check for c[] (arrays)
  //if ((base.flags & tokFarMask) && (tok != '*'))
  //  ERROR(0x016B, FALSE);
  
  for (;;) {
    lastSyntaxPtr = SyntaxStackCnt;
    
    /* derived type */
    tok = ParseDerived(tok, base.flags, ident_flags);
    
    /* base type */
    PushBase(&base);
    
    if ((tok && strchr(",;{=", tok)) || ((tok == ')') && ExprLevel)) {
      bool isIncompleteArr, isFxn, isLocal = FALSE, isGlobal = FALSE, isStruct, isArray;
      unsigned alignment = 0;
      int staticLabel = 0;
      
      // Disallow void variables
      if (SyntaxStack[SyntaxStackCnt - 1].tok == tokVoid) {
        if (SyntaxStack[SyntaxStackCnt - 2].tok == tokIdent && !(cast | typeDef))
          ERROR(0x0213, FALSE, IDENT_STR(SyntaxStack[lastSyntaxPtr].param));
      }
      
      isFxn = (SyntaxStack[lastSyntaxPtr + 1].tok == '(');
      
      // if we don't want to support returning of STRUCTs in functions, add these lines
      //if (isFxn &&
      //    SyntaxStack[SyntaxStackCnt - 1].tok == tokStructPtr &&
      //    SyntaxStack[SyntaxStackCnt - 2].tok == ')')
      //  // structure returning isn't supported currently
      //  ERROR(0x0160, FALSE);
      
      isArray = (SyntaxStack[lastSyntaxPtr + 1].tok == '[');
      isIncompleteArr = (isArray && ((int) SyntaxStack[lastSyntaxPtr + 2].param == 0));
      
      isStruct = (SyntaxStack[lastSyntaxPtr + 1].tok == tokStructPtr);
      
      if (!(ExprLevel || structInfo) && !(external | typeDef | Static) &&
          !strcmp(IDENT_STR(SyntaxStack[lastSyntaxPtr].param), "<something>") && (tok == ';')) {
        if (isStruct) {
          // This is either an incomplete tagged structure/union declaration, e.g. "struct sometag;",
          // or a tagged complete structure/union declaration, e.g. "struct sometag { ... };", without an instance variable,
          // or an untagged complete structure/union declaration, e.g. "struct { ... };", without an instance variable
          int declPtr;
          bool curScope;
          int j = (int) SyntaxStack[lastSyntaxPtr + 1].param;
          
          if (((j + 2) < SyntaxStackCnt) &&
               (IDENT_STR(SyntaxStack[j + 1].param)[0] == '<') && // without tag
               (SyntaxStack[j + 2].tok == tokSizeof))  // but with the {} "body"
            ERROR(0x0160, FALSE);
          
          // If a structure/union with this tag has been declared in an outer scope,
          // this new declaration should override it
          declPtr = FindTaggedDecl(IDENT_STR(SyntaxStack[j + 1].param), lastSyntaxPtr - 1, &curScope);
          if (declPtr >= 0 && !curScope) {
            // If that's the case, unbind this declaration from the old declaration
            // and make it a new incomplete declaration
            PushSyntax(SyntaxStack[j].tok); // tokStruct or tokUnion
            PushSyntax2(tokTag, SyntaxStack[j + 1].param);
            SyntaxStack[lastSyntaxPtr + 1].param = (bit32u) (SyntaxStackCnt - 2);
          }
          return GetToken();
        } else if (SyntaxStack[lastSyntaxPtr + 1].tok == tokEnumPtr)
          return GetToken();
      }
      
      // Convert enums into ints
      if (SyntaxStack[SyntaxStackCnt - 1].tok == tokEnumPtr) {
        SyntaxStack[SyntaxStackCnt - 1].tok = tokInt;
        SyntaxStack[SyntaxStackCnt - 1].param = 0;
        SyntaxStack[SyntaxStackCnt - 1].flags = 0;
      }
      
      // Structure/union members can't be initialized nor be functions nor
      // be incompletely typed arrays inside structure/union declarations
      if (structInfo && ((tok == '=') | isFxn | (tok == '{') | isIncompleteArr))
        ERROR(0x0160, FALSE);
      
      if (typeDef & ((tok == '=') | (tok == '{')))
        ERROR(0x0160, FALSE);
      
      // Error conditions in declarations(/definitions/initializations):
      // Legend:
      //   +  error
      //   -  no error
      //
      // file scope          fxn   fxn {}  var   arr[]   arr[]...[]   arr[incomplete]   arr[incomplete]...[]
      //                     -     -       -     -       -            +                 +
      // file scope          fxn=          var=  arr[]=  arr[]...[]=  arr[incomplete]=  arr[incomplete]...[]=
      //                     +             -     -       +            -                 +
      // file scope  extern  fxn   fxn {}  var   arr[]   arr[]...[]   arr[incomplete]   arr[incomplete]...[]
      //                     -     -       -     -       -            -                 -
      // file scope  extern  fxn=          var=  arr[]=  arr[]...[]=  arr[incomplete]=  arr[incomplete]...[]=
      //                     +             +     +       +            +                 +
      // file scope  static  fxn   fxn {}  var   arr[]   arr[]...[]   arr[incomplete]   arr[incomplete]...[]
      //                     -     -       -     -       -            +                 +
      // file scope  static  fxn=          var=  arr[]=  arr[]...[]=  arr[incomplete]=  arr[incomplete]...[]=
      //                     +             -     -       +            -                 +
      // fxn scope           fxn   fxn {}  var   arr[]   arr[]...[]   arr[incomplete]   arr[incomplete]...[]
      //                     -     +       -     -       -            +                 +
      // fxn scope           fxn=          var=  arr[]=  arr[]...[]=  arr[incomplete]=  arr[incomplete]...[]=
      //                     +             -     +       +            +                 +
      // fxn scope   extern  fxn   fxn {}  var   arr[]   arr[]...[]   arr[incomplete]   arr[incomplete]...[]
      //                     -     +       -     -       -            -                 -
      // fxn scope   extern  fxn=          var=  arr[]=  arr[]...[]=  arr[incomplete]=  arr[incomplete]...[]=
      //                     +             +     +       +            +                 +
      // fxn scope   static  fxn   fxn {}  var   arr[]   arr[]...[]   arr[incomplete]   arr[incomplete]...[]
      //                     +     +       +     +       +            +                 +
      // fxn scope   static  fxn=          var=  arr[]=  arr[]...[]=  arr[incomplete]=  arr[incomplete]...[]=
      //                     +             +     +       +            +                 +
      
      if (isFxn & (tok == '='))
        ERROR(0x0175, FALSE);
      
      if ((isFxn & (tok == '{')) && ParseLevel)
        ERROR(0x0161, FALSE);
      
      if ((isFxn & Static) && ParseLevel)
        ERROR(0x0162, FALSE);
      
      if (external & (tok == '='))
        ERROR(0x0176, FALSE);

      if (isIncompleteArr & !(external | typeDef | (tok == '=')))
        ERROR(0x0163, FALSE);
      
      if (!strcmp(IDENT_STR(SyntaxStack[lastSyntaxPtr].param), "<something>")) {
        // Disallow nameless variables, prototypes, structure/union members and typedefs.
        if (structInfo || typeDef || !ExprLevel)
          ERROR(0x0168, FALSE);
      } else {
        // Disallow named variables and prototypes in sizeof(typedecl) and (typedecl).
        if (ExprLevel && !structInfo)
          ERROR(0x0169, FALSE);
      }
      
      if (!isFxn && !typeDef) {
        // This is a variable or a variable (member) in a struct/union declaration
        int sz = GetDeclSize(lastSyntaxPtr, 0);
        
        if (!((sz | (int) isIncompleteArr) || ExprLevel))  // incomplete type
          ERROR(0x0160, FALSE); // TBD!!! different error when struct/union tag is not found
        
        if (isArray && !GetDeclSize(lastSyntaxPtr + 4, 0))
          // incomplete type of array element (e.g. struct/union)
          ERROR(0x0160, FALSE);
        
        alignment = GetDeclAlignment(lastSyntaxPtr);
        
        if (structInfo) {
          // It's a variable (member) in a struct/union declaration
          unsigned tmp;
          unsigned newAlignment = alignment;
          if (alignment > (unsigned) PragmaPackValue)
            newAlignment = PragmaPackValue;
          // Update structure/union alignment
          if (structInfo->alignment < newAlignment)
            structInfo->alignment = newAlignment;
          // Align structure member
          tmp = structInfo->offset;
          structInfo->offset = (structInfo->offset + newAlignment - 1) & ~(newAlignment - 1);
          if (structInfo->offset < tmp || structInfo->offset != truncUint(structInfo->offset))
            ERROR(0x0172, FALSE);
          // Change tokIdent to tokMemberIdent and insert a local var offset token
          SyntaxStack[lastSyntaxPtr].tok = tokMemberIdent;
          InsertSyntax3(lastSyntaxPtr + 1, tokLocalOfs, (int) structInfo->offset, base.flags);
          
          // Advance member offset for structures, keep it zero for unions
          if (structInfo->type == tokStruct) {
            tmp = structInfo->offset;
            structInfo->offset += sz;
            if (structInfo->offset < tmp || structInfo->offset != truncUint(structInfo->offset))
              ERROR(0x0172, FALSE);
          }
          // Update max member size for unions
          else if (structInfo->max_size < (unsigned) sz)
            structInfo->max_size = sz;
        } else if (ParseLevel && !((external | Static) || ExprLevel)) {
          // It's a local variable
          isLocal = TRUE;
          
          // mark the Ident as Local and if a struct or an array, mark as initialized.
          //   (It isn't actually initialized, though we can use its memory)
          struct S_IDENT_PREFIX *p = (struct S_IDENT_PREFIX *) SyntaxStack[lastSyntaxPtr].param;
          p->flags |= IDENT_FLAGS_ISLOCAL;
          if (isStruct || isArray)
            p->flags |= IDENT_FLAGS_ASSIGNED;
          
          // Defer size calculation until initialization
          // Insert a local var offset token, the offset is to be updated
          InsertSyntax3(lastSyntaxPtr + 1, tokLocalOfs, 0, base.flags);
        } else if (!ExprLevel) {
          // It's a global variable (external, static or neither)
          isGlobal = TRUE;
          
          if (Static && ParseLevel) {
            // It's a static variable in function scope, "rename" it by providing
            // an alternative unique numeric identifier right next to it and use it
            staticLabel = LabelCnt++;
            //InsertSyntax2(++lastSyntaxPtr, tokIdent, AddNumericIdent(staticLabel));
            InsertSyntax3(++lastSyntaxPtr, tokIdent, (bit32u) AddNumericIdent(staticLabel), base.flags);
          }
        }
      }
      
      // If it's a type declaration in a sizeof(typedecl) expression or
      // in an expression with a cast, e.g. (typedecl)expr, we're done
      if (ExprLevel && !structInfo)
        return tok;
      
      if (typeDef) {
        bool CurScope;
        char *s = IDENT_STR(SyntaxStack[lastSyntaxPtr].param);
        SyntaxStack[lastSyntaxPtr].tok = 0; // hide tokIdent for now
        if ((FindTypedef(s, &CurScope, FALSE) >= 0) && CurScope)
          ERROR(0x0171, FALSE, s);
        SyntaxStack[lastSyntaxPtr].tok = tokTypedef; // change tokIdent to tokTypedef
      } else
      
      // fallthrough
      if (isLocal || isGlobal) {
        bool hasInit = (tok == '=');
        bool needsGlobalInit = (isGlobal & !external);
        int sz = GetDeclSize(lastSyntaxPtr, 0);
        int skipLabel = 0;
        int initLabel = 0;
        
        if (hasInit)
          tok = GetToken();
        
        if (isLocal & hasInit)
          needsGlobalInit = isArray | (isStruct & (tok == '{'));
        
        if (needsGlobalInit) {
          if (isLocal | (Static && ParseLevel)) {
            // Global data appears inside code of a function
            if (OutputFormat == FORMATFLAT) {
              skipLabel = LabelCnt++;
              GenJumpUncond(skipLabel);
            } else
              SwitchSection(SECTION_IS_DATA);
          } else {
            // Global data appears between functions
            if (OutputFormat != FORMATFLAT)
              SwitchSection(SECTION_IS_DATA);
          }
          
          // imperfect condition for alignment
          if ((alignment != 1) && (PragmaPackValue > 1))
            GenWordAlignment(PragmaPackValue);
          
          // Generate variable name:  "_vairable"
          if (isGlobal)
            if (Static && ParseLevel)
              GenNumLabel(staticLabel);
            else
              GenLabelIP(SyntaxStack[lastSyntaxPtr].param, Static);
          else {
            // Generate numeric labels for global initializers of local vars
            GenNumLabel(initLabel = LabelCnt++);
          }
          
          // Generate global initializers
          if (hasInit) {
            tok = InitVar(lastSyntaxPtr, tok);
            // Update the size in case it's an incomplete array
            sz = GetDeclSize(lastSyntaxPtr, 0);
          } else
            GenZeroData(sz);
          
          if (isLocal | (Static && ParseLevel)) {
            // Global data appears inside code of a function
//            if (OutputFormat == FORMATFLAT) {
              GenNumLabel(skipLabel);
//            } else {
//              SwitchSection(SECTION_IS_CODE);
//            }
          } else {
//            // Global data appears between functions
//            if (OutputFormat != FORMATFLAT)
//              puts_out(DataFooter);
          }
////        
        }

        if (isLocal)
          // Now that the size of the local is certainly known,
          // update its offset in the offset token
          SyntaxStack[lastSyntaxPtr + 1].param = AllocLocal(sz);
        
        // Copy global initializers into local vars
        if (isLocal && needsGlobalInit) {
          if (SizeOfWord == 2) {
            if (!StructCpyLabel16)
              StructCpyLabel16 = LabelCnt++;
          } else {
            if (!StructCpyLabel32)
              StructCpyLabel32 = LabelCnt++;
          }
          
          sp = 0;
          push2('(', SizeOfWord * 3);
          push2(tokLocalOfs, SyntaxStack[lastSyntaxPtr + 1].param);
          push(',');
          push2(tokIdent, (bit32u) AddNumericIdent(initLabel));
          push(',');
          push2(tokNumUint, sz);
          push(',');
          if (SizeOfWord == 2)
            push2(tokIdent, (bit32u) AddNumericIdent(StructCpyLabel16));
          else
            push2(tokIdent, (bit32u) AddNumericIdent(StructCpyLabel32));
          push2(')', SizeOfWord * 3);
          
          GenExpr();
        }
        
        // Initialize local vars with expressions
        else if (hasInit && !needsGlobalInit) {
          int gotUnary, synPtr, constExpr, exprVal;
          bool brace = FALSE;
          
          // Initializers for scalars can be optionally enclosed in braces
          if ((!isStruct) & (tok == '{')) {
            brace = TRUE;
            tok = GetToken();
          }
          
          // ParseExpr() will transform the initializer expression into an assignment expression here
          tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, '=', (int) SyntaxStack[lastSyntaxPtr].param);
          
          if (!gotUnary)
            ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));

          if (brace) {
            if (tok != '}')
              ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
            tok = GetToken();
          }

          if (!isStruct) {
            // This is a special case for initialization of integers smaller than int.
            // Since a local integer variable always takes as much space as a whole int,
            // we can optimize code generation a bit by storing the initializer as an int.
            // This is an old accidental optimization and I preserve it for now.
            stack[sp - 1].param = (bit32u) SizeOfWord;
            stack[sp - 1].flags = 0;
          }
          
          // Storage of string literal data from the initializing expression
          // occurs here.
          GenExpr();
        }
      } else if (tok == '{') {
        // It's a function body. Let's add function parameters as
        // local variables to the symbol table and parse the body.
        int undoSymbolsPtr = SyntaxStackCnt;
        struct S_IDENT_PREFIX *undoIdents = LastIdent;
        int locAllocLabel = (LabelCnt += 2) - 2;
        int i;
        
        CurFxnName = IDENT_STR(SyntaxStack[lastSyntaxPtr].param);
        isMain = !strcmp(CurFxnName, "main");
        
        gotoLabCnt = 0;
        
        if (verbose)
          printf("%s()\n", CurFxnName);
        
        ParseLevel++;
        GetFxnInfo(lastSyntaxPtr, &CurFxnParamCntMin, &CurFxnParamCntMax, &CurFxnReturnExprTypeSynPtr, NULL); // get return type

        // Make sure the return structure type is complete
        if (CurFxnReturnExprTypeSynPtr >= 0 &&
            SyntaxStack[CurFxnReturnExprTypeSynPtr].tok == tokStructPtr &&
            !GetDeclSize(CurFxnReturnExprTypeSynPtr, 0))
          ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
        
        if (OutputFormat != FORMATFLAT)
          SwitchSection(SECTION_IS_CODE);
        // Generate Function name:  "_function:"
        //puts_out("");
        GenLabelIP(SyntaxStack[lastSyntaxPtr].param, Static);
        
        if (!NakedFlag) {
          //if (SyntaxStack[lastSyntaxPtr + 1].param & 1)
          //  GenIsrProlog();
          //else // fallthrough
            GenFxnProlog();
        }
        CurFxnEpilogLabel = 0;
        
        AddFxnParamSymbols(lastSyntaxPtr);
        
        {
          CurFxnNameLabel = LabelCnt++;
          SyntaxStack[SymFuncPtr].param = (bit32u) AddNumericIdent(CurFxnNameLabel);
          SyntaxStack[SymFuncPtr + 2].param = (bit32u) strlen(CurFxnName) + 1;
        }
        
        tok = ParseBlock(NULL, 0);
        ParseLevel--;
        if (tok != '}')
          ERROR(0x011B, FALSE, GetTokenName(tok));
        
        for (i = 0; i < gotoLabCnt; i++)
          if (gotoLabels[i].stat == 2)
            ERROR(0x0104, FALSE, IDENT_STR(gotoLabels[i].ident));
        
        // if execution of main() reaches here, before the epilog (i.e. without using return),
        // main() should return 0.
        if (isMain) {
          sp = 0;
          push(tokNumInt);
          push(tokReturn); // value produced by generated code is used
          GenExpr();
        }
        
        // only need to include the label if we used it
        if (CurFxnEpilogLabel > 0)
          GenNumLabel(CurFxnEpilogLabel); // label after last statement in function
        
        if (!NakedFlag) {
          //if (SyntaxStack[lastSyntaxPtr + 1].param & 1)
          //  GenIsrEpilog();
          //else // fallthrough
            GenFxnEpilog();
        }
        if (OutputFormat != FORMATFLAT)
          puts_out(CodeFooter);
        
        NakedFlag = FALSE;
        
        if (CurFxnNameLabel < 0) {
          //CurFxnNameLabel = -AddString(-CurFxnNameLabel, CurFxnName, (int) SyntaxStack[SymFuncPtr + 2].param);
          
          if (OutputFormat != FORMATFLAT)
            SwitchSection(SECTION_IS_DATA);
          
          GenNumLabel(-CurFxnNameLabel);
          //GenLabelIP(SyntaxStack[SymFuncPtr].param, TRUE);
          
          //sp = 1;
          //stack[0].tok = tokIdent;
          //stack[0].param = (bit32u) ((int) SyntaxStack[SymFuncPtr].param + 2);
          //GenStrData(0, 0, 0);
          GenStartAsciiString(FALSE);
          printf_out("\"%s\"\n", CurFxnName);
          GenZeroData(1);
          
          if (OutputFormat != FORMATFLAT)
            puts_out(DataFooter);
          
          CurFxnNameLabel = 0;
        }
        
        CurFxnName = NULL;
        UndoNonLabelIdents(undoIdents, FALSE); // remove all temporary identifier names
        SyntaxStackCnt = undoSymbolsPtr; // remove all params and locals
      }
      
      if ((tok == ';') || (tok == '}'))
        break;
      
      tok = GetToken();
      continue;
    }
    
    ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  }
  
  tok = GetToken();
  return tok;
}

void ParseFxnParams(int tok) {
  struct S_SYNTAX_STACK base;
  int lastSyntaxPtr;
  int cnt = 0;
  int ellCnt = 0;
  bit32u ident_flags = 0;
  
  for (;;) {
    lastSyntaxPtr = SyntaxStackCnt;
    
    if (tok == ')') /* unspecified params */
      break;
    
    if (!TokenStartsDeclaration(tok, 1)) {
      if (tok == tokEllipsis) {
        // "..." cannot be the first parameter and
        // it can be only one
        if (!cnt || ellCnt)
          ERROR(0x011C, FALSE);
        ellCnt++;
      } else
        ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
      base.tok = tok; // "..."
      base.param = 0;
      PushSyntax2(tokIdent, (bit32u) AddIdent("<something>", 0));
      tok = GetToken();
    } else {
      if (ellCnt)
        ERROR(0x011D, FALSE);
      
      /* base type */
      tok = ParseBase(tok, &base, &ident_flags);
      
      /* derived type */
      tok = ParseDerived(tok, base.flags, ident_flags);
    }
    
    /* base type */
    PushBase(&base);
    
    // Convert enums into ints
    if (SyntaxStack[SyntaxStackCnt - 1].tok == tokEnumPtr) {
      SyntaxStack[SyntaxStackCnt - 1].tok = tokInt;
      SyntaxStack[SyntaxStackCnt - 1].param = 0;
      SyntaxStack[SyntaxStackCnt - 1].flags = 0;
    }
    
    /* Decay arrays to pointers */
    lastSyntaxPtr++; /* skip name */
    if (SyntaxStack[lastSyntaxPtr].tok == '[') {
      int t;
      // TODO: first get SyntaxStack[lastSyntaxPtr].flags then restore it below. ????
      DeleteSyntax(lastSyntaxPtr, 1);
      t = SyntaxStack[lastSyntaxPtr].tok;
      if (t == tokNumInt || t == tokNumUint)
        DeleteSyntax(lastSyntaxPtr, 1);
      SyntaxStack[lastSyntaxPtr].tok = '*';
    }
    /* "(Un)decay" functions to function pointers */
    else if (SyntaxStack[lastSyntaxPtr].tok == '(') {
      InsertSyntax(lastSyntaxPtr, '*');
    }
    lastSyntaxPtr--; /* "unskip" name */
    
    cnt++;
    
    if (tok == ')' || tok == ',') {
      int t = SyntaxStack[SyntaxStackCnt - 2].tok;
      if (SyntaxStack[SyntaxStackCnt - 1].tok == tokVoid) {
        // Disallow void variables. TBD!!! de-uglify
        if (t == tokIdent &&
            !(!strcmp(IDENT_STR(SyntaxStack[SyntaxStackCnt - 2].param), "<something>") &&
              cnt == 1 && tok == ')'))
          ERROR(0x0213, FALSE, IDENT_STR(SyntaxStack[lastSyntaxPtr].param));
      }
      
      // if we don't want to support returning of STRUCTs in functions, add these lines
      //if ((SyntaxStack[SyntaxStackCnt - 1].tok == tokStructPtr) && (t != '*') && (t != ']'))
      //  // structure passing and returning isn't supported currently
      //  ERROR(0x0160, FALSE);
      
      if (tok == ')')
        break;
      
      tok = GetToken();
      continue;
    }
    
    ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  }
}

void AddFxnParamSymbols(int SyntaxPtr) {
  int i;
  unsigned paramOfs = SizeOfWord + SizeOfEIP;  // size of the return address and size of (e)bp
  
  if (SyntaxPtr < 0 ||
      SyntaxPtr > SyntaxStackCnt - 3 ||
      SyntaxStack[SyntaxPtr].tok != tokIdent ||
      SyntaxStack[SyntaxPtr + 1].tok != '(')
    ERROR(0xE010, FALSE);
  
  CurFxnSyntaxPtr = SyntaxPtr;
  CurFxnLocalOfs = 0;
  CurFxnMinLocalOfs = 0;
  
  if (CurFxnReturnExprTypeSynPtr >= 0 &&
    SyntaxStack[CurFxnReturnExprTypeSynPtr].tok == tokStructPtr) {
    // The function returns a struct/union via an implicit param/arg (pointer to struct/union)
    // before its first formal param/arg, add this implicit param/arg
    PushSyntax2(tokIdent, (bit32u) AddIdent("@", 0)); // special implicit param/arg (pretval) pointing to structure receptacle
    PushSyntax2(tokLocalOfs, paramOfs);
    PushSyntax('*');
    PushSyntax2(tokStructPtr, SyntaxStack[CurFxnReturnExprTypeSynPtr].param);
    paramOfs += SizeOfWord;
  }
  
  SyntaxPtr += 2; // skip "ident("
  
  for (i = SyntaxPtr; i < SyntaxStackCnt; i++) {
    int tok = SyntaxStack[i].tok;
    
    if (tok == tokIdent) {
      unsigned sz;
      
      if (i + 1 >= SyntaxStackCnt)
        ERROR(0xE011, FALSE);
      
      if (SyntaxStack[i + 1].tok == tokVoid) // "ident(void)" = no params
        break;
      if (SyntaxStack[i + 1].tok == tokEllipsis) // "ident(something,...)" = no more params
        break;
      
      sz = GetDeclSize(i, 0);
      if (sz == 0)
        ERROR(0xE012, FALSE);
      
      PushSyntax2(SyntaxStack[i].tok, SyntaxStack[i].param);
      PushSyntax2(tokLocalOfs, paramOfs);
      
      if (sz + SizeOfWord - 1 < sz)
        ERROR(0x0172, FALSE);
      sz = (sz + SizeOfWord - 1) & ~(SizeOfWord - 1u);
      if (paramOfs + sz < paramOfs)
        ERROR(0x0172, FALSE);
      paramOfs += sz;
      if (paramOfs > (unsigned) GenMaxLocalsSize())
        ERROR(0x0172, FALSE);
      
      // Duplicate this parameter in the symbol table
      i++;
      while (i < SyntaxStackCnt) {
        tok = SyntaxStack[i].tok;
        if (tok == tokIdent || tok == ')') {
          CurFxnParamCnt++;
          i--;
          break;
        } else if (tok == '(') {
          int c = 1;
          i++;
          PushSyntax(tok);
          while (c && i < SyntaxStackCnt) {
            tok = SyntaxStack[i].tok;
            c += (tok == '(') - (tok == ')');
            PushSyntax2(SyntaxStack[i].tok, SyntaxStack[i].param);
            // PushSyntax3(SyntaxStack[i].tok, SyntaxStack[i].param, SyntaxStack[i].flags);  // ?????????
            i++;
          }
        } else {
          PushSyntax3(SyntaxStack[i].tok, SyntaxStack[i].param, SyntaxStack[i].flags);
          i++;
        }
      }
    } else if (tok == ')') // endof "ident(" ... ")"
      break;
    else
      ERROR(0xE013, FALSE, GetTokenName(tok));
  }
}

int ParseStatement(int tok, struct S_BRKCNTSWCHTARGET *BrkCntTarget, int casesIndex) {
/*
  labeled statements:
  + ident : statement
  + case const-expr : statement
  + default : statement

  compound statement:
  + { declaration(s)/statement(s)-opt }

  expression statement:
  + expression-opt ;

  selection statements:
  + if ( expression ) statement
  + if ( expression ) statement else statement
  + switch ( expression ) { statement(s)-opt }

  iteration statements:
  + while ( expression ) statement
  + do statement while ( expression ) ;
  + for ( expression-opt ; expression-opt ; expression-opt ) statement

  jump statements:
  + goto ident ;
  + continue ;
  + break ;
  + return expression-opt ;
*/
  int gotUnary, synPtr,  constExpr, exprVal;
  struct S_BRKCNTSWCHTARGET brkCntTarget;
  bool statementNeeded;

  do {
    statementNeeded = FALSE;
    
    if (tok == ';')
      tok = GetToken();
    
    else if (tok == '{') {
      // A new {} block begins in the function body
      int undoSymbolsPtr = SyntaxStackCnt;
      int undoLocalOfs = CurFxnLocalOfs;
      struct S_IDENT_PREFIX *undoIdents = LastIdent;
      ParseLevel++;
      tok = ParseBlock(BrkCntTarget, casesIndex);
      ParseLevel--;
      if (tok != '}')
        ERROR(0x011B, FALSE, GetTokenName(tok));
      UndoNonLabelIdents(undoIdents, TRUE); // remove all identifier names, except those of labels
      SyntaxStackCnt = undoSymbolsPtr; // remove all params and locals
      CurFxnLocalOfs = undoLocalOfs; // destroy on-stack local variables
      tok = GetToken();
    } else if (tok == tokReturn) {
      // DONE: functions returning void vs non-void
      // TBD??? functions returning void should be able to return void
      //        return values from other functions returning void
      int retVoid = CurFxnReturnExprTypeSynPtr >= 0 &&
                    SyntaxStack[CurFxnReturnExprTypeSynPtr].tok == tokVoid;
      tok = GetToken();
      if (tok == ';') {
        gotUnary = 0;
        if (!retVoid)
          ERROR(0x011E, FALSE);
      } else {
        if (retVoid)
          ERROR(0x011F, FALSE);
        if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ';')
          ERROR(0x0120, FALSE);
        if (gotUnary)
          if (nonVoidTypeCheck(synPtr)) ERROR(0x020B, FALSE);
      }
      if (gotUnary) {
        int structs = (synPtr >= 0 && SyntaxStack[synPtr].tok == tokStructPtr) +
          (CurFxnReturnExprTypeSynPtr >= 0 && SyntaxStack[CurFxnReturnExprTypeSynPtr].tok == tokStructPtr) * 2;
        
        if (structs) {
          if (structs != 3 || SyntaxStack[synPtr].param != SyntaxStack[CurFxnReturnExprTypeSynPtr].param)
            ERROR(0x0202, FALSE, GetTokenName(tok));
          
          // Transform "return *pstruct" into structure assignment ("*pretval = *pstruct")
          // via function call "fxn(sizeof *pretval, pstruct, pretval)".
          
          // There are a couple of differences to how this is implemented in the assignment operator:
          // - the structure dereference has already been dropped from *pstruct by ParseExpr(),
          //   so it isn't removed here
          // - we don't add the structure dereference on top of the value returned by "fxn()"
          //   because the return statement is not an expression that can be an operand into another
          //   operator
          ins(0, ',');
          ins2(0, tokUnaryStar, SizeOfWord); // dereference to extract the implicit param/arg (pretval) from the stack
          ins2(0, tokLocalOfs, SyntaxStack[FindSymbol("@") + 1].param); // special implicit param/arg (pretval) pointing to structure receptacle
          ins2(0, '(', SizeOfWord * 3);
          push(',');
          push2(tokNumUint, GetDeclSize(synPtr, 0));
          push(',');
          if (SizeOfWord == 2) {
            if (!StructCpyLabel16)
              StructCpyLabel16 = LabelCnt++;
            push2(tokIdent, (bit32u) AddNumericIdent(StructCpyLabel16));
          } else {
            if (!StructCpyLabel32)
              StructCpyLabel32 = LabelCnt++;
            push2(tokIdent, (bit32u) AddNumericIdent(StructCpyLabel32));
          }
          push2(')', SizeOfWord * 3);
        } else {
          // If return value (per function declaration) is a scalar type smaller than machine word,
          // properly zero- or sign-extend the returned value to machine word size.
          // TBD??? Move this cast to the caller?
          int castSize = GetDeclSize(CurFxnReturnExprTypeSynPtr, 1);
          if (castSize != SizeOfWord && (synPtr < 0 || castSize != GetDeclSize(synPtr, 1))) {
            switch (castSize) {
              case 1:
                push(tokUChar);
                break;
              case -1:
                push(tokSChar);
                break;
              case 2:
                push(tokUShort);
                break;
              case -2:
                push(tokShort);
                break;
            }
          }
        }
        push(tokReturn); // value produced by generated code is used
        GenExpr();
      }
      tok = GetToken();
      // If this return is the last statement in the function, the epilogue immediately
      // follows and there's no need to jump to it.
      if (!(tok == '}' && ParseLevel == 1 && !isMain)) {
        if (CurFxnEpilogLabel == 0)
          CurFxnEpilogLabel = LabelCnt++;
        GenJumpUncond(CurFxnEpilogLabel);
      }
    }
    else if (tok == tokWhile) {
      int labelBefore = LabelCnt++;
      int labelAfter = LabelCnt++;
      int forever = 0;
      
      tok = GetToken();
      if (tok != '(')
        ERROR(0x0121, FALSE);
      
      tok = GetToken();
      if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ')')
        ERROR(0x0122, FALSE);
      
      if (!gotUnary)
        ERROR(0x0123, FALSE);
      
      // don't allow void control expressions
      if (scalarTypeCheck(synPtr))
        ERROR(0x0124, FALSE);
      
      GenNumLabel(labelBefore);
      
      if (constExpr) {
        // Special cases for while(0) and while(1)
        if (!(forever = truncInt(exprVal)))
          GenJumpUncond(labelAfter);
      } else {
        switch (stack[sp - 1].tok) {
          case '<':
          case '>':
          case tokEQ:
          case tokNEQ:
          case tokLEQ:
          case tokGEQ:
          case tokULess:
          case tokUGreater:
          case tokULEQ:
          case tokUGEQ:
            push2(tokIfNot, labelAfter);
            GenExpr();
            break;
          default:
            push(tokReturn); // value produced by generated code is used
            GenExpr();
            GenJumpIfZero(labelAfter);
            break;
        }
      }
      
      tok = GetToken();
      
      brkCntTarget.Break = labelAfter;  // break target
      brkCntTarget.Continue = labelBefore; // continue target
      tok = ParseStatement(tok, &brkCntTarget, casesIndex);
      
      // Special case for while(0)
      if (!(constExpr && !forever))
        GenJumpUncond(labelBefore);
      GenNumLabel(labelAfter);
      
    } else if (tok == tokDo) {
      int labelBefore = LabelCnt++;
      int labelWhile = LabelCnt++;
      int labelAfter = LabelCnt++;
      GenNumLabel(labelBefore);
      
      tok = GetToken();
      brkCntTarget.Break = labelAfter; // break target
      brkCntTarget.Continue = labelWhile; // continue target
      tok = ParseStatement(tok, &brkCntTarget, casesIndex);
      
      if (tok != tokWhile)
        ERROR(0x0125, FALSE);
      
      tok = GetToken();
      if (tok != '(')
        ERROR(0x0126, FALSE);
      
      tok = GetToken();
      if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ')')
        ERROR(0x0127, FALSE);
      
      if (!gotUnary)
        ERROR(0x0123, FALSE);
      
      tok = GetToken();
      if (tok != ';')
        ERROR(0x0128, FALSE);

      // DONE: void control expressions
      if (scalarTypeCheck(synPtr)) ERROR(0x020F, FALSE);
      
      GenNumLabel(labelWhile);
      
      if (constExpr) {
        // Special cases for while(0) and while(1)
        if (truncInt(exprVal))
          GenJumpUncond(labelBefore);
      } else {
        switch (stack[sp - 1].tok) {
          case '<':
          case '>':
          case tokEQ:
          case tokNEQ:
          case tokLEQ:
          case tokGEQ:
          case tokULess:
          case tokUGreater:
          case tokULEQ:
          case tokUGEQ:
            push2(tokIf, labelBefore);
            GenExpr();
            break;
          default:
            push(tokReturn); // value produced by generated code is used
            GenExpr();
            GenJumpIfNotZero(labelBefore);
            break;
        }
      }
      
      GenNumLabel(labelAfter);

      tok = GetToken();
    } else if (tok == tokIf) {
      int labelAfterIf = LabelCnt++;
      int labelAfterElse = LabelCnt++;
      int labelShortJmp = LabelCnt++;
      
      tok = GetToken();
      if (tok != '(')
        ERROR(0x0129, FALSE);
      
      tok = GetToken();
      if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ')')
        ERROR(0x012A, FALSE);
      
      if (!gotUnary)
        ERROR(0x012B, FALSE);

      // void control expressions
      if (scalarTypeCheck(synPtr)) ERROR(0x020E, FALSE);

      if (constExpr) {
        // Special cases for if(0) and if(1)
        if (!truncInt(exprVal))
          GenJumpUncond(labelAfterIf);
      } else {
        switch (stack[sp - 1].tok) {
          case '<':
          case '>':
          case tokEQ:
          case tokNEQ:
          case tokLEQ:
          case tokGEQ:
          case tokULess:
          case tokUGreater:
          case tokULEQ:
          case tokUGEQ:
            if (SizeOfWord == 2) {
              // must jump over the short jcc jmp in 16-bit code
              push2(tokIf, labelShortJmp);
              GenExpr();
              GenJumpUncond(labelAfterIf);
              GenNumLabel(labelShortJmp);
            } else {
              // in 32-bit code, we have a longer jump
              push2(tokIfNot, labelAfterIf);
              GenExpr();
            }
            break;
          default:
            push(tokReturn); // value produced by generated code is used
            GenExpr();
            GenJumpIfZero(labelAfterIf);
            break;
        }
      }
      
      tok = GetToken();
      tok = ParseStatement(tok, BrkCntTarget, casesIndex);

      // else
      if (tok == tokElse) {
        GenJumpUncond(labelAfterElse);
        GenNumLabel(labelAfterIf);
        tok = GetToken();
        tok = ParseStatement(tok, BrkCntTarget, casesIndex);
        GenNumLabel(labelAfterElse);
      } else
        GenNumLabel(labelAfterIf);
      
    } else if (tok == tokFor) {
      int labelBefore = LabelCnt++;
      int labelExpr3 = LabelCnt++;
      int labelBody = LabelCnt++;
      int labelAfter = LabelCnt++;
      int cond = -1;
      static struct S_SYNTAX_STACK expr3Stack[129 / 2];
      static int expr3Sp;
      
      tok = GetToken();
      if (tok != '(')
        ERROR(0x012C, FALSE);
      
      tok = GetToken();
      
      // This allows for 'for(int i=0; i< ....)
      // Must save this position to restore later.  The C99 says that any declarations
      //  in this for() must not be available outside the scope of this for()
      int forUndoSymbolsPtr, forUndoLocalOfs;
      struct S_IDENT_PREFIX *forUndoIdents;
      bool forUsedDeclarations = FALSE;
      if (TokenStartsDeclaration(tok, 1)) {
        if (!allowC99code) ERROR(0x1000, FALSE);
        forUndoSymbolsPtr = SyntaxStackCnt;
        forUndoLocalOfs = CurFxnLocalOfs;
        forUndoIdents = LastIdent;
        forUsedDeclarations = TRUE;
        PushSyntax('#'); // mark the beginning of a new scope
        tok = ParseDecl(tok, NULL, FALSE, 0);
      } else {
        if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ';')
          ERROR(0x012D, FALSE);
              
        if (gotUnary)
          GenExpr();
        
        tok = GetToken();
      }
      
      GenNumLabel(labelBefore);
      // at 'while' expression
      if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ';')
        ERROR(0x012E, FALSE);
      
      if (gotUnary) {
        // DONE: void control expressions
        if (scalarTypeCheck(synPtr)) ERROR(0x020C, FALSE);
        
        /* If the 'condition' is a positive constant, there is no need
         *  produce the condition code.
         * If it is a zero constant, we can simply produce a jmp to the
         *  labelAfter.
         * Please see the note in the while() above about having to still
         *  produce the code though.
         */
        if (constExpr) {
          // Special cases for for(...; 0; ...) and for(...; 1; ...)
          cond = truncInt(exprVal) != 0;
        } else {
          switch (stack[sp - 1].tok) {
            case '<':
            case '>':
            case tokEQ:
            case tokNEQ:
            case tokLEQ:
            case tokGEQ:
            case tokULess:
            case tokUGreater:
            case tokULEQ:
            case tokUGEQ:
              push2(tokIfNot, labelAfter);
              GenExpr();
              break;
            default:
              push(tokReturn); // value produced by generated code is used
              GenExpr();
              GenJumpIfZero(labelAfter);
              break;
          }
        }
      } else
        // Special case for for(...; ; ...)
        cond = 1;
      
      if (!cond)
        // Special case for for(...; 0; ...)
        GenJumpUncond(labelAfter);
      
      tok = GetToken();
      if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ')')
        ERROR(0x012F, FALSE);
      
      // Try to reorder expr3 with body to reduce the number of jumps, favor small expr3's
      if (gotUnary && (sp <= 16) && ((unsigned) sp <= (sizeof(expr3Stack) / sizeof(expr3Stack[0]) - expr3Sp))) {
        int cnt = sp;
        // Stash the stack containing expr3
        memcpy(expr3Stack + expr3Sp, stack, cnt * sizeof(stack[0]));
        expr3Sp += cnt;
        
        // Body
        tok = GetToken();
        brkCntTarget.Break = labelAfter; // break target
        brkCntTarget.Continue = labelExpr3; // continue target
        tok = ParseStatement(tok, &brkCntTarget, casesIndex);
        
        // Unstash expr3 and generate code for it
        expr3Sp -= cnt;
        memcpy(stack, expr3Stack + expr3Sp, cnt * sizeof(stack[0]));
        sp = cnt;
        GenNumLabel(labelExpr3);
        GenExpr();
        
        // Special case for for(...; 0; ...)
        if (cond)
          GenJumpUncond(labelBefore);
      } else {
        if (gotUnary) {
          GenJumpUncond(labelBody);
          // expr3
          GenNumLabel(labelExpr3);
          GenExpr();
          GenJumpUncond(labelBefore);
          GenNumLabel(labelBody);
        }

        // Body
        tok = GetToken();
        brkCntTarget.Break = labelAfter; // break target
        brkCntTarget.Continue = gotUnary ? labelExpr3 : (cond ? labelBefore : labelAfter); // continue target
        tok = ParseStatement(tok, &brkCntTarget, casesIndex);
        
        // Special case for for(...; 0; ...)
        if (brkCntTarget.Continue != labelAfter)
          GenJumpUncond(brkCntTarget.Continue);
      }
      
      GenNumLabel(labelAfter);
      
      // undo any declarations done in the for() parameter set.
      if (forUsedDeclarations) {
        UndoNonLabelIdents(forUndoIdents, FALSE); // remove all temporary identifier names
        SyntaxStackCnt = forUndoSymbolsPtr; // remove all params and locals
        CurFxnLocalOfs = forUndoLocalOfs;   // destroy on-stack local variables
      }
      
    } else if (tok == tokBreak) {
      if ((tok = GetToken()) != ';')
        ERROR(0x0120, FALSE);
      tok = GetToken();
      
      if (BrkCntTarget == NULL)
        ERROR(0x013C, FALSE);
      else
        GenJumpUncond(BrkCntTarget->Break);

    } else if (tok == tokCont) {
      if ((tok = GetToken()) != ';')
        ERROR(0x0120, FALSE);
      tok = GetToken();
      
      if ((BrkCntTarget == NULL) || (BrkCntTarget->Continue == 0))
        ERROR(0x013D, FALSE);
      GenJumpUncond(BrkCntTarget->Continue);
      
    } else if (tok == tokSwitch) {
      int undoCases = CasesCnt;
      int brkLabel = LabelCnt++;
      int lbl = LabelCnt++;
      int i;
      
      tok = GetToken();
      // token must start and end with ()'s
      if (tok != '(')
        ERROR(0x0130, FALSE);
      
      // get the first part of the expression
      tok = GetToken();
      if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ')')
        ERROR(0x0131, FALSE);
      
      // must be TRUE or FALSE
      if (!gotUnary)
        ERROR(0x0132, FALSE);
      
      // cannot be void control expressions
      if (scalarTypeCheck(synPtr))
        ERROR(0x020D, FALSE);
      
      // generate the expression
      push(tokReturn); // value produced by generated code is used
      GenExpr();
      
      tok = GetToken();
      
      // Skip the code for the cases
      GenJumpUncond(lbl);
      
      brkCntTarget.Break = brkLabel; // break target
      brkCntTarget.Continue = 0; // continue target
      if (BrkCntTarget)
        // Preserve the continue target
        brkCntTarget.Continue = BrkCntTarget->Continue; // continue target
      
      // Reserve a slot in the case table for the default label
      AddCase(0, 0);
      
      tok = ParseStatement(tok, &brkCntTarget, CasesCnt);
      
      // If there's no default target, will use the break target as default
      if (!Cases[undoCases].Label)
        Cases[undoCases].Label = brkLabel;
      
      // End of switch reached (not via break), skip conditional jumps
      GenJumpUncond(brkLabel);
      // Generate conditional jumps
      GenNumLabel(lbl);
      for (i = undoCases + 1; i < CasesCnt; i++)
        GenJumpIfEqual(Cases[i].Value, Cases[i].Label);
      
      // If none of the cases matches, take the default case
      if (Cases[undoCases].Label != brkLabel)
        GenJumpUncond(Cases[undoCases].Label);
      GenNumLabel(brkLabel); // break label
      
      CasesCnt = undoCases;
      
    } else if (tok == tokCase) {
      int i;
      
      if (!casesIndex)
        ERROR(0x013E, FALSE);
      
      tok = GetToken();
      if ((tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, 0, 0)) != ':')
        ERROR(0x0142, FALSE);
      
      if (!gotUnary || !constExpr || (synPtr >= 0 && SyntaxStack[synPtr].tok == tokVoid)) // TBD???
        ERROR(0x0143, FALSE);
      
      // Check for dups
      exprVal = truncInt(exprVal);
      for (i = casesIndex; i < CasesCnt; i++)
        if (Cases[i].Value == exprVal)
          ERROR(0x0144, FALSE, exprVal);
      
      AddCase(exprVal, LabelCnt);
      GenNumLabel(LabelCnt++); // case exprVal:
      
      tok = GetToken();
      
      // a statement is needed after "case:"
      statementNeeded = TRUE;
      
    } else if (tok == tokDefault) {
      if (!casesIndex)
        ERROR(0x013F, FALSE);
      
      if (Cases[casesIndex - 1].Label)
        ERROR(0x0145, FALSE);
      
      tok = GetToken();
      if (tok != ':')
        ERROR(0x0142, FALSE);
      
      tok = GetToken();
      
      GenNumLabel(Cases[casesIndex - 1].Label = LabelCnt++); // default:
      
      // a statement is needed after "default:"
      statementNeeded = TRUE;
    } else if (tok == tok_Asm) {
      tok = GetToken();
      if (tok != '(')
        ERROR(0x0137, FALSE);
      
      tok = GetToken();
      if (tok != tokLitStr)
        ERROR(0x0138, FALSE);
      
      do {
        GetString('"', 'a');
        tok = GetToken();
      } while (tok == tokLitStr); // concatenate adjacent string literals
      printf_out("\n");
      
      if (tok != ')')
        ERROR(0x0139, FALSE);
      
      tok = GetToken();
      if (tok != ';')
        ERROR(0x0120, FALSE);
      
      tok = GetToken();
    }
    
    else if (tok == tokGoto) {
      if ((tok = GetToken()) != tokIdent)
        ERROR(0x010F, FALSE, GetTokenName(tok));
      GenJumpUncond(AddGotoLabel(TokenIdentName, 0));
      if ((tok = GetToken()) != ';')
        ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
      tok = GetToken();
    } else {
      tok = ParseExpr(tok, &gotUnary, &synPtr, &constExpr, &exprVal, tokGotoLabel, 0);
      if (tok == tokGotoLabel) {
        // found a label
        GenNumLabel(AddGotoLabel(IDENT_STR(stack[0].param), 1));
        // a statement is needed after "label:"
        statementNeeded = TRUE;
      } else {
        if (tok != ';')
          ERROR(0x0120, FALSE);
        if (gotUnary)
          GenExpr();
      }
      tok = GetToken();
    }
  } while (statementNeeded);
  
  return tok;
}
int ParseBlock(struct S_BRKCNTSWCHTARGET *BrkCntTarget, int casesIndex) {
  int tok = GetToken();
  
  PushSyntax('#'); // mark the beginning of a new scope
  
  for (;;) {
    if (tok == 0)
      return tok;
    
    if (tok == '}' && ParseLevel > 0)
      return tok;
    
    if (TokenStartsDeclaration(tok, 0)) {
      tok = ParseDecl(tok, NULL, FALSE, 1);
      if (tok == tokGotoLabel) {
        GenNumLabel(AddGotoLabel(TokenIdentName, 1));
        tok = GetToken();
        // a statement is needed after "label:"
        tok = ParseStatement(tok, BrkCntTarget, casesIndex);
      }
    }
    else if ((ParseLevel > 0) || (tok == tok_Asm))
      tok = ParseStatement(tok, BrkCntTarget, casesIndex);
    else
      ERROR(0x010F, FALSE, (tok == tokIdent) ? TokenIdentName : GetTokenName(tok));
  }
}

int main(int argc, char *argv[]) {
  int i, len;
  char *p, targ_filename[MAX_FILE_NAME_LEN + 1];
  
  // print title to strerr (not redirectable)
  fprintf(stderr, strtstr);
  
  GenInit();
  
  //if (sizeof(int) != sizeof(bit32u))
  //  ERROR();
  
  // Parse the command line parameters
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-seg16")) {
      OutputFormat = FORMATSEGMENTED; 
      SizeOfWord = 2;
      continue;
    }  
    
    else if (!strcmp(argv[i], "-flat16")) {
      OutputFormat = FORMATFLAT; 
      SizeOfWord = 2;
      continue;
    }

    else if (!strcmp(argv[i], "-seg32")) {
      OutputFormat = FORMATSEGMENTED; 
      SizeOfWord = 4;
      continue;
    }
    
    else if (!strcmp(argv[i], "-flat32")) {
      OutputFormat = FORMATFLAT; 
      SizeOfWord = 4;
      continue;
    }
    
    else if (!strcmp(argv[i], "-is-lib")) {
      is_library_code = TRUE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-signed-char")) {
      // this is the default option
      CharIsSigned = TRUE;
      continue;
    } 
    
    else if (!strcmp(argv[i], "-unsigned-char")) {
      CharIsSigned = FALSE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-ctor-fxn")) {
      if (i + 1 < argc) {
        MainPrologCtorFxn = argv[++i];
        continue;
      }
    }
    
    else if (!strcmp(argv[i], "-leading-underscore")) {
      // this is the default option for x86
      UseLeadingUnderscores = TRUE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-no-leading-underscore")) {
      // this is the default option for MIPS
      UseLeadingUnderscores = FALSE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-label")) {
      if ((i + 1) < argc) {
        LabelCnt = atoi(argv[++i]);
        continue;
      }
    }
    
    else if (!strcmp(argv[i], "-no-externs")) {
      GenExterns = FALSE;
      continue;
    }

    else if (!strcmp(argv[i], "-dump-tables")) {
      dump_tables = TRUE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-c98-only")) {
      allowC99code = FALSE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-allow-c99")) {
      allowC99code = TRUE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-verbose")) {
      verbose = TRUE;
      continue;
    }
    
    else if (!strcmp(argv[i], "-ver")) {
      printf(verbstr);
      return 0;
    }
    
    else if (!strcmp(argv[i], "-help")) {
      printf(helpstr);
      return 0;
    }
    
    else if (!strcmp(argv[i], "-Wall")) {
      warning_level = 4;
      continue;
    }

    else if (!strcmp(argv[i], "-efi")) {
      use_efi = TRUE;
      OutputFormat = FORMATSEGMENTED;
      continue;
    }
    
    else if (!strcmp(argv[i], "-fasm")) {
      use_offset = FALSE;
      use_nbasm_out = FALSE;
      use_fasm_out = TRUE;
      OutputFormat = FORMATFLAT;
      continue;
    }
    
    else if (!strcmp(argv[i], "-nbasm")) {
      use_offset = TRUE;
      use_nbasm_out = TRUE;
      use_fasm_out = FALSE;
      OutputFormat = FORMATFLAT;
      continue;
    }
    
    else if ((argv[i][0] == '-') && (argv[i][1] == 'W') && (argv[i][2] >= '0' && argv[i][2] <= '4')) {
      warning_level = (argv[i][2] - '0');
      continue;
    }
    
    else if (!strcmp(argv[i], "-I") || !strcmp(argv[i], "-SI")) {
      if (i + 1 < argc) {
        len = strlen(argv[++i]) + 1;
        if (argv[i][1] == 'I') {
          if ((MAX_SEARCH_PATH - SearchPathsLen) < len) {
            printf("\n Invalid or too long file name or path name.");
            return -1;
          }
          strcpy(SearchPaths + SearchPathsLen, argv[i]);
          SearchPathsLen += len;
        } else {
          if (MAX_SEARCH_PATH - SysSearchPathsLen < len) {
            printf("\n Invalid or too long file name or path name.");
            return -1;
          }
          strcpy(SysSearchPaths + SysSearchPathsLen, argv[i]);
          SysSearchPathsLen += len;
        }
        continue;
      }
    }
    
    // '-D macro[=expansion]': '#define macro 1' when there's no '=expansion'
    else if (!strcmp(argv[i], "-D")) {
      if (i + 1 < argc) {
        char id[MAX_IDENT_LEN + 1];
        char *e = strchr(argv[++i], '=');
        if (e) {
          len = e - argv[i];
          e++;
        } else {
          len = strlen(argv[i]);
          e = "1";
        }
        if (len > 0 && len <= MAX_IDENT_LEN) {
          int j, bad = 1;
          memcpy(id, argv[i], len);
          id[len] = '\0';
          for (j = 0; j < len; j++)
            if ((bad = !(id[j] == '_' || (!j * isalpha(id[j] & 0xFFu) + j * isalnum(id[j] & 0xFFu)))) != 0)
              break;
          if (!bad) {
            DefineMacro(id, e);
            continue;
          }
        }
      }
    }
    
    else if (argv[i][0] == '-') {
      // unknown option
    }
    
    else if (cur_file < 0) {
      // If it's none of the known options,
      // assume it's the source code file name
      if (strlen(argv[i]) > MAX_FILE_NAME_LEN) {
        printf("\n Invalid or too long file name or path name.");
        return 1;
      }
      strcpy(files[0].filename, argv[i]);
      // if the filename does not end with ".c" add it
      if (!strchr(files[0].filename, '.'))
        strcat(files[0].filename, ".c");      
      if ((files[0].fp = fopen(files[0].filename, "r")) == NULL) {
        printf("\n Unable to open, read, write or close file \"%s\"", files[0].filename);
        return 1;
      }
      files[0].LineNo = 1;
      files[0].LinePos = 1;
      cur_file = 0;
      continue;
    }
    
    else if ((cur_file == 0) && (targ_fp == NULL)) {
      // This should be the output file name
      if ((targ_fp = fopen(argv[i], "w")) == NULL) {
        printf("\n Unable to open, read, write or close file \"%s\"", argv[i]);
        return 1;
      }
      continue;
    }
    
    printf("\n Invalid or unsupported command line option");
    return 1;
  }
  
  // append the environment variable "nbcinc" to the end of SysSearchPaths
  char *env = getenv("NBCINC");
  if (env != NULL) {
    len = strlen(env) + 1;
    if ((MAX_SEARCH_PATH - SysSearchPathsLen) < len) {
      printf("\n Invalid or too long environment name or path name.");
      return 1;
    }
    strcpy(SysSearchPaths + SysSearchPathsLen, env);
    SysSearchPathsLen += len;
  }
  
  // get the NBCLIB path for our .asm includes
  env = getenv("NBCLIB");
  if (env != NULL) {
    strcpy(NbcLibPath, env);
    // and include it in the system search paths
    len = strlen(env) + 1;
    if ((MAX_SEARCH_PATH - SysSearchPathsLen) < len) {
      printf("\n Invalid or too long environment name or path name.");
      return 1;
    }
    strcpy(SysSearchPaths + SysSearchPathsLen, env);
    SysSearchPathsLen += len;
  }
  
  if (cur_file < 0) {
    printf("\n Did not specify source file...");
    return 1;
  }
  
  // if the target file name was not given, create it
  if (targ_fp == NULL) {
    strcpy(targ_filename, files[0].filename);
    p = strchr(targ_filename, '.');
    if (p) strcpy(p, ".asm");
    if ((targ_fp = fopen(targ_filename, "w")) == NULL) {
      printf("\n Unable to open, read, write or close file \"%s\"", targ_filename);
      return 1;
    }
  }
  
  // Allocate the string and macro tables
  StringTable = StringTablePtr = (struct S_STRING_TABLE *) calloc(MAX_STRING_TABLE_LEN, sizeof(struct S_STRING_TABLE));
  MacroTable = MacroTablePtr = (struct S_MACRO_TABLE *) calloc(MAX_MACRO_TABLE_LEN, sizeof(struct S_MACRO_TABLE));
  SyntaxStack = (struct S_SYNTAX_STACK *) calloc(SYNTAX_STACK_MAX, sizeof(struct S_SYNTAX_STACK));
  IdentTablePtr = (struct S_IDENT_PREFIX *) calloc(MAX_IDENT_TABLE_LEN, 1);
  if (!StringTablePtr || !MacroTablePtr || !SyntaxStack || !IdentTablePtr) {
    printf("\n Error allocating String/Macro/Ident Table or SyntaxStack...");
    if (targ_fp)
      fclose(targ_fp);
    if (StringTablePtr) free(StringTablePtr);
    if (MacroTablePtr) free(MacroTablePtr);
    if (SyntaxStack) free(SyntaxStack);
    if (IdentTablePtr) free(IdentTablePtr);
    return 1;
  }
  
  // initialize the SyntaxStack array
  SyntaxStack[0].tok = tokVoid;      // SymVoidSynPtr
  SyntaxStack[1].tok = tokInt;       // SymIntSynPtr
  SyntaxStack[2].tok = tokUnsigned;  // SymUintSynPtr
  SyntaxStack[3].tok = tokIdent;     // SymFuncPtr
  SyntaxStack[4].tok = '[';
  SyntaxStack[5].tok = tokNumUint;
  SyntaxStack[6].tok = ']';
  SyntaxStack[7].tok = tokChar;
  SyntaxStackCnt = 8; // number of explicitly initialized elements in SyntaxStack[]
  
  if (!is_library_code)
    GenInitFinalize();
  
  // define some internal define's
  DefineMacro("_NBC_", "");
  DefineMacro("_VER_", VERSION_INT);
  
  // populate CharQueue[] with the initial file characters
  ShiftChar();
  
  puts_out(FileHeader);
  
  // compile
  PragmaPackValue = SizeOfWord;
  ParseBlock(NULL, 0);
  
  PurgeStringTable();
  GenFin();
  
  // if eof found before #endif(s), give warning
  if (PrepSp > 0)
    warning(0x0300, 0);
  
  if (dump_tables) {
    DumpSynDecls();
    DumpMacroTable();
    DumpIdentTable();
    printf_out("; Next label number: %d\n", LabelCnt);
  }
  
  printf("\n     %i Errors Found.\n"
           "     %i Warnings Found.\n", error_cnt, warn_cnt);
  
  if (targ_fp)
    fclose(targ_fp);
  
  // free some memory used
  if (StringTablePtr) free(StringTablePtr);
  if (MacroTablePtr) free(MacroTablePtr);
  if (SyntaxStack) free(SyntaxStack);
  if (IdentTablePtr) free(IdentTablePtr);
  
  return 0;
}

/*

notes:
/////////////////////////////////////
// here is an example of the # and ## pre-processor functions
#define COMMAND(NAME) { #NAME, NAME ## _command }
 
struct command commands[] =
{
COMMAND (quit),
COMMAND (help),
};

// will create
struct command
{
char *name;
void (*function) (void);
};
 
struct command commands[] =
{
{ "quit", quit_command },
{ "help", help_command },
};

//// http://hackaday.com/2015/10/16/code-craft-when-define-is-considered-harmful/



*/