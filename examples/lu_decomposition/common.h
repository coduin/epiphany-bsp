#define LOC_M 0x4000
#define LOC_N 0x4001
#define LOC_DIM 0x4002
#define LOC_MATRIX 0x4003

#define LOC_RS 0x5800
#define LOC_ARK (LOC_RS + sizeof(int) * M)
#define LOC_R (LOC_ARK + sizeof(float))
#define LOC_PI (LOC_R + sizeof(int))
#define LOC_PI_IN (LOC_PI + sizeof(int) * N)
#define LOC_ROW_IN (LOC_PI_IN + sizeof(int) * 2)

#define LOC_RESULT 0x6000
