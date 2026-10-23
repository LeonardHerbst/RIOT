#include "lwip/pbuf.h"

void ipfrag_free_pbuf_custom(pbuf_t *p);
void ip6_frag_free_pbuf_custom(pbuf_t *p);

void pbuf_free_custom_fn_dispatch(pbuf_free_custom_fn fn, pbuf_t *p)
{
    switch (fn) {
#if LWIP_IPV4
        case PBUF_FREE_CUSTOM_FN_IPFRAG_FREE_PBUF_CUSTOM:
            ipfrag_free_pbuf_custom(p);
            break;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
        case PBUF_FREE_CUSTOM_FN_IP6_FRAG_FREE_PBUF_CUSTOM:
            ip6_frag_free_pbuf_custom(p);
            break;
#endif /* LWIP_IPV6 */
        default:
            LWIP_ASSERT("ICD ERROR illegal pbuf_free_custom_fn!\n", false);
            break;
    }
}
