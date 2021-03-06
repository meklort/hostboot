/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/common/plat/mem/prdfMemUtils.H $            */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2013,2018                        */
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

#ifndef prdfMemUtils_H
#define prdfMemUtils_H

/** @file  prdfMemUtils.H
 *  @brief General utility functions for memory
 */

//------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------

#include <vector>
#include <prdfMemSymbol.H>
#include <prdfParserEnums.H>

namespace PRDF
{

class ExtensibleChip;
struct STEP_CODE_DATA_STRUCT;

namespace MemUtils
{

//==============================================================================
//  Maintenance statistics
//==============================================================================

/**
 * @brief Collects CE symbol data.
 */
struct SymbolData
{
    MemSymbol symbol;
    uint8_t count;
    SymbolData() : count(0) {}
};

typedef std::vector<SymbolData> MaintSymbols;

/** Functor for MaintSymbols to sort by count. */
inline bool sortSymDataCount( const SymbolData & i, const SymbolData & j )
{
    return i.count < j.count;
}

/**
 * @brief  Queries the per symbol counters and returns a sorted list of symbols
 *         with a count greater than or equal to the given threshold.
 * @note   The returned list will be sorted from lowest count to highest count.
 * @param  i_chip        Target mba or mca chip.
 * @param  i_rank        Target rank.
 * @param  o_maintStats  Returns the list of symbols and counts.
 * @param  o_chipMark    This represents the DRAM in which a chip mark should be
 *                       placed, if necessary, based on the following:
 *                         - In x4 mode, this represents the DRAM with the
 *                           highest total per symbol count with at least 1
 *                           symbol that has exceeded threshold.
 *                         - In x8 mode, this represents the DRAM with the
 *                           highest total per symbol count with at least 2
 *                           symbols that have exceeded threshold.
 *                       It is possible that there will be symbols returned in
 *                       o_maintStats that do not meet the criteria stated
 *                       above. Therefore, the user must check if o_chipMark is
 *                       valid before using it.
 * @param  i_thr         The count threshold. Each symbol count must be greater
 *                       than or equal to this value to be added to the list.
 *                       The default is 1, which means all non-zero counts will
 *                       be added to the list. A value of 0 will result in a bad
 *                       return code.
 * @return Non-SUCCESS if an internal function fails, SUCCESS otherwise.
 */
template<TARGETING::TYPE T>
int32_t collectCeStats( ExtensibleChip * i_chip, const MemRank & i_rank,
                        MaintSymbols & o_maintStats, MemSymbol & o_chipMark,
                        uint8_t i_thr = 1 );

/**
 * @brief  Gets DRAM size for an MBA or MCA.
 * @param  i_chip     MBA or MCA chip.
 * @param  i_dimmSlct DIMM select. Optional for MBA chip.
 * @return size for a DRAM
 */
template<TARGETING::TYPE T>
uint8_t getDramSize( ExtensibleChip * i_chip, uint8_t i_dimmSlct = 0 );

/**
 * @brief  determines the type of Centaur based raw card  associated with MBA.
 * @param  i_mba    mba target
 * @param  o_type   raw card type.
 * @return Non-SUCCESS if an internal function fails, SUCCESS otherwise.
 * @note   Function is not intended for IS DIMM.
 */
int32_t getRawCardType( TARGETING::TargetHandle_t i_mba,
                        CEN_SYMBOL::WiringType & o_type );

/**
 * @brief  When handling attentions on a memory buffer, there will be FIR bits
 *         on the processor side of the bus that must be manually cleared in
 *         order to completely clear the attention.
 * @note   Intended to be called in the memory buffer PostAnalysis after the
 *         rule code has cleared the analyzed attention.
 * @param  i_chip MEMBUF chip.
 * @param  io_sc  The step code data struct.
 */
template<TARGETING::TYPE T>
void cleanupChnlAttns( ExtensibleChip * i_chip, STEP_CODE_DATA_STRUCT & io_sc );

/**
 * @brief  Intended to be called in the memory subsystem PreAnalysis plugins.
 *         Checks for channel fail attentions and does analysis as needed.
 * @note   In some cases, analysis will be completed within this function. In
 *         other cases, this function will set the secondary attention type to
 *         UNIT_CS and other appropriate flags in the SDC, then exit.
 *         Afterwards, we will rely on the normal analysis path to analyze the
 *         channel failure appropriately.
 * @note   If the secondary attention type is HOST_ATTN, the function exits and
 *         waits for the next attention. This is for the rare scenario where a
 *         channel failure occurs after PRD is called to handle a HOST_ATTN.
 * @param  i_chip MEMBUF, DMI, or MC chip.
 * @param  io_sc  The step code data struct.
 * @return True if analysis has been completed and no further analysis is
 *         required. False if the PreAnalysis plugin should exit and continue
 *         normal analysis.
 */
template<TARGETING::TYPE T>
bool analyzeChnlFail( ExtensibleChip * i_chip, STEP_CODE_DATA_STRUCT & io_sc );

/**
 * @brief  After analyzing a memory channel failure, we want to mask off all
 *         possible attentions on that channel to ensure we don't get any errant
 *         attentions.
 * @note   Intended to be called in the memory buffer PostAnalysis after the
 *         rule code has cleared the analyzed attention.
 * @param  i_chip MEMBUF, DMI, or MC chip.
 * @param  io_sc  The step code data struct.
 */
template<TARGETING::TYPE T>
void cleanupChnlFail( ExtensibleChip * i_chip, STEP_CODE_DATA_STRUCT & io_sc );

/**
 * @brief  Reverses the bit placement in the inputted bit string.
 * @param  i_val     The bit string to be reversed.
 * @param  i_numBits The number of bits in the bit string to reverse.
 * @return The reversed bit string value.
 */

uint64_t reverseBits( uint64_t i_val, uint64_t i_numBits );

} // end namespace MemUtils

} // end namespace PRDF

#endif
