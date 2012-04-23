#include "types.h"
#include "nsf.h"

#define VSTRING "0.3.0"

NSF_HEADER *FESTAI_Load(uint8 *buf, uint32 size);

int32 *FESTAI_Emulate(int *Count);

void FESTAI_Sound(int32 Rate);
void FESTAI_SetVolume(uint32 volume);

int FESTAI_NSFControl(int z, int o);
void FESTAI_Close(void);
void FESTAI_Disable(int);

void FESTAI_SetSoundQuality(int q);

