/* Plugin support for BFD.
   Copyright (C) 2009-2014 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"

#if BFD_SUPPORTS_PLUGINS

#include <assert.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#elif defined (HAVE_WINDOWS_H)
#include <windows.h>
#else
#error Unknown how to handle dynamic-load-libraries.
#endif
#include <stdarg.h>
#include "plugin-api.h"
#include "plugin.h"
#include "libbfd.h"
#include "libiberty.h"
#include <dirent.h>

#if !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)

#define RTLD_NOW 0      /* Dummy value.  */

static void *
dlopen (const char *file, int mode ATTRIBUTE_UNUSED)
{
  return LoadLibrary (file);
}

static void *
dlsym (void *handle, const char *name)
{
  return GetProcAddress (handle, name);
}

static int ATTRIBUTE_UNUSED
dlclose (void *handle)
{
  FreeLibrary (handle);
  return 0;
}

static const char *
dlerror (void)
{
  return "Unable to load DLL.";
}

#endif /* !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)  */

#define bfd_plugin_close_and_cleanup                  _bfd_generic_close_and_cleanup
#define bfd_plugin_bfd_free_cached_info               _bfd_generic_bfd_free_cached_info
#define bfd_plugin_new_section_hook                   _bfd_generic_new_section_hook
#define bfd_plugin_get_section_contents               _bfd_generic_get_section_contents
#define bfd_plugin_get_section_contents_in_window     _bfd_generic_get_section_contents_in_window
#define bfd_plugin_bfd_copy_private_header_data       _bfd_generic_bfd_copy_private_header_data
#define bfd_plugin_bfd_merge_private_bfd_data         _bfd_generic_bfd_merge_private_bfd_data
#define bfd_plugin_bfd_copy_private_header_data       _bfd_generic_bfd_copy_private_header_data
#define bfd_plugin_bfd_set_private_flags              _bfd_generic_bfd_set_private_flags
#define bfd_plugin_core_file_matches_executable_p     generic_core_file_matches_executable_p
#define bfd_plugin_bfd_is_local_label_name            _bfd_nosymbols_bfd_is_local_label_name
#define bfd_plugin_bfd_is_target_special_symbol       ((bfd_boolean (*) (bfd *, asymbol *)) bfd_false)
#define bfd_plugin_get_lineno                         _bfd_nosymbols_get_lineno
#define bfd_plugin_find_nearest_line                  _bfd_nosymbols_find_nearest_line
#define bfd_plugin_find_line                          _bfd_nosymbols_find_line
#define bfd_plugin_find_inliner_info                  _bfd_nosymbols_find_inliner_info
#define bfd_plugin_bfd_make_debug_symbol              _bfd_nosymbols_bfd_make_debug_symbol
#define bfd_plugin_read_minisymbols                   _bfd_generic_read_minisymbols
#define bfd_plugin_minisymbol_to_symbol               _bfd_generic_minisymbol_to_symbol
#define bfd_plugin_set_arch_mach                      bfd_default_set_arch_mach
#define bfd_plugin_set_section_contents               _bfd_generic_set_section_contents
#define bfd_plugin_bfd_get_relocated_section_contents bfd_generic_get_relocated_section_contents
#define bfd_plugin_bfd_relax_section                  bfd_generic_relax_section
#define bfd_plugin_bfd_link_hash_table_create         _bfd_generic_link_hash_table_create
#define bfd_plugin_bfd_link_add_symbols               _bfd_generic_link_add_symbols
#define bfd_plugin_bfd_link_just_syms                 _bfd_generic_link_just_syms
#define bfd_plugin_bfd_final_link                     _bfd_generic_final_link
#define bfd_plugin_bfd_link_split_section             _bfd_generic_link_split_section
#define bfd_plugin_bfd_gc_sections                    bfd_generic_gc_sections
#define bfd_plugin_bfd_lookup_section_flags           bfd_generic_lookup_section_flags
#define bfd_plugin_bfd_merge_sections                 bfd_generic_merge_sections
#define bfd_plugin_bfd_is_group_section               bfd_generic_is_group_section
#define bfd_plugin_bfd_discard_group                  bfd_generic_discard_group
#define bfd_plugin_section_already_linked             _bfd_generic_section_already_linked
#define bfd_plugin_bfd_define_common_symbol           bfd_generic_define_common_symbol
#define bfd_plugin_bfd_copy_link_hash_symbol_type     _bfd_generic_copy_link_hash_symbol_type

static enum ld_plugin_status
message (int level ATTRIBUTE_UNUSED,
	 const char * format, ...)
{
  va_list args;
  va_start (args, format);
  printf ("bfd plugin: ");
  vprintf (format, args);
  putchar ('\n');
  va_end (args);
  return LDPS_OK;
}

/* Register a claim-file handler. */
static ld_plugin_claim_file_handler claim_file;

static enum ld_plugin_status
register_claim_file (ld_plugin_claim_file_handler handler)
{
  claim_file = handler;
  return LDPS_OK;
}

static enum ld_plugin_status
add_symbols (void * handle,
	     int nsyms,
	     const struct ld_plugin_symbol * syms)
{
  bfd *abfd = handle;
  struct plugin_data_struct *plugin_data =
    bfd_alloc (abfd, sizeof (plugin_data_struct));

  plugin_data->nsyms = nsyms;
  plugin_data->syms = syms;

  if (nsyms != 0)
    abfd->flags |= HAS_SYMS;

  abfd->tdata.plugin_data = plugin_data;
  return LDPS_OK;
}

static const char *plugin_program_name;

void
bfd_plugin_set_program_name (const char *program_name)
{
  plugin_program_name = program_name;
}

static int
try_claim (bfd *abfd)
{
  int claimed = 0;
  struct ld_plugin_input_file file;
  bfd *iobfd;

  file.name = abfd->filename;

  if (abfd->my_archive)
    {
      iobfd = abfd->my_archive;
      file.offset = abfd->origin;
      file.filesize = arelt_size (abfd);
    }
  else
    {
      iobfd = abfd;
      file.offset = 0;
      file.filesize = 0;
    }

  if (!iobfd->iostream && !bfd_open_file (iobfd))
    return 0;

  file.fd = fileno ((FILE *) iobfd->iostream);

  if (!abfd->my_archive)
    {
      struct stat stat_buf;
      if (fstat (file.fd, &stat_buf))
        return 0;
      file.filesize = stat_buf.st_size;
    }

  file.handle = abfd;
  off_t cur_offset = lseek(file.fd, 0, SEEK_CUR);
  claim_file (&file, &claimed);
  lseek(file.fd, cur_offset, SEEK_SET);
  if (!claimed)
    return 0;

  return 1;
}

static int
try_load_plugin (const char *pname, bfd *abfd)
{
  void *plugin_handle;
  int tv_size = 4;
  struct ld_plugin_tv tv[tv_size];
  int i;
  ld_plugin_onload onload;
  enum ld_plugin_status status;

  plugin_handle = dlopen (pname, RTLD_NOW);
  if (!plugin_handle)
    {
      (*_bfd_error_handler)("%s\n", dlerror ());
      return 0;
    }

  onload = dlsym (plugin_handle, "onload");
  if (!onload)
    goto err;

  i = 0;
  tv[i].tv_tag = LDPT_MESSAGE;
  tv[i].tv_u.tv_message = message;

  ++i;
  tv[i].tv_tag = LDPT_REGISTER_CLAIM_FILE_HOOK;
  tv[i].tv_u.tv_register_claim_file = register_claim_file;

  ++i;
  tv[i].tv_tag = LDPT_ADD_SYMBOLS;
  tv[i].tv_u.tv_add_symbols = add_symbols;

  ++i;
  tv[i].tv_tag = LDPT_NULL;
  tv[i].tv_u.tv_val = 0;

  status = (*onload)(tv);

  if (status != LDPS_OK)
    goto err;

  if (!claim_file)
    goto err;

  if (!try_claim (abfd))
    goto err;

  return 1;

 err:
  plugin_handle = NULL;
  return 0;
}

static const char *plugin_name;

void
bfd_plugin_set_plugin (const char *p)
{
  plugin_name = p;
}

static int
load_plugin (bfd *abfd)
{
  char *plugin_dir;
  char *p;
  DIR *d;
  struct dirent *ent;
  int found = 0;

  if (plugin_name)
    return try_load_plugin (plugin_name, abfd);

  if (plugin_program_name == NULL)
    return 0;

  plugin_dir = concat (BINDIR, "/../lib/bfd-plugins", NULL);
  p = make_relative_prefix (plugin_program_name,
			    BINDIR,
			    plugin_dir);
  free (plugin_dir);
  plugin_dir = NULL;

  d = opendir (p);
  if (!d)
    goto out;

  while ((ent = readdir (d)))
    {
      char *full_name;
      struct stat s;

      full_name = concat (p, "/", ent->d_name, NULL);
      if (stat(full_name, &s) == 0 && S_ISREG (s.st_mode))
	found = try_load_plugin (full_name, abfd);
      free (full_name);
      if (found)
	break;
    }

 out:
  free (p);
  if (d)
    closedir (d);

  return found;
}


static const bfd_target *
bfd_plugin_object_p (bfd *abfd)
{
  if (!load_plugin (abfd))
    return NULL;

  return abfd->xvec;
}

/* Copy any private info we understand from the input bfd
   to the output bfd.  */

static bfd_boolean
bfd_plugin_bfd_copy_private_bfd_data (bfd *ibfd ATTRIBUTE_UNUSED,
				      bfd *obfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return TRUE;
}

/* Copy any private info we understand from the input section
   to the output section.  */

static bfd_boolean
bfd_plugin_bfd_copy_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
					  asection *isection ATTRIBUTE_UNUSED,
					  bfd *obfd ATTRIBUTE_UNUSED,
					  asection *osection ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return TRUE;
}

/* Copy any private info we understand from the input symbol
   to the output symbol.  */

static bfd_boolean
bfd_plugin_bfd_copy_private_symbol_data (bfd *ibfd ATTRIBUTE_UNUSED,
					 asymbol *isymbol ATTRIBUTE_UNUSED,
					 bfd *obfd ATTRIBUTE_UNUSED,
					 asymbol *osymbol ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return TRUE;
}

static bfd_boolean
bfd_plugin_bfd_print_private_bfd_data (bfd *abfd ATTRIBUTE_UNUSED, PTR ptr ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return TRUE;
}

static char *
bfd_plugin_core_file_failing_command (bfd *abfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return NULL;
}

static int
bfd_plugin_core_file_failing_signal (bfd *abfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return 0;
}

static int
bfd_plugin_core_file_pid (bfd *abfd ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return 0;
}

static long
bfd_plugin_get_symtab_upper_bound (bfd *abfd)
{
  struct plugin_data_struct *plugin_data = abfd->tdata.plugin_data;
  long nsyms = plugin_data->nsyms;

  BFD_ASSERT (nsyms >= 0);

  return ((nsyms + 1) * sizeof (asymbol *));
}

static flagword
convert_flags (const struct ld_plugin_symbol *sym)
{
 switch (sym->def)
   {
   case LDPK_DEF:
   case LDPK_COMMON:
   case LDPK_UNDEF:
     return BSF_GLOBAL;

   case LDPK_WEAKUNDEF:
   case LDPK_WEAKDEF:
     return BSF_GLOBAL | BSF_WEAK;

   default:
     BFD_ASSERT (0);
     return 0;
   }
}

static long
bfd_plugin_canonicalize_symtab (bfd *abfd,
				asymbol **alocation)
{
  struct plugin_data_struct *plugin_data = abfd->tdata.plugin_data;
  long nsyms = plugin_data->nsyms;
  const struct ld_plugin_symbol *syms = plugin_data->syms;
  static asection fake_section;
  static asection fake_common_section;
  int i;

  fake_section.name = ".text";
  fake_common_section.flags = SEC_IS_COMMON;

  for (i = 0; i < nsyms; i++)
    {
      asymbol *s = bfd_alloc (abfd, sizeof (asymbol));

      BFD_ASSERT (s);
      alocation[i] = s;

      s->the_bfd = abfd;
      s->name = syms[i].name;
      s->value = 0;
      s->flags = convert_flags (&syms[i]);
      switch (syms[i].def)
	{
	case LDPK_COMMON:
	  s->section = &fake_common_section;
	  break;
	case LDPK_UNDEF:
	case LDPK_WEAKUNDEF:
	  s->section = bfd_und_section_ptr;
	  break;
	case LDPK_DEF:
	case LDPK_WEAKDEF:
	  s->section = &fake_section;
	  break;
	default:
	  BFD_ASSERT (0);
	}

      s->udata.p = (void *) &syms[i];
    }

  return nsyms;
}

static void
bfd_plugin_print_symbol (bfd *abfd ATTRIBUTE_UNUSED,
			 PTR afile ATTRIBUTE_UNUSED,
			 asymbol *symbol ATTRIBUTE_UNUSED,
			 bfd_print_symbol_type how ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
}

static void
bfd_plugin_get_symbol_info (bfd *abfd ATTRIBUTE_UNUSED,
			    asymbol *symbol,
			    symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);
}

/* Make an empty symbol. */

static asymbol *
bfd_plugin_make_empty_symbol (bfd *abfd)
{
  asymbol *new_symbol = bfd_zalloc (abfd, sizeof (asymbol));
  if (new_symbol == NULL)
    return new_symbol;
  new_symbol->the_bfd = abfd;
  return new_symbol;
}

static int
bfd_plugin_sizeof_headers (bfd *a ATTRIBUTE_UNUSED,
			   struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (0);
  return 0;
}

const bfd_target plugin_vec =
{
  "plugin",			/* Name.  */
  bfd_target_unknown_flavour,
  BFD_ENDIAN_LITTLE,		/* Target byte order.  */
  BFD_ENDIAN_LITTLE,		/* Target headers byte order.  */
  (HAS_RELOC | EXEC_P |		/* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
  (SEC_CODE | SEC_DATA | SEC_ROM | SEC_HAS_CONTENTS
   | SEC_ALLOC | SEC_LOAD | SEC_RELOC),	/* Section flags.  */
  0,				/* symbol_leading_char.  */
  '/',				/* ar_pad_char.  */
  15,				/* ar_max_namelen.  */
  0,				/* match priority.  */

  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* data */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* hdrs */

  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    bfd_plugin_object_p,
    bfd_generic_archive_p,
    _bfd_dummy_target
  },
  {				/* bfd_set_format.  */
    bfd_false,
    bfd_false,
    _bfd_generic_mkarchive,
    bfd_false,
  },
  {				/* bfd_write_contents.  */
    bfd_false,
    bfd_false,
    _bfd_write_archive_contents,
    bfd_false,
  },

  BFD_JUMP_TABLE_GENERIC (bfd_plugin),
  BFD_JUMP_TABLE_COPY (bfd_plugin),
  BFD_JUMP_TABLE_CORE (bfd_plugin),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),
  BFD_JUMP_TABLE_SYMBOLS (bfd_plugin),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (bfd_plugin),
  BFD_JUMP_TABLE_LINK (bfd_plugin),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL  			/* backend_data.  */
};
#endif /* BFD_SUPPORTS_PLUGIN */
