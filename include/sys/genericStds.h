/*-----------------------------------------------------------------------------
Software License for The Fraunhofer FDK MPEG-H Software

Copyright (c) 2018 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. and Contributors
All rights reserved.

1. INTRODUCTION

The "Fraunhofer FDK MPEG-H Software" is software that implements the ISO/MPEG
MPEG-H 3D Audio standard for digital audio or related system features. Patent
licenses for necessary patent claims for the Fraunhofer FDK MPEG-H Software
(including those of Fraunhofer), for the use in commercial products and
services, may be obtained from the respective patent owners individually and/or
from Via LA (www.via-la.com).

Fraunhofer supports the development of MPEG-H products and services by offering
additional software, documentation, and technical advice. In addition, it
operates the MPEG-H Trademark Program to ease interoperability testing of end-
products. Please visit www.mpegh.com for more information.

2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification,
are permitted without payment of copyright license fees provided that you
satisfy the following conditions:

* You must retain the complete text of this software license in redistributions
of the Fraunhofer FDK MPEG-H Software or your modifications thereto in source
code form.

* You must retain the complete text of this software license in the
documentation and/or other materials provided with redistributions of
the Fraunhofer FDK MPEG-H Software or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of
the Fraunhofer FDK MPEG-H Software and your modifications thereto to recipients
of copies in binary form.

* The name of Fraunhofer may not be used to endorse or promote products derived
from the Fraunhofer FDK MPEG-H Software without prior written permission.

* You may not charge copyright license fees for anyone to use, copy or
distribute the Fraunhofer FDK MPEG-H Software or your modifications thereto.

* Your modified versions of the Fraunhofer FDK MPEG-H Software must carry
prominent notices stating that you changed the software and the date of any
change. For modified versions of the Fraunhofer FDK MPEG-H Software, the term
"Fraunhofer FDK MPEG-H Software" must be replaced by the term "Third-Party
Modified Version of the Fraunhofer FDK MPEG-H Software".

3. No PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without
limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE.
Fraunhofer provides no warranty of patent non-infringement with respect to this
software. You may use this Fraunhofer FDK MPEG-H Software or modifications
thereto only for purposes that are authorized by appropriate patent licenses.

4. DISCLAIMER

This Fraunhofer FDK MPEG-H Software is provided by Fraunhofer on behalf of the
copyright holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED
WARRANTIES, including but not limited to the implied warranties of
merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE for any direct, indirect,
incidental, special, exemplary, or consequential damages, including but not
limited to procurement of substitute goods or services; loss of use, data, or
profits, or business interruption, however caused and on any theory of
liability, whether in contract, strict liability, or tort (including
negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.

5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Division Audio and Media Technologies - MPEG-H FDK
Am Wolfsmantel 33
91058 Erlangen, Germany
www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
-----------------------------------------------------------------------------*/

/************************* System integration library **************************

   Author(s):

   Description:

*******************************************************************************/

/** \file   genericStds.h
    \brief  Generic Run-Time Support function wrappers and heap allocation monitoring.
 */

#if !defined(GENERICSTDS_H)
#define GENERICSTDS_H

#include "machine_type.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846 /*!< Pi. Only used in example projects. */
#endif

/**
 * Identifiers for various memory locations. They are used along with memory allocation
 * functions like mpegh_FDKcalloc_L() to specify the requested memory's location.
 */
typedef enum {
  /* Internal */
  SECT_DATA_L1 = 0x2000,
  SECT_DATA_L2,
  SECT_DATA_L1_A,
  SECT_DATA_L1_B,
  SECT_CONSTDATA_L1,

  /* External */
  SECT_DATA_EXTERN = 0x4000,
  SECT_CONSTDATA_EXTERN

} MEMORY_SECTION;

/*! \addtogroup SYSLIB_MEMORY_MACROS FDK memory macros
 *
 * The \c H_ prefix indicates that the macro is to be used in a header file, the \c C_ prefix
 * indicates that the macro is to be used in a source file.
 *
 * Declaring memory areas requires to specify a unique name and a data type.
 *
 * For defining a memory area you require additionally one or two sizes, depending if the
 * memory should be organized into one or two dimensions.
 *
 * The macros containing the keyword \c AALLOC instead of \c ALLOC additionally  take care of
 * returning aligned memory addresses (beyond the natural alignment of its type). The preprocesor
 * macro
 * ::ALIGNMENT_DEFAULT indicates the aligment to be used (this is hardware specific).
 *
 * The \c _L suffix indicates that the memory will be located in a specific section. This is
 * useful to allocate critical memory section into fast internal SRAM for example.
 *
 * @{
 */

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define H_ALLOC_MEM(name, type) \
  type* Get##name(int n = 0);   \
  void Free##name(type** p);    \
  UINT GetRequiredMem##name(void);

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define H_ALLOC_MEM_OVERLAY(name, type) \
  type* Get##name(int n = 0);           \
  void Free##name(type** p);            \
  UINT GetRequiredMem##name(void);

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_ALLOC_MEM(name, type, num)               \
  type* Get##name(int n) {                         \
    FDK_ASSERT((n) == 0);                          \
    return ((type*)mpegh_FDKcalloc(num, sizeof(type)));  \
  }                                                \
  void Free##name(type** p) {                      \
    if (p != NULL) {                               \
      mpegh_FDKfree(*p);                                 \
      *p = NULL;                                   \
    }                                              \
  }                                                \
  UINT GetRequiredMem##name(void) {                \
    return ALGN_SIZE_EXTRES((num) * sizeof(type)); \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_ALLOC_MEM2(name, type, n1, n2)                 \
  type* Get##name(int n) {                               \
    FDK_ASSERT((n) < (n2));                              \
    return ((type*)mpegh_FDKcalloc(n1, sizeof(type)));         \
  }                                                      \
  void Free##name(type** p) {                            \
    if (p != NULL) {                                     \
      mpegh_FDKfree(*p);                                       \
      *p = NULL;                                         \
    }                                                    \
  }                                                      \
  UINT GetRequiredMem##name(void) {                      \
    return ALGN_SIZE_EXTRES((n1) * sizeof(type)) * (n2); \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_MEM(name, type, num)                                 \
  type* Get##name(int n) {                                            \
    type* ap;                                                         \
    FDK_ASSERT((n) == 0);                                             \
    ap = ((type*)mpegh_FDKaalloc((num) * sizeof(type), ALIGNMENT_DEFAULT)); \
    return ap;                                                        \
  }                                                                   \
  void Free##name(type** p) {                                         \
    if (p != NULL) {                                                  \
      mpegh_FDKafree(*p);                                                   \
      *p = NULL;                                                      \
    }                                                                 \
  }                                                                   \
  UINT GetRequiredMem##name(void) {                                   \
    return A_ALGN_SIZE_EXTRES((num) * sizeof(type));                  \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_MEM2(name, type, n1, n2)                            \
  type* Get##name(int n) {                                           \
    type* ap;                                                        \
    FDK_ASSERT((n) < (n2));                                          \
    ap = ((type*)mpegh_FDKaalloc((n1) * sizeof(type), ALIGNMENT_DEFAULT)); \
    return ap;                                                       \
  }                                                                  \
  void Free##name(type** p) {                                        \
    if (p != NULL) {                                                 \
      mpegh_FDKafree(*p);                                                  \
      *p = NULL;                                                     \
    }                                                                \
  }                                                                  \
  UINT GetRequiredMem##name(void) {                                  \
    return A_ALGN_SIZE_EXTRES((n1) * sizeof(type)) * (n2);           \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_ALLOC_MEM_L(name, type, num, s)              \
  type* Get##name(int n) {                             \
    FDK_ASSERT((n) == 0);                              \
    return ((type*)mpegh_FDKcalloc_L(num, sizeof(type), s)); \
  }                                                    \
  void Free##name(type** p) {                          \
    if (p != NULL) {                                   \
      mpegh_FDKfree_L(*p);                                   \
      *p = NULL;                                       \
    }                                                  \
  }                                                    \
  UINT GetRequiredMem##name(void) {                    \
    return ALGN_SIZE_EXTRES((num) * sizeof(type));     \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_ALLOC_MEM2_L(name, type, n1, n2, s)            \
  type* Get##name(int n) {                               \
    FDK_ASSERT((n) < (n2));                              \
    return (type*)mpegh_FDKcalloc_L(n1, sizeof(type), s);      \
  }                                                      \
  void Free##name(type** p) {                            \
    if (p != NULL) {                                     \
      mpegh_FDKfree_L(*p);                                     \
      *p = NULL;                                         \
    }                                                    \
  }                                                      \
  UINT GetRequiredMem##name(void) {                      \
    return ALGN_SIZE_EXTRES((n1) * sizeof(type)) * (n2); \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_MEM_L(name, type, num, s)                                 \
  type* Get##name(int n) {                                                 \
    type* ap;                                                              \
    FDK_ASSERT((n) == 0);                                                  \
    ap = ((type*)mpegh_FDKaalloc_L((num) * sizeof(type), ALIGNMENT_DEFAULT, s)); \
    return ap;                                                             \
  }                                                                        \
  void Free##name(type** p) {                                              \
    if (p != NULL) {                                                       \
      mpegh_FDKafree_L(*p);                                                      \
      *p = NULL;                                                           \
    }                                                                      \
  }                                                                        \
  UINT GetRequiredMem##name(void) {                                        \
    return A_ALGN_SIZE_EXTRES((num) * sizeof(type));                       \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_MEM2_L(name, type, n1, n2, s)                            \
  type* Get##name(int n) {                                                \
    type* ap;                                                             \
    FDK_ASSERT((n) < (n2));                                               \
    ap = ((type*)mpegh_FDKaalloc_L((n1) * sizeof(type), ALIGNMENT_DEFAULT, s)); \
    return ap;                                                            \
  }                                                                       \
  void Free##name(type** p) {                                             \
    if (p != NULL) {                                                      \
      mpegh_FDKafree_L(*p);                                                     \
      *p = NULL;                                                          \
    }                                                                     \
  }                                                                       \
  UINT GetRequiredMem##name(void) {                                       \
    return A_ALGN_SIZE_EXTRES((n1) * sizeof(type)) * (n2);                \
  }

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_ALLOC_MEM_OVERLAY(name, type, num, sect, tag) C_AALLOC_MEM_L(name, type, num, sect)

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_SCRATCH_START(name, type, n)           \
  char _##name[sizeof(type) * (n) + ALIGNMENT_DEFAULT]; \
  type* name = (type*)ALIGN_PTR(_##name);               \
  C_ALLOC_ALIGNED_REGISTER(name, (n) * sizeof(type));

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_ALLOC_SCRATCH_START(name, type, n) type name[n];

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_SCRATCH_END(name, type, n) C_ALLOC_ALIGNED_UNREGISTER(name);
/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_ALLOC_SCRATCH_END(name, type, n)

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_STACK_START(name, type, n)             \
  char _##name[sizeof(type) * (n) + ALIGNMENT_DEFAULT]; \
  type* name = (type*)ALIGN_PTR(_##name);               \
  C_ALLOC_ALIGNED_REGISTER(name, (n) * sizeof(type));

/** See \ref SYSLIB_MEMORY_MACROS for description. */
#define C_AALLOC_STACK_END(name, type, n) C_ALLOC_ALIGNED_UNREGISTER(name);

/*! @} */

#define C_ALLOC_ALIGNED_REGISTER(x, size)
#define C_ALLOC_ALIGNED_UNREGISTER(x)
#define C_ALLOC_ALIGNED_CHECK(x)
#define C_ALLOC_ALIGNED_CHECK2(x, y)
#define FDK_showBacktrace(a, b)

/*! \addtogroup SYSLIB_EXITCODES Unified exit codes
 *  Exit codes to be used as return values of FDK software test and demonstration applications.
 *  Not as return values of product modules and/or libraries.
 *  @{
 */
#define FDK_EXITCODE_OK 0 /*!< Successful termination. No errors. */
#define FDK_EXITCODE_USAGE                                                            \
  64 /*!< The command/application was used incorrectly, e.g. with the wrong number of \
          arguments, a bad flag, a bad syntax in a parameter, or whatever. */
#define FDK_EXITCODE_DATAERROR                                                   \
  65 /*!< The input data was incorrect in some way. This should only be used for \
          user data and not system files. */
#define FDK_EXITCODE_NOINPUT \
  66 /*!< An input file (not a system file) did not exist or was not readable. */
#define FDK_EXITCODE_UNAVAILABLE                                                     \
  69 /*!< A service is unavailable. This can occur if a support program or file does \
          not exist. This can also be used as a catchall message when something you  \
          wanted to do doesn't work, but you don't know why. */
#define FDK_EXITCODE_SOFTWARE                                                          \
  70 /*!< An internal software error has been detected. This should be limited to non- \
          operating system related errors as possible. */
#define FDK_EXITCODE_CANTCREATE 73 /*!< A (user specified) output file cannot be created. */
#define FDK_EXITCODE_IOERROR 74    /*!< An error occurred while doing I/O on some file. */
/*! @} */

/*--------------------------------------------
 * Runtime support declarations
 *---------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* Wrapper for <ctypes.h>'s isalpha(). */
INT FDKisalpha(INT c);

/* Wrapper for <ctypes.h>'s tolower(). */
INT FDKtolower(INT c);

void mpegh_FDKprintf(const char* szFmt, ...);

void mpegh_FDKprintfErr(const char* szFmt, ...);

/** Wrapper for <stdio.h>'s getchar(). */
INT mpegh_FDKgetchar(void);

INT mpegh_FDKfprintf(void* stream, const char* format, ...);

char* mpegh_FDKstrchr(char* s, INT c);
const char* mpegh_FDKstrstr(const char* haystack, const char* needle);
char* mpegh_FDKstrcpy(char* dest, const char* src);
char* mpegh_FDKstrncpy(char* dest, const char* src, const UINT n);

#define FDK_MAX_OVERLAYS 9 /**< Maximum number of memory overlays. */

void* mpegh_FDKcalloc(const UINT n, const UINT size);
void* mpegh_FDKmalloc(const UINT size);
void mpegh_FDKfree(void* ptr);

/**
 *  Allocate and clear an aligned memory area. Use mpegh_FDKafree() instead of mpegh_FDKfree() for these memory
 * areas.
 *
 * \param size       Size of requested memory in bytes.
 * \param alignment  Alignment of requested memory in bytes.
 * \return           Pointer to allocated memory.
 */
void* mpegh_FDKaalloc(const UINT size, const UINT alignment);

/**
 *  Free an aligned memory area.
 *
 * \param ptr  Pointer to be freed.
 */
void mpegh_FDKafree(void* ptr);

/**
 *  Allocate memory in a specific memory section.
 *  Requests can be made for internal or external memory. If internal memory is
 *  requested, mpegh_FDKcalloc_L() first tries to use L1 memory, which sizes are defined
 *  by ::DATA_L1_A_SIZE and ::DATA_L1_B_SIZE. If no L1 memory is available, then
 *  mpegh_FDKcalloc_L() tries to use L2 memory. If that fails as well, the requested
 *  memory is allocated at an extern location using the fallback mpegh_FDKcalloc().
 *
 * \param n     See MSDN documentation on calloc().
 * \param size  See MSDN documentation on calloc().
 * \param s     Memory section.
 * \return      See MSDN documentation on calloc().
 */
void* mpegh_FDKcalloc_L(const UINT n, const UINT size, MEMORY_SECTION s);

/**
 *  Allocate aligned memory in a specific memory section.
 *  See mpegh_FDKcalloc_L() description for details - same applies here.
 */
void* mpegh_FDKaalloc_L(const UINT size, const UINT alignment, MEMORY_SECTION s);

/**
 *  Free memory that was allocated in a specific memory section.
 */
void mpegh_FDKfree_L(void* ptr);

/**
 *  Free aligned memory that was allocated in a specific memory section.
 */
void mpegh_FDKafree_L(void* ptr);

/**
 * Copy memory. Source and destination memory must not overlap.
 * Either use implementation from a Standard Library, or, if no Standard Library
 * is available, a generic implementation.
 * The define ::USE_BUILTIN_MEM_FUNCTIONS in genericStds.cpp controls what to use.
 * The function arguments correspond to the standard memcpy(). Please see MSDN
 * documentation for details on how to use it.
 */
void mpegh_FDKmemcpy(void* dst, const void* src, const UINT size);

/**
 * Copy memory. Source and destination memory are allowed to overlap.
 * Either use implementation from a Standard Library, or, if no Standard Library
 * is available, a generic implementation.
 * The define ::USE_BUILTIN_MEM_FUNCTIONS in genericStds.cpp controls what to use.
 * The function arguments correspond to the standard memmove(). Please see MSDN
 * documentation for details on how to use it.
 */
void mpegh_FDKmemmove(void* dst, const void* src, const UINT size);

/**
 * Clear memory.
 * Either use implementation from a Standard Library, or, if no Standard Library
 * is available, a generic implementation.
 * The define ::USE_BUILTIN_MEM_FUNCTIONS in genericStds.cpp controls what to use.
 * The function arguments correspond to the standard memclear(). Please see MSDN
 * documentation for details on how to use it.
 */
void mpegh_FDKmemclear(void* memPtr, const UINT size);

/**
 * Fill memory with values.
 * The function arguments correspond to the standard memset(). Please see MSDN
 * documentation for details on how to use it.
 */
void mpegh_FDKmemset(void* memPtr, const INT value, const UINT size);

/* Compare function wrappers */
INT mpegh_FDKmemcmp(const void* s1, const void* s2, const UINT size);
INT mpegh_FDKstrcmp(const char* s1, const char* s2);
INT mpegh_FDKstrncmp(const char* s1, const char* s2, const UINT size);

UINT mpegh_FDKstrlen(const char* s);

#define FDKmax(a, b) ((a) > (b) ? (a) : (b))
#define FDKmin(a, b) ((a) < (b) ? (a) : (b))

#define FDK_INT_MAX ((INT)0x7FFFFFFF)
#define FDK_INT_MIN ((INT)0x80000000)

/* FILE I/O */

/*!
 *  Check platform for endianess.
 *
 * \return  1 if platform is little endian, non-1 if platform is big endian.
 */
int MPEGH_IS_LITTLE_ENDIAN(void);

/*!
 *  Convert input value to little endian format.
 *
 * \param val  Value to be converted. It may be in both big or little endian.
 * \return     Value in little endian format.
 */
UINT MPEGH_TO_LITTLE_ENDIAN(UINT val);

/*!
 * \fn     FDKFILE *mpegh_FDKfopen(const char *filename, const char *mode);
 *         Standard fopen() wrapper.
 * \fn     INT mpegh_FDKfclose(FDKFILE *FP);
 *         Standard fclose() wrapper.
 * \fn     INT mpegh_FDKfseek(FDKFILE *FP, LONG OFFSET, int WHENCE);
 *         Standard fseek() wrapper.
 * \fn     INT mpegh_FDKftell(FDKFILE *FP);
 *         Standard ftell() wrapper.
 * \fn     INT FDKfseek64(FDKFILE *FP, INT64 OFFSET, int WHENCE);
 *         Machine dependent fseek() wrapper for files >4GB.
 * \fn     INT64 FDKftell64(FDKFILE *FP);
 *         Machine dependent ftell() wrapper for files >4GB.
 * \fn     INT mpegh_FDKfflush(FDKFILE *fp);
 *         Standard fflush() wrapper.
 * \fn     UINT mpegh_FDKfwrite(const void *ptrf, INT size, UINT nmemb, FDKFILE *fp);
 *         Standard fwrite() wrapper.
 * \fn     UINT mpegh_FDKfread(void *dst, INT size, UINT nmemb, FDKFILE *fp);
 *         Standard fread() wrapper.
 */
typedef void FDKFILE;
extern const INT MPEGH_FDKSEEK_SET, MPEGH_FDKSEEK_CUR, MPEGH_FDKSEEK_END;

FDKFILE* mpegh_FDKfopen(const char* filename, const char* mode);
INT mpegh_FDKfclose(FDKFILE* FP);
INT mpegh_FDKfseek(FDKFILE* FP, LONG OFFSET, int WHENCE);
INT mpegh_FDKftell(FDKFILE* FP);
INT FDKfseek64(FDKFILE* fp, INT64 OFFSET, int WHENCE);
INT64 FDKftell64(FDKFILE* FP);
INT mpegh_FDKfflush(FDKFILE* fp);
UINT mpegh_FDKfwrite(const void* ptrf, INT size, UINT nmemb, FDKFILE* fp);
UINT mpegh_FDKfread(void* dst, INT size, UINT nmemb, FDKFILE* fp);
char* mpegh_FDKfgets(void* dst, INT size, FDKFILE* fp);
void mpegh_FDKrewind(FDKFILE* fp);
INT mpegh_FDKfeof(FDKFILE* fp);

/**
 * \brief        Write each member in little endian order. Convert automatically to host endianess.
 * \param ptrf   Pointer to memory where to read data from.
 * \param size   Size of each item to be written.
 * \param nmemb  Number of items to be written.
 * \param fp     File pointer of type FDKFILE.
 * \return       Number of items read on success and fread() error on failure.
 */
UINT mpegh_FDKfwrite_EL(const void* ptrf, INT size, UINT nmemb, FDKFILE* fp);

/**
 * \brief        Read variable of size "size" as little endian. Convert automatically to host
 * endianess. 4-byte alignment is enforced for 24 bit data, at 32 bit full scale.
 * \param dst    Pointer to memory where to store data into.
 * \param size   Size of each item to be read.
 * \param nmemb  Number of items to be read.
 * \param fp     File pointer of type FDKFILE.
 * \return       Number of items read on success and fread() error on failure.
 */
UINT mpegh_FDKfread_EL(void* dst, INT size, UINT nmemb, FDKFILE* fp);

#ifdef __cplusplus
}
#endif

#endif /* GENERICSTDS_H */
