/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/common/include/p9_xbus_scom_addresses_fixes.H $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
///
/// @file xbus_scom_addresses_fixes.H
/// @brief The *scom_addresses.H files are  generated form figtree, but
///        the figree can be wrong.  This file is included at the end
///        of scom_addresses.H and allows incorrect constants to be
///        fixed manually.
///
// *HWP HWP Owner: Ben Gass <bgass@us.ibm.com>
// *HWP FW Owner: ? <?>
// *HWP Team: SAO
// *HWP Level: 3
// *HWP Consumed by: FSP:HB:HS:OCC:SBE:CME:SGPE:PGPE:FPPE:IPPE

#ifndef __P9_XBUS_SCOM_ADDRESSES_FIXES_H
#define __P9_XBUS_SCOM_ADDRESSES_FIXES_H

//Example,
//Copy the whole line from the *scom_addresses.H file.  Then add
//FIX in front of REG, and add another paramter that is the new
//corrected value.
//FIXREG64( PU_ALTD_ADDR_REG,
//          RULL(0x05022800), SH_UNT, SH_ACS_SCOM,
//          RULL(0x00090000)
//        );

#endif
