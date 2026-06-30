#define K2_DEBUG_INFO 1

#include <types.h>
#include <console.h>
#include <dma.h>

// Query machine related info
// Circle machineinfo.cpp. preserve the logic here for future rpi4 porting

enum TMachineModel
{
    MachineModelA,
    MachineModelBRelease1MB256,
    MachineModelBRelease2MB256,
    MachineModelBRelease2MB512,
    MachineModelAPlus,
    MachineModelBPlus,
    MachineModelZero,
    MachineModelZeroW,
    MachineModel2B,
    MachineModel3B,
    MachineModel3APlus,
    MachineModel3BPlus,
    MachineModelCM,
    MachineModelCM3,
    MachineModelCM3Plus,
    MachineModel4B,
    MachineModel400,
    MachineModelCM4,
    MachineModelUnknown
};

enum TSoCType
{
    SoCTypeBCM2835,
    SoCTypeBCM2836,
    SoCTypeBCM2837,
    SoCTypeBCM2711,
    SoCTypeUnknown
};


// channel bit set if channel is free
static unsigned short  m_usDMAChannelMap = 0x1F35;    // default mapping

// nChannel must be DMA_CHANNEL_NORMAL, _LITE, _EXTENDED or an explicit channel number
// returns the allocated channel number or DMA_CHANNEL_NONE on failure
unsigned CMachineInfo_AllocateDMAChannel(unsigned nChannel) {
    trace("in  : chan: 0x%x, map: 0x%x", nChannel, m_usDMAChannelMap);
    if (!(nChannel & ~DMA_CHANNEL__MASK)) {
        // explicit channel allocation
        assert(nChannel <= DMA_CHANNEL_MAX);
        if (m_usDMAChannelMap & (1 << nChannel)) {
            m_usDMAChannelMap &= ~(1 << nChannel);
            trace("out1: chan: 0x%x, map: 0x%x", nChannel, m_usDMAChannelMap);
            return nChannel;
        }
    } else {
        // arbitrary channel allocation
        int i = nChannel == DMA_CHANNEL_NORMAL ? 6 : DMA_CHANNEL_MAX;
        int nMin = 0;
        for (; i >= nMin; i--) {
            if (m_usDMAChannelMap & (1 << i)) {
                m_usDMAChannelMap &= ~(1 << i);
                trace("out2: chan: 0x%x, map: 0x%x", i, m_usDMAChannelMap);
                return (unsigned)i;
            }
        }
    }
    return DMA_CHANNEL_NONE;
}

void CMachineInfo_FreeDMAChannel (unsigned nChannel) {
    trace("CMachineInfo_FreeDMAChannel: chan: %d", nChannel);
    trace("chan map: 0x%x", m_usDMAChannelMap);
    assert (nChannel <= DMA_CHANNEL_MAX);
    assert (!(m_usDMAChannelMap & (1 << nChannel)));
    m_usDMAChannelMap |= 1 << nChannel;
}

