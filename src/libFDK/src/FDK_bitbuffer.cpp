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

/******************* Library for basic calculation routines ********************

   Author(s):   M. Lohwasser

   Description: common bitbuffer read/write routines

*******************************************************************************/

#include "FDK_bitbuffer.h"

#include "genericStds.h"
#include "common_fix.h"
#include "fixminmax.h"

const UINT BitMask[32 + 1] = {0x0,        0x1,        0x3,       0x7,       0xf,       0x1f,
                              0x3f,       0x7f,       0xff,      0x1ff,     0x3ff,     0x7ff,
                              0xfff,      0x1fff,     0x3fff,    0x7fff,    0xffff,    0x1ffff,
                              0x3ffff,    0x7ffff,    0xfffff,   0x1fffff,  0x3fffff,  0x7fffff,
                              0xffffff,   0x1ffffff,  0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff,
                              0x3fffffff, 0x7fffffff, 0xffffffff};

void FDK_CreateBitBuffer(HANDLE_FDK_BITBUF* hBitBuf, UCHAR* pBuffer, UINT bufSize) {
  FDK_InitBitBuffer(*hBitBuf, pBuffer, bufSize, 0);

  mpegh_FDKmemclear((*hBitBuf)->Buffer, bufSize * sizeof(UCHAR));
}

void FDK_DeleteBitBuffer(HANDLE_FDK_BITBUF hBitBuf) {
  ;
}

void FDK_InitBitBuffer(HANDLE_FDK_BITBUF hBitBuf, UCHAR* pBuffer, UINT bufSize, UINT validBits) {
  hBitBuf->ValidBits = validBits;
  hBitBuf->ReadOffset = 0;
  hBitBuf->WriteOffset = 0;
  hBitBuf->BitNdx = 0;

  hBitBuf->Buffer = pBuffer;
  hBitBuf->bufSize = bufSize;
  hBitBuf->bufBits = (bufSize << 3);
  /*assure bufsize (2^n) */
  FDK_ASSERT(hBitBuf->ValidBits <= (INT)hBitBuf->bufBits);
  FDK_ASSERT((bufSize > 0) && (bufSize <= MAX_BUFSIZE_BYTES));
  {
    UINT x = 0, n = bufSize;
    for (x = 0; n > 0; x++, n >>= 1) {
    }
    if (bufSize != ((UINT)1 << (x - 1))) {
      FDK_ASSERT(0);
    }
  }
}

void FDK_ResetBitBuffer(HANDLE_FDK_BITBUF hBitBuf) {
  hBitBuf->ValidBits = 0;
  hBitBuf->ReadOffset = 0;
  hBitBuf->WriteOffset = 0;
  hBitBuf->BitNdx = 0;
}

#ifndef FUNCTION_FDK_get
INT FDK_get(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits) {
  UINT byteOffset = hBitBuf->BitNdx >> 3;
  UINT bitOffset = hBitBuf->BitNdx & 0x07;

  hBitBuf->BitNdx = (hBitBuf->BitNdx + numberOfBits) & (hBitBuf->bufBits - 1);
  hBitBuf->ValidBits -= numberOfBits;

  UINT byteMask = hBitBuf->bufSize - 1;

  UINT tx = (hBitBuf->Buffer[byteOffset & byteMask] << 24) |
            (hBitBuf->Buffer[(byteOffset + 1) & byteMask] << 16) |
            (hBitBuf->Buffer[(byteOffset + 2) & byteMask] << 8) |
            hBitBuf->Buffer[(byteOffset + 3) & byteMask];

  if (bitOffset) {
    tx <<= bitOffset;
    tx |= hBitBuf->Buffer[(byteOffset + 4) & byteMask] >> (8 - bitOffset);
  }

  return (tx >> (32 - numberOfBits));
}
#endif /* #ifndef FUNCTION_FDK_get */

#ifndef FUNCTION_FDK_get32
#if defined(__clang__) && (__clang_major__ >= 12)
__attribute__((no_sanitize("unsigned-shift-base")))
#endif
INT FDK_get32 (HANDLE_FDK_BITBUF hBitBuf)
{
  UINT BitNdx = hBitBuf->BitNdx + 32;
  hBitBuf->BitNdx = BitNdx & (hBitBuf->bufBits - 1);
  hBitBuf->ValidBits -= 32;

  UINT byteOffset = (BitNdx - 1) >> 3;
  if (BitNdx <= hBitBuf->bufBits) {
    UINT cache = (hBitBuf->Buffer[(byteOffset - 3)] << 24) |
                 (hBitBuf->Buffer[(byteOffset - 2)] << 16) |
                 (hBitBuf->Buffer[(byteOffset - 1)] << 8) | hBitBuf->Buffer[(byteOffset - 0)];

    if ((BitNdx = (BitNdx & 7)) != 0) {
      cache = (cache >> (8 - BitNdx)) | ((UINT)hBitBuf->Buffer[byteOffset - 4] << (24 + BitNdx));
    }
    return (cache);
  } else {
    UINT byte_mask = hBitBuf->bufSize - 1;
    UINT cache = (hBitBuf->Buffer[(byteOffset - 3) & byte_mask] << 24) |
                 (hBitBuf->Buffer[(byteOffset - 2) & byte_mask] << 16) |
                 (hBitBuf->Buffer[(byteOffset - 1) & byte_mask] << 8) |
                 hBitBuf->Buffer[(byteOffset - 0) & byte_mask];

    if ((BitNdx = (BitNdx & 7)) != 0) {
      cache = (cache >> (8 - BitNdx)) |
              ((UINT)hBitBuf->Buffer[(byteOffset - 4) & byte_mask] << (24 + BitNdx));
    }
    return (cache);
  }
}
#endif

INT FDK_getBwd(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits) {
  UINT byteOffset = hBitBuf->BitNdx >> 3;
  UINT bitOffset = hBitBuf->BitNdx & 0x07;
  UINT byteMask = hBitBuf->bufSize - 1;
  int i;

  hBitBuf->BitNdx = (hBitBuf->BitNdx - numberOfBits) & (hBitBuf->bufBits - 1);
  hBitBuf->ValidBits += numberOfBits;

  UINT tx = hBitBuf->Buffer[(byteOffset - 3) & byteMask] << 24 |
            hBitBuf->Buffer[(byteOffset - 2) & byteMask] << 16 |
            hBitBuf->Buffer[(byteOffset - 1) & byteMask] << 8 |
            hBitBuf->Buffer[byteOffset & byteMask];
  UINT txa = 0x0;

  tx >>= (8 - bitOffset);

  if (bitOffset && numberOfBits > 24) {
    tx |= hBitBuf->Buffer[(byteOffset - 4) & byteMask] << (24 + bitOffset);
  }

  /* in place turn around */
  for (i = 0; i < 16; i++) {
    UINT bitMaskR = 0x00000001 << i;
    UINT bitMaskL = 0x80000000 >> i;

    txa |= (tx & bitMaskR) << (31 - (i << 1));
    txa |= (tx & bitMaskL) >> (31 - (i << 1));
  }

  return (txa >> (32 - numberOfBits));
}

void FDK_put(HANDLE_FDK_BITBUF hBitBuf, UINT value, const UINT numberOfBits) {
  if (numberOfBits != 0) {
    UINT byteOffset0 = hBitBuf->BitNdx >> 3;
    UINT bitOffset = hBitBuf->BitNdx & 0x7;

    hBitBuf->BitNdx = (hBitBuf->BitNdx + numberOfBits) & (hBitBuf->bufBits - 1);
    hBitBuf->ValidBits += numberOfBits;

    UINT byteMask = hBitBuf->bufSize - 1;

    UINT byteOffset1 = (byteOffset0 + 1) & byteMask;
    UINT byteOffset2 = (byteOffset0 + 2) & byteMask;
    UINT byteOffset3 = (byteOffset0 + 3) & byteMask;

    // Create tmp containing free bits at the left border followed by bits to write, LSB's are
    // cleared, if available Create mask to apply upon all buffer bytes
    UINT tmp = (value << (32 - numberOfBits)) >> bitOffset;
    UINT mask = ~((BitMask[numberOfBits] << (32 - numberOfBits)) >> bitOffset);

    // read all 4 bytes from buffer and create a 32-bit cache
    UINT cache = (((UINT)hBitBuf->Buffer[byteOffset0]) << 24) |
                 (((UINT)hBitBuf->Buffer[byteOffset1]) << 16) |
                 (((UINT)hBitBuf->Buffer[byteOffset2]) << 8) |
                 (((UINT)hBitBuf->Buffer[byteOffset3]) << 0);

    cache = (cache & mask) | tmp;
    hBitBuf->Buffer[byteOffset0] = (UCHAR)(cache >> 24);
    hBitBuf->Buffer[byteOffset1] = (UCHAR)(cache >> 16);
    hBitBuf->Buffer[byteOffset2] = (UCHAR)(cache >> 8);
    hBitBuf->Buffer[byteOffset3] = (UCHAR)(cache >> 0);

    if ((bitOffset + numberOfBits) > 32) {
      UINT byteOffset4 = (byteOffset0 + 4) & byteMask;
      // remaining bits: in range 1..7
      // replace MSBits of next byte in buffer by LSBits of "value"
      int bits = (bitOffset + numberOfBits) & 7;
      cache = (UINT)hBitBuf->Buffer[byteOffset4] & (~(BitMask[bits] << (8 - bits)));
      cache |= value << (8 - bits);
      hBitBuf->Buffer[byteOffset4] = (UCHAR)cache;
    }
  }
}

void FDK_putBwd(HANDLE_FDK_BITBUF hBitBuf, UINT value, const UINT numberOfBits) {
  UINT byteOffset = hBitBuf->BitNdx >> 3;
  UINT bitOffset = 7 - (hBitBuf->BitNdx & 0x07);
  UINT byteMask = hBitBuf->bufSize - 1;

  UINT mask = ~(BitMask[numberOfBits] << bitOffset);
  UINT tmp = 0x0000;
  int i;

  hBitBuf->BitNdx = (hBitBuf->BitNdx - numberOfBits) & (hBitBuf->bufBits - 1);
  hBitBuf->ValidBits -= numberOfBits;

  /* in place turn around */
  for (i = 0; i < 16; i++) {
    UINT bitMaskR = 0x00000001 << i;
    UINT bitMaskL = 0x80000000 >> i;

    tmp |= (value & bitMaskR) << (31 - (i << 1));
    tmp |= (value & bitMaskL) >> (31 - (i << 1));
  }
  value = tmp;
  tmp = value >> (32 - numberOfBits) << bitOffset;

  hBitBuf->Buffer[byteOffset & byteMask] =
      (hBitBuf->Buffer[byteOffset & byteMask] & (mask)) | (UCHAR)(tmp);
  hBitBuf->Buffer[(byteOffset - 1) & byteMask] =
      (hBitBuf->Buffer[(byteOffset - 1) & byteMask] & (mask >> 8)) | (UCHAR)(tmp >> 8);
  hBitBuf->Buffer[(byteOffset - 2) & byteMask] =
      (hBitBuf->Buffer[(byteOffset - 2) & byteMask] & (mask >> 16)) | (UCHAR)(tmp >> 16);
  hBitBuf->Buffer[(byteOffset - 3) & byteMask] =
      (hBitBuf->Buffer[(byteOffset - 3) & byteMask] & (mask >> 24)) | (UCHAR)(tmp >> 24);

  if ((bitOffset + numberOfBits) > 32) {
    hBitBuf->Buffer[(byteOffset - 4) & byteMask] =
        (UCHAR)(value >> (64 - numberOfBits - bitOffset)) |
        (hBitBuf->Buffer[(byteOffset - 4) & byteMask] &
         ~(BitMask[bitOffset] >> (32 - numberOfBits)));
  }
}

#ifndef FUNCTION_FDK_pushBack
void FDK_pushBack(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits, UCHAR config) {
  hBitBuf->ValidBits = (config == 0) ? (hBitBuf->ValidBits + (INT)numberOfBits)
                                     : (hBitBuf->ValidBits - (INT)numberOfBits);
  hBitBuf->BitNdx =
      ((UINT)((INT)hBitBuf->BitNdx - (INT)numberOfBits)) & ((INT)hBitBuf->bufBits - 1);
}
#endif

void FDK_pushForward(HANDLE_FDK_BITBUF hBitBuf, const UINT numberOfBits, UCHAR config) {
  hBitBuf->ValidBits = (config == 0) ? (hBitBuf->ValidBits - (INT)numberOfBits)
                                     : (hBitBuf->ValidBits + (INT)numberOfBits);
  hBitBuf->BitNdx = (UINT)((INT)hBitBuf->BitNdx + (INT)numberOfBits) & (hBitBuf->bufBits - 1);
}

#ifndef FUNCTION_FDK_getValidBits
INT FDK_getValidBits(HANDLE_FDK_BITBUF hBitBuf) {
  return hBitBuf->ValidBits;
}
#endif /* #ifndef FUNCTION_FDK_getValidBits */

INT FDK_getFreeBits(HANDLE_FDK_BITBUF hBitBuf) {
  return (hBitBuf->bufBits - hBitBuf->ValidBits);
}

void FDK_Feed(HANDLE_FDK_BITBUF hBitBuf, const UCHAR* RESTRICT inputBuffer, const UINT bufferSize,
              UINT* bytesValid) {
  inputBuffer = &inputBuffer[bufferSize - *bytesValid];

  UINT bTotal = 0;

  UINT bToRead =
      fMin(hBitBuf->bufBits, (UINT)fMax(0, ((INT)hBitBuf->bufBits - hBitBuf->ValidBits))) >> 3;
  UINT noOfBytes = fMin(bToRead, *bytesValid);  //(bToRead < *bytesValid) ? bToRead : *bytesValid ;

  while (noOfBytes > 0) {
    /* split read to buffer size */
    bToRead = hBitBuf->bufSize - hBitBuf->ReadOffset;
    bToRead = fMin(bToRead, noOfBytes);  //(bToRead < noOfBytes) ? bToRead : noOfBytes ;

    /* copy 'bToRead' bytes from 'ptr' to inputbuffer */
    mpegh_FDKmemcpy(&hBitBuf->Buffer[hBitBuf->ReadOffset], inputBuffer, bToRead * sizeof(UCHAR));

    /* add noOfBits to number of valid bits in buffer */
    hBitBuf->ValidBits += (INT)(bToRead << 3);
    bTotal += bToRead;
    inputBuffer += bToRead;

    hBitBuf->ReadOffset = (hBitBuf->ReadOffset + bToRead) & (hBitBuf->bufSize - 1);
    noOfBytes -= bToRead;
  }

  *bytesValid -= bTotal;
}

void CopyAlignedBlock(HANDLE_FDK_BITBUF h_BitBufSrc, UCHAR* RESTRICT dstBuffer, UINT bToRead) {
  UINT byteOffset = h_BitBufSrc->BitNdx >> 3;
  const UINT byteMask = h_BitBufSrc->bufSize - 1;

  UCHAR* RESTRICT pBBB = h_BitBufSrc->Buffer;
  for (UINT i = 0; i < bToRead; i++) {
    dstBuffer[i] = pBBB[(byteOffset + i) & byteMask];
  }

  bToRead <<= 3;

  h_BitBufSrc->BitNdx = (h_BitBufSrc->BitNdx + bToRead) & (h_BitBufSrc->bufBits - 1);
  h_BitBufSrc->ValidBits -= bToRead;
}

void FDK_Copy(HANDLE_FDK_BITBUF h_BitBufDst, HANDLE_FDK_BITBUF h_BitBufSrc, UINT* bytesValid) {
  INT bTotal = 0;

  /* limit noOfBytes to valid bytes in src buffer and available bytes in dst buffer */
  UINT bToRead = h_BitBufSrc->ValidBits >> 3;
  UINT noOfBytes = fMin(bToRead, *bytesValid);  //(*bytesValid < bToRead) ? *bytesValid : bToRead ;
  bToRead = FDK_getFreeBits(h_BitBufDst);
  noOfBytes = fMin(bToRead, noOfBytes);  //(bToRead < noOfBytes) ? bToRead : noOfBytes;

  while (noOfBytes > 0) {
    /* Split Read to buffer size */
    bToRead = h_BitBufDst->bufSize - h_BitBufDst->ReadOffset;
    bToRead = fMin(noOfBytes, bToRead);  //(noOfBytes < bToRead) ? noOfBytes : bToRead ;

    /* copy 'bToRead' bytes from buffer to buffer */
    if (!(h_BitBufSrc->BitNdx & 0x07)) {
      CopyAlignedBlock(h_BitBufSrc, h_BitBufDst->Buffer + h_BitBufDst->ReadOffset, bToRead);
    } else {
      for (UINT i = 0; i < bToRead; i++) {
        h_BitBufDst->Buffer[h_BitBufDst->ReadOffset + i] = (UCHAR)FDK_get(h_BitBufSrc, 8);
      }
    }

    /* add noOfBits to number of valid bits in buffer */
    h_BitBufDst->ValidBits += bToRead << 3;
    bTotal += bToRead;

    h_BitBufDst->ReadOffset = (h_BitBufDst->ReadOffset + bToRead) & (h_BitBufDst->bufSize - 1);
    noOfBytes -= bToRead;
  }

  *bytesValid -= bTotal;
}

void FDK_Fetch(HANDLE_FDK_BITBUF hBitBuf, UCHAR* outBuf, UINT* writeBytes) {
  UCHAR* RESTRICT outputBuffer = outBuf;
  UINT bTotal = 0;

  UINT bToWrite = (hBitBuf->ValidBits) >> 3;
  UINT noOfBytes =
      fMin(bToWrite, *writeBytes);  //(bToWrite < *writeBytes) ? bToWrite : *writeBytes ;

  while (noOfBytes > 0) {
    /* split write to buffer size */
    bToWrite = hBitBuf->bufSize - hBitBuf->WriteOffset;
    bToWrite = fMin(bToWrite, noOfBytes);  //(bToWrite < noOfBytes) ? bToWrite : noOfBytes ;

    /* copy 'bToWrite' bytes from bitbuffer to outputbuffer */
    mpegh_FDKmemcpy(outputBuffer, &hBitBuf->Buffer[hBitBuf->WriteOffset], bToWrite * sizeof(UCHAR));

    /* sub noOfBits from number of valid bits in buffer */
    hBitBuf->ValidBits -= bToWrite << 3;
    bTotal += bToWrite;
    outputBuffer += bToWrite;

    hBitBuf->WriteOffset = (hBitBuf->WriteOffset + bToWrite) & (hBitBuf->bufSize - 1);
    noOfBytes -= bToWrite;
  }

  *writeBytes = bTotal;
}
