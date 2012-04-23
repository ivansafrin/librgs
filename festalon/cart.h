extern uint8 *Page[32];

void ResetCartMapping(void);
void SetupCartPRGMapping(int chip, uint8 *p, uint32 size, int ram);
void SetupCartCHRMapping(int chip, uint8 *p, uint32 size, int ram);
void SetupCartMirroring(int m, int hard, uint8 *extra);

DECLFR(CartBROB);
DECLFR(CartBR);
DECLFW(CartBW);

extern uint8 *PRGptr[32];
extern uint32 PRGsize[32];

extern uint32 PRGmask2[32];
extern uint32 PRGmask4[32];
extern uint32 PRGmask8[32];
extern uint32 PRGmask16[32];
extern uint32 PRGmask32[32];

void FASTAPASS(2) setprg2(uint32 A, uint32 V);
void FASTAPASS(2) setprg4(uint32 A, uint32 V);
void FASTAPASS(2) setprg8(uint32 A, uint32 V);
void FASTAPASS(2) setprg16(uint32 A, uint32 V);
void FASTAPASS(2) setprg32(uint32 A, uint32 V);

void FASTAPASS(3) setprg2r(int r, unsigned int A, unsigned int V);
void FASTAPASS(3) setprg4r(int r, unsigned int A, unsigned int V);
void FASTAPASS(3) setprg8r(int r, unsigned int A, unsigned int V);
void FASTAPASS(3) setprg16r(int r, unsigned int A, unsigned int V);
void FASTAPASS(3) setprg32r(int r, unsigned int A, unsigned int V);

