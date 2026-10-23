#include "lwip/raw.h"

#if LWIP_RAW
u8_t raw_recv_fn_dispatch(raw_recv_fn fn, raw_pcb_t *pcb, pbuf_t *p, const ip_addr_t *addr)
{
    switch (fn) {
        /* No registrant exists in this build (raw_recv() users live in
         * sockets.c/netdb.c, which are compiled out). Extend here with a
         * dispatch case when a registrant is added; read the callback
         * argument from pcb->recv_arg. */
        default:
            LWIP_ASSERT("ICD ERROR illegal raw_recv_fn!\n", false);
            return 0;
    }
}
#endif /* LWIP_RAW */
