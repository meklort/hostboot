/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/mmio/mmio.H $                                 */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2018                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef __USR_MMIO_MMIO_H
#define __USR_MMIO_MMIO_H

/** @file  mmio.H
 *  @brief Provides the interface to initialize MMIO operations
 *  */

namespace MMIO
{
/**
 * @brief Memory map OCMBs.
 * This function maps OCMB memory and registers it into Hostboot virtual memory.
 *
 * @return  nullptr on success, valid error log(errlHndl_t) if an error occurred
 */
errlHndl_t mmioSetup();

}; // End namespace

#endif
