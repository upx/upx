The GNU bfd binary file descriptor package, part of the GNU binutils package,
contains code to recognize an Elf file by looking at fields in the file header.
Many programs (such as gdb, size, objdump, objcopy, strip and others) use bfd.
Unfortunately, bfd has been overly strict.  In releases binutils-2.9.1.0.4,
binutils-2.9.1.0.23, binutils-2.9.5.0.0, gdb-4.17, and probably others,
bfd does not recognize a file with zero in Elf32_Ehdr.e_shoff, .e_shentsize,
or .e_shnum, even though the operating system kernel does not care, and many
bfd clients would work just fine in these cases.

Because of its desire to create small size compressed executables, UPX
generates and uses Elf files with zero in .e_shoff, .e_shentsizes, and .e_shnum.
As a result, many widely-distributed instances of gdb, size, objdump, etc.,
refuse to work on UPX-packed executables.

Here is a patch to bfd/elfcode.h which enables many bfd clients to work
with UPX-packed executables.  The last two lines of the patch are
       if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) == -1)
 	return -1;


--- elfcode.h.orig	Mon Jun 16 11:45:32 1997
+++ elfcode.h	Sat Apr 22 10:18:56 2000
@@ -1,5 +1,5 @@
 /* ELF executable support for BFD.
-   Copyright 1991, 92, 93, 94, 95, 96, 1997 Free Software Foundation, Inc.
+   Copyright 1991, 92, 93, 94, 95, 96, 97, 1998 Free Software Foundation, Inc.
 
    Written by Fred Fish @ Cygnus Support, from information published
    in "UNIX System V Release 4, Programmers Guide: ANSI C and
@@ -517,14 +517,14 @@
   elf_debug_file (i_ehdrp);
 #endif
 
-  /* If there is no section header table, we're hosed. */
-  if (i_ehdrp->e_shoff == 0)
+  /* If there is no section header table, and relocatable, then we're hosed. */
+  if (i_ehdrp->e_shoff == 0 && i_ehdrp->e_type == ET_REL)
     goto got_wrong_format_error;
 
   /* As a simple sanity check, verify that the what BFD thinks is the
      size of each section header table entry actually matches the size
-     recorded in the file. */
-  if (i_ehdrp->e_shentsize != sizeof (x_shdr))
+     recorded in the file, but only if there are any sections. */
+  if (i_ehdrp->e_shentsize != sizeof (x_shdr) &&  i_ehdrp->e_shnum != 0)
     goto got_wrong_format_error;
 
   ebd = get_elf_backend_data (abfd);
@@ -579,6 +579,7 @@
   /* Allocate space for a copy of the section header table in
      internal form, seek to the section header table in the file,
      read it in, and convert it to internal form.  */
+ if (i_ehdrp->e_shnum!=0) {
   i_shdrp = ((Elf_Internal_Shdr *)
 	     bfd_alloc (abfd, sizeof (*i_shdrp) * i_ehdrp->e_shnum));
   elf_elfsections (abfd) = ((Elf_Internal_Shdr **)
@@ -609,6 +610,7 @@
       if (! bfd_section_from_shdr (abfd, i_ehdrp->e_shstrndx))
 	goto got_no_match;
     }
+ }
 
   /* Read in the program headers.  */
   if (i_ehdrp->e_phnum == 0)
@@ -644,6 +646,7 @@
      bfd_section_from_shdr with it (since this particular strtab is
      used to find all of the ELF section names.) */
 
+ if (i_ehdrp->e_shstrndx != 0) {
   shstrtab = bfd_elf_get_str_section (abfd, i_ehdrp->e_shstrndx);
   if (!shstrtab)
     goto got_no_match;
@@ -657,6 +660,7 @@
       if (! bfd_section_from_shdr (abfd, shindex))
 	goto got_no_match;
     }
+ }
 
   /* Let the backend double check the format and override global
      information.  */
@@ -930,7 +934,7 @@
 {
   Elf_Internal_Shdr *hdr;
   Elf_Internal_Shdr *verhdr;
-  long symcount;		/* Number of external ELF symbols */
+  unsigned long symcount;	/* Number of external ELF symbols */
   elf_symbol_type *sym;		/* Pointer to current bfd symbol */
   elf_symbol_type *symbase;	/* Buffer for generated bfd symbols */
   Elf_Internal_Sym i_sym;
@@ -978,7 +982,7 @@
     sym = symbase = NULL;
   else
     {
-      long i;
+      unsigned long i;
 
       if (bfd_seek (abfd, hdr->sh_offset, SEEK_SET) == -1)
 	return -1;
