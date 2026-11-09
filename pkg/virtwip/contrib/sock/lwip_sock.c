/*
 * SPDX-FileCopyrightText: 2016 Freie Universität Berlin
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <assert.h>
#include <stdio.h>

#include "lwip/sock_internal.h"

#include "net/af.h"
#include "net/ipv4/addr.h"
#include "net/ipv6/addr.h"
#include "net/sock.h"
#include "timex.h"

#include "lwip/err.h"
#include "lwip/ip.h"
#include "lwip/netif.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"

#include "mbox.h"
#include "flow_pool.h"
#include "virtwip_glue.h"
#include "sock_types.h"

#if !LWIP_IPV4 && !LWIP_IPV6
#error "virtwip_sock needs IPv4 or IPv6 support"
#endif

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Checks if an address family is *not* supported by the lwIP
 *          implementation
 *
 * @param[in] af    An address family
 *
 * @return  true, if @p af is *not* supported.
 * @return  false, if @p af is supported.
 */
static inline bool _af_not_supported(int af)
{
    switch (af) {
#if LWIP_IPV4
    case AF_INET:
        return false;
#endif
#if LWIP_IPV6
    case AF_INET6:
        return false;
#endif
    default:
        return true;
    }
}

#if LWIP_IPV4 && LWIP_IPV6
static inline u8_t lwip_af_to_ip_addr_type(int af)
{
    switch (af) {
    case AF_INET:
        return IPADDR_TYPE_V4;
    case AF_INET6:
    case AF_UNSPEC: /* in case of any address */
        return IPADDR_TYPE_V6;
    default:
        return 0xff;
    }
}
#endif

static bool _ep_isany(const struct _sock_tl_ep *ep)
{
    uint8_t *ep_addr;

    if (ep == NULL) {
        return true;
    }

    ep_addr = (uint8_t *)&ep->addr;
    for (unsigned i = 0; i < sizeof(ep->addr); i++) {
#if LWIP_IPV4
        /* stop checking IPv6 for IPv4 addresses */
        if ((ep->family == AF_INET) && i >= sizeof(ep->addr.ipv4)) {
            break;
        }
#endif
        if (ep_addr[i] != 0) {
            return false;
        }
    }

    return true;
}

/* Maps an AF_* family to the lwIP IPADDR_TYPE_* the VM entry points expect
 * (UINT32_MAX for unsupported families, which the VM side rejects) */
static uint32_t _family_to_vm_ip_type(int family)
{
    switch (family) {
    case AF_INET:
        return IPADDR_TYPE_V4;
    case AF_INET6:
        return IPADDR_TYPE_V6;
    default:
        return UINT32_MAX;
    }
}

static const ip_addr_t *_netif_to_bind_addr(int family, uint16_t netif_num)
{
    if (netif_num > UINT8_MAX) {
        return NULL;
    }

    /* RIOT-LWIP-BORDER: lwIP call | netif_list is VM-internal; returns a
     * pointer into the netif's address (VM memory) */
    virtwip_sock_netif_num_args_t args = { .ip_type = _family_to_vm_ip_type(family),
                                           .netif_num = netif_num };
    int64_t res;
    if (virtwip_flow_call("virtwip_sock_netif_to_bind_addr", &args, sizeof(args), &res) != 0 ||
        res == 0) {
        return NULL;
    }
    return (const ip_addr_t *)(uintptr_t)res;
}

static bool _addr_on_netif(int family, int netif_num, const ip_addr_t *addr)
{
    assert(addr != NULL);
    assert((netif_num >= 0) && (netif_num <= UINT8_MAX));

    virtwip_sock_addr_on_netif_args_t args = { .ip_type = _family_to_vm_ip_type(family),
                                               .netif_num = (uint32_t)netif_num,
                                               .addr = *addr };
    int64_t res;
    if (virtwip_flow_call("virtwip_sock_addr_on_netif", &args, sizeof(args), &res) != 0) {
        return false;
    }
    return (bool)res;
}

#ifdef MODULE_VIRTWIP_SOCK_IP
#define _set_port(p, ep, type)   \
    if (!((type) & NETCONN_RAW)) { \
        p = (ep)->port; \
    }
#else
#define _set_port(p, ep, type)   \
    p = (ep)->port;
#endif

static void _convert_ip_addr(ip_addr_t *lwip_addr, int family,
                             const void *sock_addr, size_t sock_addr_size)
{
    memcpy(lwip_addr, sock_addr, sock_addr_size);
#if LWIP_IPV6 && LWIP_IPV4
    if (family == AF_INET6) {
        ip6_addr_clear_zone(&lwip_addr->u_addr.ip6);
    }

    lwip_addr->type = lwip_af_to_ip_addr_type(family);
#elif LWIP_IPV6
    (void)family;
    ip6_addr_clear_zone(lwip_addr);
#else
    (void)family;
#endif
}

static int _sock_ep_to_netconn_pars(const struct _sock_tl_ep *local,
                                    const struct _sock_tl_ep *remote,
                                    ip_addr_t *local_addr, u16_t *local_port,
                                    ip_addr_t *remote_addr, u16_t *remote_port,
                                    int *type)
{
    bool res = 0;
    int family = AF_UNSPEC;
    uint16_t netif = SOCK_ADDR_ANY_NETIF;

    if ((local != NULL) && (remote != NULL) &&
        (remote->netif != SOCK_ADDR_ANY_NETIF) &&
        (local->netif != SOCK_ADDR_ANY_NETIF) &&
        (remote->netif != local->netif)) {
        return -EINVAL;
    }

#if LWIP_IPV6
    *type &= ~NETCONN_TYPE_IPV6;
#else
    (void)type; /* is read but not set => compiler complains */
#endif
    if (local != NULL) {
        if (_af_not_supported(local->family)) {
            return -EAFNOSUPPORT;
        }

        if (local->netif != SOCK_ADDR_ANY_NETIF) {
            netif = local->netif;
        }

        family = local->family;
        _set_port(*local_port, local, *type);
    }
    if (remote != NULL) {
        if (_af_not_supported(remote->family) ||
            ((local != NULL) && (local->family != remote->family))) {
            return -EAFNOSUPPORT;
        }

        if ((remote->netif != SOCK_ADDR_ANY_NETIF) &&
            (local != NULL) && (local->netif != SOCK_ADDR_ANY_NETIF)) {
            netif = remote->netif;
        }

        family = remote->family;
        _convert_ip_addr(remote_addr, family, &remote->addr,
                         sizeof(remote->addr));

        if (ip_addr_isany(remote_addr)) {
            return -EINVAL;
        }

        _set_port(*remote_port, remote, *type);
    }

#if LWIP_IPV6
    if (family == AF_INET6) {
        *type |= NETCONN_TYPE_IPV6;
    }
#endif

    if (netif != SOCK_ADDR_ANY_NETIF) {
        if (_ep_isany(local)) {
            const ip_addr_t *tmp = _netif_to_bind_addr(family, netif);
            if (tmp != NULL) {
                memcpy(local_addr, tmp, sizeof(ip_addr_t));
                res = 1;
            }
            else {
                /* netif was not a valid interface */
                return -EINVAL;
            }
        }
        /* case (local == NULL) is included in _ep_isany() */
        /* cast to ip_addr_t alright, since type field is never used */
        else if (_addr_on_netif(family, netif, (ip_addr_t *)&local->addr)) {
            _convert_ip_addr(local_addr, family, &local->addr,
                             sizeof(local->addr));
            res = 1;
        }
        else {
            return -EINVAL;
        }
    }
    else if (local != NULL) {
        _convert_ip_addr(local_addr, family, &local->addr, sizeof(local->addr));
        res = 1;
    }

    return res;
}

static err_t _vm_netconn_getaddr(struct netconn *conn, ip_addr_t *addr, u16_t *port, int local)
{
    virtwip_netconn_getaddr_args_t args = { .conn = (bpfptr_t)(uintptr_t)conn,
                                            .local = (uint32_t)local };
    int64_t res;
    if (virtwip_flow_call("virtwip_netconn_getaddr", &args, sizeof(args), &res) != 0) {
        return ERR_MEM;
    }
    *addr = args.addr;
    *port = (u16_t)args.port;
    return (err_t)res;
}

/* RIOT-LWIP-BORDER: lwIP => RIOT | lwIP calls this via netconn callback
 * (registered with lwIP in _create()) whenever netconn state changes */
void netconn_cb(struct netconn *conn, enum netconn_evt evt, u16_t len)
{
#if IS_ACTIVE(SOCK_HAS_ASYNC)
    lwip_sock_base_t *sock = netconn_get_callback_arg(conn);

    if (sock && conn->pcb.raw &&
        /* lwIP's TCP implementation initializes callback_arg.socket with -1
         * when not provided */
        (conn->callback_arg.socket != -1)) {
        sock_async_flags_t flags = 0;

        (void)len;
        switch (evt) {
        case NETCONN_EVT_RCVPLUS:
            if (LWIP_TCP && (conn->type & NETCONN_TCP)) {
#if LWIP_TCP    /* additional guard needed due to dependent member access */
                switch (conn->pcb.tcp->state) {
                case CLOSED:
                case CLOSE_WAIT:
                case CLOSING:
                    flags |= SOCK_ASYNC_CONN_FIN;
                    break;
                default:
                    break;
                }

                if (mbox_avail(virtwip_vm_conn_mbox(&conn->acceptmbox))) {
                    flags |= SOCK_ASYNC_CONN_RECV;
                }

                if (mbox_avail(virtwip_vm_conn_mbox(&conn->recvmbox))) {
                    flags |= SOCK_ASYNC_MSG_RECV;
                }
#endif
            }
            else {
                flags |= SOCK_ASYNC_MSG_RECV;
            }
            break;
        case NETCONN_EVT_SENDPLUS:
            flags |= SOCK_ASYNC_MSG_SENT;
            break;
        case NETCONN_EVT_ERROR:
            if (LWIP_TCP && (conn->type & NETCONN_TCP)) {
                /* try to report this */
                flags |= SOCK_ASYNC_CONN_FIN;
            }
            break;
        case NETCONN_EVT_RCVMINUS:
        case NETCONN_EVT_SENDMINUS:
            break;
        default:
            LWIP_ASSERT("unknown event", 0);
            break;
        }

        if (flags && sock->async_cb.gen) {
            sock->async_cb.gen(sock, flags, sock->async_cb_arg);
        }
    }
#else
    (void)conn;
    (void)evt;
    (void)len;
#endif
}

static int _create(int type, int proto, uint16_t flags, struct netconn **out)
{
    /* RIOT-LWIP-BORDER: lwIP call | also registers netconn_cb as netconn
     * callback (lwIP calls back into RIOT from its TCP/IP thread) */
    virtwip_netconn_new_args_t args = {
        .type = (uint32_t)type,
        .proto = proto,
        .callback = IS_ACTIVE(SOCK_HAS_ASYNC) ? BPF_FUNC_CALL_NETCONN_CB : 0,
    };
    int64_t res;
    if (virtwip_flow_call("virtwip_netconn_new", &args, sizeof(args), &res) != 0) {
        return -ENOMEM;
    }
    if ((*out = (struct netconn *)(uintptr_t)res) == NULL) {
        return -ENOMEM;
    }

    netconn_set_callback_arg(*out, NULL);   /* RIOT-LWIP-BORDER: lwIP call */
#if SO_REUSE
    if (flags & SOCK_FLAGS_REUSE_EP) {
        ip_set_option((*out)->pcb.ip, SOF_REUSEADDR);   /* RIOT-LWIP-BORDER: lwIP call */
    }
#else
    (void)flags;
#endif
    return 0;
}

int lwip_sock_create(struct netconn **conn, const struct _sock_tl_ep *local,
                     const struct _sock_tl_ep *remote, int proto,
                     uint16_t flags, int type)
{
    assert(conn != NULL);
#if LWIP_IPV6
    assert(!(type & NETCONN_TYPE_IPV6));
#endif

    ip_addr_t local_addr, remote_addr;
    u16_t local_port = 0, remote_port = 0;   /* 0 is used by lwIP to
                                              * automatically generate
                                              * port */
    /* convert parameters */
    int bind = _sock_ep_to_netconn_pars(local, remote, &local_addr, &local_port,
                                        &remote_addr, &remote_port, &type);

    /* error occurred during parameter conversion */
    if (bind < 0) {
        return bind;
    }

    if ((remote != NULL) && ip_addr_isany_val(remote_addr)) {
        return -EINVAL;
    }
    /* if local or remote parameters are given */
    else if ((local != NULL) || (remote != NULL)) {
        int res = 0;

        if ((res = _create(type, proto, flags, conn)) < 0) {
            return res;
        }

        /* if parameters (local->netif, remote->netif, local->addr or
         * local->port) demand binding */
        if (bind) {
            virtwip_netconn_ep_args_t cargs = { .conn = (bpfptr_t)(uintptr_t)*conn,
                                                .addr = local_addr, .port = local_port };
            int64_t res64;
            err_t err;
            if (virtwip_flow_call("virtwip_netconn_bind", &cargs, sizeof(cargs), &res64) != 0) {
                err = ERR_MEM;  /* RIOT-LWIP-BORDER: lwIP call */
            }
            else {
                err = (err_t)res64;
            }
            switch (err) {
#if LWIP_TCP
            case ERR_BUF:
                res = -ENOMEM;
                break;
#endif
            case ERR_USE:
                res = -EADDRINUSE;
                break;
            case ERR_VAL:
                res = -EINVAL;
                break;
            default:
                break;
            }

            if (res < 0) {
                virtwip_vm_conn_delete(*conn); /* RIOT-LWIP-BORDER: lwIP call */
                return res;
            }
        }

        if (remote != NULL) {
            virtwip_netconn_ep_args_t cargs = { .conn = (bpfptr_t)(uintptr_t)*conn,
                                                .addr = remote_addr, .port = remote_port };
            int64_t res64;
            err_t err;
            if (virtwip_flow_call("virtwip_netconn_connect", &cargs, sizeof(cargs), &res64) != 0) {
                err = ERR_MEM;  /* RIOT-LWIP-BORDER: lwIP call */
            }
            else {
                err = (err_t)res64;
            }
            switch (err) {
#if LWIP_TCP
            case ERR_BUF:
                res = -ENOMEM;
                break;
            case ERR_INPROGRESS:
                res = -EINPROGRESS;
                break;
            case ERR_ISCONN:
                res = -EISCONN;
                break;
            case ERR_IF:
            case ERR_RTE:
                res = -ENETUNREACH;
                break;
            case ERR_ABRT:
                res = -ETIMEDOUT;
                break;
#endif
            case ERR_USE:
                res = -EADDRINUSE;
                break;
            case ERR_VAL:
                res = -EINVAL;
                break;
            default:
                break;
            }

            if (res < 0) {
                virtwip_vm_conn_delete(*conn); /* RIOT-LWIP-BORDER: lwIP call */
                return res;
            }
        }
    }

    return 0;
}

uint16_t lwip_sock_bind_addr_to_netif(const ip_addr_t *bind_addr)
{
    assert(bind_addr != NULL);
    /* RIOT-LWIP-BORDER: lwIP call | netif_list and the address match are
     * VM-internal, so the lookup runs in the VM (0 == SOCK_ADDR_ANY_NETIF) */
    virtwip_sock_bind_addr_args_t args = { .addr = *bind_addr };
    int64_t res;
    if (virtwip_flow_call("virtwip_sock_bind_addr_to_netif", &args, sizeof(args), &res) != 0) {
        return SOCK_ADDR_ANY_NETIF;
    }
    return (uint16_t)res;
}

int lwip_sock_get_addr(struct netconn *conn, struct _sock_tl_ep *ep, u8_t local)
{
    ip_addr_t addr;
    int res;
#ifdef MODULE_VIRTWIP_SOCK_IP
    u16_t port = UINT16_MAX;
    u16_t *port_ptr = &port;
    /* addr needs to be NULL because netconn_getaddr returns error on connected
     * conns as "as connecting is only a helper for upper layers [sic]" */
    memset(&addr, 0, sizeof(addr));
#else
    u16_t *port_ptr = &ep->port;
#endif

    assert(ep != NULL);
    if (conn == NULL) {
        return 1;
    }

#ifdef MODULE_VIRTWIP_SOCK_IP
    if (!(conn->type & NETCONN_RAW)) {
        port_ptr = &ep->port;
    }
#endif
    if ((res = _vm_netconn_getaddr(conn, &addr, port_ptr, local)) != ERR_OK   /* RIOT-LWIP-BORDER: lwIP call */
#ifdef MODULE_VIRTWIP_SOCK_IP
        /* XXX lwIP's API is very inconsistent here so we need to check if addr
         * was changed */
            && !local && ip_addr_isany_val(addr)
#endif
        ) {
        return res;
    }

    if (NETCONNTYPE_ISIPV6(conn->type)) {
        ep->family = AF_INET6;
    }
    else if (IS_ACTIVE(LWIP_IPV4)) {
        ep->family = AF_INET;
    }
    else {
        ep->family = AF_UNSPEC;
    }

    if (local) {
        ep->netif = lwip_sock_bind_addr_to_netif(&addr);
    }
    else {
        ep->netif = SOCK_ADDR_ANY_NETIF;
    }

    memcpy(&ep->addr, &addr, sizeof(ep->addr));

    return 0;
}

#if defined(MODULE_VIRTWIP_SOCK_UDP) || defined(MODULE_VIRTWIP_SOCK_IP)
int lwip_sock_recv(struct netconn *conn, uint32_t timeout, struct netbuf **buf)
{
    int res;

    if (conn == NULL) {
        return -EADDRNOTAVAIL;
    }

#if LWIP_SO_RCVTIMEO
    if ((timeout != 0) && (timeout != SOCK_NO_TIMEOUT)) {
        netconn_set_recvtimeout(conn, lwip_sock_timeout_ms(timeout));   /* RIOT-LWIP-BORDER: lwIP call */
    }
    else
#endif
    if ((timeout == 0) && !mbox_avail(virtwip_vm_conn_mbox(&conn->recvmbox))) {
        return -EAGAIN;
    }

    virtwip_netconn_recv_args_t rargs = { .conn = (bpfptr_t)(uintptr_t)conn };
    int64_t res64;
    err_t err;
    if (virtwip_flow_call("virtwip_netconn_recv", &rargs, sizeof(rargs), &res64) != 0) {
        err = ERR_MEM;  /* RIOT-LWIP-BORDER: lwIP call (blocks on netconn) */
    }
    else {
        err = (err_t)res64;
    }
    *buf = (struct netbuf *)(uintptr_t)rargs.buf;
    switch (err) {
    case ERR_OK:
        res = 0;
        break;
#if LWIP_SO_RCVTIMEO
    case ERR_TIMEOUT:
        res = -ETIMEDOUT;
        break;
#endif
    case ERR_MEM:
        res = -ENOMEM;
        break;
    default:
        res = -EPROTO;
        break;
    }

    /* unset flags */
#if LWIP_SO_RCVTIMEO
    netconn_set_recvtimeout(conn, 0);   /* RIOT-LWIP-BORDER: lwIP call */
#endif
#if IS_ACTIVE(SOCK_HAS_ASYNC)
    lwip_sock_base_t *sock = netconn_get_callback_arg(conn);    /* RIOT-LWIP-BORDER: lwIP call */

    if (sock && sock->async_cb.gen && mbox_avail(virtwip_vm_conn_mbox(&conn->recvmbox))) {
        sock->async_cb.gen(sock, SOCK_ASYNC_MSG_RECV, sock->async_cb_arg);
    }
#endif
    return res;
}
#endif /* defined(MODULE_VIRTWIP_SOCK_UDP) || defined(MODULE_VIRTWIP_SOCK_IP) */

ssize_t lwip_sock_sendv(struct netconn *conn, const iolist_t *snips,
                        int proto, const struct _sock_tl_ep *remote, int type)
{
    ip_addr_t remote_addr;
    struct netconn *tmp;
    struct netbuf *buf = NULL;
    size_t payload_len = 0;
    ssize_t res;
    err_t err= ERR_OK;
    u16_t remote_port = 0;

#if LWIP_IPV6
    assert(!(type & NETCONN_TYPE_IPV6));
#endif
    if (remote != NULL) {
        if ((res = _sock_ep_to_netconn_pars(NULL, remote, NULL, NULL,
                                            &remote_addr, &remote_port,
                                            &type)) < 0) {
            return res;
        }

        if (ip_addr_isany_val(remote_addr)) {
            DEBUG("[lwip_sock_sendv] remote_addr is all zero\n");
            return -EINVAL;
        }
    }

    int64_t res64;
    if (virtwip_flow_call("virtwip_netbuf_new", NULL, 0, &res64) != 0) {
        res64 = 0;  /* RIOT-LWIP-BORDER: lwIP call */
    }
    buf = (struct netbuf *)(uintptr_t)res64;

    virtwip_netbuf_alloc_args_t alloc = { .buf = (bpfptr_t)(uintptr_t)buf,
                                          .size = (u16_t)iolist_size(snips) };
    if (virtwip_flow_call("virtwip_netbuf_alloc", &alloc, sizeof(alloc), &res64) != 0 ||
        res64 == 0) {   /* RIOT-LWIP-BORDER: lwIP call */
        virtwip_vm_netbuf_delete(buf);
        return -ENOMEM;
    }

    for (const iolist_t *snip = snips; snip != NULL; snip = snip->iol_next) {
        virtwip_pbuf_take_at_args_t take = { .p = (bpfptr_t)(uintptr_t)buf->p,
                                             .buf = (bpfptr_t)(uintptr_t)snip->iol_base,
                                             .len = (u16_t)snip->iol_len,
                                             .offset = (u16_t)payload_len };
        if (virtwip_flow_call_buf("virtwip_pbuf_take_at", &take, sizeof(take),
                                   snip->iol_base, snip->iol_len, &res64) != 0) {
            res64 = ERR_MEM;    /* RIOT-LWIP-BORDER: lwIP call */
        }
        if ((err_t)res64 != ERR_OK) {
            virtwip_vm_netbuf_delete(buf);
            return -ENOMEM;
        }
        payload_len += snip->iol_len;
    }

    if ((conn == NULL) && (remote != NULL)) {
        if ((res = _create(type, proto, 0, &tmp)) < 0) {
            virtwip_vm_netbuf_delete(buf);
            return res;
        }
    }
    else if (conn != NULL) {
        ip_addr_t addr;
        u16_t port;

        if ((remote != NULL)
                && (remote->netif != SOCK_ADDR_ANY_NETIF)
                && (_vm_netconn_getaddr(conn, &addr, &port, 1) == 0)) { /* RIOT-LWIP-BORDER: lwIP call */
            if (IS_ACTIVE(ENABLE_DEBUG)) {
                DEBUG("[lwip_sock_sendv] sending from: \"");
                /* lwip's ip_addr_debug triggers a -Waddress warning when
                 * compiled with the ESP toolchain. In absence of a upstream
                 * solution, we just disable the warning here. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"
                ip_addr_debug_print(LWIP_DBG_ON, &addr);
#pragma GCC diagnostic pop
                DEBUG("\"\n");
            }
            uint16_t netif = SOCK_ADDR_ANY_NETIF;
            virtwip_sock_bind_addr_args_t bargs = { .addr = addr };
            if (virtwip_flow_call("virtwip_sock_bind_addr_to_netif", &bargs,
                                  sizeof(bargs), &res64) == 0) {
                netif = (uint16_t)res64;
            }
            if ((remote->netif != netif)
                    && (netif != SOCK_ADDR_ANY_NETIF)) {
                DEBUG("[lwip_sock_sendv] lwip_sock_bind_addr_to_netif() "
                      "returned %u, but expected %u\n",
                      (unsigned)netif, (unsigned)remote->netif);
                virtwip_vm_netbuf_delete(buf);
                return -EINVAL;
            }
        }
        tmp = conn;
    }
    else {
        virtwip_vm_netbuf_delete(buf);
        return -ENOTCONN;
    }

    res = payload_len;  /* set for non-TCP calls */

    if (remote != NULL) {
        virtwip_netconn_sendto_args_t sargs = { .conn = (bpfptr_t)(uintptr_t)tmp,
                                                .netbuf = (bpfptr_t)(uintptr_t)buf,
                                                .addr = remote_addr, .port = remote_port };
        if (virtwip_flow_call("virtwip_netconn_sendto", &sargs, sizeof(sargs), &res64) != 0) {
            err = ERR_MEM;
        }
        else {
            err = (err_t)res64;
        }   /* RIOT-LWIP-BORDER: lwIP call */
    }
#if LWIP_TCP
    else if (tmp->type & NETCONN_TCP) {
        virtwip_netconn_write_args_t wargs = { .conn = (bpfptr_t)(uintptr_t)tmp,
                                               .dataptr = (bpfptr_t)(uintptr_t)buf->p->payload,
                                               .len = (u32_t)buf->p->len, .apiflags = 0 };
        if (virtwip_flow_call("virtwip_netconn_write_partly", &wargs, sizeof(wargs), &res64) != 0) {
            err = ERR_MEM;
        }
        else {
            err = (err_t)res64;
            res = (ssize_t)wargs.written;
        }   /* RIOT-LWIP-BORDER: lwIP call */
    }
#endif /* LWIP_TCP */
    else {
        virtwip_netconn_send_args_t sargs = { .conn = (bpfptr_t)(uintptr_t)tmp,
                                              .netbuf = (bpfptr_t)(uintptr_t)buf };
        if (virtwip_flow_call("virtwip_netconn_send", &sargs, sizeof(sargs), &res64) != 0) {
            err = ERR_MEM;
        }
        else {
            err = (err_t)res64;
        }   /* RIOT-LWIP-BORDER: lwIP call */
    }

    switch (err) {
    case ERR_OK:
        break;
    case ERR_BUF:
    case ERR_MEM:
        res = -ENOMEM;
        break;
    case ERR_RTE:
    case ERR_IF:
        res = -EHOSTUNREACH;
        break;
    case ERR_VAL:
    default:
        res = -EINVAL;
        break;
    }

    virtwip_vm_netbuf_delete(buf); /* RIOT-LWIP-BORDER: lwIP call */

    if (conn == NULL) {
        virtwip_vm_conn_delete(tmp);   /* RIOT-LWIP-BORDER: lwIP call */
    }

    return res;
}

/** @} */
