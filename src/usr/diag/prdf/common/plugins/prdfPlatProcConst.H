/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/common/plugins/prdfPlatProcConst.H $        */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2018                        */
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

#ifndef __prdfPlatProcConst_H
#define __prdfPlatProcConst_H

/**
 * @file  prdfPlatProcConst.H
 * @brief Defines platform specific constants for the processor.
 */
namespace PRDF
{

namespace TOD
{
/**
 * @brief summarizes error analysis for a TOD topology.
 */
enum
{
    NO_TOD_ERROR                 = 0,
    MASTER_PATH_ERROR            = 1,
    INTERNAL_PATH_ERROR          = 2,
    SLAVE_PATH_NETWORK_ERROR     = 3,
    UNKNOWN_TOD_ERROR            = 4,
    LAST_TOD_ERROR               = 5,
};

/**
 * @brief TOD error analysis data to be added to special user section of e-log.
 */
struct TodErrorSummary
{
#if __BYTE_ORDER == __LITTLE_ENDIAN

    uint32_t reserved               :17;
    uint32_t backUpTopologyMastPath :2; //master path for backup topology
    uint32_t activeTopologyMastPath :2; // master path for active topology
    uint32_t backUpTopologySummary  :3; // backup topology error status
    uint32_t activeTopologySummary  :3; // active topology error status
    uint32_t activeTopology         :1; // Topology acting as Active
    uint32_t topologyResetRequested :1; // topology reset request status
    uint32_t topologySwitchByPhyp   :1; // topology switch event detected
    uint32_t phypDetectedTodError   :1; // TOD error detected by Phyp
    uint32_t hardwareSwitchFlip     :1; // Master Path Failover by Hw

#else

    uint32_t hardwareSwitchFlip     :1; // Master Path Failover by Hw
    uint32_t phypDetectedTodError   :1; // TOD error detected by Phyp
    uint32_t topologySwitchByPhyp   :1; // topology switch event detected
    uint32_t topologyResetRequested :1; // topology reset request status
    uint32_t activeTopology         :1; // Topology acting as Active
    uint32_t activeTopologySummary  :3; // active topology error status
    uint32_t backUpTopologySummary  :3; // backup topology error status
    uint32_t activeTopologyMastPath :2; // master path for active topology
    uint32_t backUpTopologyMastPath :2; //master path for backup topology
    uint32_t reserved               :17;

#endif
    uint32_t activeMdmt;       // HUID of active mdmt
    uint32_t backUpMdmt;       // HUID of backup MDMT

    TodErrorSummary():
        activeMdmt(0),
        backUpMdmt(0)
    {};

};


}//end namespace TOD

} // end namespace PRDF

#endif // __prdfPlatProcConst_H

