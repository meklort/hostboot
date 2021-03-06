/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/isteps/istep08/call_proc_check_slave_sbe_seeprom_complete.C $ */
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
/**
 *  @file call_proc_check_slave_sbe_seeprom_complete.C
 *
 *  Support file for IStep: slave_sbe
 *   Slave SBE
 *
 *  HWP_IGNORE_VERSION_CHECK
 */

/******************************************************************************/
// Includes
/******************************************************************************/
#include <stdint.h>
#include <trace/interface.H>
#include <initservice/taskargs.H>
#include <errl/errlentry.H>
#include <initservice/isteps_trace.H>
#include <initservice/initserviceif.H>
#include <initservice/initsvcreasoncodes.H>
#include <sys/time.h>
#include <devicefw/userif.H>
#include <i2c/i2cif.H>
#include <sbe/sbeif.H>
#include <util/misc.H>
#include <ipmi/ipmiwatchdog.H>

//  targeting support
#include <targeting/common/commontargeting.H>
#include <targeting/common/utilFilter.H>
#include <targeting/namedtarget.H>
#include <targeting/attrsync.H>

#include <isteps/hwpisteperror.H>
#include <errl/errludtarget.H>

#include <fapi2/target.H>
#include <fapi2/plat_hwp_invoker.H>
#include <return_code.H>

#include <p9_extract_sbe_rc.H>
#include <p9_get_sbe_msg_register.H>
#include <p9_getecid.H>
#include <sbeio/sbe_retry_handler.H>
#include <sbeio/sbeioif.H>

using namespace ISTEP;
using namespace ISTEP_ERROR;
using namespace ERRORLOG;
using namespace TARGETING;
using namespace fapi2;

const uint64_t MS_TO_WAIT_FIRST = 2500; //(2.5 s)
const uint64_t MS_TO_WAIT_OTHERS= 100; //(100 ms)
namespace ISTEP_08
{

//******************************************************************************
// call_proc_check_slave_sbe_seeprom_complete function
//******************************************************************************
void* call_proc_check_slave_sbe_seeprom_complete( void *io_pArgs )
{
    errlHndl_t  l_errl(nullptr);
    IStepError  l_stepError;

    TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
               "call_proc_check_slave_sbe_seeprom_complete entry" );

    //
    //  get the master Proc target, we want to IGNORE this one.
    //
    TARGETING::Target* l_pMasterProcTarget = NULL;
    TARGETING::targetService().masterProcChipTargetHandle(l_pMasterProcTarget);

    //
    //  get a list of all the procs in the system
    //
    TARGETING::TargetHandleList l_cpuTargetList;
    getAllChips(l_cpuTargetList, TYPE_PROC);

    TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
        "proc_check_slave_sbe_seeprom_complete: %d procs in the system.",
        l_cpuTargetList.size() );

    // loop thru all the cpu's
    for (const auto & l_cpu_target: l_cpuTargetList)
    {
        if ( l_cpu_target  ==  l_pMasterProcTarget )
        {
            TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                       "Master SBE found, HUID %.8X, "
                       "skipping to look for Slave SBE's.",
                       TARGETING::get_huid(l_cpu_target));
            // we are just checking the Slave SBE's, skip the master
            continue;
        }

        TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                "Processor target HUID %.8X",
                TARGETING::get_huid(l_cpu_target));

        const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> l_fapi2ProcTarget(
                            const_cast<TARGETING::Target*> (l_cpu_target));

        TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                   "Running p9_get_sbe_msg_register HWP"
                   " on processor target %.8X",
                   TARGETING::get_huid(l_cpu_target));

        //Note no PLID passed in
        SBEIO::SbeRetryHandler l_SBEobj = SBEIO::SbeRetryHandler(
                SBEIO::SbeRetryHandler::SBE_MODE_OF_OPERATION::ATTEMPT_REBOOT);

        l_SBEobj.setSbeRestartMethod(SBEIO::SbeRetryHandler::
                                     SBE_RESTART_METHOD::START_CBS);

        // We want to tell the retry handler that we have just powered
        // on the sbe, to distinguish this case from other cases where
        // we have determine there is something wrong w/ the sbe and
        // want to diagnose the problem
        l_SBEobj.setInitialPowerOn(true);

        l_SBEobj.main_sbe_handler(l_cpu_target);

        // We will judge whether or not the SBE had a succesful
        // boot or not depending on if it made it to runtime or not
        if(l_SBEobj.isSbeAtRuntime())
        {
            // Set attribute indicating that SBE is started
            l_cpu_target->setAttr<ATTR_SBE_IS_STARTED>(1);

            // Make the FIFO call to get and apply the SBE Capabilities
            l_errl = SBEIO::getFifoSbeCapabilities(l_cpu_target);

            if (l_errl)
            {
                // Commit Error
                errlCommit (l_errl, ISTEP_COMP_ID);
            }

            // Switch to using SBE SCOM
            ScomSwitches l_switches =
                l_cpu_target->getAttr<ATTR_SCOM_SWITCHES>();
            ScomSwitches l_switches_before = l_switches;

            // Turn on SBE SCOM and turn off FSI SCOM.
            l_switches.useFsiScom = 0;
            l_switches.useSbeScom = 1;

            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                      "proc_check_slave_sbe_seeprom_complete: changing SCOM "
                      "switches from 0x%.2X to 0x%.2X for proc 0x%.8X",
                      l_switches_before,
                      l_switches,
                      TARGETING::get_huid(l_cpu_target));
            l_cpu_target->setAttr<ATTR_SCOM_SWITCHES>(l_switches);

            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                      "SUCCESS : proc_check_slave_sbe_seeprom_complete"
                      " completed ok for proc 0x%.8X",
                      TARGETING::get_huid(l_cpu_target));
        }
        else
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                      "FAILURE : proc_check_slave_sbe_seeprom_complete"
                      "SBE for proc 0x%.8X did not reach runtime",
                      TARGETING::get_huid(l_cpu_target));
           /*@
            * @reasoncode RC_FAILED_TO_BOOT_SBE
            * @severity   ERRORLOG::ERRL_SEV_UNRECOVERABLE
            * @moduleid   MOD_CHECK_SLAVE_SBE_SEEPROM_COMPLETE
            * @userdata1  HUID of proc that failed to boot its SBE
            * @userdata2  Unused
            * @devdesc    Failed to boot a slave SBE
            * @custdesc   Processor Error
            */
            l_errl = new ERRORLOG::ErrlEntry(ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                            MOD_CHECK_SLAVE_SBE_SEEPROM_COMPLETE,
                                            RC_FAILED_TO_BOOT_SBE,
                                            TARGETING::get_huid(l_cpu_target),
                                            0);
            l_errl->collectTrace( "ISTEPS_TRACE", 256 );
            l_stepError.addErrorDetails( l_errl);
            errlCommit(l_errl, ISTEP_COMP_ID);


        }

/* @TODO-RTC:100963 This should only be called when the SBE has completely
              crashed. There is a path in OpenPower where HB may get an
              attention and need to call it. For now, though, just associate
              with this story for tracking.

        // Not a way to pass in -soft_err, assuming that is default behavior
        TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                "Running p9_extract_sbe_rc HWP"
                " on processor target %.8X",
                  TARGETING::get_huid(l_cpu_target) );
**/
    }   // end of going through all processors


    //  Once the sbe's are up correctly, fetch all the proc ECIDs and
    //  store them in an attribute.
    for (const auto & l_cpu_target: l_cpuTargetList)
    {
      const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> l_fapi2ProcTarget(
                          const_cast<TARGETING::Target*> (l_cpu_target));

      TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
              "Running p9_getecid HWP"
              " on processor target %.8X",
              TARGETING::get_huid(l_cpu_target) );

      //  p9_getecid should set the fuse string to 112 bytes long.
      fapi2::variable_buffer  l_fuseString(112);

      // Invoke the HWP
      FAPI_INVOKE_HWP(l_errl,
                      p9_getecid,
                      l_fapi2ProcTarget,
                      l_fuseString  );

      if (l_errl)
      {
        if (l_cpu_target->getAttr<ATTR_HWAS_STATE>().functional)
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                    "ERROR : p9_getecid",
                    " failed, returning errorlog" );

            // capture the target data in the elog
            ErrlUserDetailsTarget(l_cpu_target).addToLog( l_errl );

            // Create IStep error log and cross reference error that
            // occurred
            l_stepError.addErrorDetails( l_errl );

            // Commit error log
            errlCommit( l_errl, HWPF_COMP_ID );
        }
        else // Not functional, proc deconfigured, don't report error
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                    "ERROR : p9_getecid"
                    " failed, proc target deconfigured" );
            delete l_errl;
            l_errl = NULL;
        }
      }
      else
      {
          TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                    "SUCCESS : proc_getecid"
                    " completed ok");

          // Update HDAT_EC to account for anything lower than the minor EC
          auto l_miniEC = l_cpu_target->getAttr<TARGETING::ATTR_MINI_EC>();
          if( l_miniEC != 0 )
          {
              auto l_hdatEC = l_cpu_target->getAttr<TARGETING::ATTR_HDAT_EC>();
              auto l_EC = l_cpu_target->getAttr<TARGETING::ATTR_EC>();
              auto l_newHdatEC = l_EC + l_miniEC;
              TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                         "MINI_EC=%d, HDAT_EC changing from %d->%d",
                         l_miniEC, l_hdatEC, l_newHdatEC );
              l_cpu_target->setAttr<TARGETING::ATTR_HDAT_EC>(l_newHdatEC);
          }
      }

    }  // end of going through all processors

    TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
              "call_proc_check_slave_sbe_seeprom_complete exit");

    // end task, returning any errorlogs to IStepDisp
    return l_stepError.getErrorHandle();
}

};
