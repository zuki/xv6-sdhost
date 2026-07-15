//
// synchronize64.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2021  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <usb/synchronize.h>
#include <types.h>
#include <arm.h>
#include <console.h>

#define MAX_CRITICAL_LEVEL    20        // EnterCritical()の最大ネストレベル

unsigned current_execution_level(void)
{
    uint32_t flags;
    __asm__ __volatile__ ("mrs %0, daif" : "=r" (flags));

    if (flags & 0x40)
        return FIQ_LEVEL;

    if (flags & 0x80)
        return IRQ_LEVEL;

    return TASK_LEVEL;
}

#ifdef ARM_ALLOW_MULTI_CORE

static __volatile__ unsigned CRITICAL_LEBEL[CORES] = {0};
static __volatile__ uint32_t FLAGS[CORES][MAX_CRITICAL_LEVEL];

// 各CPU単位でMAX_CRITICAL_LEVEL(20)回までネスト可能
void enter_critical(unsigned level)
{
    assert (level == IRQ_LEVEL || level == FIQ_LEVEL);

    unsigned core = cpuid();

    uint32_t flags;
    __asm__ __volatile__ ("mrs %0, daif" : "=r" (flags));

    // 現在、同じか上位レベルにしか移行できない
    assert (level == FIQ_LEVEL || !(flags & 0x40));

    __asm__ __volatile__("msr DAIFSet, #3");    // IRQとFIQを共に無効に

    assert (CRITICAL_LEBEL[core] < MAX_CRITICAL_LEVEL);
    FLAGS[core][CRITICAL_LEBEL[core]++] = flags;

    if (level == IRQ_LEVEL)
        enable_fiq();

    dmb();
}

void leave_critical(void)
{
    unsigned core = cpuid();

    dmb();

    disable_fiq();

    assert (CRITICAL_LEBEL[core] > 0);
    uint32_t flags = FLAGS[core][--CRITICAL_LEBEL[core]];

    __asm__ __volatile__ ("msr daif, %0" :: "r" (flags));
}

#else

static __volatile__ unsigned CRITICAL_LEBEL = 0;
static __volatile__ uint32_t FLAGS[MAX_CRITICAL_LEVEL];

void enter_critical(unsigned level)
{
    assert (level == IRQ_LEVEL || level == FIQ_LEVEL);

    uint32_t flags;
    __asm__ __volatile__ ("mrs %0, daif" : "=r" (flags));

    // if we are already on FIQ_LEVEL, we must not go back to IRQ_LEVEL here
    assert (level == FIQ_LEVEL || !(flags & 0x40));

    __asm__ __volatile__ ("msr DAIFSet, #3");    // disable both IRQ and FIQ

    assert (CRITICAL_LEBEL < MAX_CRITICAL_LEVEL);
    FLAGS[CRITICAL_LEBEL++] = flags;

    if (level == IRQ_LEVEL)
    {
        enable_fiq();
    }

    dmb();
}

void leave_critical(void)
{
    dmb();

    disable_fiq();

    assert (CRITICAL_LEBEL > 0);
    uint32_t flags = FLAGS[--CRITICAL_LEBEL];

    __asm__ __volatile__ ("msr daif, %0" :: "r" (flags));
}

#endif

//
// Cache maintenance operations for ARMv8-A
//
// NOTE: The following functions should hold all variables in CPU registers. Currently this will be
//     ensured using the maximum optimation (see circle/synchronize64.h).
//
//     The following numbers can be determined (dynamically) using CTR_EL0, CSSELR_EL1, CCSIDR_EL1
//     and CLIDR_EL1. As long we use the Cortex-A53/A72 implementation in the BCM2837/BCM2711 these
//     static values will work:
//

#define SETWAY_LEVEL_SHIFT          1

#define L1_DATA_CACHE_SETS          128
#define L1_DATA_CACHE_WAYS          4
    #define L1_SETWAY_WAY_SHIFT         30  // 32-Log2(L1_DATA_CACHE_WAYS)
#define L1_DATA_CACHE_LINE_LENGTH   64
    #define L1_SETWAY_SET_SHIFT         6   // Log2(L1_DATA_CACHE_LINE_LENGTH)

#define L2_CACHE_SETS               512
#define L2_CACHE_WAYS               16
    #define L2_SETWAY_WAY_SHIFT         28  // 32-Log2(L2_CACHE_WAYS)
#define L2_CACHE_LINE_LENGTH        64
    #define L2_SETWAY_SET_SHIFT         6   // Log2(L2_CACHE_LINE_LENGTH)

void invalidate_data_cache(void)
{
    // invalidate L1 data cache
    for (unsigned set = 0; set < L1_DATA_CACHE_SETS; set++) {
        for (unsigned way = 0; way < L1_DATA_CACHE_WAYS; way++) {
            uint64_t waylevel = way << L1_SETWAY_WAY_SHIFT
                       | set << L1_SETWAY_SET_SHIFT
                       | 0 << SETWAY_LEVEL_SHIFT;

            __asm__ __volatile__ ("dc isw, %0" : : "r" (waylevel) : "memory");
        }
    }

    // invalidate L2 unified cache
    for (unsigned set = 0; set < L2_CACHE_SETS; set++) {
        for (unsigned way = 0; way < L2_CACHE_WAYS; way++) {
            uint64_t waylevel =   way << L2_SETWAY_WAY_SHIFT
                       | set << L2_SETWAY_SET_SHIFT
                       | 1 << SETWAY_LEVEL_SHIFT;

            __asm__ __volatile__ ("dc isw, %0" : : "r" (waylevel) : "memory");
        }
    }

    dsb();
}

void invalidate_data_cache_l1_only(void)
{
    // invalidate L1 data cache
    for (unsigned set = 0; set < L1_DATA_CACHE_SETS; set++) {
        for (unsigned way = 0; way < L1_DATA_CACHE_WAYS; way++) {
            uint64_t waylevel =   way << L1_SETWAY_WAY_SHIFT
                       | set << L1_SETWAY_SET_SHIFT
                       | 0 << SETWAY_LEVEL_SHIFT;

            __asm__ __volatile__ ("dc isw, %0" : : "r" (waylevel) : "memory");
        }
    }

    dsb();
}

void clean_data_cache (void)
{
    // clean L1 data cache
    for (unsigned set = 0; set < L1_DATA_CACHE_SETS; set++) {
        for (unsigned way = 0; way < L1_DATA_CACHE_WAYS; way++) {
            uint64_t waylevel =   way << L1_SETWAY_WAY_SHIFT
                       | set << L1_SETWAY_SET_SHIFT
                       | 0 << SETWAY_LEVEL_SHIFT;

            __asm__ __volatile__ ("dc csw, %0" : : "r" (waylevel) : "memory");
        }
    }

    // clean L2 unified cache
    for (unsigned set = 0; set < L2_CACHE_SETS; set++) {
        for (unsigned way = 0; way < L2_CACHE_WAYS; way++) {
            uint64_t waylevel =   way << L2_SETWAY_WAY_SHIFT
                       | set << L2_SETWAY_SET_SHIFT
                       | 1 << SETWAY_LEVEL_SHIFT;

            __asm__ __volatile__ ("dc csw, %0" : : "r" (waylevel) : "memory");
        }
    }

    dsb();
}

void invalidate_data_cache_range(uint64_t addr, uint64_t length)
{
    while (1) {
        __asm__ __volatile__ ("dc ivac, %0" : : "r" (addr) : "memory");

        if (length <= DATA_CACHE_LINE_LENGTH_MIN) {
            break;
        }

        addr += DATA_CACHE_LINE_LENGTH_MIN;
        length  -= DATA_CACHE_LINE_LENGTH_MIN;
    }

    dsb();
}

void clean_data_cache_range(uint64_t addr, uint64_t length)
{
    while (1) {
        __asm__ __volatile__ ("dc cvac, %0" : : "r" (addr) : "memory");

        if (length <= DATA_CACHE_LINE_LENGTH_MIN) {
            break;
        }

        addr += DATA_CACHE_LINE_LENGTH_MIN;
        length  -= DATA_CACHE_LINE_LENGTH_MIN;
    }

    dsb();
}

/// @brief データキャッシュaddr+lengthをクリアして無効化する
/// @param addr 開始データアドレス
/// @param length サイズ
void clean_and_invalidate_data_cache_range(uint64_t addr, uint64_t length)
{
    trace("1: addr=0x%016llx, length=0x%llx", addr, length);
    while (1) {
        __asm__ __volatile__ ("dc civac, %0" : : "r" (addr) : "memory");

        if (length <= DATA_CACHE_LINE_LENGTH_MIN) {
            break;
        }
        addr += DATA_CACHE_LINE_LENGTH_MIN;
        length  -= DATA_CACHE_LINE_LENGTH_MIN;
        trace("1-1: addr=0x%016llx, length=0x%016llx", addr, length);
    }

    dsb();
}

void sync_data_and_instruction_cache(void)
{
    clean_data_cache();
    //dsb();        // included in CleanDataCache()

    invalidate_instruction_cache();
    dsb();

    isb();
}
