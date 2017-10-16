
#define MAX_GLOBALS_TABLE_LEN MAX_IDENT_TABLE_LEN

/*
  Globals table entry format:
    use char:       use: bit 0 = defined, bit 1 = used
    idlen char:     string length (<= 127)
    id char[idlen]: string (ASCIIZ)
*/
char GlobalsTable[MAX_GLOBALS_TABLE_LEN];
int GlobalsTableLen = 0;

void GenAddGlobal(const char *s, int use) {
  int i = 0;
  int l;
  if (OutputFormat != FORMATFLAT && GenExterns) {
    while (i < GlobalsTableLen) {
      if (!strcmp(GlobalsTable + i + 2, s)) {
        GlobalsTable[i] |= use;
        return;
      }
      i += GlobalsTable[i + 1] + 2;
    }
    l = strlen(s) + 1;
    if (GlobalsTableLen + l + 2 > MAX_GLOBALS_TABLE_LEN)
      ERROR(0x0001, FALSE);
    GlobalsTable[GlobalsTableLen++] = use;
    GlobalsTable[GlobalsTableLen++] = l;
    memcpy(GlobalsTable + GlobalsTableLen, s, l);
    GlobalsTableLen += l;
  }
}

void SwitchSection(int section) {
  if (WhatSection == section)
    return;
  
  if (WhatSection == SECTION_IS_DATA)
    puts_out(DataFooter);
  else if (WhatSection == SECTION_IS_CODE)
    puts_out(CodeFooter);
  
  if (section == SECTION_IS_CODE)
    puts_out(CodeHeader);
  else if (section == SECTION_IS_DATA)
    puts_out(DataHeader);
  
  WhatSection = section;
}

void GenInit(void) {
  // initialization of target-specific code generator
  SizeOfWord = 4;
  OutputFormat = FORMATFLAT;
  // Parsing of the Parameters in main() will allow this to change
  UseLeadingUnderscores = TRUE;
}

void GenInitFinalize(void) {
  // finalization of initialization of target-specific code generator

  // Change the output assembly format/content according to the options
  if (use_nbasm_out) {
    if (OutputFormat == FORMATSEGMENTED) {
      FileHeader = ".model small\n";
      CodeHeader = ".code\n";
      CodeFooter = "; .code\n";
      DataHeader = ".data\n";
      DataFooter = "; .data\n";
    } else if (OutputFormat == FORMATFLAT) {
      FileHeader = ".model tiny\n"
                   ".code\n";
    }
    FileFooter = ".end\n";
  } else if (use_fasm_out) {
    if (use_efi) {
      if (UseLeadingUnderscores)
        FileHeader = "format pe dll efi\n"
                     "entry _efi_main\n";
      else
        FileHeader = "format pe dll efi\n"
                     "entry efi_main\n";
    } else {
      if (UseLeadingUnderscores)
        FileHeader = "format binary\n"
                     "entry _main\n";
      else
        FileHeader = "format binary\n"
                     "entry main\n";
    }
    if (OutputFormat == FORMATSEGMENTED) {
      CodeHeader = "\nsection '.text' code executable readable\n";
      DataHeader = "\nsection '.data' data readable writeable\n";
      FileFooter = "\nsection '.reloc' fixups data discardable\n";
    }
  }
}

void GenWordAlignment(const int alignment) {
  if (use_nbasm_out)
    printf_out(".align %d\n", alignment);
  else if (use_fasm_out)
    printf_out("align %d\n", alignment);
}

void GenLabel(const char *Label, const bool NoUnderScore, const bool Static) {
  if (UseLeadingUnderscores && !NoUnderScore) {
//    if ((OutputFormat != FORMATFLAT) && !Static && GenExterns)
//      printf_out("  global  _%s\n", Label);
    printf_out("_%s:\n", Label);
  } else {
//    if ((OutputFormat != FORMATFLAT) && !Static && GenExterns)
//      printf_out("  global  %s\n", Label);
    printf_out("%s:\n", Label);
  }
  GenAddGlobal(Label, 1);
}

void GenLabelIP(const bit32u ptr, const bool Static) {
  GenLabel(IDENT_STR(ptr), IDENT_NO_US(ptr), Static);
}

void GenPrintLabel(const char *label, const bool NoUnderScore) {
  if (UseLeadingUnderscores && !NoUnderScore) {
    if (isdigit(*label))
      printf_out("L%s", label);
    else
      printf_out("_%s", label);
  } else {
    if (isdigit(*label))
      printf_out("..@L%s", label);
    else
      printf_out("%s", label);
  }
}

void GenNumLabel(int Label) {
  if (UseLeadingUnderscores)
    printf_out("L%d:\n", Label);
  else
    printf_out("..@L%d:\n", Label);
}

void GenPrintNumLabel(int label) {
  if (UseLeadingUnderscores)
    printf_out("L%d", label);
  else
    printf_out("..@L%d", label);
}

void GenZeroData(unsigned Size) {
  if (use_nbasm_out)
    printf_out("  dup  %u,0\n", truncUint(Size));
  else if (use_fasm_out)
    printf_out("  db  %u dup(0)\n", truncUint(Size));
}

void GenIntData(int size, int Val) {
  Val = truncInt(Val);
  if (size == 1)
    printf_out("  db  %i", Val);
  else if (size == 2)
    printf_out("  dw  %i", Val);
  else if (size == 4)
    printf_out("  dd  %i", Val);
}

void GenStartAsciiString(bool wide) {
  if (wide) {
#if (WIDTH_OF_WIDECHAR == 1)
    printf_out("  db  ");
#elif (WIDTH_OF_WIDECHAR == 2)
    if (use_nbasm_out)
      printf_out("  dw  ");
    else if (use_fasm_out)
      printf_out("  du  ");
#elif (WIDTH_OF_WIDECHAR == 4)
    printf_out("  dd  ");
#else
#error "Width of wchar_t must be 1, 2, or 4"
#endif
  } else
    printf_out("  db  ");
}

void GenAddrData(int Size, const bit32u ptr, int ofs) {
  ofs = truncInt(ofs);
  if (Size == 1)
    printf_out("  db  ");
  else if (Size == 2)
    printf_out("  dw  ");
  else if (Size == 4)
    printf_out("  dd  ");
  GenPrintLabel(IDENT_STR(ptr), IDENT_NO_US(ptr));
  if (ofs)
    printf_out(" %+d", ofs);
  puts_out("");
  if (!isdigit(*IDENT_STR(ptr)))
    GenAddGlobal(IDENT_STR(ptr), 2);
}

#define X86InstrMov    0x00
#define X86InstrMovSx  0x01
#define X86InstrMovZx  0x02
#define X86InstrXchg   0x03
#define X86InstrLea    0x04
#define X86InstrPush   0x05
#define X86InstrPop    0x06
#define X86InstrInc    0x07
#define X86InstrDec    0x08
#define X86InstrAdd    0x09
#define X86InstrSub    0x0A
#define X86InstrAnd    0x0B
#define X86InstrXor    0x0C
#define X86InstrOr     0x0D
#define X86InstrCmp    0x0E
#define X86InstrTest   0x0F
#define X86InstrMul    0x10
#define X86InstrImul   0x11
#define X86InstrIdiv   0x12
#define X86InstrDiv    0x13
#define X86InstrShl    0x14
#define X86InstrSar    0x15
#define X86InstrShr    0x16
#define X86InstrNeg    0x17
#define X86InstrNot    0x18
#define X86InstrCbw    0x19
#define X86InstrCwd    0x1A
#define X86InstrCdq    0x1B
#define X86InstrSetCc  0x1C
#define X86InstrJcc    0x1D
#define X86InstrJNotCc 0x1E
#define X86InstrLeave  0x1F
#define X86InstrCall   0x20
#define X86InstrRet    0x21
#define X86InstrJmp    0x22

char *winstrs[] = {
  "mov  ",
  "movsx",
  "movzx",
  "xchg ",
  "lea  ",
  "push ",
  "pop  ",
  "inc  ",
  "dec  ",
  "add  ",
  "sub  ",
  "and  ",
  "xor  ",
  "or   ",
  "cmp  ",
  "test ",
  "mul  ",
  "imul ",
  "idiv ",
  "div  ",
  "shl  ",
  "sar  ",
  "shr  ",
  "neg  ",
  "not  ",
  "cbw  ",
  "cwd  ",
  "cdq  ",
  0, // setcc
  0, // jcc
  0, // j!cc
  0, // leave
  "call ",
  0, // ret
  "jmp  ",
};

void GenPrintInstr(int instr, int val) {
  char* p = "";

  switch (instr) {
    case X86InstrLeave: 
      p = "leave";
      break;
      
    case X86InstrRet: 
      p = "ret  ";
      break;

    case X86InstrJcc:
      switch (val) {
        case '<':         p = "jl   "; break;
        case tokULess:    p = "jb   "; break;
        case '>':         p = "jg   "; break;
        case tokUGreater: p = "ja   "; break;
        case tokLEQ:      p = "jle  "; break;
        case tokULEQ:     p = "jbe  "; break;
        case tokGEQ:      p = "jge  "; break;
        case tokUGEQ:     p = "jae  "; break;
        case tokEQ:       p = "je   "; break;
        case tokNEQ:      p = "jne  "; break;
      }
      break;
    case X86InstrJNotCc:
      switch (val) {
        case '<':         p = "jge  "; break;
        case tokULess:    p = "jae  "; break;
        case '>':         p = "jle  "; break;
        case tokUGreater: p = "jbe  "; break;
        case tokLEQ:      p = "jg   "; break;
        case tokULEQ:     p = "ja   "; break;
        case tokGEQ:      p = "jl   "; break;
        case tokUGEQ:     p = "jb   "; break;
        case tokEQ:       p = "jne  "; break;
        case tokNEQ:      p = "je   "; break;
      }
      break;

    case X86InstrSetCc:
      switch (val) {
        case '<':         p = "setl "; break;
        case tokULess:    p = "setb "; break;
        case '>':         p = "setg "; break;
        case tokUGreater: p = "seta "; break;
        case tokLEQ:      p = "setle"; break;
        case tokULEQ:     p = "setbe"; break;
        case tokGEQ:      p = "setge"; break;
        case tokUGEQ:     p = "setae"; break;
        case tokEQ:       p = "sete "; break;
        case tokNEQ:      p = "setne"; break;
      }
      break;
      
    default:
      p = winstrs[instr];
      break;
  }
  
  switch (instr) {
    case X86InstrCbw:
    case X86InstrCwd:
    case X86InstrCdq:
    case X86InstrLeave:
    case X86InstrRet:
      printf_out("  %s", p);
      break;
    default:
      printf_out("  %s  ", p);
      break;
  }
}

#define X86OpRegAByte                   0x00
#define X86OpRegAByteHigh               0x01
#define X86OpRegCByte                   0x02
#define X86OpRegAWord                   0x03
#define X86OpRegBWord                   0x04
#define X86OpRegCWord                   0x05
#define X86OpRegDWord                   0x06
#define X86OpRegAHalfWord               0x07
#define X86OpRegCHalfWord               0x08
#define X86OpRegBpWord                  0x09
#define X86OpRegSpWord                  0x0A
#define X86OpRegAByteOrWord             0x0B
#define X86OpRegCByteOrWord             0x0C
#define X86OpConst                      0x0D
#define X86OpLabel                      0x0E
#define X86OpLabelOff                   0x0F
#define X86OpNumLabel                   0x10
#define X86OpIndLabel                   0x11
#define X86OpIndLabelExplicitByte       0x12
#define X86OpIndLabelExplicitWord       0x13
#define X86OpIndLabelExplicitHalfWord   0x14
#define X86OpIndLabelExplicitByteOrWord 0x15
#define X86OpIndLocal                   0x16
#define X86OpIndLocalExplicitByte       0x17
#define X86OpIndLocalExplicitWord       0x18
#define X86OpIndLocalExplicitHalfWord   0x19
#define X86OpIndLocalExplicitByteOrWord 0x1A
#define X86OpIndRegB                    0x1B
#define X86OpIndRegBExplicitByte        0x1C
#define X86OpIndRegBExplicitWord        0x1D
#define X86OpIndRegBExplicitHalfWord    0x1E
#define X86OpIndRegBExplicitByteOrWord  0x1F

int GenSelectByteOrWord(int op, int opSz) {
  switch (op) {
    case X86OpRegAByteOrWord:
      op = X86OpRegAByte;
      if (opSz == SizeOfWord)
        op = X86OpRegAWord;
      else if (opSz == 2 || opSz == -2)
        op = X86OpRegAHalfWord;
      break;
    case X86OpRegCByteOrWord:
      op = X86OpRegCByte;
      if (opSz == SizeOfWord)
        op = X86OpRegCWord;
      else if (opSz == 2 || opSz == -2)
        op = X86OpRegCHalfWord;
      break;
    case X86OpIndLabelExplicitByteOrWord:
      op = X86OpIndLabelExplicitByte;
      if (opSz == SizeOfWord)
        op = X86OpIndLabelExplicitWord;
      else if (opSz == 2 || opSz == -2)
        op = X86OpIndLabelExplicitHalfWord;
      break;
    case X86OpIndLocalExplicitByteOrWord:
      op = X86OpIndLocalExplicitByte;
      if (opSz == SizeOfWord)
        op = X86OpIndLocalExplicitWord;
      else if (opSz == 2 || opSz == -2)
        op = X86OpIndLocalExplicitHalfWord;
      break;
    case X86OpIndRegBExplicitByteOrWord:
      op = X86OpIndRegBExplicitByte;
      if (opSz == SizeOfWord)
        op = X86OpIndRegBExplicitWord;
      else if (opSz == 2 || opSz == -2)
        op = X86OpIndRegBExplicitHalfWord;
      break;
  }
  return op;
}

const char FarTagStr[5][4] = { "cs:", "ds:", "es:", "fs:", "gs:" };
const char *GenFarTag(const int tok) {
  switch (tok) {
    case tokFarC: return FarTagStr[0];
    case tokFarD: return FarTagStr[1];
    case tokFarE: return FarTagStr[2];
    case tokFarF: return FarTagStr[3];
    case tokFarG: return FarTagStr[4];
    default: return "";
  }  
}

void GenOffset() {
  if (use_offset)
    printf_out("offset ");
}

void GenPrintOperand(int op, int val, int flags) {
  if (SizeOfWord == 2) {
    switch (op) {
      case X86OpRegAByte: printf_out("al"); break;
      case X86OpRegAByteHigh: printf_out("ah"); break;
      case X86OpRegCByte: printf_out("cl"); break;
      case X86OpRegAWord: printf_out("ax"); break;
      case X86OpRegBWord: printf_out("bx"); break;
      case X86OpRegCWord: printf_out("cx"); break;
      case X86OpRegDWord: printf_out("dx"); break;
      case X86OpRegBpWord: printf_out("bp"); break;
      case X86OpRegSpWord: printf_out("sp"); break;
      case X86OpConst: printf_out("%d", truncInt(val)); break;
      case X86OpLabel: GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); break;
      case X86OpLabelOff: GenOffset(); GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); break;
      case X86OpNumLabel: GenPrintNumLabel(val); break;
      case X86OpIndLabel: 
        printf_out("%s[", GenFarTag(flags));
        GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); 
        printf_out("]"); 
        break;
      case X86OpIndLabelExplicitByte: 
        printf_out("byte %s[", GenFarTag(flags)); 
        GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); 
        printf_out("]"); 
        break;
      case X86OpIndLabelExplicitWord: 
        printf_out("word %s[", GenFarTag(flags));
        GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); 
        printf_out("]"); 
        break;
      case X86OpIndLocal: printf_out("[bp%+d]", truncInt(val)); break;
      case X86OpIndLocalExplicitByte: printf_out("byte [bp%+d]", truncInt(val)); break;
      case X86OpIndLocalExplicitWord: printf_out("word [bp%+d]", truncInt(val)); break;
      case X86OpIndRegB: 
        printf_out("%s[bx]", GenFarTag(flags));
        break;
      case X86OpIndRegBExplicitByte: 
        printf_out("byte %s[bx]", GenFarTag(flags)); 
        break;
      case X86OpIndRegBExplicitWord: 
        printf_out("word %s[bx]", GenFarTag(flags));
        break;
    }
  } else {
    char *frame = "ebp";
    char *base = "ebx";
    switch (op) {
      case X86OpRegAByte: printf_out("al"); break;
      case X86OpRegAByteHigh: printf_out("ah"); break;
      case X86OpRegCByte: printf_out("cl"); break;
      case X86OpRegAWord: printf_out("eax"); break;
      case X86OpRegBWord: printf_out("ebx"); break;
      case X86OpRegCWord: printf_out("ecx"); break;
      case X86OpRegDWord: printf_out("edx"); break;
      case X86OpRegAHalfWord: printf_out("ax"); break;
      case X86OpRegCHalfWord: printf_out("cx"); break;
      case X86OpRegBpWord: printf_out("ebp"); break;
      case X86OpRegSpWord: printf_out("esp"); break;
      case X86OpConst: printf_out("%d", truncInt(val)); break;
      case X86OpLabel: GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); break;
      case X86OpLabelOff: GenOffset(); GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); break;
      case X86OpNumLabel: GenPrintNumLabel(val); break;
      case X86OpIndLabel: 
        printf_out("%s[", GenFarTag(flags)); 
        GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); 
        printf_out("]"); 
        break;
      case X86OpIndLabelExplicitByte: 
        printf_out("byte %s[", GenFarTag(flags)); 
        GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); 
        printf_out("]"); 
        break;
      case X86OpIndLabelExplicitWord: 
        printf_out("dword %s[", GenFarTag(flags)); 
        GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val)); 
        printf_out("]"); 
        break;
      case X86OpIndLabelExplicitHalfWord: 
        printf_out("word %s[", GenFarTag(flags)); 
        GenPrintLabel(IDENT_STR(val), IDENT_NO_US(val));
        printf_out("]");
        break;
      case X86OpIndLocal: printf_out("[%s%+d]", frame, truncInt(val)); break;
      case X86OpIndLocalExplicitByte: printf_out("byte [%s%+d]", frame, truncInt(val)); break;
      case X86OpIndLocalExplicitWord: printf_out("dword [%s%+d]", frame, truncInt(val)); break;
      case X86OpIndLocalExplicitHalfWord: printf_out("word [%s%+d]", frame, truncInt(val)); break;
      case X86OpIndRegB: 
        printf_out("%s[%s]", GenFarTag(flags), base);
        break;
      case X86OpIndRegBExplicitByte: 
        printf_out("byte %s[%s]", GenFarTag(flags), base);
        break;
      case X86OpIndRegBExplicitWord: 
        printf_out("dword %s[%s]", GenFarTag(flags), base);
        break;
      case X86OpIndRegBExplicitHalfWord: 
        printf_out("word %s[%s]", GenFarTag(flags), base);
        break;
    }
  }
}

void GenPrintOperandSeparator(void) {
  printf_out(",");
}

void GenPrintNewLine(void) {
  puts_out("");
}

void GenPrintInstrNoOperand(int instr) {
  GenPrintInstr(instr, 0);
  GenPrintNewLine();
}

void GenPrintInstr1Operand(int instr, int instrval, int operand, int operandval, int flags) {
  if (instr == X86InstrPush) {
    if (operand == X86OpConst) {
      if (SizeOfWord == 4)
        printf_out("  push dword %i\n", truncInt(operandval));
      else
        printf_out("  push %i\n", truncInt(operandval));
      return;
    } else if (operand == X86OpLabelOff) {
      if (SizeOfWord == 4)
        printf_out("  push dword ");
      else
        printf_out("  push ");
      GenOffset();
      GenPrintLabel(IDENT_STR(operandval), IDENT_NO_US(operandval));
      puts_out("");
      return;
    }
  }
  
  GenPrintInstr(instr, instrval);
  GenPrintOperand(operand, operandval, flags);
  GenPrintNewLine();
}

void GenPrintInstr2Operands(int instr, int instrval, int operand1, int operand1val, int operand2, int operand2val, int flags) {
  if (operand2 == X86OpConst && truncUint(operand2val) == 0 &&
      (instr == X86InstrAdd || instr == X86InstrSub))
    return;
  
  if (operand2 == X86OpConst &&
      (operand2val == 1 || operand2val == -1) &&
      (instr == X86InstrAdd || instr == X86InstrSub)) {
    if ((operand2val == 1 && instr == X86InstrAdd) ||
        (operand2val == -1 && instr == X86InstrSub))
      GenPrintInstr(X86InstrInc, 0);
    else
      GenPrintInstr(X86InstrDec, 0);
    GenPrintOperand(operand1, operand1val, flags);
    GenPrintNewLine();
    return;
  }
  
  GenPrintInstr(instr, instrval);
  GenPrintOperand(operand1, operand1val, flags);
  GenPrintOperandSeparator();
  GenPrintOperand(operand2, operand2val, flags);
  GenPrintNewLine();
}

void GenPrintInstr3Operands(int instr, int instrval,
                            int operand1, int operand1val,
                            int operand2, int operand2val,
                            int operand3, int operand3val,
                            int flags) {
  GenPrintInstr(instr, instrval);
  GenPrintOperand(operand1, operand1val, flags);
  GenPrintOperandSeparator();
  GenPrintOperand(operand2, operand2val, flags);
  GenPrintOperandSeparator();
  GenPrintOperand(operand3, operand3val, flags);
  GenPrintNewLine();
}

void GenExtendRegAIfNeeded(int opSz) {
  if (SizeOfWord == 2) {
    if (opSz == -1)
      GenPrintInstrNoOperand(X86InstrCbw);
    else if (opSz == 1)
      GenPrintInstr2Operands(X86InstrMov, 0,
                             X86OpRegAByteHigh, 0,
                             X86OpConst, 0, 0);
  } else {
    if (opSz == -1)
      GenPrintInstr2Operands(X86InstrMovSx, 0,
                             X86OpRegAWord, 0,
                             X86OpRegAByte, 0, 0);
    else if (opSz == 1)
    GenPrintInstr2Operands(X86InstrMovZx, 0,
                             X86OpRegAWord, 0,
                             X86OpRegAByte, 0, 0);
    else if (opSz == -2)
      GenPrintInstr2Operands(X86InstrMovSx, 0,
                             X86OpRegAWord, 0,
                             X86OpRegAHalfWord, 0, 0);
    else if (opSz == 2)
      GenPrintInstr2Operands(X86InstrMovZx, 0,
                             X86OpRegAWord, 0,
                             X86OpRegAHalfWord, 0, 0);
  }
}

void GenJumpUncond(int label) {
  GenPrintInstr1Operand(X86InstrJmp, 0,
                        X86OpNumLabel, label, 0);
}

void GenJumpIfNotEqual(int val, int label) {
  GenPrintInstr2Operands(X86InstrCmp, 0,
                         X86OpRegAWord, 0,
                         X86OpConst, val, 0);
  GenPrintInstr1Operand(X86InstrJcc, tokNEQ,
                        X86OpNumLabel, label, 0);
}

void GenJumpIfEqual(int val, int label) {
  GenPrintInstr2Operands(X86InstrCmp, 0,
                         X86OpRegAWord, 0,
                         X86OpConst, val, 0);
  GenPrintInstr1Operand(X86InstrJcc, tokEQ,
                        X86OpNumLabel, label, 0);
}

void GenJumpIfZero(int label) {
  GenPrintInstr2Operands(X86InstrTest, 0,
                         X86OpRegAWord, 0,
                         X86OpRegAWord, 0, 0);
  GenPrintInstr1Operand(X86InstrJcc, tokEQ,
                        X86OpNumLabel, label, 0);
}

void GenJumpIfNotZero(int label) {
  GenPrintInstr2Operands(X86InstrTest, 0,
                         X86OpRegAWord, 0,
                         X86OpRegAWord, 0, 0);
  GenPrintInstr1Operand(X86InstrJcc, tokNEQ,
                        X86OpNumLabel, label, 0);
}

fpos_t GenPrologPos;

void GenWriteFrameSize(void) {
  unsigned size = -CurFxnMinLocalOfs;
  int pfx = size ? ' ' : ';';
  
  if (SizeOfWord == 2)
    printf_out(" %csub    sp,%-10u\n", pfx, size); // 10 chars are enough for 32-bit unsigned ints
  else
    printf_out(" %csub    esp,%-10u\n", pfx, size); // 10 chars are enough for 32-bit unsigned ints
}

void GenFxnProlog(void) {
  GenPrintInstr1Operand(X86InstrPush, 0,
                        X86OpRegBpWord, 0, 0);
  GenPrintInstr2Operands(X86InstrMov, 0,
                         X86OpRegBpWord, 0,
                         X86OpRegSpWord, 0, 0);
  
  // do frame size stuff
  fgetpos(targ_fp, &GenPrologPos);
  GenWriteFrameSize();
}

void GenLocalAlloc(int size) {
  GenPrintInstr2Operands(X86InstrSub, 0,
                         X86OpRegSpWord, 0,
                         X86OpConst, size, 0);
}

void GenFxnEpilog(void) {
  fpos_t pos;
  
  fgetpos(targ_fp, &pos);
  fsetpos(targ_fp, &GenPrologPos);
  GenWriteFrameSize();
  fsetpos(targ_fp, &pos);
  
  // if we would like to use LEAVE instead
  // GenPrintInstrNoOperand(X86InstrLeave);
  // or do the 'leave' ourselves
  GenPrintInstr2Operands(X86InstrMov, 0,
                         X86OpRegSpWord, 0,
                         X86OpRegBpWord, 0, 0);
  GenPrintInstr1Operand(X86InstrPop, 0,
                        X86OpRegBpWord, 0, 0); //
  
  // ret
  GenPrintInstrNoOperand(X86InstrRet);
}

/*
struct INTREGS
{
  unsigned short gs, fs, es, ds;
  unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
  unsigned short ss, ip, cs, flags;
};
void __interrupt isr(struct INTREGS** ppRegs)
{
  // **ppRegs (input/output values of registers) can be modified to
  // handle software interrupts requested via the int instruction and
  // communicate data via registers

  // *ppRegs (directly related to the stack pointer) can be modified to
  // return to a different location & implement preemptive scheduling,
  // e.g. save *ppRegs of the interrupted task somewhere, update *ppRegs
  // with a value from another interrupted task.
}
*/
void GenIsrProlog(void) {
  // The CPU has already done these:
  //   push flags
  //   push cs
  //   push ip

  puts_out("  push  ss");
  puts_out("  pushad");
  puts_out("  push  ds\n"
        "  push  es\n"
        "  push  fs\n"
        "  push  gs");

  // The context has been saved

  puts_out("  xor  eax,eax\n  mov  ax,ss"); // mov r32, sreg leaves top 16 bits undefined on pre-Pentium CPUs
  puts_out("  xor  ebx,ebx\n  mov  bx,sp"); // top 16 bits of esp can contain garbage as well
  puts_out("  shl  eax,4\n  add  eax,ebx");
  puts_out("  push eax"); // pointer to the structure with register values
  puts_out("  sub  eax,4\n  push  eax"); // pointer to the pointer to the structure with register values

  puts_out("  push eax"); // fake return address allowing to use the existing bp-relative addressing of locals and params

  puts_out("  push ebp\n"
        "  mov  ebp,esp");
}

void GenIsrEpilog(void) {
  puts_out("  db  0x66\n  leave");

  puts_out("  pop  eax"); // fake return address

  puts_out("  pop  eax"); // pointer to the pointer to the structure with register values
  puts_out("  pop  ebx"); // pointer to the structure with register values
  puts_out("  ror  ebx, 4\n  mov  ds, bx\n  shr  ebx, 28"); // ds:bx = pointer to the structure with register values
  puts_out("  mov  ax, [bx+4*10]\n  mov  bx, [bx+4*5]\n  sub  bx, 4*10"); // ax:bx = proper pointer (with original segment) to the struct...
  puts_out("  mov  ss, ax\n  mov  sp, bx"); // restore ss:sp that we had after push gs

  // The context is now going to be restored

  puts_out("  pop  gs\n"
        "  pop  fs\n"
        "  pop  es\n"
        "  pop  ds");
  puts_out("  popad");
  puts_out("  pop  ss");

  puts_out("  iret");
}

void GenReadIdent(int opSz, int label) {
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLabel, label, 0);
  GenExtendRegAIfNeeded(opSz);
}

void GenReadLocal(int opSz, int ofs) {
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLocal, ofs, 0);
  GenExtendRegAIfNeeded(opSz);
}

void GenReadIndirect(int opSz, int flags) {
  GenPrintInstr2Operands(X86InstrMov, 0,
                         X86OpRegBWord, 0,
                         X86OpRegAWord, 0, 0);
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndRegB, 0, flags);
  GenExtendRegAIfNeeded(opSz);
}

void GenReadCRegIdent(int opSz, int label) {
  if (opSz == -1)
    GenPrintInstr2Operands(X86InstrMovSx, 0,
                           X86OpRegCWord, 0,
                           X86OpIndLabelExplicitByte, label, 0);
  else if (opSz == 1)
    GenPrintInstr2Operands(X86InstrMovZx, 0,
                           X86OpRegCWord, 0,
                           X86OpIndLabelExplicitByte, label, 0);
  else if (opSz != SizeOfWord && -opSz != SizeOfWord)
  {
    if (opSz == -2)
      GenPrintInstr2Operands(X86InstrMovSx, 0,
                             X86OpRegCWord, 0,
                             X86OpIndLabelExplicitHalfWord, label, 0);
    else if (opSz == 2)
      GenPrintInstr2Operands(X86InstrMovZx, 0,
                             X86OpRegCWord, 0,
                             X86OpIndLabelExplicitHalfWord, label, 0);
  }
  else
    GenPrintInstr2Operands(X86InstrMov, 0,
                           X86OpRegCWord, 0,
                           X86OpIndLabel, label, 0);
}

void GenReadCRegLocal(int opSz, int ofs) {
  if (opSz == -1)
    GenPrintInstr2Operands(X86InstrMovSx, 0,
                           X86OpRegCWord, 0,
                           X86OpIndLocalExplicitByte, ofs, 0);
  else if (opSz == 1)
    GenPrintInstr2Operands(X86InstrMovZx, 0,
                           X86OpRegCWord, 0,
                           X86OpIndLocalExplicitByte, ofs, 0);
  else if (opSz != SizeOfWord && -opSz != SizeOfWord)
  {
    if (opSz == -2)
      GenPrintInstr2Operands(X86InstrMovSx, 0,
                             X86OpRegCWord, 0,
                             X86OpIndLocalExplicitHalfWord, ofs, 0);
    else if (opSz == 2)
      GenPrintInstr2Operands(X86InstrMovZx, 0,
                             X86OpRegCWord, 0,
                             X86OpIndLocalExplicitHalfWord, ofs, 0);
  }
  else
    GenPrintInstr2Operands(X86InstrMov, 0,
                           X86OpRegCWord, 0,
                           X86OpIndLocal, ofs, 0);
}

void GenReadCRegIndirect(int opSz, const bit32u flags) {
  
  GenPrintInstr2Operands(X86InstrMov, 0,
                         X86OpRegBWord, 0,
                         X86OpRegAWord, 0, 0);
  if (opSz == -1)
    GenPrintInstr2Operands(X86InstrMovSx, 0,
                           X86OpRegCWord, 0,
                           X86OpIndRegBExplicitByte, 0, flags);
  else if (opSz == 1)
    GenPrintInstr2Operands(X86InstrMovZx, 0,
                           X86OpRegCWord, 0,
                           X86OpIndRegBExplicitByte, 0, flags);
  else if (opSz != SizeOfWord && -opSz != SizeOfWord) {
    if (opSz == -2)
      GenPrintInstr2Operands(X86InstrMovSx, 0,
                             X86OpRegCWord, 0,
                             X86OpIndRegBExplicitHalfWord, 0, flags); // (haven't found this one yet, but sure we need it)
    else if (opSz == 2)
      GenPrintInstr2Operands(X86InstrMovZx, 0,
                             X86OpRegCWord, 0,
                             X86OpIndRegBExplicitHalfWord, 0, flags);
  } else
    GenPrintInstr2Operands(X86InstrMov, 0,
                           X86OpRegCWord, 0,
                           X86OpIndRegB, 0, flags);
}

void GenIncDecIdent(int opSz, int label, int tok) {
  int instr = X86InstrInc;
  
  if (tok != tokInc)
    instr = X86InstrDec;
  
  GenPrintInstr1Operand(instr, 0,
                        GenSelectByteOrWord(X86OpIndLabelExplicitByteOrWord, opSz), label, 0);
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLabel, label, 0);
  GenExtendRegAIfNeeded(opSz);
}

void GenIncDecLocal(int opSz, int ofs, int tok) {
  int instr = X86InstrInc;
  
  if (tok != tokInc)
    instr = X86InstrDec;

  GenPrintInstr1Operand(instr, 0,
                        GenSelectByteOrWord(X86OpIndLocalExplicitByteOrWord, opSz), ofs, 0);
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLocal, ofs, 0);
  GenExtendRegAIfNeeded(opSz);
}

void GenIncDecIndirect(int opSz, int tok, const bit32u flags) {
  int instr = X86InstrInc;

  if (tok != tokInc)
    instr = X86InstrDec;

  GenPrintInstr2Operands(X86InstrMov, 0,
                         X86OpRegBWord, 0,
                         X86OpRegAWord, 0, 0);
  GenPrintInstr1Operand(instr, 0,
                        GenSelectByteOrWord(X86OpIndRegBExplicitByteOrWord, opSz), 0, flags);
// TODO: Some instructions don't need these two remaining lines... ?????  rr->t->fp[0]--;
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndRegB, 0, flags);
  GenExtendRegAIfNeeded(opSz);
}

void GenPostIncDecIdent(int opSz, int label, int tok) {
  int instr = X86InstrInc;

  if (tok != tokPostInc)
    instr = X86InstrDec;

  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLabel, label, 0);
  GenExtendRegAIfNeeded(opSz);
  GenPrintInstr1Operand(instr, 0,
                        GenSelectByteOrWord(X86OpIndLabelExplicitByteOrWord, opSz), label, 0);
}

void GenPostIncDecLocal(int opSz, int ofs, int tok) {
  int instr = X86InstrInc;

  if (tok != tokPostInc)
    instr = X86InstrDec;

  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLocal, ofs, 0);
  GenExtendRegAIfNeeded(opSz);
  GenPrintInstr1Operand(instr, 0,
                        GenSelectByteOrWord(X86OpIndLocalExplicitByteOrWord, opSz), ofs, 0);
}

void GenPostIncDecIndirect(int opSz, int tok, const bit32u flags) {
  int instr = X86InstrInc;
  
  if (tok != tokPostInc)
    instr = X86InstrDec;

  GenPrintInstr2Operands(X86InstrMov, 0,
                         X86OpRegBWord, 0,
                         X86OpRegAWord, 0, 0);
// TODO: Some instructions don't need these two first lines... ?????  rr->t->fp[0]--;
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndRegB, 0, flags);
  GenExtendRegAIfNeeded(opSz);
  GenPrintInstr1Operand(instr, 0,
                        GenSelectByteOrWord(X86OpIndRegBExplicitByteOrWord, opSz), 0, flags);
}

void GenPostAddSubIdent(int opSz, int val, int label, int tok) {
  int instr = X86InstrAdd;

  if (tok != tokPostAdd)
    instr = X86InstrSub;

  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLabel, label, 0);
  GenExtendRegAIfNeeded(opSz);
  GenPrintInstr2Operands(instr, 0,
                         GenSelectByteOrWord(X86OpIndLabelExplicitByteOrWord, opSz), label,
                         X86OpConst, val, 0);
}

void GenPostAddSubLocal(int opSz, int val, int ofs, int tok) {
  int instr = X86InstrAdd;

  if (tok != tokPostAdd)
    instr = X86InstrSub;

  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndLocal, ofs, 0);
  GenExtendRegAIfNeeded(opSz);
  GenPrintInstr2Operands(instr, 0,
                         GenSelectByteOrWord(X86OpIndLocalExplicitByteOrWord, opSz), ofs,
                         X86OpConst, val, 0);
}

void GenPostAddSubIndirect(int opSz, int val, int tok) {
  int instr = X86InstrAdd;

  if (tok != tokPostAdd)
    instr = X86InstrSub;
  
  GenPrintInstr2Operands(X86InstrMov, 0,
                         X86OpRegBWord, 0,
                         X86OpRegAWord, 0, 0);
  GenPrintInstr2Operands(X86InstrMov, 0,
                         GenSelectByteOrWord(X86OpRegAByteOrWord, opSz), 0,
                         X86OpIndRegB, 0, 0);
  GenExtendRegAIfNeeded(opSz);
  GenPrintInstr2Operands(instr, 0,
                         GenSelectByteOrWord(X86OpIndRegBExplicitByteOrWord, opSz), 0,
                         X86OpConst, val, 0);
}

#define tokOpNumInt      0x100
#define tokOpNumUint     0x101
#define tokOpIdent       0x102
#define tokOpLocalOfs    0x103
#define tokOpAcc         0x104
#define tokOpIndIdent    0x105
#define tokOpIndLocalOfs 0x106
#define tokOpIndAcc      0x107
#define tokOpStack       0x108
#define tokOpIndStack    0x109

#define tokPushAcc       0x200

int GetOperandInfo(int idx, int lvalSize, int *val, int *size, int *delDeref, bit32u *flags) {
  int idx0 = idx;
  
  *delDeref = 0;
  
  while ((stack[idx].tok >= tokOpNumInt) && (stack[idx].tok <= tokOpIndAcc))
    idx--;
  
  if (stack[idx].tok == tokUnaryStar) {
    if (lvalSize) {
      // lvalue dereference is implied for the left operand of =
      // and for operands of ++/--, these operands contain the
      // lvalue address
      *size = lvalSize;
      *val = 0;
      return tokOpIndAcc;
    }
    
    *size = stack[idx].param; // take size from tokUnaryStar
    *flags = stack[idx].flags;
    
    *delDeref = 1;
    *val = stack[idx + 1].param; // operand "value" is in tokUnaryStar's operand
    return stack[idx + 1].tok + tokOpIndIdent - tokOpIdent; // add indirection
  }
  
  idx = idx0;
  
  if (lvalSize) {
    // lvalue dereference is implied for the left operand of =
    // and for operands of ++/--
    *size = lvalSize;
    *val = stack[idx].param;
    
    switch (stack[idx].tok) {
      case tokIdent:
        return tokOpIndIdent;
      case tokLocalOfs:
        return tokOpIndLocalOfs;
      default:
        *val = 0;
        return tokOpIndAcc;
    }
  }
  
  *size = SizeOfWord;
  *val = stack[idx].param;
  
  switch (stack[idx].tok) {
    case tokNumInt:
      return tokOpNumInt;
    case tokNumUint:
      return tokOpNumUint;
    case tokIdent:
      return tokOpIdent;
    case tokLocalOfs:
      return tokOpLocalOfs;
    default:
      *val = 0;
      return tokOpAcc;
  }
}

void GenFuse(int *idx) {
  int tok;
  int oldIdxRight, oldSpRight;
  int oldIdxLeft, oldSpLeft;
  int opSzRight, opSzLeft;
  int opTypRight, opTypLeft;
  int opValRight, opValLeft;
  int delDerefRight, delDerefLeft;
  int num, lvalSize;
  bit32u rFlags = 0, lFlags = 0;
  
  if (*idx < 0)
    ERROR(0xE015, FALSE);
  
  tok = stack[*idx].tok;
  
  --*idx;
  
  oldIdxRight = *idx;
  oldSpRight = sp;
  
  switch (tok) {
    case tokNumInt:
    case tokNumUint:
    case tokIdent:
    case tokLocalOfs:
      break;
      
    case tokShortCirc:
    case tokGoto:
      GenFuse(idx);
      break;
      
    case tokUnaryStar:
      opSzRight = stack[*idx + 1].param;
      GenFuse(idx);
      oldIdxRight -= oldSpRight - sp;
      
      switch (stack[oldIdxRight].tok) {
        case tokIdent:
        case tokLocalOfs:
          if (stack[oldIdxRight].tok == tokIdent)
            stack[oldIdxRight + 1].tok = tokOpIdent;
          else
            stack[oldIdxRight + 1].tok = tokOpLocalOfs;
          stack[oldIdxRight + 1].param = stack[oldIdxRight].param;
          stack[oldIdxRight].tok = tok;
          stack[oldIdxRight].param = opSzRight;
          break;
        default:
          ins(oldIdxRight + 2, tokOpAcc);
          break;
      }
      break;

    case tokInc:
    case tokDec:
    case tokPostInc:
    case tokPostDec:
      opSzRight = stack[*idx + 1].param;
      GenFuse(idx);
      oldIdxRight -= oldSpRight - sp;
      
      switch (stack[oldIdxRight].tok) {
        case tokIdent:
        case tokLocalOfs:
          if (stack[oldIdxRight].tok == tokIdent)
            stack[oldIdxRight + 1].tok = tokOpIndIdent;
          else
            stack[oldIdxRight + 1].tok = tokOpIndLocalOfs;
          stack[oldIdxRight + 1].param = stack[oldIdxRight].param;
          stack[oldIdxRight].tok = tok;
          stack[oldIdxRight].param = opSzRight;
          break;
        default:
          ins(oldIdxRight + 2, tokOpIndAcc);
          break;
      }
      break;

    case '~':
    case tokUnaryPlus:
    case tokUnaryMinus:
    case tok_Bool:
    case tokVoid:
    case tokUChar:
    case tokSChar:
    case tokShort:
    case tokUShort:
      GenFuse(idx);
      oldIdxRight -= oldSpRight - sp;
      if (tok == tokUnaryPlus)
        del(oldIdxRight + 1, 1);
      break;

    case tokPostAdd:
    case tokPostSub:
      opSzRight = stack[*idx + 1].param;
      num = stack[*idx].param;
      oldIdxRight = --*idx; // skip tokNum
      GenFuse(idx);
      oldIdxRight -= oldSpRight - sp;
      switch (stack[oldIdxRight].tok) {
        case tokIdent:
        case tokLocalOfs:
          stack[oldIdxRight + 2].tok = tokOpNumInt;
          stack[oldIdxRight + 2].param = num;
          if (stack[oldIdxRight].tok == tokIdent)
            stack[oldIdxRight + 1].tok = tokOpIndIdent;
          else
            stack[oldIdxRight + 1].tok = tokOpIndLocalOfs;
          stack[oldIdxRight + 1].param = stack[oldIdxRight].param;
          stack[oldIdxRight].tok = tok;
          stack[oldIdxRight].param = opSzRight;
          break;
        default:
          stack[oldIdxRight + 1].tok = tok;
          stack[oldIdxRight + 1].param = opSzRight;
          stack[oldIdxRight + 2].tok = tokOpIndAcc;
          ins2(oldIdxRight + 3, tokOpNumInt, num);
          //ins3(oldIdxRight + 3, tokOpNumInt, num, rFlags);
          break;
      }
      break;

/*
  Operator-operand fusion:

  ac = lft:       ac op= rht:        lft = ac:
  (load)          ("execute")        (store)

  *(id/l)         *(id/l)            *(id/l)
    mov a?,mlft     op a?,mrht         mov mlft,a?
                    ---
                    mov cl,mrht
                    shift ax,cl
                    ---
                    mov c?,mrht
                    cwd
                    idiv cx
                    opt: mov ax,dx

  *ac             *ac                *ac
    mov bx,ax       < mov bx,ax        ; bx preserved
    mov a?,[bx]     < mov c?,[bx]      mov [bx],a?
                    op ax,cx(cl)

  *ac-stack       n/a                *ac-stack
    pop bx                             ; bx preserved
    mov a?,[bx]                        mov [bx],a?

  id/num          id/num
    mov ax,ilft     op ax,irht
                    ---
                    mov cx,irht
                    op ax,cx

  l               l
    lea ax,llft     lea cx,lrht
                    op ax,cx

  ac              ac
    nop             < mov cx,ax
                    op ax,cx

  ac-stack        n/a
    pop ax

  lft (*)ac -> lft (*)ac-stack IFF rht is (*)ac

  Legend:
  - lft/rht - left/right operand
  - num - number
  - id - global/static identifier/location
  - l - local variable location
  - * - dereference operator
  - m - memory operand at address id/l
  - i - immediate/number/constant operand
  - ac - accumulator (al or ax)
  - a? - accumulator (al or ax), depending on operand size
  - b? - bl or bx, depending on operand size
  - >push axlft - need to insert "push ax" at the end of the left operand evaluation

    instruction operand combinations (dst/lft, src/rht):
    - r/m, r/imm
    - r, m

    special instructions:
    - lea r, m
    - shl/sar
    - mul/imul/idiv
    - cbw/cwd
    - movsx/movzx
*/

    case '=':
    case tokAssignAdd:
    case tokAssignSub:
    case tokAssignMul:
    case tokAssignDiv:
    case tokAssignUDiv:
    case tokAssignMod:
    case tokAssignUMod:
    case tokAssignLSh:
    case tokAssignRSh:
    case tokAssignURSh:
    case tokAssignAnd:
    case tokAssignXor:
    case tokAssignOr:
    case '+':
    case '-':
    case '*':
    case '/':
    case tokUDiv:
    case '%':
    case tokUMod:
    case tokLShift:
    case tokRShift:
    case tokURShift:
    case '&':
    case '^':
    case '|':
    case '<':
    case '>':
    case tokLEQ:
    case tokGEQ:
    case tokEQ:
    case tokNEQ:
    case tokULess:
    case tokUGreater:
    case tokULEQ:
    case tokUGEQ:
    case tokLogAnd:
    case tokLogOr:
    case tokComma:
      switch (tok) {
        case '=':
        case tokAssignAdd:
        case tokAssignSub:
        case tokAssignMul:
        case tokAssignDiv:
        case tokAssignUDiv:
        case tokAssignMod:
        case tokAssignUMod:
        case tokAssignLSh:
        case tokAssignRSh:
        case tokAssignURSh:
        case tokAssignAnd:
        case tokAssignXor:
        case tokAssignOr:
          lvalSize = stack[*idx + 1].param;
          lFlags = stack[*idx + 1].flags;
          break;
        default:
          lvalSize = 0;
          lFlags = 0;
          break;
      }
      
      GenFuse(idx);
      oldIdxRight -= oldSpRight - sp;
      opTypRight = GetOperandInfo(oldIdxRight, 0, &opValRight, &opSzRight, &delDerefRight, &rFlags);
      
      oldIdxLeft = *idx; oldSpLeft = sp;
      GenFuse(idx);
      oldIdxLeft -= oldSpLeft - sp;
      oldIdxRight -= oldSpLeft - sp;
      opTypLeft = GetOperandInfo(oldIdxLeft, lvalSize, &opValLeft, &opSzLeft, &delDerefLeft, &lFlags);
      
      // operands of &&, || and comma aren't to be fused into &&, || and comma
      if (tok == tokLogAnd || tok == tokLogOr || tok == tokComma)
        break;
      
      if (opTypLeft != tokOpAcc && opTypLeft != tokOpIndAcc) {
        // the left operand will be fully fused into the operator, remove it
        int cnt = oldIdxLeft - *idx;
        del(*idx + 1, cnt);
        oldIdxLeft -= cnt;
        oldIdxRight -= cnt;
      } else if (opTypRight == tokOpAcc || opTypRight == tokOpIndAcc) {
        // preserve ax after the evaluation of the left operand
        // because the right operand's value ends up in ax as well
        ins(++oldIdxLeft, tokPushAcc);
        oldIdxRight++;
        // adjust the left operand "type"/location
        if (opTypLeft == tokOpAcc)
          opTypLeft = tokOpStack;
        else
          opTypLeft = tokOpIndStack;
        if (delDerefLeft) {
          // remove the dereference, fusing will take care of it
          del(oldIdxLeft -= 2, 2);
          oldIdxRight -= 2;
        }
      } else if (delDerefLeft) {
        // remove the dereference, fusing will take care of it
        del(oldIdxLeft - 1, 2);
        oldIdxLeft -= 2;
        oldIdxRight -= 2;
      }

      if (opTypRight != tokOpAcc && opTypRight != tokOpIndAcc) {
        // the right operand will be fully fused into the operator, remove it
        int cnt = oldIdxRight - oldIdxLeft;
        del(oldIdxLeft + 1, cnt);
        oldIdxRight -= cnt;
      } else if (delDerefRight) {
        // remove the dereference, fusing will take care of it
        del(oldIdxRight - 1, 2);
        oldIdxRight -= 2;
      }
      
      // store the operand sizes into the operator
      stack[oldIdxRight + 1].param = (opSzLeft + 8) * 16 + (opSzRight + 8);
      
      // fuse the operands into the operator
      ins3(oldIdxRight + 2, opTypRight, opValRight, rFlags);
      ins3(oldIdxRight + 2, opTypLeft, opValLeft, lFlags);
      break;
      
    case ')':
      while (stack[*idx].tok != '(') {
        GenFuse(idx);
        if (stack[*idx].tok == ',')
          --*idx;
      }
      --*idx;
      break;
      
    default:
      ERROR(0xE016, FALSE, GetTokenName(tok));
  }
}

int GenGetBinaryOperatorInstr(int tok) {
  switch (tok) {
    case tokPostAdd:
    case tokAssignAdd:
    case '+':
      return X86InstrAdd;
    case tokPostSub:
    case tokAssignSub:
    case '-':
      return X86InstrSub;
    case '&':
    case tokAssignAnd:
      return X86InstrAnd;
    case '^':
    case tokAssignXor:
      return X86InstrXor;
    case '|':
    case tokAssignOr:
      return X86InstrOr;
    case '<':
    case '>':
    case tokLEQ:
    case tokGEQ:
    case tokEQ:
    case tokNEQ:
    case tokULess:
    case tokUGreater:
    case tokULEQ:
    case tokUGEQ:
      return X86InstrCmp;
    case '*':
    case tokAssignMul:
      return X86InstrMul;
    case '/':
    case '%':
    case tokAssignDiv:
    case tokAssignMod:
      return X86InstrIdiv;
    case tokUDiv:
    case tokUMod:
    case tokAssignUDiv:
    case tokAssignUMod:
      return X86InstrDiv;
    case tokLShift:
    case tokAssignLSh:
      return X86InstrShl;
    case tokRShift:
    case tokAssignRSh:
      return X86InstrSar;
    case tokURShift:
    case tokAssignURSh:
      return X86InstrShr;

    default:
      ERROR(0xE017, FALSE);
      return 0;
    }
}

void GenExpr1(void) {
  int s = sp - 1;
  int i;
  
  if ((stack[s].tok == tokIf) || (stack[s].tok == tokIfNot) || (stack[s].tok == tokReturn))
    s--;
  
  GenFuse(&s);
  
  for (i = 0; i < sp; i++) {
    int tok = stack[i].tok;
    int v = stack[i].param;
    bit32u flags = stack[i].flags;
    int instr;
    
    switch (tok) {
      case tokNumInt:
      case tokNumUint:
        // Don't load operand into ax when ax is going to be pushed next, push it directly
        if (!(i + 1 < sp && stack[i + 1].tok == ','))
          GenPrintInstr2Operands(X86InstrMov, 0,
                                 X86OpRegAWord, 0,
                                 X86OpConst, v, 0);
        break;
      case tokIdent:
        // Don't load operand into ax when ax is going to be pushed next, push it directly
        if (!(i + 1 < sp && (stack[i + 1].tok == ',' || stack[i + 1].tok == ')')))
          GenPrintInstr2Operands(X86InstrMov, 0,
                                 X86OpRegAWord, 0,
                                 X86OpLabel, v, 0); // ;';';';'; TODO: should this be X86OpLabelOff ????
        break;
      case tokLocalOfs:
        GenPrintInstr2Operands(X86InstrLea, 0,
                               X86OpRegAWord, 0,
                               X86OpIndLocal, v, 0);
        break;

      case '~':
        GenPrintInstr1Operand(X86InstrNot, 0,
                              X86OpRegAWord, 0, 0);
        break;
      case tokUnaryMinus:
        GenPrintInstr1Operand(X86InstrNeg, 0,
                              X86OpRegAWord, 0, 0);
        break;
      case tok_Bool:
        GenPrintInstr2Operands(X86InstrTest, 0,
                               X86OpRegAWord, 0,
                               X86OpRegAWord, 0, 0);
        GenPrintInstr1Operand(X86InstrSetCc, tokNEQ,
                              X86OpRegAByte, 0, 0);
        // fallthrough
      case tokSChar:
        if (SizeOfWord == 2)
          GenPrintInstrNoOperand(X86InstrCbw);
        else
          GenPrintInstr2Operands(X86InstrMovSx, 0,
                                 X86OpRegAWord, 0,
                                 X86OpRegAByte, 0, 0);
        break;
      case tokUChar:
        GenPrintInstr2Operands(X86InstrAnd, 0,
                               X86OpRegAWord, 0, 
                               X86OpConst, 0xFF, 0);
        break;
      case tokShort:
        GenPrintInstr2Operands(X86InstrMovSx, 0,
                               X86OpRegAWord, 0,
                               X86OpRegAHalfWord, 0, 0);
        break;
      case tokUShort:
        GenPrintInstr2Operands(X86InstrMovZx, 0,
                               X86OpRegAWord, 0,
                               X86OpRegAHalfWord, 0, 0);
        break;

      case tokShortCirc:
        if (v >= 0)
          GenJumpIfZero(v); // &&
        else
          GenJumpIfNotZero(-v); // ||
        break;
      case tokGoto:
        GenJumpUncond(v);
        break;
      case tokLogAnd:
      case tokLogOr:
        GenNumLabel(v);
        break;

      case tokPushAcc: // push accumulator
        // TBD??? handle similarly to ','???
        GenPrintInstr1Operand(X86InstrPush, 0,
                              X86OpRegAWord, 0, 0);
        break;

      case ',':
        // push operand directly if it hasn't been loaded into ax
        if ((stack[i - 2].tok == tokUnaryStar) && (stack[i - 2].param == SizeOfWord)) {
          switch (stack[i - 1].tok) {
            case tokOpIdent:
              GenPrintInstr1Operand(X86InstrPush, 0,
                                    X86OpIndLabelExplicitWord, stack[i - 1].param, 0);
              break;
            case tokOpLocalOfs:
              GenPrintInstr1Operand(X86InstrPush, 0,
                                    X86OpIndLocalExplicitWord, stack[i - 1].param, 0);
              break;
            case tokOpAcc:
              GenPrintInstr2Operands(X86InstrMov, 0,
                                     X86OpRegBWord, 0,
                                     X86OpRegAWord, 0, 0);
              GenPrintInstr1Operand(X86InstrPush, 0,
                                    X86OpIndRegBExplicitWord, 0, stack[i - 2].flags);
              break;
          }
        } else {
          switch (stack[i - 1].tok) {
            case tokNumInt:
            case tokNumUint:
              GenPrintInstr1Operand(X86InstrPush, 0,
                                    X86OpConst, stack[i - 1].param, 0);
              break;
            case tokIdent:
              GenPrintInstr1Operand(X86InstrPush, 0,
                                    X86OpLabelOff, stack[i - 1].param, 0);
              break;
            default:
              GenPrintInstr1Operand(X86InstrPush, 0,
                                    X86OpRegAWord, 0, 0);
              break;
          }
        }
        break;

      case tokUnaryStar:
        // Don't load operand into ax when ax is going to be pushed next, push it directly
        if (!(v == SizeOfWord && i + 2 < sp && stack[i + 2].tok == ',')) {
          switch (stack[i + 1].tok) {
            case tokOpIdent:
              GenReadIdent(v, stack[i + 1].param);
              break;
            case tokOpLocalOfs:
              GenReadLocal(v, stack[i + 1].param);
              break;
            case tokOpAcc:
              GenReadIndirect(v, stack[i].flags);
              break;
            }
        }
        i++;
        break;

      case tokInc:
      case tokDec:
        switch (stack[i + 1].tok) {
          case tokOpIndIdent:
            GenIncDecIdent(v, stack[i + 1].param, tok);
            break;
          case tokOpIndLocalOfs:
            GenIncDecLocal(v, stack[i + 1].param, tok);
            break;
          case tokOpIndAcc:
            GenIncDecIndirect(v, tok, stack[i].flags);
            break;
        }
        i++;
        break;

      case tokPostInc:
      case tokPostDec:
        switch (stack[i + 1].tok) {
        case tokOpIndIdent:
          GenPostIncDecIdent(v, stack[i + 1].param, tok);
          break;
        case tokOpIndLocalOfs:
          GenPostIncDecLocal(v, stack[i + 1].param, tok);
          break;
        case tokOpIndAcc:
          GenPostIncDecIndirect(v, tok, stack[i].flags);
          break;
        }
        i++;
        break;

      case tokPostAdd:
      case tokPostSub:
        switch (stack[i + 1].tok) {
          case tokOpIndIdent:
            GenPostAddSubIdent(v, stack[i + 2].param, stack[i + 1].param, tok);
            break;
          case tokOpIndLocalOfs:
            GenPostAddSubLocal(v, stack[i + 2].param, stack[i + 1].param, tok);
            break;
          case tokOpIndAcc:
            GenPostAddSubIndirect(v, stack[i + 2].param, tok);
            break;
        }
        i += 2;
        break;

      case '=':
      case tokAssignAdd:
      case tokAssignSub:
      case tokAssignMul:
      case tokAssignDiv:
      case tokAssignUDiv:
      case tokAssignMod:
      case tokAssignUMod:
      case tokAssignLSh:
      case tokAssignRSh:
      case tokAssignURSh:
      case tokAssignAnd:
      case tokAssignXor:
      case tokAssignOr:
      case '+':
      case '-':
      case '*':
      case '/':
      case tokUDiv:
      case '%':
      case tokUMod:
      case tokLShift:
      case tokRShift:
      case tokURShift:
      case '&':
      case '^':
      case '|':
      case '<':
      case '>':
      case tokLEQ:
      case tokGEQ:
      case tokEQ:
      case tokNEQ:
      case tokULess:
      case tokUGreater:
      case tokULEQ:
      case tokUGEQ:
        // save the right operand from ax in cx, so it's not
        // overwritten by the left operand in ax
        if (tok != '=') {
          if (stack[i + 2].tok == tokOpAcc)
            GenPrintInstr2Operands(X86InstrMov, 0,
                                   X86OpRegCWord, 0,
                                   X86OpRegAWord, 0, 0);
          else if (stack[i + 2].tok == tokOpIndAcc)
            GenReadCRegIndirect(v % 16 - 8, stack[i + 2].flags);
        }

        // load the left operand into ax (or the right operand if it's '=')
        if (tok == '=') {
          if (stack[i + 1].tok == tokOpIndAcc) {
            GenPrintInstr2Operands(X86InstrMov, 0,
                                   X86OpRegBWord, 0,
                                   X86OpRegAWord, 0, 0);
          }
          // "swap" left and right operands
          i++;
          v = v / 16 + v % 16 * 16;
        }
        
        switch (stack[i + 1].tok) {
          case tokOpNumInt:
          case tokOpNumUint:
            GenPrintInstr2Operands(X86InstrMov, 0,
                                   X86OpRegAWord, 0,
                                   X86OpConst, stack[i + 1].param, 0);
            break;
          case tokOpIdent:
            GenPrintInstr2Operands(X86InstrMov, 0,
                                   X86OpRegAWord, 0,
                                   X86OpLabelOff, stack[i + 1].param, 0);
            break;
          case tokOpLocalOfs:
            GenPrintInstr2Operands(X86InstrLea, 0,
                                   X86OpRegAWord, 0,
                                   X86OpIndLocal, stack[i + 1].param, 0);
            break;
          case tokOpAcc:
            break;
          case tokOpIndIdent:
            GenReadIdent(v / 16 - 8, stack[i + 1].param);
            break;
          case tokOpIndLocalOfs:
            GenReadLocal(v / 16 - 8, stack[i + 1].param);
            break;
          case tokOpIndAcc:
            GenReadIndirect(v / 16 - 8, stack[i + 1].flags);
            break;
          case tokOpStack:
            GenPrintInstr1Operand(X86InstrPop, 0,
                                  X86OpRegAWord, 0, 0);
            break;
          case tokOpIndStack:
            GenPrintInstr1Operand(X86InstrPop, 0,
                                  X86OpRegBWord, 0, 0);
            GenPrintInstr2Operands(X86InstrMov, 0,
                                   GenSelectByteOrWord(X86OpRegAByteOrWord, v / 16 - 8), 0,
                                   X86OpIndRegB, 0, stack[i + 1].flags);
            GenExtendRegAIfNeeded(v / 16 - 8);
            break;
        }
        
        if (tok == '=') {
          // "unswap" left and right operands
          i--;
          v = v / 16 + v % 16 * 16;
          
          if (stack[i + 1].tok == tokOpIndStack)
            GenPrintInstr1Operand(X86InstrPop, 0,
                                  X86OpRegBWord, 0, 0);
        }
        
        // operator
        switch (tok) {
          case tokAssignAdd:
          case tokAssignSub:
          case tokAssignAnd:
          case tokAssignXor:
          case tokAssignOr:
          case '+':
          case '-':
          case '&':
          case '^':
          case '|':
          case '<':
          case '>':
          case tokLEQ:
          case tokGEQ:
          case tokEQ:
          case tokNEQ:
          case tokULess:
          case tokUGreater:
          case tokULEQ:
          case tokUGEQ:
            instr = GenGetBinaryOperatorInstr(tok);
            
            switch (stack[i + 2].tok) {
              case tokOpNumInt:
              case tokOpNumUint:
                GenPrintInstr2Operands(instr, 0,
                                       X86OpRegAWord, 0,
                                       X86OpConst, stack[i + 2].param, 0);
                break;
              case tokOpIdent:
                GenPrintInstr2Operands(instr, 0,
                                       X86OpRegAWord, 0,
                                       X86OpLabel, stack[i + 2].param, 0);
                break;
              case tokOpLocalOfs:
                GenPrintInstr2Operands(X86InstrLea, 0,
                                       X86OpRegCWord, 0,
                                       X86OpIndLocal, stack[i + 2].param, 0);
                GenPrintInstr2Operands(instr, 0,
                                       X86OpRegAWord, 0,
                                       X86OpRegCWord, 0, 0);
                break;
              case tokOpAcc:
              case tokOpIndAcc:
                // right operand in cx already
                GenPrintInstr2Operands(instr, 0,
                                       X86OpRegAWord, 0,
                                       X86OpRegCWord, 0, 0);
                break;
              case tokOpIndIdent:
                if (v % 16 - 8 != SizeOfWord)
                {
                  GenReadCRegIdent(v % 16 - 8, stack[i + 2].param);
                  GenPrintInstr2Operands(instr, 0,
                                         X86OpRegAWord, 0,
                                         X86OpRegCWord, 0, 0);
                }
                else
                {
                  GenPrintInstr2Operands(instr, 0,
                                         X86OpRegAWord, 0,
                                         X86OpIndLabel, stack[i + 2].param, 0);
                }
                break;
              case tokOpIndLocalOfs:
                if (v % 16 - 8 != SizeOfWord) {
                  GenReadCRegLocal(v % 16 - 8, stack[i + 2].param);
                  GenPrintInstr2Operands(instr, 0,
                                         X86OpRegAWord, 0,
                                         X86OpRegCWord, 0, 0);
                } else {
                  GenPrintInstr2Operands(instr, 0,
                                         X86OpRegAWord, 0,
                                         X86OpIndLocal, stack[i + 2].param, 0);
                }
                break;
            }
            
            if (i + 3 < sp && (stack[i + 3].tok == tokIf || stack[i + 3].tok == tokIfNot)) {
              switch (tok) {
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
                  if (stack[i + 3].tok == tokIf) {
                    GenPrintInstr1Operand(X86InstrJcc, tok, X86OpNumLabel, stack[i + 3].param, 0);
                  } else {
                    GenPrintInstr1Operand(X86InstrJNotCc, tok, X86OpNumLabel, stack[i + 3].param, 0);
                  }
                  break;
              }
            } else {
              switch (tok) {
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
                  GenPrintInstr1Operand(X86InstrSetCc, tok,
                                        X86OpRegAByte, 0, 0);
                  if (SizeOfWord == 2)
                    GenPrintInstrNoOperand(X86InstrCbw);
                  else
                    GenPrintInstr2Operands(X86InstrMovZx, 0,
                                           X86OpRegAWord, 0,
                                           X86OpRegAByte, 0, 0);
                  break;
              }
            }
            break;

          case '*':
          case tokAssignMul:
            instr = GenGetBinaryOperatorInstr(tok);

            switch (stack[i + 2].tok) {
            case tokOpNumInt:
            case tokOpNumUint:
              GenPrintInstr3Operands(X86InstrImul, 0,
                                     X86OpRegAWord, 0,
                                     X86OpRegAWord, 0,
                                     X86OpConst, stack[i + 2].param, 0);
              break;
            case tokOpIdent:
              GenPrintInstr3Operands(X86InstrImul, 0,
                                     X86OpRegAWord, 0,
                                     X86OpRegAWord, 0,
                                     X86OpLabel, stack[i + 2].param, 0);
              break;
            case tokOpLocalOfs:
              GenPrintInstr2Operands(X86InstrLea, 0,
                                     X86OpRegCWord, 0,
                                     X86OpIndLocal, stack[i + 2].param, 0);
              GenPrintInstr1Operand(instr, 0,
                                    X86OpRegCWord, 0, 0);
              break;
            case tokOpAcc:
            case tokOpIndAcc:
              // right operand in cx already
              GenPrintInstr1Operand(instr, 0,
                                    X86OpRegCWord, 0, 0);
              break;
            case tokOpIndIdent:
              if (v % 16 - 8 != SizeOfWord) {
                GenReadCRegIdent(v % 16 - 8, stack[i + 2].param);
                GenPrintInstr1Operand(instr, 0,
                                      X86OpRegCWord, 0, 0);
              }
              else
              {
                GenPrintInstr1Operand(instr, 0,
                                      X86OpIndLabelExplicitWord, stack[i + 2].param, 0); //
              }
              break;
            case tokOpIndLocalOfs:
              if (v % 16 - 8 != SizeOfWord) {
                GenReadCRegLocal(v % 16 - 8, stack[i + 2].param);
                GenPrintInstr1Operand(instr, 0,
                                      X86OpRegCWord, 0, 0);
              }
              else
              {
                GenPrintInstr1Operand(instr, 0,
                                      X86OpIndLocalExplicitWord, stack[i + 2].param, 0); //
              }
              break;
            }
            break;

          case '/':
          case tokUDiv:
          case '%':
          case tokUMod:
          case tokAssignDiv:
          case tokAssignUDiv:
          case tokAssignMod:
          case tokAssignUMod:
            instr = GenGetBinaryOperatorInstr(tok);
            switch (tok) {
              case '/':
              case '%':
              case tokAssignDiv:
              case tokAssignMod:
                if (SizeOfWord == 2)
                  GenPrintInstrNoOperand(X86InstrCwd);
                else
                  GenPrintInstrNoOperand(X86InstrCdq);
                break;
              default:
                GenPrintInstr2Operands(X86InstrMov, 0,
                                       X86OpRegDWord, 0,
                                       X86OpConst, 0, 0);
                break;
            }

            switch (stack[i + 2].tok) {
            case tokOpNumInt:
            case tokOpNumUint:
              GenPrintInstr2Operands(X86InstrMov, 0,
                                     X86OpRegCWord, 0,
                                     X86OpConst, stack[i + 2].param, 0);
              GenPrintInstr1Operand(instr, 0,
                                    X86OpRegCWord, 0, 0);
              break;
            case tokOpIdent:
              GenPrintInstr2Operands(X86InstrMov, 0,
                                     X86OpRegCWord, 0,
                                     X86OpLabel, stack[i + 2].param, 0);
              GenPrintInstr1Operand(instr, 0,
                                    X86OpRegCWord, 0, 0);
              break;
            case tokOpLocalOfs:
              GenPrintInstr2Operands(X86InstrLea, 0,
                                     X86OpRegCWord, 0,
                                     X86OpIndLocal, stack[i + 2].param, 0);
              GenPrintInstr1Operand(instr, 0,
                                    X86OpRegCWord, 0, 0);
              break;
            case tokOpAcc:
            case tokOpIndAcc:
              // right operand in cx already
              GenPrintInstr1Operand(instr, 0,
                                    X86OpRegCWord, 0, 0);
              break;
            case tokOpIndIdent:
              if (v % 16 - 8 != SizeOfWord)
              {
                GenReadCRegIdent(v % 16 - 8, stack[i + 2].param);
                GenPrintInstr1Operand(instr, 0,
                                      X86OpRegCWord, 0, 0);
              }
              else
              {
                GenPrintInstr1Operand(instr, 0,
                                      X86OpIndLabelExplicitWord, stack[i + 2].param, 0); //
              }
              break;
            case tokOpIndLocalOfs:
              if (v % 16 - 8 != SizeOfWord) {
                GenReadCRegLocal(v % 16 - 8, stack[i + 2].param);
                GenPrintInstr1Operand(instr, 0,
                                      X86OpRegCWord, 0, 0);
              }
              else {
                GenPrintInstr1Operand(instr, 0,
                                      X86OpIndLocalExplicitWord, stack[i + 2].param, 0); //
              }
            }

            if (tok == '%' || tok == tokAssignMod ||
                tok == tokUMod || tok == tokAssignUMod)
              GenPrintInstr2Operands(X86InstrMov, 0,
                                     X86OpRegAWord, 0,
                                     X86OpRegDWord, 0, 0);
            break;

          case tokLShift:
          case tokRShift:
          case tokURShift:
          case tokAssignLSh:
          case tokAssignRSh:
          case tokAssignURSh:
            instr = GenGetBinaryOperatorInstr(tok);

            switch (stack[i + 2].tok) {
            case tokOpNumInt:
            case tokOpNumUint:
              GenPrintInstr2Operands(instr, 0,
                                     X86OpRegAWord, 0,
                                     X86OpConst, stack[i + 2].param, 0);
              break;
            case tokOpIdent:
              GenPrintInstr2Operands(instr, 0,
                                     X86OpRegAWord, 0,
                                     X86OpLabel, stack[i + 2].param, 0);
              break;
            case tokOpLocalOfs:
              GenPrintInstr2Operands(X86InstrLea, 0,
                                     X86OpRegCWord, 0,
                                     X86OpIndLocal, stack[i + 2].param, 0);
              GenPrintInstr2Operands(instr, 0,
                                     X86OpRegAWord, 0,
                                     X86OpRegCByte, 0, 0);
              break;
            case tokOpAcc:
            case tokOpIndAcc:
              // right operand in cx already
              GenPrintInstr2Operands(instr, 0,
                                     X86OpRegAWord, 0,
                                     X86OpRegCByte, 0, 0);
              break;
            case tokOpIndIdent:
              GenPrintInstr2Operands(X86InstrMov, 0,
                                     X86OpRegCByte, 0,
                                     X86OpIndLabel, stack[i + 2].param, 0);
              GenPrintInstr2Operands(instr, 0,
                                     X86OpRegAWord, 0,
                                     X86OpRegCByte, 0, 0);
              break;
            case tokOpIndLocalOfs:
              GenPrintInstr2Operands(X86InstrMov, 0,
                                     X86OpRegCByte, 0,
                                     X86OpIndLocal, stack[i + 2].param, 0);
              GenPrintInstr2Operands(instr, 0,
                                     X86OpRegAWord, 0,
                                     X86OpRegCByte, 0, 0);
              break;
            }
            break;

          case '=':
            break;

          default:
            ERROR(0xE018, FALSE, GetTokenName(tok));
            break;
        }
        
        // store ax into the left operand, if needed
        switch (tok) {
          case '=':
          case tokAssignAdd:
          case tokAssignSub:
          case tokAssignMul:
          case tokAssignDiv:
          case tokAssignUDiv:
          case tokAssignMod:
          case tokAssignUMod:
          case tokAssignLSh:
          case tokAssignRSh:
          case tokAssignURSh:
          case tokAssignAnd:
          case tokAssignXor:
          case tokAssignOr:
            switch (stack[i + 1].tok) {
              case tokOpIndIdent:
                GenPrintInstr2Operands(X86InstrMov, 0,
                                       X86OpIndLabel, stack[i + 1].param,
                                       GenSelectByteOrWord(X86OpRegAByteOrWord, v / 16 - 8), 0, 0);
                break;
              case tokOpIndLocalOfs:
                GenPrintInstr2Operands(X86InstrMov, 0,
                                       X86OpIndLocal, stack[i + 1].param,
                                       GenSelectByteOrWord(X86OpRegAByteOrWord, v / 16 - 8), 0, 0);
                break;
              case tokOpIndAcc:
              case tokOpIndStack:
                GenPrintInstr2Operands(X86InstrMov, 0,
                                       X86OpIndRegB, 0,
                                       GenSelectByteOrWord(X86OpRegAByteOrWord, v / 16 - 8), 0, stack[i].flags);
                break;
            }
            // the result of the expression is of type of the
            // left lvalue operand, so, "truncate" it if needed
            GenExtendRegAIfNeeded(v / 16 - 8);
          }
          i += 2;
          break;

        case ')':
          // DONE: "call ident"
          if (stack[i - 1].tok == tokIdent)
            GenPrintInstr1Operand(X86InstrCall, 0, X86OpLabel, stack[i - 1].param, 0); //
          else
            GenPrintInstr1Operand(X86InstrCall, 0, X86OpRegAWord, 0, 0);
          if (v)
            GenLocalAlloc(-v);
          break;

        case '(':
        case tokIf:
        case tokIfNot:
        case tokReturn:
          break;
          
        case tokVoid:
        case tokComma:
          break;
          
        default:
          ERROR(0xE019, FALSE, GetTokenName(tok));
          //break;
    }
  }
}

int GenMaxLocalsSize(void) {
  if (SizeOfWord == 4)
    return 0x7FFFFFFF;
  return 0x7FFF;
}

#define GENSTRDATALEN 64
int GenStrData(char *str, int len, int tok) {
  bool quot = FALSE;
  //int extra = 0;  // how many extra over 'len' did we do
  int llen = 0;
  
  if (!len)
    return 0;
  
  GenStartAsciiString(tok == tokWChar);
  while (len--) {
    llen++;
    // quote ASCII chars for better readability
    if ((*str >= 0x20) && (*str <= 0x7E) && (*str != '\"')) {
      if (!quot) {
        quot = TRUE;
        printf_out("\"");
      }
      printf_out("%c", *str);
      if (llen > GENSTRDATALEN) {
        printf_out("\"\n");
        if (len > 0)
          GenStartAsciiString(tok == tokWChar);
        quot = FALSE;
        llen = 0;
      }
    } else {
      if (quot){
        quot = FALSE;
        printf_out("\",");
      }
      printf_out("%u", *str);
      if (llen > GENSTRDATALEN) {
        printf_out("\n");
        if (len > 0)
          GenStartAsciiString(tok == tokWChar);
        llen = 0;
      } else 
        if (len > 0)
          printf_out(",", len);
    }
    str++;
  }
  
  if (quot)
    printf_out("\"");
  
  if (llen > 0)
    puts_out("");
  
  //return extra;
  return 0;
}

void GenExpr(void) {
  int i;
  
  if ((OutputFormat != FORMATFLAT) && GenExterns) {
    for (i = 0; i < sp; i++)
      if (stack[i].tok == tokIdent && !isdigit(IDENT_STR(stack[i].param)[0]))
        GenAddGlobal(IDENT_STR(stack[i].param), 2);
  }
  GenExpr1();
}

void GenFin(void) {
  if (StructCpyLabel16) {
    if (OutputFormat != FORMATFLAT)
      SwitchSection(SECTION_IS_CODE);
    
    GenNumLabel(StructCpyLabel16);
    
    SizeOfWord = 2;
    
    GenFxnProlog();
    
    puts_out("  mov  di,[bp+8]\n"
             "  mov  si,[bp+6]\n"
             "  mov  cx,[bp+4]\n"
             "  cld    \n"
             "  rep    \n"
             "    movsb\n"
             "  mov  ax,[bp+8]");
    GenFxnEpilog();
    
    if (OutputFormat != FORMATFLAT)
      puts_out(CodeFooter);
  }
  
  if (StructCpyLabel32) {
    if (OutputFormat != FORMATFLAT)
      SwitchSection(SECTION_IS_CODE);
    
    GenNumLabel(StructCpyLabel32);

    SizeOfWord = 4;
    puts_out(".386\n");
    
    GenFxnProlog();
    
    // TODO: SizeOfEIP could have changed before we got here.
    // pass it list we do 'StructCpyLabel32' above
    if (SizeOfEIP == 2) {
      // we can't use MOVSx, since es might not be what we want it to be.
      // However, ds should always be okay
      puts_out("  mov  edi,[ebp+14]\n"
               "  mov  esi,[ebp+10]\n"
               "  mov  ecx,[ebp+6] \n"
               "@@:                \n"
               ".adsize            \n"
               "  lodsb            \n"
               "  mov  [edi],al    \n"
               "  inc  edi         \n"
               "  loop @b          \n"
               "  mov  eax,[ebp+14]");
    } else {
      puts_out("  mov  edi,[ebp+16]\n"
               "  mov  esi,[ebp+12]\n"
               "  mov  ecx,[ebp+8] \n"
               "@@:                \n"
               "  lodsb            \n"
               "  mov  [edi],al    \n"
               "  inc  edi         \n"
               "  loop @b          \n"
               "  mov  eax,[ebp+16]");
    }
    GenFxnEpilog();
    
    if (OutputFormat != FORMATFLAT)
      puts_out(CodeFooter);
  }
  
  if ((OutputFormat != FORMATFLAT) && GenExterns) {
    int i = 0;
    
    puts_out("");
    while (i < GlobalsTableLen) {
      if (GlobalsTable[i] == 2) {
        printf_out("  extern  ");
        GenPrintLabel(GlobalsTable + i + 2, FALSE);
        puts_out("");
      }
      i += GlobalsTable[i + 1] + 2;
    }
  }
  
  if (strlen(NbcLibPath)) {
    if (do_start_code)
      printf_out("\n\ninclude %s\\c0.asm\n\n", NbcLibPath);
    
    
  }
  
  puts_out(FileFooter);
}
