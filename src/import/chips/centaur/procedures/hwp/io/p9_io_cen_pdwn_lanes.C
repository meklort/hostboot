/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/centaur/procedures/hwp/io/p9_io_cen_pdwn_lanes.C $ */
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
///
/// @file p9_io_cen_pdwn_lanes.C
/// @brief P9 Centaur Power Down Lanes.
///----------------------------------------------------------------------------
/// *HWP HWP Owner        : Chris Steffen <cwsteffen@us.ibm.com>
/// *HWP HWP Backup Owner : Gary Peterson <garyp@us.ibm.com>
/// *HWP FW Owner         : Jamie Knight <rjknight@us.ibm.com>
/// *HWP Team             : IO
/// *HWP Level            : 2
/// *HWP Consumed by      : FSP:HB
///----------------------------------------------------------------------------
///
/// @verbatim
/// High-level procedure flow:
///
/// A HWP that powers down lanes.  This procedure should power down rx lanes,
///   tx lanes, or both rx & tx lanes.
///
/// Procedure Prereq:
///     - System clocks are running.
///
/// @endverbatim
///----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include "p9_io_cen_pdwn_lanes.H"
#include "p9_io_scom.H"
#include "p9_io_regs.H"

// ----------------------------------------------------------------------------
// Procedure Function
// ----------------------------------------------------------------------------
fapi2::ReturnCode rx_pdwn_lanes(
    const fapi2::Target< fapi2::TARGET_TYPE_MEMBUF_CHIP >& i_target,
    const std::vector< uint8_t >& i_bad_lanes);
fapi2::ReturnCode tx_pdwn_lanes(
    const fapi2::Target< fapi2::TARGET_TYPE_MEMBUF_CHIP >& i_target,
    const std::vector< uint8_t >& i_bad_lanes);


/**
 * @brief A HWP that powers down the specified lanes on a given EDI+ Centaur
 *   target.  The rx & tx lanes in the vectors will be powered down on the
 *   given target.  Note: This procedure does not power down any lanes on the
 *   connected target of the link.
 * @param[in] i_target        FAPI2 Target
 * @param[in] i_rx_bad_lanes  Vector of Rx Bad Lanes
 * @param[in] i_tx_bad_lanes  Vector of Tx Bad Lanes
 * @retval ReturnCode
 */
fapi2::ReturnCode p9_io_cen_pdwn_lanes(
    const fapi2::Target< fapi2::TARGET_TYPE_MEMBUF_CHIP >& i_target,
    const std::vector< uint8_t >&                   i_rx_bad_lanes,
    const std::vector< uint8_t >&                   i_tx_bad_lanes)
{
    FAPI_IMP("Entering...");

    FAPI_DBG("Rx Bad Lanes Size: %d", i_rx_bad_lanes.size());

    if(!i_rx_bad_lanes.empty())
    {
        FAPI_TRY(rx_pdwn_lanes(i_target, i_rx_bad_lanes),
                 "Rx Power Down Lanes Failed");
    }

    FAPI_DBG("Tx Bad Lanes Size: %d", i_tx_bad_lanes.size());

    if(!i_tx_bad_lanes.empty())
    {
        FAPI_TRY(tx_pdwn_lanes(i_target, i_tx_bad_lanes),
                 "Tx Power Down Lanes Failed");
    }

fapi_try_exit:
    FAPI_IMP("Exiting...");
    return fapi2::current_err;
}

/**
 * @brief A HWP that powers down the specified lanes on a given EDI+ Centaur
 *   target.  The rx lanes in the vector will be powered down on the
 *   given target.  Note: This procedure does not power down any lanes on the
 *   connected target of the link.
 * @param[in] i_target        FAPI2 Target
 * @param[in] i_bad_lanes     Vector of Bad Lanes
 * @retval ReturnCode
 */
fapi2::ReturnCode rx_pdwn_lanes(
    const fapi2::Target< fapi2::TARGET_TYPE_MEMBUF_CHIP >& i_target,
    const std::vector< uint8_t >&                   i_bad_lanes)
{
    const uint8_t GRP0 = 0;
    FAPI_IMP("rx_pdwn_lanes: Enter Size(%d)", i_bad_lanes.size());
    char target_string[fapi2::MAX_ECMD_STRING_LEN];
    fapi2::toString(i_target, target_string, fapi2::MAX_ECMD_STRING_LEN);

    if(!i_bad_lanes.empty())
    {
        for(uint8_t index = 0; index < i_bad_lanes.size(); ++index)
        {
            FAPI_DBG("Powering Down Rx Lane[%d/%d]: Target(%s:g%d:l%d)",
                     index,
                     i_bad_lanes.size() - 1,
                     target_string,
                     GRP0,
                     i_bad_lanes[index]);

            FAPI_TRY(io::rmw(EDI_RX_LANE_PDWN, i_target, GRP0, i_bad_lanes[index], 1),
                     "Failed rmw rx pdwn reg");

            // Set rx_wt_lane_disabled for this lane; see SW244284, SW280992
            FAPI_TRY(io::rmw(EDI_RX_WT_LANE_DISABLED, i_target, GRP0, i_bad_lanes[index], 1),
                     "Failed rmw rx wt lane disabled reg");


        }
    }

fapi_try_exit:
    FAPI_IMP("rx_pdwn_lanes: Exiting.");
    return fapi2::current_err;
}

/**
 * @brief A HWP that powers down the specified lanes on a given EDI+ Centaur
 *   target.  The tx lanes in the vector will be powered down on the
 *   given target.  Note: This procedure does not power down any lanes on the
 *   connected target of the link.
 * @param[in] i_target        FAPI2 Target
 * @param[in] i_bad_lanes     Vector of Bad Lanes
 * @retval ReturnCode
 */
fapi2::ReturnCode tx_pdwn_lanes(
    const fapi2::Target< fapi2::TARGET_TYPE_MEMBUF_CHIP >& i_target,
    const std::vector< uint8_t >&                   i_bad_lanes)
{
    FAPI_IMP("tx_pdwn_lanes: Enter Size(%d)", i_bad_lanes.size());
    const uint8_t GRP0 = 0;
    const uint8_t LN0 = 0;
    uint8_t l_msbswap = 0;
    uint8_t l_end_lane = 0;
    uint8_t l_lane = 0;
    uint64_t l_data = 0;
    char target_string[fapi2::MAX_ECMD_STRING_LEN];
    fapi2::toString(i_target, target_string, fapi2::MAX_ECMD_STRING_LEN);

    if(!i_bad_lanes.empty())
    {
        FAPI_TRY(io::read(EDI_TX_MSBSWAP, i_target, GRP0, LN0, l_data),
                 "Failed read edi_tx_msbswap");
        l_msbswap = io::get(EDI_TX_MSBSWAP, l_data);

        if(l_msbswap == 0x1)
        {
            FAPI_TRY(io::read(EDI_TX_END_LANE_ID, i_target, GRP0, LN0, l_data));
            l_end_lane = io::get(EDI_TX_END_LANE_ID, l_data);
            FAPI_DBG("edi_tx_msbswap: tx_end_lane_id(%d).", l_end_lane);
        }

        for(uint8_t index = 0; index < i_bad_lanes.size(); ++index)
        {
            l_lane = i_bad_lanes[index];

            if(l_msbswap == 0x1)
            {
                l_lane = l_end_lane - i_bad_lanes[index];
                FAPI_DBG("edi_tx_msbswap: tx_end_lane_id(%d) lane(%d -> %d).",
                         l_end_lane, i_bad_lanes[index], l_lane);
            }

            FAPI_DBG("Powering Down Tx Lane[%d/%d]: Target(%s:g%d:l%d)",
                     index,
                     i_bad_lanes.size() - 1,
                     target_string,
                     GRP0,
                     l_lane);

            FAPI_TRY(io::rmw(EDI_TX_LANE_PDWN, i_target, GRP0, l_lane, 1));

        }
    }

fapi_try_exit:
    FAPI_IMP("tx_pdwn_lanes: Exiting");
    return fapi2::current_err;
}
