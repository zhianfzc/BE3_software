#include "kl520_include.h"
#include "rtc.h"
#include "power.h"
#include "dbg.h"
#include "kdp_e2e_settings.h"
#include "kl520_api_system.h"
#include "kl520_api.h"
#include "user_io.h"

bool g_bPowerDown = FALSE;

void kl520_api_settings_delete(void)
{
    kdp_e2e_settings_delete();
}

void kl520_api_factory(void)
{
    dbg_msg_user("[kl520_api_factory]");
    kl520_api_settings_delete();
    kl520_api_face_del(1, (u8)0);

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( ENCRYPTION_MODE&AES_ENCRYPTION ) || ( ENCRYPTION_MODE&XOR_ENCRYPTION )
    kl520_api_customer_clr();
#endif
#endif
}

extern osThreadId_t tid_fdfr_update_fr;
void kl520_api_poweroff(void)
{
    u16 cnt = 0;
    g_bPowerDown = TRUE;
    while(tid_fdfr_update_fr != NULL && cnt < 100) { 
        osDelay(50);
        cnt++; 
    }
    dbg_msg_algo("%d", cnt);
    
    kl520_api_face_close();

    user_io_poweroff();

    dbg_msg_api("!!! shutdown ...\n\n");

    /* Disable alarm */
    rtc_alarm_disable(ALARM_IN_SECS);

    /* Power off everything except RTC */
    power_mgr_softoff(POWER_MGR_MODE_RTC);
    __WFI();

    dbg_msg_api("!!! shutdown failed!\n");
    for (;;);
    
}

void kl520_api_poweroff_only(void)
{
    power_mgr_set_domain(POWER_DOMAIN_NPU, 0);

    err_msg("!!! shutdown ...\n\n");

    /* Disable alarm */
    rtc_alarm_disable(ALARM_IN_SECS);

    /* Power off everything except RTC */
    power_mgr_softoff(POWER_MGR_MODE_RTC);
    __WFI();

    err_msg("!!! shutdown failed!\n");
    for (;;);
}

