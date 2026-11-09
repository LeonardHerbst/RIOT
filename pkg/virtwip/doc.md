@defgroup pkg_lwip   lwIP network stack
@ingroup  pkg
@ingroup  net
@brief    Provides the lwIP network stack
@see      https://savannah.nongnu.org/projects/lwip/
@see      https://www.nongnu.org/lwip/2_1_x/

lwIP is a lightweight TCP/IP stack primarily for usage with Ethernet.
It can be used with the @ref net_sock API.

## Debugging

Debug prints can be enabled by setting the defines from the [lwIP documentation]
in the package's Makefile in `pkg/lwip/Makefile`, for example

```makefile
CFLAGS += -DLWIP_DEBUG
CFLAGS += -DUDP_DEBUG=LWIP_DBG_ON
```

[lwIP documentation](https://www.nongnu.org/lwip/2_0_x/group__lwip__opts__debugmsg.html)

## Host/VM layout check

Host glue and VM image access some structures from both sides (`struct
netif`, `struct pbuf`, `struct netconn`, `struct netbuf` and the argument
bundles in `include_shared/shared.h`). Their layouts must be identical even
though both sides are built by different compilers and with different pointer
widths. Enable the optional check with

```makefile
USEMODULE += virtwip_layout_check
```

At VM startup the VM reports `sizeof`/`offsetof` of every entry in
`include_shared/virtwip_layout_list.h`, the host compares them with its own
values, prints every mismatch and refuses to start the VM if any differ.
Extend the list whenever a structure becomes shared across the border.
