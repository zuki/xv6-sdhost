
// dmachannel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2025  R. Stange <rsta2@gmx.net>

#define K2_DEBUG_INFO 1

#include <types.h>
#include <arm.h>
#include <dma.h>
#include <spinlock.h>
//#include "file.h"
#include <linux/fcntl.h>
#include <console.h>
#include <mm.h>
#include <string.h>
#include <machineinfo.h>
#include <irq.h>

#define DMA_CHANNELS            (DMA_CHANNEL_MAX + 1)


// 30-bit DMA-able memory (Circle HEAP_DMA30)
// per Circle "high memory region (memory >= 3 GB is not safe to be DMA-able and is not used)"
// rpi3 (1GB of mem) should be fine
void* malloc_dma30(unsigned size) {
    void *p = kmalloc(size);
    trace("malloc_dma30 returns %lx", (unsigned long)p);
    return p;
}

#define DATA_CACHE_LINE_LENGTH_MIN      64
#define DATA_CACHE_LINE_LENGTH_MAX      64

static void CleanAndInvalidateDataCacheRange (uint64_t nAddress, uint64_t nLength)
{
    trace("addrss: 0x%lx, len: %lu", nAddress, nLength);
    while (1)
    {
        __asm__ __volatile__ ("dc civac, %0" : : "r" (nAddress) : "memory");

        if (nLength <= DATA_CACHE_LINE_LENGTH_MIN) {
            break;
        }

        nAddress += DATA_CACHE_LINE_LENGTH_MIN;
        nLength  -= DATA_CACHE_LINE_LENGTH_MIN;
    }

    __asm__ __volatile__ ("dsb sy" ::: "memory");
}


struct CDMAChannel *dma_init(unsigned nChannel)
{
    trace("dma_init called");
    struct CDMAChannel *chan = (struct CDMAChannel *)malloc_dma30(sizeof (struct CDMAChannel));
    memset(chan, 0, sizeof(struct CDMAChannel));
    chan->m_nChannel = CMachineInfo_AllocateDMAChannel(nChannel);
    chan->m_nBuffers = 0;
    chan->m_bIRQConnected = false;
    chan->m_pCompletionRoutine = 0;
    chan->m_pCompletionParam = 0;
    chan->m_bStatus = false;

    //PeripheralEntry ();

    assert(chan->m_nChannel != DMA_CHANNEL_NONE);
    assert(chan->m_nChannel < DMA_CHANNELS);
    trace("dma chan: %d", chan->m_nChannel);
    for (unsigned i = 0; i < MaxCyclicBuffers; i++) {
        chan->m_pControlBlockBuffer[i] = malloc_dma30(sizeof(struct TDMAControlBlock) + 31);
        assert(chan->m_pControlBlockBuffer[i] != 0);
        chan->m_pControlBlock[i] = (struct TDMAControlBlock *) (((uintptr_t)chan->m_pControlBlockBuffer[i] + 31) & ~31);

        chan->m_pControlBlock[i]->nReserved[0] = 0;
        chan->m_pControlBlock[i]->nReserved[1] = 0;
    }

    put32(ARM_DMA_ENABLE, get32(ARM_DMA_ENABLE) | (1 << chan->m_nChannel));
    delayus(1000);

    put32(ARM_DMACHAN_CS (chan->m_nChannel), CS_RESET);
    while (get32(ARM_DMACHAN_CS (chan->m_nChannel)) & CS_RESET) {}
    //PeripheralExit ();

    return chan;
}

void dma_deinit(struct CDMAChannel *chan)
{
    //PeripheralEntry ();

    assert (chan->m_nChannel < DMA_CHANNELS);

    put32(ARM_DMACHAN_CS (chan->m_nChannel), CS_RESET);
    while (get32(ARM_DMACHAN_CS (chan->m_nChannel)) & CS_RESET)
    {
        // do nothing
    }

    put32(ARM_DMA_ENABLE, get32(ARM_DMA_ENABLE) & ~(1 << chan->m_nChannel));

    //PeripheralExit ();

    chan->m_pCompletionRoutine = 0;

    CMachineInfo_FreeDMAChannel(chan->m_nChannel);

    for (int i = 0; i < MaxCyclicBuffers; i++)
    {
        trace("free cb %d", i);
        chan->m_pControlBlock[i] = 0;
        kmfree(chan->m_pControlBlockBuffer[i]);
        chan->m_pControlBlockBuffer[i] = 0;
    }
}

void CDMAChannelSetupMemCopy(struct CDMAChannel *chan, void *pDestination,
            const void *pSource, size_t nLength,
            unsigned nBurstLength, boolean bCached)
{
    assert (pDestination != 0);
    assert (pSource != 0);
    assert (nLength > 0);
    assert (nBurstLength <= 15);

    assert (chan->m_pControlBlock[0] != 0);
    assert (nLength <= TXFR_LEN_MAX);
    assert ( !(get32(ARM_DMACHAN_DEBUG(chan->m_nChannel)) & DEBUG_LITE)
            || nLength <= TXFR_LEN_MAX_LITE);

    chan->m_pControlBlock[0]->nTransferInformation = (nBurstLength << TI_BURST_LENGTH_SHIFT)
        | TI_SRC_WIDTH
        | TI_SRC_INC
        | TI_DEST_WIDTH
        | TI_DEST_INC;
    chan->m_pControlBlock[0]->nSourceAddress           = BUS_ADDRESS((uintptr_t) pSource);
    chan->m_pControlBlock[0]->nDestinationAddress      = BUS_ADDRESS((uintptr_t) pDestination);
    chan->m_pControlBlock[0]->nTransferLength          = nLength;
    chan->m_pControlBlock[0]->n2DModeStride            = 0;
    chan->m_pControlBlock[0]->nNextControlBlockAddress = 0;

    if (bCached) {
        chan->m_nDestinationAddress = (uintptr_t) pDestination;
        chan->m_nBufferLength = nLength;

        CleanAndInvalidateDataCacheRange((uint64_t)pSource, nLength);
        CleanAndInvalidateDataCacheRange((uint64_t)pDestination, nLength);
    } else {
        chan->m_nDestinationAddress = 0;
    }

    chan->m_nBuffers = 1;
}

void CDMAChannelSetupIORead(struct CDMAChannel *chan, void *pDestination, uintptr_t ulIOAddress, size_t nLength, TDREQ DREQ)
{
    assert (pDestination != 0);
    assert (nLength > 0);
    assert (nLength <= TXFR_LEN_MAX);
    assert (   !(get32(ARM_DMACHAN_DEBUG (chan->m_nChannel)) & DEBUG_LITE)
        || nLength <= TXFR_LEN_MAX_LITE);

    ulIOAddress &= 0xFFFFFF;
    assert (ulIOAddress != 0);
    ulIOAddress += GPU_IO_BASE;

    assert (chan->m_pControlBlock[0] != 0);
    chan->m_pControlBlock[0]->nTransferInformation     =   (DREQ << TI_PERMAP_SHIFT)
                               | (DEFAULT_BURST_LENGTH << TI_BURST_LENGTH_SHIFT)
                               | TI_SRC_DREQ
                               | TI_DEST_WIDTH
                               | TI_DEST_INC
                               | TI_WAIT_RESP;
    chan->m_pControlBlock[0]->nSourceAddress           = ulIOAddress;
    chan->m_pControlBlock[0]->nDestinationAddress      = BUS_ADDRESS ((uintptr_t) pDestination);
    chan->m_pControlBlock[0]->nTransferLength          = nLength;
    chan->m_pControlBlock[0]->n2DModeStride            = 0;
    chan->m_pControlBlock[0]->nNextControlBlockAddress = 0;

    chan->m_nDestinationAddress = (uintptr_t) pDestination;
    chan->m_nBufferLength = nLength;

    CleanAndInvalidateDataCacheRange((uint64_t)pDestination, nLength);

    chan->m_nBuffers = 1;
}

void CDMAChannelSetupIOWrite(struct CDMAChannel *chan, uintptr_t ulIOAddress,
    const void *pSource, size_t nLength, TDREQ DREQ)
{
    assert (pSource != 0);
    assert (nLength > 0);
    assert (nLength <= TXFR_LEN_MAX);
    assert (   !(get32(ARM_DMACHAN_DEBUG (chan->m_nChannel)) & DEBUG_LITE)
        || nLength <= TXFR_LEN_MAX_LITE);

    ulIOAddress &= 0xFFFFFF;
    assert (ulIOAddress != 0);
    ulIOAddress += GPU_IO_BASE;

    assert (chan->m_pControlBlock[0] != 0);
    chan->m_pControlBlock[0]->nTransferInformation     =   (DREQ << TI_PERMAP_SHIFT)
                               | (DEFAULT_BURST_LENGTH << TI_BURST_LENGTH_SHIFT)
                               | TI_SRC_WIDTH
                               | TI_SRC_INC
                               | TI_DEST_DREQ
                               | TI_WAIT_RESP;
    chan->m_pControlBlock[0]->nSourceAddress           = BUS_ADDRESS ((uintptr_t) pSource);
    chan->m_pControlBlock[0]->nDestinationAddress      = ulIOAddress;
    chan->m_pControlBlock[0]->nTransferLength          = nLength;
    chan->m_pControlBlock[0]->n2DModeStride            = 0;
    chan->m_pControlBlock[0]->nNextControlBlockAddress = 0;

    chan->m_nDestinationAddress = 0;

    CleanAndInvalidateDataCacheRange((uint64_t)pSource, nLength);

    chan->m_nBuffers = 1;
}

void CDMAChannelSetupCyclicIOWrite (struct CDMAChannel *chan, uintptr_t ulIOAddress,
    const void *ppSources[], unsigned nBuffers, size_t ulLength, TDREQ DREQ)
{
    trace("chan: %p, ioaddr: 0x%lx, source[0]: %p, source[1]: %p",
        chan, ulIOAddress, ppSources[0], ppSources[1]);
    trace("  nb: %d, len: %lu", nBuffers, ulLength);
    assert (ppSources != 0);
    assert (ulLength > 0);
    assert (ulLength <= TXFR_LEN_MAX);
    assert (   !(get32(ARM_DMACHAN_DEBUG (chan->m_nChannel)) & DEBUG_LITE)
        || ulLength <= TXFR_LEN_MAX_LITE);

    ulIOAddress &= 0xFFFFFF;
    assert (ulIOAddress != 0);
    ulIOAddress += GPU_IO_BASE;
    trace("nBuffers: %d, ppSources[0]: %p, [1]: %p", nBuffers, ppSources[0], ppSources[1]);
    for (unsigned i = 0; i < nBuffers; i++)
    {
        if (!ppSources[i]) {
            warn("ppSources[%d] is null", i);
        }
        assert (ppSources[i]);
        assert (chan->m_pControlBlock[i] != 0);
        chan->m_pControlBlock[i]->nTransferInformation     =   (DREQ << TI_PERMAP_SHIFT)
                                   | (DEFAULT_BURST_LENGTH << TI_BURST_LENGTH_SHIFT)
                                   | TI_SRC_WIDTH
                                   | TI_SRC_INC
                                   | TI_DEST_DREQ
                                   | TI_WAIT_RESP;
        chan->m_pControlBlock[i]->nSourceAddress           = BUS_ADDRESS ((uintptr_t) ppSources[i]);
        chan->m_pControlBlock[i]->nDestinationAddress      = ulIOAddress;
        chan->m_pControlBlock[i]->nTransferLength          = ulLength;
        chan->m_pControlBlock[i]->n2DModeStride            = 0;

        if (nBuffers > 1) {
            chan->m_pControlBlock[i]->nNextControlBlockAddress =
                BUS_ADDRESS ((uintptr_t) chan->m_pControlBlock[i == nBuffers-1 ? 0 : i+1]);
        } else {
            chan->m_pControlBlock[i]->nNextControlBlockAddress = 0;
        }

        chan->m_nDestinationAddress = 0;

        CleanAndInvalidateDataCacheRange((uint64_t)ppSources[i], ulLength);

        chan->m_pBuffer[i] = ppSources[i];
    }

    chan->m_nBuffers = nBuffers;
    chan->m_nBufferLength = ulLength;
}

void CDMAChannelSetupMemCopy2D (struct CDMAChannel *chan, void *pDestination,
    const void *pSource, size_t nBlockLength, unsigned nBlockCount,
    size_t nBlockStride, unsigned nBurstLength)
{
    assert (pDestination != 0);
    assert (pSource != 0);
    assert (nBlockLength > 0);
    assert (nBlockLength <= 0xFFFF);
    assert (nBlockCount > 0);
    assert (nBlockCount <= 0x3FFF);
    assert (nBlockStride <= 0xFFFF);
    assert (nBurstLength <= 15);

    assert (!(get32(ARM_DMACHAN_DEBUG (chan->m_nChannel)) & DEBUG_LITE));

    assert (chan->m_pControlBlock[0] != 0);

    chan->m_pControlBlock[0]->nTransferInformation     =   (nBurstLength << TI_BURST_LENGTH_SHIFT)
                               | TI_SRC_WIDTH
                               | TI_SRC_INC
                               | TI_DEST_WIDTH
                               | TI_DEST_INC
                               | TI_TDMODE;
    chan->m_pControlBlock[0]->nSourceAddress           = BUS_ADDRESS ((uintptr_t) pSource);
    chan->m_pControlBlock[0]->nDestinationAddress      = BUS_ADDRESS ((uintptr_t) pDestination);
    chan->m_pControlBlock[0]->nTransferLength          =   ((nBlockCount-1) << TXFR_LEN_YLENGTH_SHIFT)
                               | (nBlockLength << TXFR_LEN_XLENGTH_SHIFT);
    chan->m_pControlBlock[0]->n2DModeStride            = nBlockStride << STRIDE_DEST_SHIFT;
    chan->m_pControlBlock[0]->nNextControlBlockAddress = 0;

    chan->m_nDestinationAddress = 0;

    CleanAndInvalidateDataCacheRange((uint64_t)pSource, nBlockLength*nBlockCount);

    chan->m_nBuffers = 1;
}

void CDMAChannelSetCompletionRoutine(struct CDMAChannel *chan, TDMACompletionRoutine *pRoutine, void *pParam)
{
    assert (chan->m_nChannel <= DMA_CHANNELS);

    if (!chan->m_bIRQConnected) {
        irq_register(chan->m_nChannel, CDMAChannelInterruptStub, chan);
        chan->m_bIRQConnected = true;
    }

    chan->m_pCompletionRoutine = pRoutine;
    assert (chan->m_pCompletionRoutine != 0);
    chan->m_pCompletionParam = pParam;
}


void CDMAChannelStart (struct CDMAChannel *chan)
{
    for (unsigned i = 0; i < chan->m_nBuffers; i++)
    {
        assert (chan->m_pControlBlock[i] != 0);

        if (chan->m_pCompletionRoutine != 0) {
            chan->m_pControlBlock[i]->nTransferInformation |= TI_INTEN;
        }

        CleanAndInvalidateDataCacheRange((uint64_t)chan->m_pControlBlock[i],
            sizeof *chan->m_pControlBlock[i]);
    }

    //PeripheralEntry ();

    assert (chan->m_nChannel < DMA_CHANNELS);
    assert (!(get32(ARM_DMACHAN_CS (chan->m_nChannel)) & CS_INT));
    assert (!(get32(ARM_DMA_INT_STATUS) & (1 << chan->m_nChannel)));

    chan->m_nCurrentBuffer = 0;
    put32(ARM_DMACHAN_CONBLK_AD (chan->m_nChannel),
        BUS_ADDRESS ((uintptr_t) chan->m_pControlBlock[0]));

    put32(ARM_DMACHAN_CS (chan->m_nChannel),   CS_WAIT_FOR_OUTSTANDING_WRITES
                          | (DEFAULT_PANIC_PRIORITY << CS_PANIC_PRIORITY_SHIFT)
                          | (DEFAULT_PRIORITY << CS_PRIORITY_SHIFT)
                          | CS_ACTIVE);

    //PeripheralExit ();
}

boolean CDMAChannelWait (struct CDMAChannel *chan)
{
    assert (chan->m_nChannel < DMA_CHANNELS);
    assert (chan->m_pCompletionRoutine == 0);

    //PeripheralEntry ();

    uint32_t nCS;
    while ((nCS = get32(ARM_DMACHAN_CS (chan->m_nChannel))) & CS_ACTIVE)
    {
        // do nothing
    }

    chan->m_bStatus = nCS & CS_ERROR ? false : true;

    if (chan->m_nDestinationAddress != 0) {
        CleanAndInvalidateDataCacheRange((uint64_t)chan->m_nDestinationAddress,
            chan->m_nBufferLength);
    }

    //PeripheralExit ();

    return chan->m_bStatus;
}

boolean CDMAChannelGetStatus(struct CDMAChannel *chan)
{
    assert (chan->m_nChannel < DMA_CHANNELS);
    assert (!(get32(ARM_DMACHAN_CS (chan->m_nChannel)) & CS_ACTIVE));

    return chan->m_bStatus;
}

void CDMAChannelCancel(struct CDMAChannel *chan)
{
    //PeripheralEntry ();

    assert (chan->m_nChannel < DMA_CHANNELS);
    put32(ARM_DMACHAN_CS (chan->m_nChannel), 0);

    chan->m_pCompletionRoutine = 0;

    //PeripheralExit ();
}

void CDMAChannelInterruptHandler(struct CDMAChannel *chan)
{
    if (chan->m_nDestinationAddress != 0)
    {
        CleanAndInvalidateDataCacheRange((uint64_t)chan->m_nDestinationAddress,
            chan->m_nBufferLength);
    }

    //PeripheralEntry ();

    assert (chan->m_nChannel < DMA_CHANNELS);

#ifndef NDEBUG
    uint32_t nIntStatus = get32(ARM_DMA_INT_STATUS);
#endif
    uint32_t nIntMask = 1 << chan->m_nChannel;
    assert (nIntStatus & nIntMask);
    put32(ARM_DMA_INT_STATUS, nIntMask);

    uint32_t nCS = get32(ARM_DMACHAN_CS (chan->m_nChannel));
    assert (nCS & CS_INT);
    put32(ARM_DMACHAN_CS (chan->m_nChannel), nCS);

    //PeripheralExit ();

    chan->m_bStatus = nCS & CS_ERROR ? false : true;

    assert (chan->m_pCompletionRoutine != 0);
    TDMACompletionRoutine *pCompletionRoutine = chan->m_pCompletionRoutine;
    if (chan->m_nBuffers == 1)
    {
        chan->m_pCompletionRoutine = 0;
    }

    assert (chan->m_nCurrentBuffer < MaxCyclicBuffers);
    (*pCompletionRoutine)(chan->m_nChannel, chan->m_nCurrentBuffer, chan->m_bStatus, chan->m_pCompletionParam);

    if (   chan->m_bStatus
        && chan->m_nBuffers > 1)
    {
        assert (chan->m_nCurrentBuffer < chan->m_nBuffers);

        CleanAndInvalidateDataCacheRange((uint64_t)chan->m_pBuffer[chan->m_nCurrentBuffer],
            chan->m_nBufferLength);

        if (++chan->m_nCurrentBuffer == chan->m_nBuffers) {
            chan->m_nCurrentBuffer = 0;
        }
    }
}

void CDMAChannelInterruptStub (void *pParam)
{
    struct CDMAChannel *chan = (struct CDMAChannel *) pParam;
    assert (chan != 0);

    CDMAChannelInterruptHandler(chan);
}
