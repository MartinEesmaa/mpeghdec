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

   Author(s):   Dirk Mahne, Arthur Tritthart

   Description:

*******************************************************************************/

#include "FDK_stftfilterbank.h"
#include "FDK_stftfilterbank_data.h"
#include "FDK_stftfilterbank_process.h"

/**********************************************************************************************************************************/

int StftFilterbank_Open(const STFT_FILTERBANK_CONFIG* stftFilterbankConfig,
                        HANDLE_STFT_FILTERBANK* ph_stft_filterbank) {
  HANDLE_STFT_FILTERBANK stft_filterbank = NULL;
  STFT_FILTERBANK_CONFIG stftFilterbankConfig_local = {
      STFT_FILTERBANK_MODE_INVALID, /* STFT_FILTERBANK_MODE    stftFilterbankMode   */
      (UINT)0,                      /* UINT                    frameSize            */
      (UINT)0};                     /* UINT                    fftSize              */

  int numZerosPaddedTwoSided = 0;

  FDK_ASSERT(stftFilterbankConfig != NULL);
  FDK_ASSERT(ph_stft_filterbank != NULL);

  /* check configuration */
  switch (stftFilterbankConfig->stftFilterbankMode) {
    case STFT_FILTERBANK_MODE_TIME_TO_FREQ:
    case STFT_FILTERBANK_MODE_FREQ_TO_TIME:
      stftFilterbankConfig_local.stftFilterbankMode = stftFilterbankConfig->stftFilterbankMode;
      break;
    default:
      return STFT_ERR_BAD_PARAM_WARNING;
  }

  stftFilterbankConfig_local.frameSize = stftFilterbankConfig->frameSize;
  numZerosPaddedTwoSided = stftFilterbankConfig->fftSize - 2 * stftFilterbankConfig->frameSize;

  FDK_ASSERT(numZerosPaddedTwoSided >= 0 &&
             ((numZerosPaddedTwoSided & 1) == 0) /* must be even and positive */);
  FDK_ASSERT(((stftFilterbankConfig->fftSize / 2) & 1) == 0 /* fftSize must be multiple of 4 */);
  stftFilterbankConfig_local.fftSize = stftFilterbankConfig->fftSize;

  /* no errors, allocate struct */
  if ((stft_filterbank = (STFT_FILTERBANK*)mpegh_FDKcalloc(1, sizeof(STFT_FILTERBANK))) == NULL) {
    return STFT_ERR_MEM_ERROR;
  } else {
    /* copy validated config to internal struct */
    stft_filterbank->stftFilterbankConfig.stftFilterbankMode =
        stftFilterbankConfig_local.stftFilterbankMode;
    stft_filterbank->stftFilterbankConfig.frameSize = stftFilterbankConfig_local.frameSize;
    stft_filterbank->stftFilterbankConfig.fftSize = stftFilterbankConfig_local.fftSize;
    stft_filterbank->numZerosPaddedTwoSided = numZerosPaddedTwoSided;
  }

  if (allocateStftFilterbankData(stft_filterbank) != 0) {
    StftFilterbank_Close(&stft_filterbank);
    return STFT_ERR_MEM_ERROR;
  } else {
    /* return allocated struct */
    *ph_stft_filterbank = stft_filterbank;
  }

  return STFT_ERR_NO_ERROR;
}

/**********************************************************************************************************************************/

/**********************************************************************************************************************************/

void StftFilterbank_Process(FIXP_DBL* RESTRICT inputBuffer, FIXP_DBL* RESTRICT outputBuffer,
                            HANDLE_STFT_FILTERBANK h_stft_filterbank, INT STFT_headroom) {
  FDK_ASSERT(h_stft_filterbank != NULL);
  FDK_ASSERT(h_stft_filterbank->prevAudioInput != NULL);

  if (h_stft_filterbank->stftFilterbankConfig.stftFilterbankMode ==
      STFT_FILTERBANK_MODE_TIME_TO_FREQ) {
    if (STFT_headroom != 0) {
      mpegh_scaleValues(inputBuffer, h_stft_filterbank->stftFilterbankConfig.frameSize, STFT_headroom);
      mpegh_scaleValues(h_stft_filterbank->prevAudioInput,
                  h_stft_filterbank->stftFilterbankConfig.frameSize, STFT_headroom);
    }

    processAnalysisSTFT(inputBuffer, outputBuffer, h_stft_filterbank);

    if (STFT_headroom != 0) {
      mpegh_scaleValues(h_stft_filterbank->prevAudioInput,
                  h_stft_filterbank->stftFilterbankConfig.frameSize, -STFT_headroom);
    }
  } else {
    /* make sure we have one bit headroom for FFT */
    if (mpegh_getScalefactor(inputBuffer, h_stft_filterbank->stftFilterbankConfig.fftSize) == 0) {
      mpegh_scaleValues(inputBuffer, h_stft_filterbank->stftFilterbankConfig.fftSize, -1);
      STFT_headroom--;
    }
    if (STFT_headroom != 0) {
      mpegh_scaleValues(h_stft_filterbank->prevAudioInput,
                  h_stft_filterbank->stftFilterbankConfig.frameSize, STFT_headroom);
      mpegh_scaleValues(outputBuffer, h_stft_filterbank->stftFilterbankConfig.frameSize, STFT_headroom);
    }

    processSynthesisSTFT(inputBuffer, outputBuffer, h_stft_filterbank);

    if (STFT_headroom != 0) {
      mpegh_scaleValues(outputBuffer, h_stft_filterbank->stftFilterbankConfig.frameSize, -STFT_headroom);
      mpegh_scaleValues(h_stft_filterbank->prevAudioInput,
                  h_stft_filterbank->stftFilterbankConfig.frameSize, -STFT_headroom);
    }
  }
}

/**********************************************************************************************************************************/

int StftFilterbank_Close(HANDLE_STFT_FILTERBANK* ph_stft_filterbank) {
  if (ph_stft_filterbank != NULL && *ph_stft_filterbank != NULL) {
    freeStftFilterbankData(*ph_stft_filterbank);

    mpegh_FDKfree(*ph_stft_filterbank);
    *ph_stft_filterbank = NULL;
  }
  return 0;
}

/**********************************************************************************************************************************/
