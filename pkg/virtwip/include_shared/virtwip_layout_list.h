/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/*
 * List of all layout properties checked by the virtwip layout check
 * (see virtwip_layout.h). Intentionally without include guard: it is an
 * X-macro list, included with VIRTWIP_LAYOUT_SIZE(type) and
 * VIRTWIP_LAYOUT_OFFSET(type, member) defined by the includer.
 *
 * Covers every structure (and the types embedded in them) that one side
 * writes and the other side reads: lwIP structs the host glue accesses
 * directly and the argument bundles of the VM entry points and helpers.
 * Fields are guarded by the same options as in their declaration, which
 * the shared config makes identical on both sides; a field missing on one
 * side therefore shows up as an offset/size mismatch of the struct.
 *
 * Keep entries in sync with the declarations when changing shared structs.
 */

/* --- basic types --------------------------------------------------------- */
VIRTWIP_LAYOUT_SIZE(bpfptr_t)
VIRTWIP_LAYOUT_SIZE(err_t)
VIRTWIP_LAYOUT_SIZE(ip_addr_t)
#if LWIP_IPV4
VIRTWIP_LAYOUT_SIZE(ip4_addr_t)
#endif
#if LWIP_IPV6
VIRTWIP_LAYOUT_SIZE(ip6_addr_t)
#if LWIP_IPV6_SCOPES
VIRTWIP_LAYOUT_OFFSET(ip6_addr_t, zone)
#endif
#endif
#if LWIP_IPV4 && LWIP_IPV6
VIRTWIP_LAYOUT_OFFSET(ip_addr_t, u_addr)
VIRTWIP_LAYOUT_OFFSET(ip_addr_t, type)
#endif
VIRTWIP_LAYOUT_SIZE(sys_sem_t)
VIRTWIP_LAYOUT_SIZE(sys_mutex_t)
VIRTWIP_LAYOUT_SIZE(sys_mbox_t)

/* enums stored in shared structs (-fshort-enums would change them) */
VIRTWIP_LAYOUT_SIZE(netif_input_fn)
#if LWIP_IPV4
VIRTWIP_LAYOUT_SIZE(netif_output_fn)
#endif
#if LWIP_IPV6
VIRTWIP_LAYOUT_SIZE(netif_output_ip6_fn)
#endif

/* --- struct pbuf ---------------------------------------------------------- */
VIRTWIP_LAYOUT_SIZE(struct pbuf)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, next)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, payload)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, tot_len)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, len)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, type_internal)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, flags)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, ref)
VIRTWIP_LAYOUT_OFFSET(struct pbuf, if_idx)

/* --- struct netif --------------------------------------------------------- */
VIRTWIP_LAYOUT_SIZE(struct netif)
#if !LWIP_SINGLE_NETIF
VIRTWIP_LAYOUT_OFFSET(struct netif, next)
#endif
#if LWIP_IPV4
VIRTWIP_LAYOUT_OFFSET(struct netif, ip_addr)
VIRTWIP_LAYOUT_OFFSET(struct netif, netmask)
VIRTWIP_LAYOUT_OFFSET(struct netif, gw)
#endif
#if LWIP_IPV6
VIRTWIP_LAYOUT_OFFSET(struct netif, ip6_addr)
VIRTWIP_LAYOUT_OFFSET(struct netif, ip6_addr_state)
#if LWIP_IPV6_ADDRESS_LIFETIMES
VIRTWIP_LAYOUT_OFFSET(struct netif, ip6_addr_valid_life)
VIRTWIP_LAYOUT_OFFSET(struct netif, ip6_addr_pref_life)
#endif
#endif
VIRTWIP_LAYOUT_OFFSET(struct netif, input)
#if LWIP_IPV4
VIRTWIP_LAYOUT_OFFSET(struct netif, output)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netif, linkoutput)
#if LWIP_IPV6
VIRTWIP_LAYOUT_OFFSET(struct netif, output_ip6)
#endif
#if LWIP_NETIF_STATUS_CALLBACK
VIRTWIP_LAYOUT_OFFSET(struct netif, status_callback)
#endif
#if LWIP_NETIF_LINK_CALLBACK
VIRTWIP_LAYOUT_OFFSET(struct netif, link_callback)
#endif
#if LWIP_NETIF_REMOVE_CALLBACK
VIRTWIP_LAYOUT_OFFSET(struct netif, remove_callback)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netif, state)
#ifdef netif_get_client_data
VIRTWIP_LAYOUT_OFFSET(struct netif, client_data)
#endif
#if LWIP_NETIF_HOSTNAME
VIRTWIP_LAYOUT_OFFSET(struct netif, hostname)
#endif
#if LWIP_CHECKSUM_CTRL_PER_NETIF
VIRTWIP_LAYOUT_OFFSET(struct netif, chksum_flags)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netif, mtu)
#if LWIP_IPV6 && LWIP_ND6_ALLOW_RA_UPDATES
VIRTWIP_LAYOUT_OFFSET(struct netif, mtu6)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netif, hwaddr)
VIRTWIP_LAYOUT_OFFSET(struct netif, hwaddr_len)
VIRTWIP_LAYOUT_OFFSET(struct netif, flags)
VIRTWIP_LAYOUT_OFFSET(struct netif, name)
VIRTWIP_LAYOUT_OFFSET(struct netif, num)
#if LWIP_IPV6_AUTOCONFIG
VIRTWIP_LAYOUT_OFFSET(struct netif, ip6_autoconfig_enabled)
#endif
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
VIRTWIP_LAYOUT_OFFSET(struct netif, rs_count)
#endif
#if MIB2_STATS
VIRTWIP_LAYOUT_OFFSET(struct netif, link_type)
VIRTWIP_LAYOUT_OFFSET(struct netif, link_speed)
VIRTWIP_LAYOUT_OFFSET(struct netif, ts)
VIRTWIP_LAYOUT_OFFSET(struct netif, mib2_counters)
#endif
#if LWIP_IPV4 && LWIP_IGMP
VIRTWIP_LAYOUT_OFFSET(struct netif, igmp_mac_filter)
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
VIRTWIP_LAYOUT_OFFSET(struct netif, mld_mac_filter)
#endif
#if LWIP_ACD
VIRTWIP_LAYOUT_OFFSET(struct netif, acd_list)
#endif
#if LWIP_NETIF_USE_HINTS
VIRTWIP_LAYOUT_OFFSET(struct netif, hints)
#endif
#if ENABLE_LOOPBACK
VIRTWIP_LAYOUT_OFFSET(struct netif, loop_first)
VIRTWIP_LAYOUT_OFFSET(struct netif, loop_last)
#if LWIP_LOOPBACK_MAX_PBUFS
VIRTWIP_LAYOUT_OFFSET(struct netif, loop_cnt_current)
#endif
#if LWIP_NETIF_LOOPBACK_MULTITHREADING
VIRTWIP_LAYOUT_OFFSET(struct netif, reschedule_poll)
#endif
#endif

#if LWIP_NETCONN
/* --- netconn API ---------------------------------------------------------- */
VIRTWIP_LAYOUT_SIZE(enum netconn_type)
VIRTWIP_LAYOUT_SIZE(enum netconn_state)
VIRTWIP_LAYOUT_SIZE(netconn_callback)

VIRTWIP_LAYOUT_SIZE(struct netconn)
VIRTWIP_LAYOUT_OFFSET(struct netconn, type)
VIRTWIP_LAYOUT_OFFSET(struct netconn, state)
VIRTWIP_LAYOUT_OFFSET(struct netconn, pcb)
VIRTWIP_LAYOUT_OFFSET(struct netconn, pending_err)
#if !LWIP_NETCONN_SEM_PER_THREAD
VIRTWIP_LAYOUT_OFFSET(struct netconn, op_completed)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netconn, recvmbox)
#if LWIP_TCP
VIRTWIP_LAYOUT_OFFSET(struct netconn, acceptmbox)
#endif
#if LWIP_NETCONN_FULLDUPLEX
VIRTWIP_LAYOUT_OFFSET(struct netconn, mbox_threads_waiting)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netconn, callback_arg)
#if LWIP_SO_SNDTIMEO
VIRTWIP_LAYOUT_OFFSET(struct netconn, send_timeout)
#endif
#if LWIP_SO_RCVTIMEO
VIRTWIP_LAYOUT_OFFSET(struct netconn, recv_timeout)
#endif
#if LWIP_SO_RCVBUF
VIRTWIP_LAYOUT_OFFSET(struct netconn, recv_bufsize)
VIRTWIP_LAYOUT_OFFSET(struct netconn, recv_avail)
#endif
#if LWIP_SO_LINGER
VIRTWIP_LAYOUT_OFFSET(struct netconn, linger)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netconn, flags)
#if LWIP_TCP
VIRTWIP_LAYOUT_OFFSET(struct netconn, current_msg)
#endif
VIRTWIP_LAYOUT_OFFSET(struct netconn, callback)

VIRTWIP_LAYOUT_SIZE(struct netbuf)
VIRTWIP_LAYOUT_OFFSET(struct netbuf, p)
VIRTWIP_LAYOUT_OFFSET(struct netbuf, ptr)
VIRTWIP_LAYOUT_OFFSET(struct netbuf, addr)
VIRTWIP_LAYOUT_OFFSET(struct netbuf, port)
#if LWIP_NETBUF_RECVINFO || LWIP_CHECKSUM_ON_COPY
VIRTWIP_LAYOUT_OFFSET(struct netbuf, flags)
VIRTWIP_LAYOUT_OFFSET(struct netbuf, toport_chksum)
#if LWIP_NETBUF_RECVINFO
VIRTWIP_LAYOUT_OFFSET(struct netbuf, toaddr)
#endif
#endif
#endif /* LWIP_NETCONN */

/* --- argument bundles and thread info (include_shared/shared.h) ----------- */
VIRTWIP_LAYOUT_SIZE(virtwip_thread_info_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, bpf_stack)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, bpf_stack_size)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, os_stack)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, os_stack_size)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, prio)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, vm_func)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, arg)
VIRTWIP_LAYOUT_OFFSET(virtwip_thread_info_t, name)
VIRTWIP_LAYOUT_SIZE(virtwip_input_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_input_args_t, p)
VIRTWIP_LAYOUT_OFFSET(virtwip_input_args_t, netif)
VIRTWIP_LAYOUT_SIZE(virtwip_netif_add_noaddr_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netif_add_noaddr_args_t, netif)
VIRTWIP_LAYOUT_OFFSET(virtwip_netif_add_noaddr_args_t, state)
VIRTWIP_LAYOUT_OFFSET(virtwip_netif_add_noaddr_args_t, init)
VIRTWIP_LAYOUT_OFFSET(virtwip_netif_add_noaddr_args_t, input)
VIRTWIP_LAYOUT_SIZE(virtwip_netif_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netif_args_t, netif)
VIRTWIP_LAYOUT_SIZE(virtwip_tcpip_init_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_tcpip_init_args_t, done)
VIRTWIP_LAYOUT_OFFSET(virtwip_tcpip_init_args_t, arg)
VIRTWIP_LAYOUT_SIZE(virtwip_ip6_addr_zone_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_ip6_addr_zone_args_t, ip6)
VIRTWIP_LAYOUT_OFFSET(virtwip_ip6_addr_zone_args_t, netif)
VIRTWIP_LAYOUT_OFFSET(virtwip_ip6_addr_zone_args_t, type)
VIRTWIP_LAYOUT_SIZE(virtwip_pbuf_alloc_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_alloc_args_t, layer)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_alloc_args_t, len)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_alloc_args_t, type)
VIRTWIP_LAYOUT_SIZE(virtwip_pbuf_p_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_p_args_t, p)
VIRTWIP_LAYOUT_SIZE(virtwip_pbuf_take_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_take_args_t, p)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_take_args_t, buf)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_take_args_t, len)
VIRTWIP_LAYOUT_SIZE(virtwip_pbuf_take_at_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_take_at_args_t, p)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_take_at_args_t, buf)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_take_at_args_t, len)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_take_at_args_t, offset)
VIRTWIP_LAYOUT_SIZE(virtwip_pbuf_header_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_header_args_t, p)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_header_args_t, size)
VIRTWIP_LAYOUT_SIZE(virtwip_pbuf_copy_partial_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_copy_partial_args_t, p)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_copy_partial_args_t, buf)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_copy_partial_args_t, len)
VIRTWIP_LAYOUT_OFFSET(virtwip_pbuf_copy_partial_args_t, offset)
VIRTWIP_LAYOUT_SIZE(virtwip_lowpan6_pan_id_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_lowpan6_pan_id_args_t, pan_id)
VIRTWIP_LAYOUT_SIZE(virtwip_netif_ip6_linklocal_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netif_ip6_linklocal_args_t, netif)
VIRTWIP_LAYOUT_OFFSET(virtwip_netif_ip6_linklocal_args_t, from_mac)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_new_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_new_args_t, type)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_new_args_t, proto)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_new_args_t, callback)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_ep_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_ep_args_t, conn)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_ep_args_t, addr)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_ep_args_t, port)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_args_t, conn)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_getaddr_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_getaddr_args_t, conn)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_getaddr_args_t, local)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_getaddr_args_t, addr)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_getaddr_args_t, port)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_recv_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_recv_args_t, conn)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_recv_args_t, buf)
VIRTWIP_LAYOUT_SIZE(virtwip_netbuf_alloc_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netbuf_alloc_args_t, buf)
VIRTWIP_LAYOUT_OFFSET(virtwip_netbuf_alloc_args_t, size)
VIRTWIP_LAYOUT_SIZE(virtwip_netbuf_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netbuf_args_t, buf)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_sendto_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_sendto_args_t, conn)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_sendto_args_t, netbuf)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_sendto_args_t, addr)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_sendto_args_t, port)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_send_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_send_args_t, conn)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_send_args_t, netbuf)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_write_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_write_args_t, conn)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_write_args_t, dataptr)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_write_args_t, len)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_write_args_t, apiflags)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_write_args_t, written)
VIRTWIP_LAYOUT_SIZE(virtwip_netconn_listen_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_listen_args_t, conn)
VIRTWIP_LAYOUT_OFFSET(virtwip_netconn_listen_args_t, backlog)
VIRTWIP_LAYOUT_SIZE(virtwip_sock_bind_addr_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_sock_bind_addr_args_t, addr)
VIRTWIP_LAYOUT_SIZE(virtwip_sock_netif_num_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_sock_netif_num_args_t, ip_type)
VIRTWIP_LAYOUT_OFFSET(virtwip_sock_netif_num_args_t, netif_num)
VIRTWIP_LAYOUT_SIZE(virtwip_sock_addr_on_netif_args_t)
VIRTWIP_LAYOUT_OFFSET(virtwip_sock_addr_on_netif_args_t, ip_type)
VIRTWIP_LAYOUT_OFFSET(virtwip_sock_addr_on_netif_args_t, netif_num)
VIRTWIP_LAYOUT_OFFSET(virtwip_sock_addr_on_netif_args_t, addr)
