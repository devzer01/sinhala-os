
This is NBASM v00.26.70, an assembler for DOS

Forever Young Software
Benjamin David Lunt
Copyright 1984-2016
25 Aug 2015

This is a release of the NewBasic package is distributed as freeware.

Please see NBASM.DOC for all copyrights, bug reports, etc. including
how to use this assembler.

Please see (free) registration below.

Get the latest version at:
  http://www.fysnet.net/newbasic.htm

Contents:
  ******* Files included
  ******* Installation
  ******* Uninstall
  ******* Disclaimer
  ******* Distribution
  ******* System Requirements
  ******* Known Issues
  ******* Contact
  ******* Mailing/Announcement List
  ******* Registeration (free)
 


******* Files included in this package:
    readme.txt     This file
    readme.com     A demo source code with a little text animation
    readme.asm     The source code for above
    whatsnew.txt   Specifies the changes in each version.  This
                     file should be viewed after each release.
    nbasm16.exe    The executable assembler file.  This is actually
                     the only file you need unless you call external
                     libraries in the 'tiny' model.
    nbasm16.doc    The Documentation for the NBASM assembler.  This
                     file has all the information needed to use
                     NBASM, including all the items supported and
                     not supported.
    nbasm.exe      The 32-bit executable assembler file.  Same as NBASM
                     except uses 32-bit processor and many new features.
                     Compiled with DJGPP for DOS and Windows Consoles.
    nbasm.doc      The Documentation for the NBASM32 assembler.  This
                     file has all the information needed to use
                     NBASM32, including all the items supported and
                     not supported.
    nbasmw.exe     Same as NBASM.EXE above, except a Windows .EXE file.
                     This is the assembler required by NBASMIDE.
    nbdisasm.exe   The executable disassembler file.
    nbdisasm.doc   The Documentation for the NBDISASM disassembler.
    nbdisasmw.exe  Same as NBDISASM.EXE above, except a Windows .EXE file.
                     This is the disassembler required by NBASMIDE.
    nbasm.faq      The (small) FAQ list for NBASM.
    nbasmc.lib     The library file for the 'tiny' model.
                     This is the library for use with the 'tiny' model.
                     The NBASM.DOC file has more information on how
                     to use this library.
    nbasmc.doc     A small documentation file on the library.  Not much
                     information except the discriptions of each of the
                     few routines included.
    nbasmc.inc     The include file for the 'tiny' model library.
                     This file is not neccessary, however, it makes
                     it easier to use the library.
    nbasme.inc     A secondary include file that will soon hold most
                     used equates.  You may modify nbasme.inc as you wish.
    nbasm.ico      An .ICO used for NBASM in Windoze
                     (also used with demo2.asm below)
    nbc.doc        A small documentation file on the NBC Compiler
    nbc.exe        The NBC C Compiler.  It is in alpha stage, so use at
                     your own risk.
    demo1.asm      A demo assembly file assembled with NBASM32.
                    (please read the comment at the top of DEMO1.ASM
                     for a change you must make using NBASM32)
    demo2.asm      A demo assembly file assembled with NBASM.
    demo3.asm      A demo assembly file assembled with NBASM for the
                     new .OBJ output format.
    nbasmtut.doc   A small tutorial on how to use NBASM and assembler.
                     Creates a small working file viewer.
    EXAMPLES\
      many examples on how to use NBASM's syntax.
      also look at all.asm for all the instructions and there syntax.
    NBC_SOURCE\
      A folder holding the source code to NBC.


******* Installation
    NBASM is simple to install.  Simply unzip the NBASMxxx.zip file
      in a directory of your choice.  If you are installing to an
      older version of NBASM, you can simply overwrite the older files
      with these newer files.  However, to be safe, you should unzip
      the newer version to a new directory, make sure the newer version
      works as expected, then delete the older directory.

******* Uninstall
    To uninstall NBASM, simply delete all files that were extracted
      from the zip file that you received.  Most likely you created
      a new directory (C:\NBASM), and placed all files and utilities
      in that directory or a directory in it, so all you have to do
      is remove the C:\NBASM directory.
    NBASM or any of the included programs do not install any items
      in the Windoze registry or the AUTOEXEC.BAT file.

******* DISCLAIMER
     DISCLAIMER:  NBASM is distributed as is...
       If NBASM destroys your computer, kills you or your family,
       or any other thinkable and unthinkable thing, I take no
       responsibility.  USE AT YOUR OWN RISK...


******* Distribution
     This software package is distributed as freeware.  No fee except
     for small copying fees can be charged.  There is no fee for any
     executable file produced with this package as long as this
     executable file is for non-commercial use.  If you use this
     package for commercial use, a small users fee is required.

     However, as an incentive to continue the whole NewBasic package,
     a small donation in US currency would be very helpful.

******* System Requirements
       The system requirements to use NBASM/NBASM32 is as follows:
        - a 186 or higher Intel based system (NBASM)
        - a 386 or higher Intel based system (NBASM32)
        - MS DOS 2.0 or higher. (NBASM)
        - MS DOS 5.0 or higher. (NBASM32) (not for sure, but to be safe)
        - a CGA or better monitor ( Monochrome should be fine too. :) )
        - a disk drive with a minimum of 512k of free disk space
            plus room for the .COM and .LST file(s).
        - Memory: a minimum of 200k: (NBASM)
        - Memory: a minimum of 1meg: (NBASM32)
        - a Keyboard   ...Duuhh! =:o)...
        - a printer capable of text printing (for hard-copies right?)

     SOFTWARE REQUIREMENTS:
       - as stated above, DOS 2.0 or higher (NBASM)
           (NBASM works in a Windoze (95) DOS session just fine)
       - as stated above, DOS 5.0 or higher and a DPMI (NBASM32)
           (NBASM32 works in a Windoze (95) DOS session just fine)
       - some sort of viewer to view this document.  (But you are
          reading this right now, so you got at least that much, right?)
       - a similar editor to create DOS ascii text files.  (EDIT.COM|EXE)
       - PKUNZIP/WINZIP to unzip the zip file.  Again, you are reading
          this file, so you must have one of these or a similar tool :0)

******* Known Issues
     KNOWN ISSUES with the current version of NBASM32:
       - the MMX registers and other similar instructions are not included
       - some of the FPU instructions are not included
       - the ST(i) registers are not working correctly
       - need to add the third pass to allow /j and .jumps to work
            (also will allow a better optimizer)
       - other issues may be present with NBASM32.
          Please take this into consideration.
       - do you know of any?  Please let me know if you do.....


******* Contact
     BUG REPORT:
       There are bound to be bugs in NBASM.  Please report bugs or any
        other comments/problems to:

        fys@fysnet.net
        http://www.fysnet.net/newbasic.htm

******* Mailing/Announcement List
     MAILING LIST:
       I now have an announcement mailing list which I will use
        to announce news and updates about NBASM and other items
        pertaining to NBASM.  It is a moderated list and will only
        be used for NBASM announcements.

       If you would like to receive mail each time I make an update
        or announce something about NBASM, please send an empty email
        message to:
          nbasm-subscribe@yahoogroups.com

       To unsubscribe from the list, please send an empty email
        message to:
          nbasm-unsubscribe@yahoogroups.com

       You can also view the archived messages at:
          http://groups.yahoo.com/group/nbasm

       If you have any problems with the above mailing list, please
        let me know at:
          fys@fysnet.net
        (Please include your Yahoogroups login in name)


******* Please Register
    There is no fee for the use of this assembler.  This is a free
      assembler.  However, I would like to know how many people are
      using my assembler.  No forms to fill out, no questionairs.
      Just please send a small comment to:
         fys@fysnet.net
      saying that you use/like/dislike/hate/etc. NBASM.

     I have started an announcement mailing list.  If you would
      like to be on this list, please see the section above.  It will
      only be used for announcement of releases and major news about
      NBASM and/or the NewBasic Project.

      Thank you
