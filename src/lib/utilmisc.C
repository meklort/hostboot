/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/utilmisc.C $                                          */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2018                        */
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
#include <util/misc.H>
#include <arch/ppc.H>

namespace Util
{

bool isSimics() __attribute__((alias("__isSimicsRunning")));
extern "C" void __isSimicsRunning() NEVER_INLINE;

void __isSimicsRunning()
{
    asm volatile("li 3, 0");
    MAGIC_INSTRUCTION(MAGIC_SIMICS_CHECK);
}

bool isSimicsRunning()
{
    static bool simics = isSimics();
    return simics;
}

static bool g_isTargetingLoaded = false;

bool isTargetingLoaded()
{
    return g_isTargetingLoaded;
}

void setIsTargetingLoaded()
{
    g_isTargetingLoaded = true;
}

static bool g_isConsoleStarted = false;

bool isConsoleStarted()
{
    return g_isConsoleStarted;
}

void setIsConsoleStarted()
{
    g_isConsoleStarted = true;
}

};

