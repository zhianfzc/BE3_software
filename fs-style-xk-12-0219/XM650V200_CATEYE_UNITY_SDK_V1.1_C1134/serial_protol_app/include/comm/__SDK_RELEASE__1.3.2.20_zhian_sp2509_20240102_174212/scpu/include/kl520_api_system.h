#ifndef __KL520_API_SYSTEM_H__
#define __KL520_API_SYSTEM_H__

void kl520_api_settings_delete(void);

/**
 * @brief Restore factory settings, including delete all face information
*/
void kl520_api_factory(void);

/**
 * @brief kl520 shutdown
*/
void kl520_api_poweroff(void);


void kl520_api_poweroff_only(void);

#endif
