/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/generic/memory/lib/utils/find.H $                  */
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
/// @file find.H
/// @brief Templates for finding things
///
// *HWP HWP Owner: Andre Marin <aamarin@us.ibm.com>
// *HWP HWP Backup: Stephen Glancy <sglancy@us.ibm.com>
// *HWP Team: Memory
// *HWP Level: 3
// *HWP Consumed by: HB:FSP

#ifndef _MSS_FIND_H
#define _MSS_FIND_H

#include <fapi2.H>
#include <vector>
#include <generic/memory/lib/utils/pos.H>
#include <generic/memory/lib/utils/c_str.H>

namespace mss
{
///
/// @brief find a set of elements based on a fapi2 target
/// @tparam M the target type to be returned
/// @tparam T the fapi2 target type of the argument
/// @param[in] i_target the fapi2 target T
/// @param[in] i_state [optional] fapi2 target state (defaults to TARGET_STATE_FUNCTIONAL)
/// @return a vector of M targets.
/// @note Only works for valid parent-child relationships
/// So calling find_targets<TARGET_TYPE_DIMM>(l_mca) will work here
/// but calling find_targets<TARGET_TYPE_DIMM>(l_mcs) will not work
/// Compiler will freak out and we'll never get a bad relationship/ error at runtime
/// If we do, it's on fapi2
///
template< fapi2::TargetType M, fapi2::TargetType T >
inline std::vector< fapi2::Target<M> > find_targets( const fapi2::Target<T>& i_target,
        fapi2::TargetState i_state = fapi2::TARGET_STATE_FUNCTIONAL )
{
    return i_target.template getChildren<M>(i_state);
}

///
/// @brief find an element based on a fapi2 target
/// @tparam M the target type to be returned
/// @tparam T the fapi2 target type of the argument
/// @param[in] i_target the fapi2 target T
/// @return an M target.
/// @note Only works for valid parent-child relationships
/// Will work for MCA and DIMM
/// Will not work for MCS and DIMM
/// The compiler will let you know if it doesn't work
///
template< fapi2::TargetType M, fapi2::TargetType T >
inline fapi2::Target<M> find_target( const fapi2::Target<T>& i_target )
{
    return i_target.template getParent<M>();
}

///
/// @brief find the union of functionl targets and any magic targets
/// @note The PHY has a logic block which is only contained in the 0th PHY in the controller.
/// This makes the 0th PHY 'magic' in that it needs to always be present if not functional.
/// This function returns all functional targets and includes the magic target whether or not
/// it is truly functional.
/// @tparam M the target type to be returned
/// @tparam T the fapi2 target type of the argument
/// @param[in] i_target the fapi2 target T
/// @return a vector of M targets.
///
template< fapi2::TargetType M, fapi2::TargetType T >
inline std::vector< fapi2::Target<M> > find_targets_with_magic( const fapi2::Target<T>& i_target);

///
/// @brief find a set of magic elements based on a fapi2 target
/// @note The PHY has a logic block which is only contained in the 0th PHY in the controller.
/// This makes the 0th PHY 'magic' in that it needs to always be present if not functional.
/// This function returns all magic targets whether or not it is truly functional.
/// It does not include other functional or present targets.
/// @tparam M the target type to be returned
/// @tparam T the fapi2 target type of the argument
/// @param[in] i_target the fapi2 target T
/// @return a vector of M targets.
///
template< fapi2::TargetType M, fapi2::TargetType T >
inline std::vector< fapi2::Target<M> > find_magic_targets( const fapi2::Target<T>& i_target);

///
/// @brief find the McBIST given a McBIST
/// @param[in] i_self the fapi2 target mcBIST
/// @return a McBIST target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_MCBIST> find_target( const fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_self)
{
    return i_self;
}

///
/// @brief find the MCS given an MCS
/// @param[in] i_self the fapi2 target MCS
/// @return a MCS target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_MCS> find_target( const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_self)
{
    return i_self;
}

///
/// @brief find the MEM_PORT given a MEM_PORT
/// @param[in] i_self the fapi2 target MEM_PORT
/// @return a MEM_PORT target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_MEM_PORT> find_target( const fapi2::Target<fapi2::TARGET_TYPE_MEM_PORT>&
        i_self)
{
    return i_self;
}

///
/// @brief find the OCMB_CHIP given a OCMB_CHIP
/// @param[in] i_self the fapi2 target OCMB_CHIP
/// @return a OCMB_CHIP target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_OCMB_CHIP> find_target( const fapi2::Target<fapi2::TARGET_TYPE_OCMB_CHIP>&
        i_self)
{
    return i_self;
}

///
/// @brief find the McBIST given a DIMM
/// @param[in] i_target the fapi2 target DIMM
/// @return a McBIST target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_MCBIST> find_target( const fapi2::Target<fapi2::TARGET_TYPE_DIMM>& i_target)
{
    return i_target.getParent<fapi2::TARGET_TYPE_MCA>().getParent<fapi2::TARGET_TYPE_MCBIST>();
}

///
/// @brief find the PROC_CHIP given a MBA
/// @param[in] i_target the fapi2 target MBA
/// @return a DMI target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> find_target( const fapi2::Target<fapi2::TARGET_TYPE_MBA>& i_target)
{
    return i_target.getParent<fapi2::TARGET_TYPE_MEMBUF_CHIP>()
           .getParent<fapi2::TARGET_TYPE_DMI>()
           .getParent<fapi2::TARGET_TYPE_PROC_CHIP>();
}

///
/// @brief find the PROC_CHIP given a OCMB_CHIP
/// @param[in] i_target the fapi2 target OCMB_CHIP
/// @return a PROC_CHIP target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> find_target( const fapi2::Target<fapi2::TARGET_TYPE_OCMB_CHIP>&
        i_target)
{
    return i_target.getParent<fapi2::TARGET_TYPE_OMI>()
           .getParent<fapi2::TARGET_TYPE_MC>()
           .getParent<fapi2::TARGET_TYPE_PROC_CHIP>();
}

///
/// @brief find the DMI given an MBA
/// @param[in] i_target the fapi2 target MBA
/// @return a DMI target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_DMI> find_target( const fapi2::Target<fapi2::TARGET_TYPE_MBA>& i_target)
{
    return i_target.getParent<fapi2::TARGET_TYPE_MEMBUF_CHIP>().getParent<fapi2::TARGET_TYPE_DMI>();
}

///
/// @brief find the DMI given a MEMBUF
/// @param[in] i_target the fapi2 target MEMBUF
/// @return a DMI target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_DMI> find_target( const fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP>& i_target)
{
    return i_target.getParent<fapi2::TARGET_TYPE_DMI>();
}

///
/// @brief find the PROC given a MEMBUF
/// @param[in] i_target the fapi2 target MEMBUF
/// @return a PROC target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> find_target( const fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP>&
        i_target)
{
    return i_target.getParent<fapi2::TARGET_TYPE_DMI>().getParent<fapi2::TARGET_TYPE_PROC_CHIP>();
}

///
/// @brief find all the DMIs connected to an PROC_CHIP
/// @param[in] i_target a fapi2::Target DMI
/// @return a vector of fapi2::TARGET_TYPE_MBA
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_DMI> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target,
              fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_DMI>(i_state);
}

///
/// @brief find all the OCMB_CHIPs connected to a PROC_CHIP
/// @param[in] i_target a fapi2::Target PROC_CHIP
/// @return a vector of fapi2::TARGET_TYPE_OCMB_CHIP
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_OCMB_CHIP> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target,
              fapi2::TargetState i_state )
{
    std::vector< fapi2::Target<fapi2::TARGET_TYPE_OCMB_CHIP> > l_ocmbs;

    for (const auto& l_mc : i_target.getChildren<fapi2::TARGET_TYPE_MC>(i_state))
    {
        for (const auto& l_omi : l_mc.getChildren<fapi2::TARGET_TYPE_OMI>(i_state))
        {
            auto l_these_ocmbs( l_omi.getChildren<fapi2::TARGET_TYPE_OCMB_CHIP>(i_state) );
            l_ocmbs.insert(l_ocmbs.end(), l_these_ocmbs.begin(), l_these_ocmbs.end());
        }
    }

    return l_ocmbs;
}

///
/// @brief find all the MEMBUFs connected to an DMI
/// @param[in] i_target a fapi2::Target DMI
/// @return a vector of fapi2::TARGET_TYPE_MEMBUF_CHIP
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_DMI>& i_target,
              fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_MEMBUF_CHIP>(i_state);
}

///
/// @brief find all the MBAs connected to an MEMBUF
/// @param[in] i_target a fapi2::Target MEMBUF_CHIP
/// @return a vector of fapi2::TARGET_TYPE_MBA
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MBA> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP>& i_target,
              fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_MBA>(i_state);
}

///
/// @brief find all the MBA connected to an DMI
/// @param[in] i_target a fapi2::Target DMI
/// @return a vector of fapi2::TARGET_TYPE_MBA
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MBA> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_DMI>& i_target,
              fapi2::TargetState i_state )
{
    std::vector< fapi2::Target<fapi2::TARGET_TYPE_MBA> > l_mbas;

    for (const auto& membuf_chip : i_target.getChildren<fapi2::TARGET_TYPE_MEMBUF_CHIP>(i_state))
    {
        auto l_these_mbas( membuf_chip.getChildren<fapi2::TARGET_TYPE_MBA>(i_state) );
        l_mbas.insert(l_mbas.end(), l_these_mbas.begin(), l_these_mbas.end());
    }

    return l_mbas;
}

///
/// @brief find all the DIMM connected to a centaur
/// @param[in] i_target a fapi2::Target TARGET_TYPE_MEMBUF_CHIP
/// @return a vector of fapi2::TARGET_TYPE_DIMM
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_DIMM> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP>& i_target,
              fapi2::TargetState i_state )
{
    std::vector< fapi2::Target<fapi2::TARGET_TYPE_DIMM> > l_dimms;

    for (const auto& l_mba : i_target.getChildren<fapi2::TARGET_TYPE_MBA>(i_state))
    {
        auto l_these_dimms( l_mba.getChildren<fapi2::TARGET_TYPE_DIMM>(i_state) );
        l_dimms.insert(l_dimms.end(), l_these_dimms.begin(), l_these_dimms.end());
    }

    return l_dimms;
}

///
/// @brief find all the dimm connected to an MCS
/// @param[in] i_target a fapi2::Target MCS
/// @return a vector of fapi2::TARGET_TYPE_DIMM
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_DIMM> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_target,
              fapi2::TargetState i_state )
{
    std::vector< fapi2::Target<fapi2::TARGET_TYPE_DIMM> > l_dimms;

    // At this time, fapi2 (cronus?) doesn't seem to recognize a DIMM is the child of an MCS.
    for (const auto& p : i_target.getChildren<fapi2::TARGET_TYPE_MCA>(i_state))
    {
        auto l_these_dimms( p.getChildren<fapi2::TARGET_TYPE_DIMM>(i_state) );
        l_dimms.insert(l_dimms.end(), l_these_dimms.begin(), l_these_dimms.end());
    }

    return l_dimms;
}

///
/// @brief find all the dimms connected to an MCBIST
/// @param[in] i_target a fapi2::Target MCBIST
/// @return a vector of fapi2::TARGET_TYPE_DIMM
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_DIMM> >
find_targets( const fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_target,
              fapi2::TargetState i_state )
{
    std::vector< fapi2::Target<fapi2::TARGET_TYPE_DIMM> > l_dimms;

    for (const auto& p : i_target.getChildren<fapi2::TARGET_TYPE_MCA>(i_state))
    {
        auto l_these_dimms( p.getChildren<fapi2::TARGET_TYPE_DIMM>(i_state) );
        l_dimms.insert(l_dimms.end(), l_these_dimms.begin(), l_these_dimms.end());
    }

    return l_dimms;
}

///
/// @brief find all the MCS connected to a PROC_CHIP
/// @param[in] i_target a fapi2::Target PROC_CHIP
/// @return a vector of fapi2::TARGET_TYPE_MCS
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCS> > find_targets
( const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target,
  fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_MCS>(i_state);
}

///
/// @brief find all the MCS connected to an MCBIST
/// @param[in] i_target a fapi2::Target MCBIST
/// @return a vector of fapi2::TARGET_TYPE_MCS
/// @note Cronus should support MCS children of an MCBIST - so this might be temporary
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCS> > find_targets
( const fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_target,
  fapi2::TargetState i_state )
{
    std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCS> > l_mcses;

    // At this time, fapi2 (cronus?) doesn't seem to recognize a MCS is the child of an MCBIST
    for (const auto& p : i_target.getChildren<fapi2::TARGET_TYPE_MCA>(i_state))
    {
        fapi2::Target<fapi2::TARGET_TYPE_MCS> l_mcs = p.getParent<fapi2::TARGET_TYPE_MCS>();

        if ( l_mcses.end() == std::find_if( l_mcses.begin(), l_mcses.end(),
                                            [l_mcs](const fapi2::Target<fapi2::TARGET_TYPE_MCS>& c)
    {
        return l_mcs == c;
    }) )
        {
            l_mcses.push_back(l_mcs);
        }
    }

    return l_mcses;
}

///
/// @brief find all the MCA connected to an MCBIST
/// @param[in] i_target a fapi2::Target MCBIST
/// @return a vector of fapi2::TARGET_TYPE_MCA
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCA> > find_targets
( const fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_target,
  fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_MCA>(i_state);
}

///
/// @brief find all the MCA connected to an MCA
/// @param[in] i_target a fapi2::Target MCA
/// @return a vector of fapi2::TARGET_TYPE_MCA
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCA> > find_targets
( const fapi2::Target<fapi2::TARGET_TYPE_MCA>& i_target,
  fapi2::TargetState i_state )
{
    // TODO - RTC:174905. Find out if we really need a find API that returns a vector of MCA from an MCA
    std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCA> > l_temp  = {i_target};
    return l_temp;
}

///
/// @brief find the magic MCA connected to an MCBIST
/// @param[in] i_target the fapi2::Target MCBIST
/// @return a vector of fapi2::TARGET_TYPE_MCA
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCA> > find_magic_targets
( const fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_target)
{
    // The magic port is in position 0, relative to the MCBIST
    constexpr uint64_t RELATIVE_MAGIC_POS = 0;

    // This is only one magic MCA on every MCBIST, so we only return a vector of one
    std::vector<fapi2::Target<fapi2::TARGET_TYPE_MCA>> l_magic_ports;

    // Get all the present MCA children and find the target with the relative position of 0
    for (const auto& p : i_target.getChildren<fapi2::TARGET_TYPE_MCA>(fapi2::TARGET_STATE_PRESENT))
    {
        if (mss::relative_pos<fapi2::TARGET_TYPE_MCBIST>(p) == RELATIVE_MAGIC_POS)
        {
            l_magic_ports.push_back(p);
        }
    }

    // We don't care if the vector is empty. We don't know what the caller will do with this
    // and they might not care if there is no magic port either ...
    return l_magic_ports;
}

///
/// @brief find the union of functionl targets and any magic targets
/// @param[in] i_target the fapi2::Target MCBIST
/// @return a vector of i2::Target<fapi2::TARGET_TYPE_MCA>
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCA> > find_targets_with_magic
( const fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_target)
{
    // We need the union of the functional target list and the magic target list. We can
    // get a little tricky with the MCA's - we know there's only one magic port.
    // So if the one magic port isn't in the list of functional ports, add it
    auto l_magic_ports = find_magic_targets<fapi2::TARGET_TYPE_MCA>(i_target);

    if (l_magic_ports.size() != 1)
    {
        FAPI_ERR("Found wrong number of magic ports on %s (%d)", mss::c_str(i_target), l_magic_ports.size());
        fapi2::Assert(false);
    }

    auto l_ports = mss::find_targets<fapi2::TARGET_TYPE_MCA>(i_target);
    const auto l_magic_pos = mss::relative_pos<fapi2::TARGET_TYPE_MCBIST>(l_magic_ports[0]);
    const auto l_magic_port = std::find_if(l_ports.begin(), l_ports.end(),
                                           [&l_magic_pos](const fapi2::Target<fapi2::TARGET_TYPE_MCA>& t)
    {
        // Check ports by relative position.
        const auto l_pos = mss::relative_pos<fapi2::TARGET_TYPE_MCBIST>(t);
        FAPI_DBG("checking for magic port at %d candidate is %d", l_magic_pos, l_pos);
        return l_magic_pos == l_pos;
    });

    if (l_magic_port == l_ports.end())
    {
        // Add the magic port to the front of the port vector.
        FAPI_DBG("inserting magic port %d", l_magic_pos);
        l_ports.insert(l_ports.begin(), l_magic_ports[0]);
    }

    // In either case, l_ports is the proper thing to return. Either the magic port was in
    // l_ports or it is now because we inserted it.
    return l_ports;
}

///
/// @brief find all the MCA connected to an MCS
/// @param[in] i_target a fapi2::Target MCS
/// @return a vector of fapi2::TARGET_TYPE_MCA
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCA> > find_targets
( const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_target,
  fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_MCA>(i_state);
}

///
/// @brief find all the DIMM connected to an MCA
/// @param[in] i_target a fapi2::Target MCA
/// @return a vector of fapi2::TARGET_TYPE_DIMM
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_DIMM> > find_targets
( const fapi2::Target<fapi2::TARGET_TYPE_MCA>& i_target,
  fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_DIMM>(i_state);
}

///
/// @brief find the MCS given a DIMM
/// @param[in] i_target the fapi2 target DIMM
/// @return a MCS target.
///
template<>
inline fapi2::Target<fapi2::TARGET_TYPE_MCS> find_target( const fapi2::Target<fapi2::TARGET_TYPE_DIMM>& i_target)
{
    return i_target.getParent<fapi2::TARGET_TYPE_MCA>().getParent<fapi2::TARGET_TYPE_MCS>();
}

///
/// @brief find all the MCBISTs connected to a PROC_CHIP
/// @param[in] i_target a fapi2::Target PROC_CHIP
/// @return a vector of fapi2::TARGET_TYPE_MCBIST
///
template<>
inline std::vector< fapi2::Target<fapi2::TARGET_TYPE_MCBIST> > find_targets(
    const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target,
    fapi2::TargetState i_state )
{
    return i_target.getChildren<fapi2::TARGET_TYPE_MCBIST>(i_state);
}

///
/// @brief find a key value from a vector of STL pairs
/// @tparam T  input type
/// @tparam OT the output type to be returned
/// @param[in] i_vector_of_pairs the input vector of pairs
/// @param[in] i_key the "map" key
/// @param[out] o_value the value found from given key
/// @return true if value is found, false otherwise
/// @note VECTOR NEEDS TO BE IN SORTED ORDER
///
template<typename T, typename OT>
bool find_value_from_key(const std::vector<std::pair<T, OT> >& i_vector_of_pairs,
                         const T& i_key,
                         OT& o_value)
{
    // Comparator lambda expression
    const auto compare = [](const std::pair<T, OT>& i_lhs, const T & i_key)
    {
        return (i_lhs.first < i_key);
    };

    // Find iterator to matching key (if it exists)
    const auto l_value_iterator  =  std::lower_bound(i_vector_of_pairs.begin(),
                                    i_vector_of_pairs.end(),
                                    i_key,
                                    compare);

    // Did you find it? Let me know.
    if( (l_value_iterator == i_vector_of_pairs.end()) || (i_key != l_value_iterator->first) )
    {
        // Static cast ensures that the below print will not fail at compile time
        FAPI_INF("Did not find a mapping value to key: %d", static_cast<uint64_t>(i_key));
        return false;
    }

    o_value = l_value_iterator->second;
    return true;

}// find_value_from_key

///
/// @brief find a key value from a vector of STL pairs
/// @tparam T  input type
/// @tparam OT the output type to be returned
/// @param[in] i_vector_of_pairs the input vector of pairs
/// @param[in] i_value the "map" value, the second entry in the pairs
/// @param[out] o_key the first entry in the pair
/// @return true if value is found, false otherwise
///
template<typename T, typename OT>
bool find_key_from_value(const std::vector<std::pair<T, OT> >& i_vector_of_pairs,
                         const OT& i_value,
                         T& o_key)
{
    // Comparator lambda expression
    const auto compare = [&i_value](const std::pair<T, OT>& i_lhs)
    {
        return (i_lhs.second == i_value);
    };

    // Find iterator to matching key (if it exists)
    const auto l_value_iterator  =  std::find_if(i_vector_of_pairs.begin(),
                                    i_vector_of_pairs.end(),
                                    compare);

    // Did you find it? Let me know.
    if( (l_value_iterator == i_vector_of_pairs.end()) || (i_value != l_value_iterator->second) )
    {
        // Static cast ensures that the below print will not fail at compile time
        FAPI_INF("Did not find a mapping key to value: %d", static_cast<uint64_t>(i_value));
        return false;
    }

    o_key = l_value_iterator->first;
    return true;

}// find_value_from_key

///
/// @brief find a key value from a C-style array of STL pairs
/// @tparam T  input type
/// @tparam OT the output type to be returned
/// @tparam N size of the array being passed in
/// @param[in] i_array the input array of pairs
/// @param[in] i_key the "map" key
/// @param[in] o_value the value found from given key
/// @return true if value is found, false otherwise
/// @note To use on short arrays. O(N), simple search
///
template<typename T, typename OT, size_t N>
bool find_value_from_key( const std::pair<T, OT> (&i_array)[N],
                          const T& i_key,
                          OT& o_value)
{
    // TK Use sort and binary search for larger arrays
    for (size_t i = 0; i < N; i++)
    {
        if (i_array[i].first == i_key)
        {
            o_value = i_array[i].second;
            return true;
        }
    }

    FAPI_ERR ("No match found for find_value_from_key");
    return false;
}

///
/// @brief Determine if a thing is functional
/// @tparam P, the type of the parent which holds the things of interest
/// @tparam I, the type of the item we want to check for
/// @param[in] i_target the parent containing the thing we're looking for
/// @param[in] i_rel_pos the relative position of the item of interest.
/// @return bool true iff the thing at i_rel_pos is noted as functional
///
template< fapi2::TargetType I, fapi2::TargetType P >
bool is_functional( const fapi2::Target<P>& i_target, const uint64_t i_rel_pos )
{
    // Not sure of a good way to do this ... we get all the functional
    // children of the parent and look for our relative position ...
    for (const auto& i : i_target.template getChildren<I>(fapi2::TARGET_STATE_FUNCTIONAL))
    {
        if (mss::template relative_pos<P>(i) == i_rel_pos)
        {
            return true;
        }
    }

    return false;
}

}// mss

#endif
