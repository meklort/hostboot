/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/isteps/istep14/call_mss_power_cleanup.C $             */
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
#include <errl/errlentry.H>
#include <errl/errlmanager.H>
#include <errl/errludtarget.H>

#include <isteps/hwpisteperror.H>
#include <initservice/isteps_trace.H>

//  targeting support
#include <targeting/common/commontargeting.H>
#include <targeting/common/util.H>
#include <targeting/common/utilFilter.H>

#include    <config.h>
#include    <fapi2.H>
#include    <fapi2/plat_hwp_invoker.H>
#include    <p9_mss_power_cleanup.H>
#include    <p9c_mss_power_cleanup.H>

#ifdef CONFIG_NVDIMM
// NVDIMM support
#include    <isteps/nvdimm/nvdimm.H>
#endif

using   namespace   ISTEP;
using   namespace   ISTEP_ERROR;
using   namespace   ERRORLOG;
using   namespace   TARGETING;

namespace ISTEP_14
{
void* call_mss_power_cleanup (void *io_pArgs)
{
    errlHndl_t  l_err  =   NULL;
    IStepError  l_stepError;

    TRACDCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
            "call_mss_power_cleanup entry" );

    TARGETING::TargetHandleList l_mcbistTargetList;
    getAllChiplets(l_mcbistTargetList, TYPE_MCBIST);

    for (const auto & l_target : l_mcbistTargetList)
    {
        // Dump current run on target
        TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                "Running mss_power_cleanup HWP on "
                "target HUID %.8X",
                TARGETING::get_huid(l_target));

        fapi2::Target <fapi2::TARGET_TYPE_MCBIST> l_fapi_target
            (l_target);

        //  call the HWP with each fapi2::Target
        FAPI_INVOKE_HWP(l_err, p9_mss_power_cleanup, l_fapi_target);

        if (l_err)
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                      "ERROR 0x%.8X: mss_power_cleanup HWP returns error",
                      l_err->reasonCode());

            // capture the target data in the elog
            ErrlUserDetailsTarget(l_target).addToLog(l_err);

            // Create IStep error log and cross reference to error that
            // occurred
            l_stepError.addErrorDetails( l_err );

            // Commit Error
            errlCommit( l_err, HWPF_COMP_ID );
        }
        else
        {
            TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                       "SUCCESS :  mss_power_cleanup HWP( )" );
        }
    }

#ifdef CONFIG_NVDIMM
    TARGETING::TargetHandleList l_procList;
    getAllChips(l_procList, TARGETING::TYPE_PROC, false);
    TARGETING::ATTR_MODEL_type l_chipModel =
            l_procList[0]->getAttr<TARGETING::ATTR_MODEL>();

    if(l_chipModel == TARGETING::MODEL_NIMBUS)
    {
        // Check for any NVDIMMs after the mss_power_cleanup
        TARGETING::TargetHandleList l_dimmTargetList;
        TARGETING::TargetHandleList l_nvdimmTargetList;
        getAllLogicalCards(l_dimmTargetList, TYPE_DIMM);

        // Walk the dimm list and collect all the nvdimm targets
        for (auto const l_dimm : l_dimmTargetList)
        {
            if (TARGETING::isNVDIMM(l_dimm))
            {
                l_nvdimmTargetList.push_back(l_dimm);
            }
        }

        // Run the nvdimm management function if the list is not empty
        if (!l_nvdimmTargetList.empty()){
            NVDIMM::nvdimm_restore(l_nvdimmTargetList);
        }
    }
#endif

    // -- Cumulus only
    // Get a list of all present Centaurs
    TargetHandleList l_presCentaurs;
    getAllChips(l_presCentaurs, TYPE_MEMBUF);
    // For each present Centaur
    for (TargetHandleList::const_iterator
            l_cenIter = l_presCentaurs.begin();
            l_cenIter != l_presCentaurs.end();
            ++l_cenIter)
    {
        // Make a local copy of the target for ease of use
        TARGETING::Target * l_pCentaur = *l_cenIter;
        // Retrieve HUID of current Centaur
        TARGETING::ATTR_HUID_type l_currCentaurHuid =
            TARGETING::get_huid(l_pCentaur);

        // Find all present MBAs associated with this Centaur
        TARGETING::TargetHandleList l_presMbas;
        getChildChiplets(l_presMbas, l_pCentaur,TYPE_MBA);

        // If not at least two MBAs found
        if (l_presMbas.size() < 2)
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
              "Not enough MBAs found for Centaur target HUID %.8X, "
              "skipping this Centaur.",
               l_currCentaurHuid);
            continue;
        }

        // Cache current MBA HUIDs for tracing
        TARGETING::ATTR_HUID_type l_currMBA0Huid =
                    TARGETING::get_huid(l_presMbas[0]);
        TARGETING::ATTR_HUID_type l_currMBA1Huid =
                    TARGETING::get_huid(l_presMbas[1]);

        TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                "Running mss_power_cleanup HWP on "
                "Centaur HUID %.8X, MBA0 HUID %.8X, "
                "MBA1 HUID %.8X, ", l_currCentaurHuid,
                        l_currMBA0Huid, l_currMBA1Huid);

        // Create FAPI Targets.
        fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP> l_fapiCentaurTarget( l_pCentaur );
        fapi2::Target<fapi2::TARGET_TYPE_MBA_CHIPLET> l_fapiMba0Target( l_presMbas[0] );
        fapi2::Target<fapi2::TARGET_TYPE_MBA_CHIPLET> l_fapiMba1Target( l_presMbas[1] );

        //  Call the HWP with each fapi::Target
        FAPI_INVOKE_HWP(l_err, p9c_mss_power_cleanup, l_fapiCentaurTarget,
                        l_fapiMba0Target, l_fapiMba1Target);

        if (l_err)
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                      "mss_power_cleanup HWP failed to perform"
                      " cleanup on centaur: 0x%.8X HWP_ERROR: 0x%.8X",
                      l_currCentaurHuid,l_err->reasonCode());
            // Capture the target data in the error log
            ErrlUserDetailsTarget(l_pCentaur).addToLog(l_err);
            // Create IStep error log and cross reference error that occurred
            l_stepError.addErrorDetails(l_err);
            // Commit error
            errlCommit(l_err, HWPF_COMP_ID);
        }
        else
        {
            // Success
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                    "Successfully ran mss_power_cleanup HWP on "
                    "Centaur HUID %.8X, MBA0 HUID %.8X, "
                    "MBA1 HUID %.8X, ", l_currCentaurHuid,
                           l_currMBA0Huid, l_currMBA1Huid);
        }
    }
    TRACDCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
            "call_mss_power_cleanup exit" );

    // end task, returning any errorlogs to IStepDisp
    return l_stepError.getErrorHandle();
}

};
