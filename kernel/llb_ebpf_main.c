/*
 * llb_ebpf_main.c: LoxiLB TC eBPF Main processing
 * Copyright (C) 2022,  NetLOX <www.netlox.io>
 * 
 * SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
 */
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_arp.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

#define HAVE_DP_FC 1
#define HAVE_DP_EXTCT 1
#define HAVE_DP_SCTP_SUM 1

#include "llb_kern_entry.c"

char _license[] SEC("license") = "Dual BSD/GPL";
