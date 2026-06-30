#ifndef H_MACHINEINFO_H
#define H_MACHINEINFO_H

#include <types.h>

unsigned CMachineInfo_AllocateDMAChannel(unsigned nChannel);
void CMachineInfo_FreeDMAChannel(unsigned nChannel);

#endif
