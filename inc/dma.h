#ifndef INC_DMA_H
#define INC_DMA_H

#include <types.h>
#include <base.h>

#define DMA_CHANNEL_MAX         12      // channels 0-12 are supported
#define DMA_CHANNEL__MASK       0x0F    // explicit channel number
#define DMA_CHANNEL_NONE        0x80    // returned if no channel available
#define DMA_CHANNEL_NORMAL      0x81    // normal DMA engine requested
#define DMA_CHANNEL_LITE        0x82    // lite (or normal) DMA engine requested

//
// DMA controller
//
// manual: "DMA Control And Status register"
#define ARM_DMA_BASE                (MMIO_BASE + 0x7000)
#define ARM_DMACHAN_CS(chan)        (ARM_DMA_BASE + ((chan) * 0x100) + 0x00)
    #define CS_RESET                        (1 << 31)
    #define CS_ABORT                        (1 << 30)
    #define CS_WAIT_FOR_OUTSTANDING_WRITES  (1 << 28)
    #define CS_PANIC_PRIORITY_SHIFT         20
        #define DEFAULT_PANIC_PRIORITY          15
    #define CS_PRIORITY_SHIFT               16
        #define DEFAULT_PRIORITY                1
    #define CS_ERROR                        (1 << 8)
    #define CS_WAITING_FOR_OUTSTANDING_WRITES (1 << 6)
    #define CS_DREQ_STOPS_DMA               (1 << 5)
    #define CS_PAUSED                       (1 << 4)
    #define CS_DREQ                         (1 << 3)
    #define CS_INT                          (1 << 2)
    #define CS_END                          (1 << 1)
    #define CS_ACTIVE                       (1 << 0)
#define ARM_DMACHAN_CONBLK_AD(chan) (ARM_DMA_BASE + ((chan) * 0x100) + 0x04)
#define ARM_DMACHAN_TI(chan)        (ARM_DMA_BASE + ((chan) * 0x100) + 0x08)    // "Transfer info"
    #define TI_PERMAP_SHIFT                 16
    #define TI_BURST_LENGTH_SHIFT           12
        #define DEFAULT_BURST_LENGTH            0
    #define TI_SRC_IGNORE                   (1 << 11)
    #define TI_SRC_DREQ                     (1 << 10)
    #define TI_SRC_WIDTH                    (1 << 9)
    #define TI_SRC_INC                      (1 << 8)
    #define TI_DEST_DREQ                    (1 << 6)
    #define TI_DEST_WIDTH                   (1 << 5)
    #define TI_DEST_INC                     (1 << 4)
    #define TI_WAIT_RESP                    (1 << 3)
    #define TI_TDMODE                       (1 << 1)
    #define TI_INTEN                        (1 << 0)
#define ARM_DMACHAN_SOURCE_AD(chan) (ARM_DMA_BASE + ((chan) * 0x100) + 0x0C)
#define ARM_DMACHAN_DEST_AD(chan)   (ARM_DMA_BASE + ((chan) * 0x100) + 0x10)
#define ARM_DMACHAN_TXFR_LEN(chan)  (ARM_DMA_BASE + ((chan) * 0x100) + 0x14)
    #define TXFR_LEN_XLENGTH_SHIFT          0
    #define TXFR_LEN_YLENGTH_SHIFT          16
    #define TXFR_LEN_MAX                    0x3FFFFFFF
    #define TXFR_LEN_MAX_LITE               0xFFFF          // fxl: 64K
#define ARM_DMACHAN_STRIDE(chan)    (ARM_DMA_BASE + ((chan) * 0x100) + 0x18)
    #define STRIDE_SRC_SHIFT                0
    #define STRIDE_DEST_SHIFT               16
#define ARM_DMACHAN_NEXTCONBK(chan) (ARM_DMA_BASE + ((chan) * 0x100) + 0x1C)
#define ARM_DMACHAN_DEBUG(chan)     (ARM_DMA_BASE + ((chan) * 0x100) + 0x20)
    #define DEBUG_LITE                  (1 << 28)
#define ARM_DMA_INT_STATUS          (ARM_DMA_BASE + 0xFE0)
#define ARM_DMA_ENABLE              (ARM_DMA_BASE + 0xFF0)

typedef enum TDREQ
{
    DREQSourceNone      = 0,
    DREQSourcePCMTX     = 2,
    DREQSourcePCMRX     = 3,
    DREQSourceSMI       = 4,
    DREQSourcePWM       = 5,
    DREQSourceSPITX     = 6,
    DREQSourceSPIRX     = 7,
    DREQSourceEMMC      = 11,
    DREQSourceUARTTX    = 12,
    DREQSourceUARTRX    = 14,
    DREQSourceHDMI      = 17
} TDREQ;

struct TDMAControlBlock
{
    uint32_t    nTransferInformation;
    uint32_t    nSourceAddress;
    uint32_t    nDestinationAddress;
    uint32_t    nTransferLength;
    uint32_t    n2DModeStride;
    uint32_t    nNextControlBlockAddress;
    uint32_t    nReserved[2];
} PACKED;

#define BUS_ADDRESS(addr)   (((addr) & ~0xC0000000) | GPU_MEM_BASE)
#define GPU_IO_BASE         0x7E000000

#define MaxCyclicBuffers    4

typedef void TDMACompletionRoutine (unsigned nChannel, unsigned nBuffer,
                boolean bStatus, void *pParam);

struct CDMAChannel {
    unsigned m_nChannel;
	uint8_t *m_pControlBlockBuffer[MaxCyclicBuffers];   // metadata for dma. to be allocated.
    struct TDMAControlBlock *m_pControlBlock[MaxCyclicBuffers];
    unsigned m_nBuffers;
    volatile unsigned m_nCurrentBuffer;
    const void *m_pBuffer[MaxCyclicBuffers];
    boolean m_bIRQConnected;
    TDMACompletionRoutine *m_pCompletionRoutine;
    void *m_pCompletionParam;
    boolean m_bStatus;
    uintptr_t m_nDestinationAddress;
    size_t m_nBufferLength;
};

void* malloc_dma30(unsigned size);
struct CDMAChannel *dma_init(unsigned nChannel);
void dma_deinit(struct CDMAChannel *chan);

void CDMAChannelSetupMemCopy(struct CDMAChannel *chan, void *pDestination,
            const void *pSource, size_t nLength,
            unsigned nBurstLength, boolean bCached);
void CDMAChannelSetupIORead(struct CDMAChannel *chan, void *pDestination, uintptr_t ulIOAddress, size_t nLength, TDREQ DREQ);
void CDMAChannelSetupIOWrite(struct CDMAChannel *chan, uintptr_t ulIOAddress, const void *pSource, size_t nLength, TDREQ DREQ);
void CDMAChannelSetupCyclicIOWrite(struct CDMAChannel *chan, uintptr_t ulIOAddress,
     const void *ppSources[], unsigned nBuffers, size_t ulLength, TDREQ DREQ);
void CDMAChannelSetupMemCopy2D(struct CDMAChannel *chan, void *pDestination,
    const void *pSource, size_t nBlockLength, unsigned nBlockCount,
    size_t nBlockStride, unsigned nBurstLength);
void CDMAChannelSetCompletionRoutine(struct CDMAChannel *chan, TDMACompletionRoutine *pRoutine, void *pParam);
void CDMAChannelStart(struct CDMAChannel *chan);
boolean CDMAChannelWait(struct CDMAChannel *chan);
boolean CDMAChannelGetStatus(struct CDMAChannel *chan);
void CDMAChannelCancel(struct CDMAChannel *chan);
void CDMAChannelInterruptHandler(struct CDMAChannel *chan);
void CDMAChannelInterruptStub (void *pParam);



#endif
