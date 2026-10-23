#include "lwip/timeouts.h"
#include "lwip/priv/tcpip_priv.h"
#include "lwip/ip4_frag.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/acd.h"
#include "lwip/igmp.h"
#include "lwip/dns.h"
#include "lwip/nd6.h"
#include "lwip/ip6_frag.h"
#include "lwip/mld6.h"

void lwip_cyclic_timer_fn_dispatch(lwip_cyclic_timer_handler fn)
{
    switch (fn) {
#if IP_REASSEMBLY
        case LWIP_CYCLIC_TIMER_FN_IP_REASS_TMR:
            ip_reass_tmr();
            break;
#endif /* IP_REASSEMBLY */
#if LWIP_ARP
        case LWIP_CYCLIC_TIMER_FN_ETHARP_TMR:
            etharp_tmr();
            break;
#endif /* LWIP_ARP */
#if LWIP_DHCP
        case LWIP_CYCLIC_TIMER_FN_DHCP_COARSE_TMR:
            dhcp_coarse_tmr();
            break;
        case LWIP_CYCLIC_TIMER_FN_DHCP_FINE_TMR:
            dhcp_fine_tmr();
            break;
#endif /* LWIP_DHCP */
#if LWIP_ACD
        case LWIP_CYCLIC_TIMER_FN_ACD_TMR:
            acd_tmr();
            break;
#endif /* LWIP_ACD */
#if LWIP_IGMP
        case LWIP_CYCLIC_TIMER_FN_IGMP_TMR:
            igmp_tmr();
            break;
#endif /* LWIP_IGMP */
#if LWIP_DNS
        case LWIP_CYCLIC_TIMER_FN_DNS_TMR:
            dns_tmr();
            break;
#endif /* LWIP_DNS */
#if LWIP_IPV6
        case LWIP_CYCLIC_TIMER_FN_ND6_TMR:
            nd6_tmr();
            break;
#if LWIP_IPV6_REASS
        case LWIP_CYCLIC_TIMER_FN_IP6_REASS_TMR:
            ip6_reass_tmr();
            break;
#endif /* LWIP_IPV6_REASS */
#if LWIP_IPV6_MLD
        case LWIP_CYCLIC_TIMER_FN_MLD6_TMR:
            mld6_tmr();
            break;
#endif /* LWIP_IPV6_MLD */
#endif /* LWIP_IPV6 */
        /* LWIP_CYCLIC_TIMER_FN_TCP_TMR is reserved (TCP not supported); it has
         * no dispatch case and falls through to the abort below. */
        default:
            LWIP_ASSERT("ICD ERROR illegal lwip_cyclic_timer_handler!\n", false);
            break;
    }
}
