#include "lwip/timeouts.h"

void lwip_cyclic_timer(void *arg);
void bridgeif_age_tmr(void *arg);
void zep_lowpan_timer(void *arg);

void sys_timeout_handler_dispatch(sys_timeout_handler fn, void *arg)
{
    switch (fn) {
        case SYS_TIMEOUT_FN_LWIP_CYCLIC_TIMER:
            lwip_cyclic_timer(arg);
            break;
        case SYS_TIMEOUT_FN_BRIDGEIF_AGE_TMR:
            bridgeif_age_tmr(arg);
            break;
#if LWIP_IPV6 && LWIP_UDP
        case SYS_TIMEOUT_FN_ZEP_LOWPAN_TIMER:
            zep_lowpan_timer(arg);
            break;
#endif /* LWIP_IPV6 && LWIP_UDP */
        /* SYS_TIMEOUT_FN_TCPIP_TCP_TIMER is reserved (TCP not supported); it
         * has no dispatch case and falls through to the abort below. */
        default:
            LWIP_ASSERT("ICD ERROR illegal sys_timeout_handler!\n", false);
            break;
    }
}
