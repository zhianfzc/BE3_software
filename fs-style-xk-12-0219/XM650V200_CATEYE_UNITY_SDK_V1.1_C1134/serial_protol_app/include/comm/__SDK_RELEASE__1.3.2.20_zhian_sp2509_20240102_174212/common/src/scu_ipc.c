/*
 * Kneron SCU IPC driver for KL520
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */

#include "kneron_mozart.h"
#include "io.h"
#include "base.h"
#include "scu_ipc.h"


/* IPC Registers */
#define SCPU_IPC_REG_ADDR(base)     (base + 0x0070)
#define NCPU_IPC_REG_ADDR(base)     (base + 0x0074)

/* IPC-From bits */
#define IPC_CLR_FROM           BIT(5)
#define IPC_STATUS_FROM        BIT(4)

/* IPC-To bits */
#define IPC_INT_TO             BIT(1)
#define IPC_ENABLE_TO          BIT(0)

#if defined(TARGET_SCPU)
void scu_ipc_enable_to_ncpu_int(void)
{
    masked_outw(SCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_ENABLE_TO, IPC_ENABLE_TO);
}

void scu_ipc_trigger_to_ncpu_int(void)
{
    masked_outw(SCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_INT_TO, IPC_INT_TO);
}

void scu_ipc_clear_from_ncpu_int(void)
{
    masked_outw(SCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_CLR_FROM, IPC_CLR_FROM);
}
#endif

#if defined(TARGET_NCPU)
void scu_ipc_enable_to_scpu_int(void)
{
    masked_outw(NCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_ENABLE_TO, IPC_ENABLE_TO);
}

void scu_ipc_trigger_to_scpu_int(void)
{
    masked_outw(NCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_INT_TO, IPC_INT_TO);
}

void scu_ipc_clear_from_scpu_int(void)
{
    masked_outw(NCPU_IPC_REG_ADDR(SCU_EXTREG_PA_BASE), IPC_CLR_FROM, IPC_CLR_FROM);
}
#endif
