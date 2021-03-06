/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/utils/imageProcs/p9_tor.C $               */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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

#ifdef WIN32
    #include "endian.h"
#else
    #include <endian.h>
#endif

#include "p9_tor.H"
#include "p9_scan_compression.H"
#include "p9_infrastruct_help.H"


///////////////////////////////////////////////////////////////////////////////////
//
//                          GET RING FROM SECTION   FUNCTION
//
//////////////////////////////////////////////////////////////////////////////////
static
int get_ring_from_ring_section( void*           i_ringSection,     // Ring section ptr
                                RingId_t        i_ringId,          // Ring ID
                                RingVariant_t   i_ringVariant,     // Base,CC,RL (SBE,CME,SGPE only)
                                uint8_t&        io_instanceId,     // Instance ID
                                RingBlockType_t i_ringBlockType,   // Single ring, Block
                                void**          io_ringBlockPtr,   // Output ring buffer
                                uint32_t&       io_ringBlockSize,  // Size of ring data
                                char*           o_ringName,        // Name of ring
                                uint32_t        i_dbgl )           // Debug option
{
    int               rc = TOR_SUCCESS;
    uint8_t           iInst, iRing, iVariant;
    TorHeader_t*      torHeader;
    uint32_t          torMagic;
    uint8_t           torVersion;
    uint8_t           chipType;
    TorCpltBlock_t*   cpltBlock;
    TorCpltOffset_t   cpltOffset; // Offset from ringSection to chiplet section
    TorRingOffset_t   ringOffset; // Offset to actual ring container
    uint32_t          torSlotNum; // TOR slot number (within a chiplet section)
    uint32_t          ringSize;   // Size of whole ring container/block.
    RingVariantOrder* ringVariantOrder;
    RingId_t          numRings;
    GenRingIdList*    ringIdListCommon;
    GenRingIdList*    ringIdListInstance;
    GenRingIdList*    ringIdList;
    uint8_t           bInstCase = 0;
    ChipletData_t*    cpltData;
    uint8_t           numVariants;
    ChipletType_t     numChiplets;
    RingProperties_t* ringProps;

    torHeader = (TorHeader_t*)i_ringSection;
    torMagic  = be32toh(torHeader->magic);
    torVersion = torHeader->version;
    chipType  = torHeader->chipType;

    rc = ringid_get_noof_chiplets( chipType,
                                   torMagic,
                                   &numChiplets);

    if (rc)
    {
        MY_ERR("ringid_get_noof_chiplets() failed w/rc=0x%08x\n", rc);
        return rc;
    }

    //
    // Looper for each SBE chipleti
    //
    for (ChipletType_t iCplt = 0; iCplt < numChiplets; iCplt++)
    {
        rc = ringid_get_properties( chipType,
                                    torMagic,
                                    torHeader->version,
                                    iCplt,
                                    &cpltData,
                                    &ringIdListCommon,
                                    &ringIdListInstance,
                                    &ringVariantOrder,
                                    &ringProps,
                                    &numVariants );

        if (rc)
        {
            MY_ERR("ringid_get_properties() failed w/rc=0x%08x\n", rc);
            return rc;
        }

        //
        // Sequentially traverse ring offset slots within a chiplet's CMN or INST section
        //
        for ( bInstCase = 0; bInstCase <= 1; bInstCase++ )
        {
            numRings = bInstCase ? cpltData->iv_num_instance_rings : cpltData->iv_num_common_rings;
            ringIdList = bInstCase ? ringIdListInstance : ringIdListCommon;

            // Adjust number of variants according to TOR version of image
            if (torVersion < 7)
            {
                // Nothing to do. Number of variants is the same for Common and Instance rings.
            }
            else
            {
                numVariants = bInstCase ? 1 : numVariants; // Only BASE variant for Instance rings
            }

            if (ringIdList) // Only proceed if chiplet has [Common/Instance] rings.
            {
                // Calc offset to chiplet's CMN or INST section, cpltOffset (steps 1-3)
                //
                // 1. Calc offset to TOR slot pointing to chiplet's COM or INST section
                cpltOffset = sizeof(TorHeader_t) +
                             iCplt * sizeof(TorCpltBlock_t) +
                             bInstCase * sizeof(cpltBlock->cmnOffset);
                // 2. Retrive offset, endian convert and make it relative to ring section origin
                cpltOffset = *(uint32_t*)( (uint8_t*)i_ringSection + cpltOffset );
                cpltOffset = be32toh(cpltOffset);
                // 3. Make offset relative to ring section origin
                cpltOffset = sizeof(TorHeader_t) + cpltOffset;

                torSlotNum = 0;

                for ( iInst = ringIdList->instanceIdMin;
                      iInst <= ringIdList->instanceIdMax;
                      iInst++ )
                {
                    for ( iRing = 0; iRing < numRings; iRing++ )
                    {
                        for ( iVariant = 0; iVariant < numVariants; iVariant++ )
                        {
                            if ( strcmp( (ringIdList + iRing)->ringName,
                                         ringProps[i_ringId].iv_name ) == 0 &&
                                 ( i_ringVariant == ringVariantOrder->variant[iVariant] ||
                                   numVariants == 1 ) &&  // If no variants, ignore i_ringVariant and assume "BASE" ring
                                 ( !bInstCase || ( bInstCase && iInst == io_instanceId ) ) )
                            {
                                strcpy(o_ringName, (ringIdList + iRing)->ringName);

                                // Calc offset to actual ring, ringOffset (steps 1-3)
                                //
                                // 1. Calc offset to TOR slot pointing to actual ring
                                ringOffset = cpltOffset + torSlotNum * sizeof(ringOffset);
                                // 2. Retrieve offset and endian convert
                                ringOffset = *(TorRingOffset_t*)( (uint8_t*)i_ringSection + ringOffset );
                                ringOffset = be16toh(ringOffset);

                                if (i_ringBlockType == GET_SINGLE_RING)
                                {
                                    ringSize = 0;

                                    if (ringOffset)
                                    {
                                        // 3. Make offset relative to ring section origin
                                        ringOffset = cpltOffset + ringOffset;

                                        ringSize = be16toh( ((CompressedScanData*)
                                                             ((uint8_t*)i_ringSection + ringOffset))->iv_size );

                                        if (io_ringBlockSize == 0)
                                        {
                                            if (i_dbgl > 0)
                                            {
                                                MY_DBG("io_ringBlockSize is zero. Returning required size.\n");
                                            }

                                            io_ringBlockSize =  ringSize;
                                            return TOR_SUCCESS;
                                        }

                                        if (io_ringBlockSize < ringSize)
                                        {
                                            MY_ERR("io_ringBlockSize is less than required size.\n");
                                            return TOR_BUFFER_TOO_SMALL;
                                        }

                                        // Produce return parms
                                        memcpy( *io_ringBlockPtr, (uint8_t*)i_ringSection + ringOffset, ringSize);
                                        io_ringBlockSize = ringSize;
                                        io_instanceId = (bInstCase) ? io_instanceId : (ringIdList + iRing)->instanceIdMin;

                                        if (i_dbgl > 0)
                                        {
                                            MY_DBG("Found a ring:\n" \
                                                   "  Name: %s\n" \
                                                   "  Blocksize: %d\n",
                                                   o_ringName, io_ringBlockSize);
                                        }

                                        rc = TOR_SUCCESS;
                                    }
                                    else
                                    {
                                        if (i_dbgl > 0)
                                        {
                                            MY_DBG("Ring %s was not found.\n", o_ringName);
                                        }

                                        rc = TOR_RING_NOT_FOUND;
                                    }

                                    if (i_dbgl > 0)
                                    {
                                        MY_DBG("Details for chiplet ring index=%d: \n"
                                               "  Full offset to chiplet section = 0x%08x \n"
                                               "  Full offset to RS4 header = 0x%08x \n"
                                               "  Ring size = 0x%08x \n",
                                               iRing, cpltOffset, ringOffset, ringSize);
                                    }

                                    return rc;

                                }
                                else if (i_ringBlockType == PUT_SINGLE_RING)
                                {
                                    if (ringOffset)
                                    {
                                        MY_ERR("Ring container is already present in image\n");
                                        MY_ERR("  Ring section addr: 0x%016lx  (First 8B: 0x%016lx)\n",
                                               (uintptr_t)i_ringSection,
                                               be64toh(*((uint64_t*)i_ringSection)));
                                        MY_ERR("  cpltOffset=0x%08x, torSlotNum=0x%x, TOR offset=0x%04x\n",
                                               cpltOffset, torSlotNum, ringOffset);
                                        return TOR_RING_AVAILABLE_IN_RINGSECTION;
                                    }

                                    // Special [mis]use of io_ringBlockPtr and io_ringBlockSize:
                                    // Put location of chiplet's CMN or INST section into ringBlockPtr
                                    memcpy( *io_ringBlockPtr, &cpltOffset, sizeof(cpltOffset));
                                    // Put location of ringOffset slot into ringBlockSize
                                    io_ringBlockSize = cpltOffset + (torSlotNum * sizeof(ringOffset));

                                    return TOR_SUCCESS;
                                }
                                else
                                {
                                    MY_ERR("Ring block type (i_ringBlockType=%d) is not supported\n", i_ringBlockType);
                                    return TOR_INVALID_RING_BLOCK_TYPE;
                                }
                            }

                            torSlotNum++; // Next TOR ring slot
                        }
                    }
                }
            }
            else // Since there's no Common/Instance rings, set RING_NOT_FOUND
            {
                // Note that if we get here, it's because the chiplet doesn't have either
                // a Common or Instance rings. This happens e.g. for Centaur which has
                // no Instance rings. And theoretically, it's possible to only have
                // Instance rings and no Common rings, so accommodating that as well here.
                if (i_dbgl > 0)
                {
                    MY_DBG("Chiplet=%d has no CMN(%d) or INST(%d) section\n",
                           iCplt, (1 - bInstCase), bInstCase);
                }

                rc = TOR_RING_NOT_FOUND;

            } // if (ringIdList)
        } // for (bInstCase)
    } // for (iCplt)

    if (i_dbgl > 0)
    {
        MY_DBG("i_ringId=0x%x is an invalid ring ID\n", i_ringId);
    }

    return TOR_INVALID_RING_ID;

} // End of get_ring_from_ring_section()



//////////////////////////////////////////////////////////////////////////////////////////
///
///                            TOR ACCESS RING   API
///
//////////////////////////////////////////////////////////////////////////////////////////
int tor_access_ring(  void*           i_ringSection,     // Ring section ptr
                      RingId_t        i_ringId,          // Ring ID
                      uint8_t         i_ddLevel,         // DD level
                      PpeType_t       i_ppeType,         // SBE,CME,SGPE
                      RingVariant_t   i_ringVariant,     // Base,CC,RL (SBE,CME,SGPE only)
                      uint8_t&        io_instanceId,     // Instance ID
                      RingBlockType_t i_ringBlockType,   // GET_SINGLE_RING,GET_PPE_LEVEL_RINGS,etc
                      void**          io_ringBlockPtr,   // Ring data buffer
                      uint32_t&       io_ringBlockSize,  // Size of ring data
                      char*           o_ringName,        // Ring name
                      uint32_t        i_dbgl )           // Debug option
{
    int rc = 0;
    uint32_t       torMagic;
    TorHeader_t*   torHeader;
    uint8_t* postHeaderStart = (uint8_t*)i_ringSection + sizeof(TorHeader_t);

    if (i_dbgl > 1)
    {
        MY_DBG("Entering tor_access_ring()...\n");
    }

    torHeader = (TorHeader_t*)i_ringSection;
    torMagic = be32toh(torHeader->magic);

    if (i_dbgl > 0)
    {
        MY_DBG("TOR header fields\n"
               "  magic:         0x%08x\n"
               "  version:       %d\n"
               "  chipType:      %d\n"
               "  ddLevel:       0x%x\n"
               "  size:          %d\n"
               "API parms\n"
               "  i_ddLevel:     0x%x\n"
               "  i_ppeType:     %d\n"
               "  i_ringVariant: %d\n",
               torMagic, torHeader->version, torHeader->chipType,
               torHeader->ddLevel,
               be32toh(torHeader->size),
               i_ddLevel, i_ppeType, i_ringVariant);

        MY_DBG("Dump of first 12 quad-word lines in ring section\n");

        for (uint8_t iLine = 0; iLine < 12; iLine++)
        {
            MY_DBG("%04x: %04x %04x %04x %04x %04x %04x %04x %04x\n",
                   iLine * 16,
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 0)) >> 48),
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 0)) >> 32),
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 0)) >> 16),
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 0)) >>  0),
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 1)) >> 48),
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 1)) >> 32),
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 1)) >> 16),
                   (uint16_t)( be64toh(*((uint64_t*)i_ringSection + 2 * iLine + 1)) >>  0));
        }
    }

    if ( torMagic >> 8 != TOR_MAGIC ||
         torHeader->version > TOR_VERSION || // Code cannot be forward compatible to a newer image
         torHeader->version == 0 ||
         torHeader->chipType >= NUM_CHIP_TYPES )
    {
        MY_ERR("Invalid TOR header:\n"
               "  magic:       0x%08x\n"
               "  version:     %d\n"
               "  chipType:    %d\n"
               "  ddLevel:     0x%x  (requested ddLevel=0x%x)\n"
               "  size:        %d\n",
               torMagic, torHeader->version, torHeader->chipType,
               torHeader->ddLevel, i_ddLevel,
               be32toh(torHeader->size));
        return TOR_INVALID_MAGIC_NUMBER;
    }

    if ( i_ddLevel != torHeader->ddLevel &&
         i_ddLevel != UNDEFINED_DD_LEVEL )
    {
        MY_ERR("Requested DD level (=0x%x) doesn't match TOR header DD level (=0x%x) nor UNDEFINED_DD_LEVEL (=0x%x) \n",
               i_ddLevel, torHeader->ddLevel, UNDEFINED_DD_LEVEL);
        return TOR_DD_LEVEL_NOT_FOUND;
    }

    if ( i_ringBlockType == GET_SINGLE_RING ||       // All Magics support GET
         ( i_ringBlockType == PUT_SINGLE_RING  &&    // Can only append to SBE,CME,SGPE
           ( torMagic == TOR_MAGIC_SBE ||
             torMagic == TOR_MAGIC_CME ||
             torMagic == TOR_MAGIC_SGPE ) ) )
    {
        void* l_ringSection = i_ringSection;

        if ( torMagic == TOR_MAGIC_HW )
        {
            // Update l_ringSection:
            // Extract the offset to the specified ppeType's ring section TOR header and update l_ringSection
            TorPpeBlock_t*  torPpeBlock;
            torPpeBlock = (TorPpeBlock_t*)(postHeaderStart + i_ppeType * sizeof(TorPpeBlock_t));
            l_ringSection = (void*)(postHeaderStart + be32toh(torPpeBlock->offset));
        }

        rc =  get_ring_from_ring_section( l_ringSection,
                                          i_ringId,
                                          i_ringVariant,
                                          io_instanceId,
                                          i_ringBlockType,
                                          io_ringBlockPtr,
                                          io_ringBlockSize,
                                          o_ringName,
                                          i_dbgl );

        return rc;
    }

    else if ( i_ringBlockType == GET_PPE_LEVEL_RINGS &&
              torMagic == TOR_MAGIC_HW &&
              (i_ppeType == PT_SBE || i_ppeType == PT_CME || i_ppeType == PT_SGPE) )
    {
        TorPpeBlock_t*  torPpeBlock;
        uint32_t ppeSize;

        torPpeBlock = (TorPpeBlock_t*)(postHeaderStart + i_ppeType * sizeof(TorPpeBlock_t));
        ppeSize = be32toh(torPpeBlock->size);

        if (io_ringBlockSize >= ppeSize)
        {
            memcpy( (uint8_t*)(*io_ringBlockPtr),
                    postHeaderStart + be32toh(torPpeBlock->offset),
                    ppeSize );
            io_ringBlockSize = ppeSize;

            return TOR_SUCCESS;
        }
        else if (io_ringBlockSize == 0)
        {
            if (i_dbgl > 0)
            {
                MY_DBG("io_ringBlockSize is zero. Returning required size.\n");
            }

            io_ringBlockSize =  ppeSize;

            return TOR_SUCCESS;
        }
        else
        {
            MY_ERR("io_ringBlockSize is less than required size.\n");

            return TOR_BUFFER_TOO_SMALL;
        }
    }
    else
    {
        MY_ERR("Ambiguity on input parms to tor_access_ring():\n" \
               "  Possibly invalid torMagic (=0x%08x)\n" \
               "  Possibly incompatible ringBlockType (=%d)\n" \
               "  Possibly unsupported ppeType (=%d)\n" \
               "  Note that we don't care about ppeType for non-HW TOR ring sections\n",
               i_ringBlockType, torMagic, i_ppeType);

        return TOR_AMBIGUOUS_API_PARMS;
    }

    MY_ERR("Code bug in tor_access_ring(): Should never be here\n");

    return TOR_CODE_BUG;

} // End of tor_access_ring()



/////////////////////////////////////////////////////////////////////////////////////
//
//                             TOR GET SINGLE RING   API
//
/////////////////////////////////////////////////////////////////////////////////////
int tor_get_single_ring ( void*         i_ringSection,     // Ring section ptr
                          uint8_t       i_ddLevel,         // DD level
                          RingId_t      i_ringId,          // Ring ID
                          PpeType_t     i_ppeType,         // SBE, CME, SGPE
                          RingVariant_t i_ringVariant,     // Base,CC,RL (SBE/CME/SGPE only)
                          uint8_t       i_instanceId,      // Instance ID
                          void**        io_ringBlockPtr,   // Output ring buffer
                          uint32_t&     io_ringBlockSize,  // Size of ring data
                          uint32_t      i_dbgl )           // Debug option
{

    uint32_t rc;
    char i_ringName[MAX_RING_NAME_LENGTH];

    if (i_dbgl > 1)
    {
        MY_DBG("Entering tor_get_single_ring()...\n");
    }

    rc = tor_access_ring( i_ringSection,
                          i_ringId,
                          i_ddLevel,
                          i_ppeType,
                          i_ringVariant,
                          i_instanceId,
                          GET_SINGLE_RING,
                          io_ringBlockPtr,
                          io_ringBlockSize,
                          i_ringName,
                          i_dbgl );

    if (i_dbgl > 1)
    {
        MY_DBG("Exiting tor_get_single_ring() (ringBlockSize=%d)...\n", io_ringBlockSize );
    }

    return rc;
}



////////////////////////////////////////////////////////////////////////////////////////
//
//                            TOR GET BLOCK OF RINGS   API
//
///////////////////////////////////////////////////////////////////////////////////////
int tor_get_block_of_rings ( void*           i_ringSection,     // Ring section ptr
                             uint8_t         i_ddLevel,         // DD level
                             PpeType_t       i_ppeType,         // SBE,CME,SGPE
                             RingVariant_t   i_ringVariant,     // Base,CC,RL
                             void**          io_ringBlockPtr,   // Output ring buffer
                             uint32_t&       io_ringBlockSize,  // Size of ring data
                             uint32_t        i_dbgl )           // Debug option
{
    uint32_t rc = 0;
    uint8_t      l_instanceId;
    char i_ringName[MAX_RING_NAME_LENGTH];
    uint32_t     torMagic;
    ChipType_t   chipType = UNDEFINED_CHIP_TYPE;
    TorHeader_t* torHeader;

    if (i_dbgl > 1)
    {
        MY_DBG("Entering tor_get_block_of_rings()...\n");
    }

    torHeader = (TorHeader_t*)i_ringSection;
    torMagic  = be32toh(torHeader->magic);
    chipType  = torHeader->chipType;

    if ( torMagic == TOR_MAGIC_HW && chipType != CT_CEN )
    {
        if (i_ppeType == PT_SBE || i_ppeType == PT_CME || i_ppeType == PT_SGPE)
        {
            // Get specific PPE block of rings
            rc = tor_access_ring( i_ringSection,
                                  UNDEFINED_RING_ID,
                                  i_ddLevel,
                                  i_ppeType,
                                  i_ringVariant,
                                  l_instanceId,
                                  GET_PPE_LEVEL_RINGS,
                                  io_ringBlockPtr,
                                  io_ringBlockSize,
                                  i_ringName,
                                  i_dbgl );
        }
        else
        {
            MY_ERR("tor_get_block_of_rings(): Ambiguous API parameters\n");
            return TOR_AMBIGUOUS_API_PARMS;
        }
    }
    else
    {
        MY_ERR("tor_get_block_of_rings(): Only the P9 HW ring section is supported. However, torMagic=0x%08x and chipType=%d\n",
               torMagic, chipType);
        return TOR_UNSUPPORTED_RING_SECTION;
    }

    if (i_dbgl > 1)
    {
        MY_DBG("Exiting tor_get_block_of_rings() (ringBlockSize=%d)...\n", io_ringBlockSize);
    }

    return rc;
}



////////////////////////////////////////////////////////////////////////////////////////
//
//                             TOR APPEND RING   API
//
///////////////////////////////////////////////////////////////////////////////////////
int tor_append_ring(  void*           i_ringSection,      // Ring section ptr
                      uint32_t&       io_ringSectionSize, // In: Exact size of ring section.
                      // Out: Updated size of ring section.
                      void*           i_ringBuffer,       // Ring work buffer
                      const uint32_t  i_ringBufferSize,   // Max size of ring work buffer
                      RingId_t        i_ringId,           // Ring ID
                      PpeType_t       i_ppeType,          // SBE, CME, SGPE
                      RingVariant_t   i_ringVariant,      // Base,CC,RL
                      uint8_t         i_instanceId,       // Instance ID
                      void*           i_rs4Container,     // RS4 ring container
                      uint32_t        i_dbgl )            // Debug option
{
    uint32_t   rc = 0;
    char       i_ringName[MAX_RING_NAME_LENGTH];
    uint32_t   l_buf = 0;
    uint32_t*  l_cpltSection = &l_buf;
    uint32_t   l_ringBlockSize;
    uint16_t   l_ringOffset16;
    uint32_t   l_torOffsetSlot;

    rc = tor_access_ring( i_ringSection,
                          i_ringId,
                          UNDEFINED_DD_LEVEL,
                          i_ppeType,
                          i_ringVariant,
                          i_instanceId,
                          PUT_SINGLE_RING,
                          (void**)&l_cpltSection, // On return, contains offset (wrt ringSection) of
                          // chiplet section's common or instance section
                          l_torOffsetSlot,        // On return, contains offset (wrt ringSection) of
                          // TOR offset slot
                          i_ringName,
                          i_dbgl);

    if (rc)
    {
        if (i_dbgl > 0)
        {
            MY_ERR("tor_append_ring() failed in call to tor_access_ring w/rc=0x%x \n", rc);
        }

        return rc;
    }

    if (i_dbgl > 1)
    {
        MY_INF(" TOR offset slot for ring address %d \n", l_torOffsetSlot );
    }

    // Explanation to the following:
    // tor_append_ring() appends a ring to the end of ringSection. The offset value to
    // that ring is wrt the beginning of the chiplet's TOR section. Below we calculate
    // the offset value and put it into the TOR slot. But first, check that the offset
    // value can be contained within the 2B of the TOR slot.
    if ( (io_ringSectionSize - *l_cpltSection) <= MAX_TOR_RING_OFFSET )
    {
        l_ringOffset16 = htobe16(io_ringSectionSize - *l_cpltSection);
        memcpy( (uint8_t*)i_ringSection + l_torOffsetSlot,
                &l_ringOffset16, sizeof(l_ringOffset16) );
    }
    else
    {
        MY_ERR("Code bug: TOR ring offset (=0x%x) exceeds MAX_TOR_RING_OFFSET (=0x%x)",
               io_ringSectionSize - *l_cpltSection, MAX_TOR_RING_OFFSET);
        return TOR_OFFSET_TOO_BIG;
    }

    // Now append the ring to the end of ringSection.
    l_ringBlockSize = be16toh( ((CompressedScanData*)i_rs4Container)->iv_size );
    memcpy( (uint8_t*)i_ringSection + io_ringSectionSize,
            (uint8_t*)i_rs4Container, (size_t)l_ringBlockSize);

    // Update the ringSectionSize
    io_ringSectionSize += l_ringBlockSize;

    // Update also the size in the TOR header
    TorHeader_t* torHeader = (TorHeader_t*)i_ringSection;
    torHeader->size = htobe32(be32toh(torHeader->size) + l_ringBlockSize);

    return TOR_SUCCESS;
}


//
// Inform caller of TOR version.
//
uint8_t tor_version( void)
{
    return (uint8_t)TOR_VERSION;
}
