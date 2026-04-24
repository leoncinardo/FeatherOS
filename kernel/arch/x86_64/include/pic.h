
#pragma once

#include <arch/x86_64/include/ioPorts.h>


#define pic1Base 0x20
#define pic2Base 0xA0
#define pic1Command	pic1Base
#define pic1Data (pic1Base + 1)
#define pic2Command pic2Base
#define pic2Data (pic2Base + 1)