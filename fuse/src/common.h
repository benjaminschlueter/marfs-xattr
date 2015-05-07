#ifndef _MARFS_COMMON_H
#define _MARFS_COMMON_H

// Must come before anything else that might include <time.h>
#include "marfs_base.h"

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#  ifdef __cplusplus
extern "C" {
#  endif


typedef uint8_t  FuseContextFlagType;

typedef enum {
   PUSHED_USER  = 0x01,         // push_security user already ran
} FuseContextFlags;

// install this into fuse_get_context()->private_data?
typedef struct {
   FuseContextFlagType  flags;
   uid_t                user;   // if (flags & PUSHED_USER)
} MarFS_FuseContextInfo;



// Human-readable argument to functions with an <is_interactive> parameter
typedef enum {
   MARFS_BATCH       = 0,
   MARFS_INTERACTIVE = 1
} MarFS_Interactivity;


// // Variation on the MarFS_Interactivity
// // Human-readable argument to functions with an <use_iperms> parameter
// typedef enum {
//    B_PERMS      = 0,
//    I_PERMS      = 1
// } MarFS_PermSelect;



// ---------------------------------------------------------------------------
// logging macros use syslog for root, printf for user.
// See syslog(3) for priority constants.
//
// NOTE: To see output to stderr/stdout, you must start fuse with '-f'
// ---------------------------------------------------------------------------


/// #define LOG_PREFIX  "marfs_fuse -- : "
#define LOG_PREFIX  "marfs_fuse [%s:%3d]%*s -- %-17s: "
#  include <syslog.h>           // we always need priority-names

#ifdef RUNAS_ROOT
// calling syslog() as a regular user on rrz seems to be an expensive no-op
#  define INIT_LOG()                   openlog(LOG_PREFIX, LOG_CONS|LOG_PERROR, LOG_USER)
#  define LOG(PRIO, FMT, ...)          syslog((PRIO), FMT, __FILE__, __LINE__, 13-(int)strlen(__FILE__), "", __FUNCTION__, ## __VA_ARGS__)

#else
// must start fuse with '-f' in order to allow stdout/stderr to work
// NOTE: print_log call merges LOG_PREFIX w/ user format at compile-time
#  define INIT_LOG()
/// #  define LOG(PRIO, FMT, ...)          printf_log((PRIO), LOG_PREFIX FMT, ## __VA_ARGS__)
#  define LOG(PRIO, FMT, ...)          printf_log((PRIO), LOG_PREFIX FMT, __FILE__, __LINE__, 13-(int)strlen(__FILE__), "", __FUNCTION__, ## __VA_ARGS__)
ssize_t   printf_log(size_t prio, const char* format, ...);
#endif




// ---------------------------------------------------------------------------
// TRY, etc
//
// Macro-wrappers around common functions allow the fuse code to be a lot
// cleaner.  The return value from every function-call has to be tested,
// and perhaps return an error-code if things aren't right.  These hide the
// test-and-return.
// ---------------------------------------------------------------------------

// Override this, if you have some fuse-handler that wants to do
// something special before any exit.  (See e.g. marfs_open)
#define RETURN(VALUE)  return(VALUE)



// TRY() macros test for return-values, and skip out early if they get a
//       non-zero.  The "do { ... } while()" just makes sure your macro
//       statements will still work correctly inside single-statment
//       conditions, like this:
//
//       if (...)
//         TRY(...);
//       else
//         TRY(...);
//
// NOTE: rc is lexically-scoped.  It's defined at the top of fuse functions
//       via PUSH_USER().  This allows us to use TRY_GE0() on functions
//       whose return value we care about.
//
// NOTE: TRY macros also invert the sign of the return value, as needed for
//       fuse.  This means they shouldn't be used within common functions,
//       which may in turn be wrapped inside TRY() by fuse routines.
//       [see __TRY0()]


#define TRY0(FUNCTION, ...)                                             \
   do {                                                                 \
      /* LOG(LOG_INFO, "TRY0(%s)\n", #FUNCTION); */                     \
      rc = (size_t)FUNCTION(__VA_ARGS__);                               \
      if (rc) {                                                         \
         LOG(LOG_ERR, "ERR TRY0(%s) returning (%d)\n\n", #FUNCTION, rc); \
         RETURN(-rc); /* negated for FUSE */                            \
      }                                                                 \
   } while (0)


// e.g. open() returns -1 or an fd.
#define TRY_GE0(FUNCTION, ...)                                          \
   do {                                                                 \
      /* LOG(LOG_INFO, "TRY_GE0(%s)\n", #FUNCTION); */                  \
      rc_ssize = (ssize_t)FUNCTION(__VA_ARGS__);                        \
      if (rc_ssize < 0) {                                               \
         LOG(LOG_ERR, "ERR GE0(%s) returning (%d) '%s'\n\n", #FUNCTION, errno, strerror(errno)); \
         RETURN(-errno); /* negated for FUSE */                         \
      }                                                                 \
   } while (0)




// FOR INTERNAL USE ONLY.  (Not for calling directly from fuse routines)
// This version doesn't invert the value of the return-code
#define __TRY0(FUNCTION, ...)                                           \
   do {                                                                 \
      LOG(LOG_INFO, "__TRY0(%s)\n", #FUNCTION);                         \
      rc = (size_t)FUNCTION(__VA_ARGS__);                               \
      if (rc) {                                                         \
         LOG(LOG_ERR, "ERR __TRY0(%s) returning (%d)\n\n", #FUNCTION, rc); \
         RETURN(rc); /* NOT negated! */                                 \
      }                                                                 \
   } while (0)




// add log items when you enter/exit a function
#define ENTRY()                                                         \
   LOG(LOG_INFO, "entry\n");                                            \
   __attribute__ ((unused)) size_t   rc = 0;                            \
   __attribute__ ((unused)) ssize_t  rc_ssize = 0

#define EXIT()                                                        \
   LOG(LOG_INFO, "exit\n\n")



#define PUSH_USER()                                                     \
   ENTRY();                                                             \
   uid_t saved_euid = -1;                                               \
   TRY0(push_user, &saved_euid)

#define POP_USER()                                                      \
   EXIT();                                                              \
   TRY0(pop_user, &saved_euid);                                         \




#define EXPAND_PATH_INFO(INFO, PATH)   TRY0(expand_path_info, (INFO), (PATH))
#define TRASH_FILE(INFO, PATH)         TRY0(trash_file,       (INFO), (PATH))
#define TRASH_DUP_FILE(INFO, PATH)     TRY0(trash_dup_file,   (INFO), (PATH))
#define TRASH_NAME(INFO, PATH)         TRY0(trash_name,       (INFO), (PATH))

#define STAT_XATTRS(INFO)              TRY0(stat_xattrs, (INFO))
#define STAT(INFO)                     TRY0(stat_regular, (INFO))

#define SAVE_XATTRS(INFO, MASK)        TRY0(save_xattrs, (INFO), (MASK))



// return an error, if all the required permission-flags are not asserted
// in the iperms or bperms of the given NS.
#define CHECK_PERMS(ACTUAL_PERMS, REQUIRED_PERMS)                       \
   do {                                                                 \
      LOG(LOG_INFO, "check_perms req:%08x actual:%08x\n", (REQUIRED_PERMS), (ACTUAL_PERMS)); \
      if (((ACTUAL_PERMS) & (REQUIRED_PERMS)) != (REQUIRED_PERMS))      \
         return -EACCES;   /* should be EPERM? (i.e. being root wouldn't help) */ \
   } while (0)

#define ACCESS(PATH, PERMS)            TRY0(access, (PATH), (PERMS))
#define CHECK_QUOTAS(INFO)             TRY0(check_quotas, (INFO))



// ---------------------------------------------------------------------------
// xattrs
// ---------------------------------------------------------------------------


// These describe xattr keys, and the type of the corresponding values, for
// all the metadata fields in a MarFS_ReservedXattr.  These support a
// generic parser for extracting and parsing xattr data from a metadata
// file (or maybe also from object metadata).
//
// As they are found in stat_xattrs(), each flag is OR'ed into a counter,
// so that has_any_xattrs() can tell you whether specific xattrs were
// found.
//
// NOTE: co-maintain XattrMaskType, ALL_MARFS_XATTRS, MARFS_MD_XATTRS


typedef uint8_t XattrMaskType;  // OR'ed XattrValueTypes

typedef enum {
   XVT_NONE       = 0,          // marks the end of <xattr_specs>

   XVT_PRE        = 0x01,
   XVT_POST       = 0x02,
   XVT_RESTART    = 0x04,
   XVT_SLAVE      = 0x08,
   
} XattrValueType;

#define MARFS_MD_XATTRS   (XVT_PRE | XVT_POST)     /* MD-related XattrValueTypes */
#define MARFS_ALL_XATTRS  (XVT_PRE | XVT_POST | XVT_RESTART | XVT_SLAVE)  /* all XattrValueTypes */





// generic description of one of our reserved xattrs
typedef struct {
   XattrValueType  value_type;
   const char*     key_name;        // does not incl MarFS_XattrPrefix (?)
} XattrSpec;


/// typdef struct MarFS_XattrList {
///   char*                   name;
///   char*                   value;
///   struct MarFS_XattrList* next;
/// } MarFS_XattrList;

// An array of XattrSpecs.  Last one has value_type == XVT_NONE.
// initialized in init_xattr_specs()
extern XattrSpec*  MarFS_xattr_specs;





// ---------------------------------------------------------------------------
// PathInfo
//
// used to accumulate FUSE-support information.
// see expand_path_info(), and stat_xattrs()
//
// NOTE: stat_xattrs() sets the flags in PathInfo.xattrs, for each
//       corresponding xattr that is found.  Use has_any_xattrs() or
//       has_all_xattrs(), with a mask, to check for presence of xattrs you
//       care about, after stat_xattr().  The <mask> is just
//       XattrValueTypes, OR'ed together.
//
//       Because this is C, and not C++, I'm not tracking which xattrs you
//       might be changing and updating.  However, when you call
//       save_xattrs(), you can provide another mask (i.e. just like
//       has_any_xattrs()), and we'll install all the xattrs that your mask
//       selects.  (You could use this to write different xattrs at
//       different times, or to update a specific one several times.)
//
// ---------------------------------------------------------------------------


typedef uint8_t  PathInfoFlagType;
typedef enum {
   PI_RESTART      = 0x01,      // file is in restart-mode (see stat_xattrs())
   PI_EXPANDED     = 0x02,      // expand_path_info() was called?
   PI_STAT_QUERY   = 0x04,      // i.e. maybe PathInfo.st empty for a reason
   PI_XATTR_QUERY  = 0x08,      // i.e. maybe PathInfo.xattr empty for a reason
   // PI_XATTRS       = 0x08,      // XATTR_QUERY found any MarFS xattrs
   PI_PRE_INIT     = 0x10,      // "pre"  field has been initialized from scratch (unused?)
   PI_POST_INIT    = 0x20,      // "post" field has been initialized from scratch (unused?)
   PI_TRASH_PATH   = 0x80,      // expand_trash_info() was called?
   //   PI_STATVFS      = 0x80,      // stvfs has been initialized from Namespace.fsinfo?
} PathInfoFlagValue;


typedef struct PathInfo {
   MarFS_Namespace*     ns;
   struct stat          st;
   // struct statvfs       stvfs;  // applies to Namespace.fsinfo

   MarFS_XattrPre       pre;
   MarFS_XattrPost      post;
   MarFS_XattrSlave     slave;
   XattrMaskType        xattrs; // OR'ed XattrValueTypes, use has_any_xattrs()

   PathInfoFlagType     flags;

   char                 md_path[MARFS_MAX_MD_PATH]; // full path to MDFS file
   char                 trash_path[MARFS_MAX_MD_PATH];
} PathInfo;



// ...........................................................................
// FileHandle
//
// Fuse open() dynamically-allocates one of these, and stores it in
// fuse_file_info.fh. The FUSE impl gives us state that may be accessed
// across multiple callbacks.  For example, marfs_open() might save info
// needed by marfs_write().
// ...........................................................................

typedef enum {
   FH_READING    = 0x01,        // might someday allow O_RDWR
   FH_WRITING    = 0x02,        // might someday allow O_RDWR
   FH_DIRECT     = 0x04,        // i.e. PathInfo.xattrs has no MD_
} FHFlags;

typedef uint16_t FHFlagType;


// read() can maintain state here
typedef struct {
   // TBD ...
} ReadStatus;


// write() can maintain state here
typedef struct {
   // TBD ...
} WriteStatus;



typedef struct {
   PathInfo     info;
   int          md_fd;          // opened for reading meta-data, or data
   ReadStatus   read_status;    // buffer_management, current_offset, etc
   WriteStatus  write_status;   // buffer-management, etc
   FHFlagType   flags;
} MarFS_FileHandle;





// // In C++ I'd just steal the templated Pool<T> classes from pftool, to
// // allow us to re-use dynamically-allocate objects.  Here, I'll make a
// // crude stack and functions to alloc/free.
// //
// // Hmmm.  That's going to lead to a locking bottleneck.  Maybe we don't
// // care, in fuse?
// 
// typedef struct Reusable {
//    void*            obj;
//    int              avail;
//    struct Reusable* next;
// } Reusable;
// 
// extern void* alloc_reusable(Reusable** ruse);
// extern void  free_reusable (Reusable** ruse, void* obj);





extern int  push_user();
extern int  pop_user();

// These initialize different parts of the PathInfo struct.
// Calling them redundantly is cheap and harmless.
extern int  expand_path_info (PathInfo* info, const char* path);
extern int  expand_trash_info(PathInfo* info, const char* path);

extern int  stat_regular     (PathInfo* info);
extern int  stat_xattrs      (PathInfo* info);
extern int  save_xattrs      (PathInfo* info, XattrMaskType mask);

extern int  md_exists        (PathInfo* info);

// initialize MarFS_xattr_specs
extern int  init_xattr_specs();

//extern int  has_marfs_xattrs (PathInfo* info);
extern int  has_all_xattrs (PathInfo* info, XattrMaskType mask);
extern int  has_any_xattrs (PathInfo* info, XattrMaskType mask);

extern int  trunc_xattr  (PathInfo* info);

// need the path to initialize info->md_trash_path
extern int  trash_file    (PathInfo* info, const char* path);
extern int  trash_dup_file(PathInfo* info, const char* path);
extern int  trash_name    (PathInfo* info, const char* path);

extern int  check_quotas  (PathInfo* info);





#  ifdef __cplusplus
}
#  endif


#endif  // _MARFS_COMMON_H