#include "version.h"

#ifdef GPC
const char bug_report_url[] = "unsupported";
const char version_string[] = "experimental 20110215";
const char pkgversion_string[] = "pkgversion_string ???";

#else
/* This is the location of the online document giving instructions for
   reporting bugs.  If you distribute a modified version of GCC,
   please configure with --with-bugurl pointing to a document giving
   instructions for reporting bugs to you, not us.  (You are of course
   welcome to forward us bugs reported to you, if you determine that
   they are not bugs in your modifications.)  */

const char bug_report_url[] = BUGURL;

/* The complete version string, assembled from several pieces.
   BASEVER, DATESTAMP, DEVPHASE, and REVISION are defined by the
   Makefile.  */

const char version_string[] = BASEVER DATESTAMP DEVPHASE REVISION;
const char pkgversion_string[] = PKGVERSION;
#endif
