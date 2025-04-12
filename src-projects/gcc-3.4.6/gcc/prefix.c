/* Utility to update paths from internal to external forms.
   Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* This file contains routines to update a path, both to canonicalize
   the directory format and to handle any prefix translation.

   This file must be compiled with -DPREFIX= to specify the "prefix"
   value used by configure.  If a filename does not begin with this
   prefix, it will not be affected other than by directory canonicalization.

   Each caller of 'update_path' may specify both a filename and
   a translation prefix and consist of the name of the package that contains
   the file ("@GCC", "@BINUTIL", "@GNU", etc).

   If the prefix is not specified, the filename will only undergo
   directory canonicalization.

   If it is specified, the string given by PREFIX will be replaced
   by the specified prefix (with a '@' in front unless the prefix begins
   with a '$') and further translation will be done as follows
   until none of the two conditions below are met:

   1) If the filename begins with '@', the string between the '@' and
   the end of the name or the first '/' or directory separator will
   be considered a "key" and looked up as follows:

   -- If this is a Win32 OS, then the Registry will be examined for
      an entry of "key" in

      HKEY_LOCAL_MACHINE\SOFTWARE\Free Software Foundation\<KEY>

      if found, that value will be used. <KEY> defaults to GCC version
      string, but can be overridden at configuration time.

   -- If not found (or not a Win32 OS), the environment variable
      key_ROOT (the value of "key" concatenated with the constant "_ROOT")
      is tried.  If that fails, then PREFIX (see above) is used.

   2) If the filename begins with a '$', the rest of the string up
   to the end or the first '/' or directory separator will be used
   as an environment variable, whose value will be returned.

   Once all this is done, any '/' will be converted to DIR_SEPARATOR,
   if they are different.

   NOTE:  using resolve_keyed_path under Win32 requires linking with
   advapi32.dll.  */


#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#if defined(_WIN32) && defined(ENABLE_WIN32_REGISTRY)
#include <windows.h>
#endif
#include "prefix.h"

static const char *std_prefix = PREFIX;

static const char *get_key_value (char *);
static char *translate_name (char *);
static char *save_string (const char *, int);
static void tr (char *, int, int);

#if defined(_WIN32) && defined(ENABLE_WIN32_REGISTRY)
static char *lookup_key (char *);
static HKEY reg_key = (HKEY) INVALID_HANDLE_VALUE;
#endif

/* Given KEY, as above, return its value.  */

static const char *
get_key_value (char *key)
{
  const char *prefix = 0;
  char *temp = 0;

#if defined(_WIN32) && defined(ENABLE_WIN32_REGISTRY)
  prefix = lookup_key (key);
#endif

  if (prefix == 0)
    prefix = getenv (temp = concat (key, "_ROOT", NULL));

  if (prefix == 0)
    prefix = std_prefix;

  if (temp)
    free (temp);

  return prefix;
}

/* Return a copy of a string that has been placed in the heap.  */

static char *
save_string (const char *s, int len)
{
  char *result = xmalloc (len + 1);

  memcpy (result, s, len);
  result[len] = 0;
  return result;
}

#if defined(_WIN32) && defined(ENABLE_WIN32_REGISTRY)

/* Look up "key" in the registry, as above.  */

static char *
lookup_key (char *key)
{
  char *dst;
  DWORD size;
  DWORD type;
  LONG res;

  if (reg_key == (HKEY) INVALID_HANDLE_VALUE)
    {
      res = RegOpenKeyExA (HKEY_LOCAL_MACHINE, "SOFTWARE", 0,
			   KEY_READ, &reg_key);

      if (res == ERROR_SUCCESS)
	res = RegOpenKeyExA (reg_key, "Free Software Foundation", 0,
			     KEY_READ, &reg_key);

      if (res == ERROR_SUCCESS)
	res = RegOpenKeyExA (reg_key, WIN32_REGISTRY_KEY, 0,
			     KEY_READ, &reg_key);

      if (res != ERROR_SUCCESS)
	{
	  reg_key = (HKEY) INVALID_HANDLE_VALUE;
	  return 0;
	}
    }

  size = 32;
  dst = xmalloc (size);

  res = RegQueryValueExA (reg_key, key, 0, &type, dst, &size);
  if (res == ERROR_MORE_DATA && type == REG_SZ)
    {
      dst = xrealloc (dst, size);
      res = RegQueryValueExA (reg_key, key, 0, &type, dst, &size);
    }

  if (type != REG_SZ || res != ERROR_SUCCESS)
    {
      free (dst);
      dst = 0;
    }

  return dst;
}
#endif

/* If NAME, a malloc-ed string, starts with a '@' or '$', apply the
   translation rules above and return a newly malloc-ed name.
   Otherwise, return the given name.  */

static char *
translate_name (char *name)
{
  char code;
  char *key, *old_name;
  const char *prefix;
  int keylen;

  for (;;)
    {
      code = name[0];
      if (code != '@' && code != '$')
	break;

      for (keylen = 0;
	   (name[keylen + 1] != 0 && !IS_DIR_SEPARATOR (name[keylen + 1]));
	   keylen++)
	;

      key = alloca (keylen + 1);
      strncpy (key, &name[1], keylen);
      key[keylen] = 0;

      if (code == '@')
	{
	  prefix = get_key_value (key);
	  if (prefix == 0)
	    prefix = std_prefix;
	}
      else
	prefix = getenv (key);

      if (prefix == 0)
	prefix = PREFIX;

      /* We used to strip trailing DIR_SEPARATORs here, but that can
	 sometimes yield a result with no separator when one was coded
	 and intended by the user, causing two path components to run
	 together.  */

      old_name = name;
      name = concat (prefix, &name[keylen + 1], NULL);
      free (old_name);
    }

  return name;
}

/* In a NUL-terminated STRING, replace character C1 with C2 in-place.  */
static void
tr (char *string, int c1, int c2)
{
  do
    {
      if (*string == c1)
	*string = c2;
    }
  while (*string++);
}

/* Strip dir/.. from a pathname when it makes sense, e.g., when this
   would turn an inaccessible pathname into an accessible one.

   We short-circuit dir/.. when dir does not exist, and when
   some/dir/../thing does not exist but some/thing does.  In case
   there are multiple possible dir/../ stripping possibilities that
   would turn an inaccessible pathname into an accessible one, the one
   closer to the end of the pathname is preferred.

   RESULT is the pathname that might contain such dotdot sequences to
   be stripped.  P points into RESULT, and indicates the location
   where we should start looking for ../ sequences.

   Even though RESULT is const, P is not, and that's because
   characters in it may be temporarily overwritten, so RESULT must not
   be in read-only storage.

   The returned value is either a newly-allocated memory area, holding
   a string that is the result of dotdot-stripping from the original
   input strip, or RESULT itself, in which case any modifications made
   to the string will have been undone.  */

static const char *
maybe_strip_dotdots (const char *result, char *p)
{
  char *temp;
  const char *path, *before, *after;
  size_t len;

  while (1)
    {
      p = strchr (p, '.');
      if (p == NULL)
	return result;
      /* Look for `/../'  */
      if (p[1] == '.'
	  && IS_DIR_SEPARATOR (p[2])
	  && (p != result && IS_DIR_SEPARATOR (p[-1])))
	break;
      else
	++p;
    }

  *p = 0;
  if (access (result, X_OK) == 0)
    {
      *p = '.';

      path = maybe_strip_dotdots (result, p + 3);
      if (access (path, F_OK) == 0)
	return path;
      if (path != result)
	free ((char *) path);
    }
  else
    *p = '.';

  /* If we couldn't access the dir, or if recursion resulted in a
     non-accessible pathname, we try stripping out dir/../.  If `dir'
     turns out to be `.', strip one more path component.  */
  before = p;
  do
    {
      --before;
      while (before != result && IS_DIR_SEPARATOR (*before))
	--before;
      while (before != result && !IS_DIR_SEPARATOR (before[-1]))
	--before;
    }
  while (before != result && *before == '.'
	 && IS_DIR_SEPARATOR (*(before + 1)));
  /* If we have something like `./..' or `/..', don't
     strip anything more.  */
  if (*before == '.' || IS_DIR_SEPARATOR (*before))
    return result;

  after = p + 3;
  while (IS_DIR_SEPARATOR (*after))
    ++after;

  len = (after - result) + strlen (after);

  temp = xmalloc (len + 1 - (after - before));
  memcpy (temp, result, before - result);
  memcpy (temp + (before - result), after, len + 1 - (after - result));

  path = maybe_strip_dotdots (temp, temp + (before - result));

  if (path != temp)
    free (temp);

  if (access (path, F_OK) == 0)
    result = path;
  else if (path != result)
    free ((char *) path);

  return result;
}

/* Update PATH using KEY if PATH starts with PREFIX.  The returned
   string is always malloc-ed, and the caller is responsible for
   freeing it.  */

char *
update_path (const char *path, const char *key)
{
  char *result, *temp;

  if (! strncmp (path, std_prefix, strlen (std_prefix)) && key != 0)
    {
      bool free_key = false;

      if (key[0] != '$')
	{
	  key = concat ("@", key, NULL);
	  free_key = true;
	}

      result = concat (key, &path[strlen (std_prefix)], NULL);
      if (free_key)
	free ((char *) key);
      result = translate_name (result);
    }
  else
    result = xstrdup (path);

  temp = result;
  result = (char *) maybe_strip_dotdots (temp, temp);

  if (result != temp)
    free (temp);

#ifdef UPDATE_PATH_HOST_CANONICALIZE
  /* Perform host dependent canonicalization when needed.  */
  UPDATE_PATH_HOST_CANONICALIZE (result);
#endif

#ifdef DIR_SEPARATOR_2
  /* Convert DIR_SEPARATOR_2 to DIR_SEPARATOR.  */
  if (DIR_SEPARATOR_2 != DIR_SEPARATOR)
    tr (result, DIR_SEPARATOR_2, DIR_SEPARATOR);
#endif

#if defined (DIR_SEPARATOR) && !defined (DIR_SEPARATOR_2)
  if (DIR_SEPARATOR != '/')
    tr (result, '/', DIR_SEPARATOR);
#endif

  return result;
}

/* Reset the standard prefix.  */
void
set_std_prefix (const char *prefix, int len)
{
  std_prefix = save_string (prefix, len);
}
