#include "version.h"
#include "board_cfg.h"

#define SCPU_VERSION0       (4)
#define SCPU_VERSION1       (1)
#define SCPU_VERSION2       (6)
#define SCPU_VERSION3       (20)
#define SCPU_VERSION_DATE    (20240105)
#if (defined(CFG_COM_BUS_TYPE) && (CFG_COM_BUS_TYPE & COM_BUS_USB) || defined(DEV_TEST_VERSION))
#define SCPU_VERSION3_REAL  ((SCPU_VERSION3 & 0xfe) + 1)
#else
#define SCPU_VERSION3_REAL  (SCPU_VERSION3 & 0xfe)
#endif

#define MODEL_VERSION0       (1)
#define MODEL_VERSION1       (3)
#define MODEL_VERSION2       (2)
#define MODEL_VERSION3       (10)
#define MODEL_VERSION_DATE   (20231013)

const struct fw_misc_data g_fw_misc __attribute__((used, section("misc_data"))) = 
{
    .version[0] = SCPU_VERSION0,
    .version[1] = SCPU_VERSION1,
    .version[2] = SCPU_VERSION2,
    .version[3] = SCPU_VERSION3_REAL,
    .date = SCPU_VERSION_DATE,
};

struct fw_misc_data g_model_version = 
{
    .version[0] = MODEL_VERSION0,
    .version[1] = MODEL_VERSION1,
    .version[2] = MODEL_VERSION2,
    .version[3] = MODEL_VERSION3,
    .date = MODEL_VERSION_DATE,
};
