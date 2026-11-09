/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   VM entry points called from the host glue via f12r_execute
 *
 * @author  Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#include <stdint.h>

#include "shared.h"
#include "net/af.h"
#include "lwip/netif.h"
#include "lwip/api.h"
#include "lwip/netbuf.h"
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"
#include "lwip/netifapi.h"
#include "netif/lowpan6.h"

#define VM_PTR(type, field) ((type)(uintptr_t)(field))

/* Finds the netif carrying the given address; returns netif->num + 1 or
 * 0 (== SOCK_ADDR_ANY_NETIF, the constant is RIOT-side) */
uint16_t virtwip_sock_bind_addr_to_netif(virtwip_sock_bind_addr_args_t *a)
{
    if (ip_addr_isany(&a->addr)) {
        return 0;
    }
    LOCK_TCPIP_CORE();
    netif_t *netif;
    NETIF_FOREACH(netif) {
        if (IP_IS_V6(&a->addr)) {
#if LWIP_IPV6
            if (netif_get_ip6_addr_match(netif, ip_2_ip6(&a->addr)) >= 0) {
                UNLOCK_TCPIP_CORE();
                return (uint16_t)netif->num + 1;
            }
#endif
        }
        else {
#if LWIP_IPV4
            if (netif_ip4_addr(netif)->addr == ip_2_ip4(&a->addr)->addr) {
                UNLOCK_TCPIP_CORE();
                return (uint16_t)netif->num + 1;
            }
#endif
        }
    }
    UNLOCK_TCPIP_CORE();
    return 0;
}

/* Returns a pointer to the netif's bind address (VM memory), 0 if none */
bpfptr_t virtwip_sock_netif_to_bind_addr(virtwip_sock_netif_num_args_t *a)
{
    netif_t *netif;
    NETIF_FOREACH(netif) {
        if (netif->num == (u8_t)(a->netif_num - 1)) {
            switch (a->family) {
#if LWIP_IPV4
            case AF_INET:
                return (bpfptr_t)(uintptr_t)&netif->ip_addr;
#endif
#if LWIP_IPV6
            case AF_INET6:
                /* link-local address is always the 0th */
                return (bpfptr_t)(uintptr_t)&netif->ip6_addr[0];
#endif
            default:
                return 0;
            }
        }
    }
    return 0;
}

uint32_t virtwip_sock_addr_on_netif(virtwip_sock_addr_on_netif_args_t *a)
{
    netif_t *netif;
    NETIF_FOREACH(netif) {
        if (netif->num == (u8_t)(a->netif_num - 1)) {
            switch (a->family) {
#if LWIP_IPV4
            case AF_INET:
                return ip_2_ip4(&netif->ip_addr)->addr == ip_2_ip4(&a->addr)->addr;
#endif
#if LWIP_IPV6
            case AF_INET6: {
                LOCK_TCPIP_CORE();
                /* link-local address is always the 0th */
                s8_t match = netif_get_ip6_addr_match(netif, ip_2_ip6(&a->addr));
                UNLOCK_TCPIP_CORE();
                return match >= 0;
            }
#endif
            default:
                return 0;
            }
        }
    }
    return 0;
}

netif_t *virtwip_netif_add_noaddr(virtwip_netif_add_noaddr_args_t *a)
{
    return netif_add_noaddr(VM_PTR(netif_t *, a->netif), VM_PTR(void *, a->state),
                            VM_PTR(netif_init_fn, a->init), (netif_input_fn)a->input);
}

/* Sets the default netif only if none is set yet (the caller cannot read
 * lwIP's netif_list global) */
void virtwip_netif_set_default(virtwip_netif_args_t *a)
{
    if (netif_default == NULL) {
        netif_set_default(VM_PTR(netif_t *, a->netif));
    }
}

void virtwip_tcpip_init(virtwip_tcpip_init_args_t *a)
{
    tcpip_init(VM_PTR(tcpip_init_done_fn, a->done), VM_PTR(void *, a->arg));
}

#if LWIP_DHCP
void virtwip_dhcp_start_all(void)
{
    netif_t *n = NULL;
    NETIF_FOREACH(n) {
        if (netif_is_flag_set(n, NETIF_FLAG_ETHERNET)) {
            netifapi_dhcp_start(n);
        }
    }
}
#endif

void virtwip_netifapi_netif_set_link_up(virtwip_netif_args_t *a)
{
    netifapi_netif_set_link_up(VM_PTR(netif_t *, a->netif));
}

void virtwip_netifapi_netif_set_link_down(virtwip_netif_args_t *a)
{
    netifapi_netif_set_link_down(VM_PTR(netif_t *, a->netif));
}

pbuf_t *virtwip_pbuf_alloc(virtwip_pbuf_alloc_args_t *a)
{
    return pbuf_alloc((pbuf_layer_t)a->layer, (u16_t)a->len, (pbuf_type_t)a->type);
}

u8_t virtwip_pbuf_free(virtwip_pbuf_p_args_t *a)
{
    return pbuf_free(VM_PTR(pbuf_t *, a->p));
}

err_t virtwip_pbuf_take(virtwip_pbuf_take_args_t *a)
{
    return pbuf_take(VM_PTR(pbuf_t *, a->p), VM_PTR(const void *, a->buf), (u16_t)a->len);
}

err_t virtwip_pbuf_take_at(virtwip_pbuf_take_at_args_t *a)
{
    return pbuf_take_at(VM_PTR(pbuf_t *, a->p), VM_PTR(const void *, a->buf),
                        (u16_t)a->len, (u16_t)a->offset);
}

err_t virtwip_pbuf_header(virtwip_pbuf_header_args_t *a)
{
    return pbuf_header(VM_PTR(pbuf_t *, a->p), (s16_t)a->size);
}

u16_t virtwip_pbuf_copy_partial(virtwip_pbuf_copy_partial_args_t *a)
{
    return pbuf_copy_partial(VM_PTR(pbuf_t *, a->p), VM_PTR(void *, a->buf),
                             (u16_t)a->len, (u16_t)a->offset);
}

void virtwip_lowpan6_set_pan_id(virtwip_lowpan6_pan_id_args_t *a)
{
    lowpan6_set_pan_id((u16_t)a->pan_id);
}

err_t virtwip_lowpan6_if_init(virtwip_netif_args_t *a)
{
    return lowpan6_if_init(VM_PTR(netif_t *, a->netif));
}

err_t virtwip_netif_create_ip6_linklocal_address(virtwip_netif_ip6_linklocal_args_t *a)
{
    return netif_create_ip6_linklocal_address(VM_PTR(netif_t *, a->netif), (int)a->from_mac);
}

#if LWIP_IPV6_SCOPES
void virtwip_ip6_addr_assign_zone(virtwip_ip6_addr_zone_args_t *a)
{
    ip6_addr_assign_zone(VM_PTR(ip6_addr_t *, a->ip6), (u8_t)a->type, VM_PTR(netif_t *, a->netif));
}
#endif

netconn_t *virtwip_netconn_new(virtwip_netconn_new_args_t *a)
{
    return netconn_new_with_proto_and_callback((enum netconn_type)a->type,
                                               (u8_t)a->proto, (netconn_callback)a->callback);
}

err_t virtwip_netconn_bind(virtwip_netconn_ep_args_t *a)
{
    return netconn_bind(VM_PTR(netconn_t *, a->conn), &a->addr, (u16_t)a->port);
}

err_t virtwip_netconn_connect(virtwip_netconn_ep_args_t *a)
{
    return netconn_connect(VM_PTR(netconn_t *, a->conn), &a->addr, (u16_t)a->port);
}

err_t virtwip_netconn_delete(virtwip_netconn_args_t *a)
{
    return netconn_delete(VM_PTR(netconn_t *, a->conn));
}

err_t virtwip_netconn_getaddr(virtwip_netconn_getaddr_args_t *a)
{
    u16_t port = 0;
    err_t err = netconn_getaddr(VM_PTR(netconn_t *, a->conn), &a->addr, &port, (int)a->local);

    a->port = port;
    return err;
}

err_t virtwip_netconn_recv(virtwip_netconn_recv_args_t *a)
{
    netbuf_t *buf = NULL;
    err_t err = netconn_recv(VM_PTR(netconn_t *, a->conn), &buf);

    a->buf = (bpfptr_t)(uintptr_t)buf;
    return err;
}

netbuf_t *virtwip_netbuf_new(void)
{
    return netbuf_new();
}

void *virtwip_netbuf_alloc(virtwip_netbuf_alloc_args_t *a)
{
    return netbuf_alloc(VM_PTR(netbuf_t *, a->buf), (u16_t)a->size);
}

void virtwip_netbuf_delete(virtwip_netbuf_args_t *a)
{
    netbuf_delete(VM_PTR(netbuf_t *, a->buf));
}

s8_t virtwip_netbuf_next(virtwip_netbuf_args_t *a)
{
    return netbuf_next(VM_PTR(netbuf_t *, a->buf));
}

err_t virtwip_netconn_sendto(virtwip_netconn_sendto_args_t *a)
{
    return netconn_sendto(VM_PTR(netconn_t *, a->conn), VM_PTR(netbuf_t *, a->netbuf),
                          &a->addr, (u16_t)a->port);
}

err_t virtwip_netconn_send(virtwip_netconn_send_args_t *a)
{
    return netconn_send(VM_PTR(netconn_t *, a->conn), VM_PTR(netbuf_t *, a->netbuf));
}

err_t virtwip_netconn_write_partly(virtwip_netconn_write_args_t *a)
{
    size_t written = 0;
    err_t err = netconn_write_partly(VM_PTR(netconn_t *, a->conn), VM_PTR(const void *, a->dataptr),
                                     (size_t)a->len, (u8_t)a->apiflags, &written);

    a->written = (u32_t)written;
    return err;
}

#if LWIP_TCP
err_t virtwip_netconn_listen_with_backlog(virtwip_netconn_listen_args_t *a)
{
    return netconn_listen_with_backlog(VM_PTR(netconn_t *, a->conn), (u8_t)a->backlog);
}

err_t virtwip_netconn_accept(virtwip_netconn_recv_args_t *a)
{
    netconn_t *conn = NULL;
    err_t err = netconn_accept(VM_PTR(netconn_t *, a->conn), &conn);

    a->buf = (bpfptr_t)(uintptr_t)conn;
    return err;
}

err_t virtwip_netconn_close(virtwip_netconn_args_t *a)
{
    return netconn_close(VM_PTR(netconn_t *, a->conn));
}

err_t virtwip_netconn_recv_tcp_pbuf(virtwip_netconn_recv_args_t *a)
{
    pbuf_t *p = NULL;
    err_t err = netconn_recv_tcp_pbuf(VM_PTR(netconn_t *, a->conn), &p);

    a->buf = (bpfptr_t)(uintptr_t)p;
    return err;
}
#endif

/** @} */
