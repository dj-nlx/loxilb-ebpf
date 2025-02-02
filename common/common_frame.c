/*
 * Copyright (c) 2022 NetLOX Inc
 *
 * SPDX short identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <assert.h>
#include "common_frame.h"
#include "common_sum.h"

int
create_raw_tcp(void *packet, size_t *plen, struct mkr_args *args)
{
  size_t orig_len;
  struct iphdr *pip;
  struct tcphdr *ptcp;

  if (!packet || !plen) return -1;

  /* Unsupported for now */
  if (args->v6 || args->protocol != 0x6) return -1;
  orig_len = *plen;

  memset(packet, 0, orig_len);
  pip = (void *)packet;

  /* Fill in the IP header */
  pip->version = 4;
  pip->ihl = 5;
  pip->tot_len = htons(sizeof(struct iphdr)+sizeof(struct tcphdr));
  pip->id = 0xbeef;
  pip->frag_off = 0x0000;
  pip->protocol = 0x6;
  pip->saddr = htonl(args->sip[0]);
  pip->daddr = htonl(args->dip[0]);
  calc_ip_csum(pip);

  /* Fill in the TCP header */
  ptcp = (struct tcphdr *)(pip+1);
  ptcp->source = htons(args->sport);
  ptcp->dest = htons(args->dport);
  ptcp->seq = htonl(args->t.seq);
  ptcp->doff = 5;
  if (args->t.fin) {
    ptcp->fin = 1;
  }
  if (args->t.syn) {
    ptcp->syn = 1;
  }
  if (args->t.rst) {
    ptcp->rst = 1;
  }
  if (args->t.ack) {
    ptcp->ack = 1;
  }
  if (args->t.psh) {
    ptcp->psh = 1;
  }
  if (args->t.urg) {
    ptcp->urg = 1;
  }

  calc_tcp_checksum(pip, (void *)ptcp);

  return 0;
}

static int
xmit_raw(void *packet, size_t plen, struct mkr_args *args)  
{
  struct sockaddr_in caddr;
  int raw_socket;
  int hdr_incl = 1;
  int sent_bytes;

  if ((raw_socket = socket(AF_INET, SOCK_RAW, args->protocol)) < 0) {
    return -1;
  }

  if (setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL,
                 &hdr_incl, sizeof(hdr_incl)) < 0) {
    close(raw_socket);
    return -1;
  }

  memset(&caddr, 0, sizeof(caddr));
  caddr.sin_family = AF_INET;
  caddr.sin_port = htons(args->dport);
  caddr.sin_addr.s_addr = htonl(args->dip[0]);

  sent_bytes = sendto(raw_socket, packet, plen, 0,
                      (struct sockaddr *)&caddr,
                      sizeof(struct sockaddr_in));
  if (sent_bytes < 0) {
    close(raw_socket);
    return -1;
  }

  close(raw_socket);
  return 0;
}

int
create_xmit_raw_tcp(struct mkr_args *args)
{
  uint8_t frame[64] = { 0 };
  size_t len;
  int ret;

  len = sizeof(struct iphdr) + sizeof(struct tcphdr);

  ret = create_raw_tcp(frame, &len, args);
  if (ret < 0) {
    return -1;
  }

  return xmit_raw(frame, len, args);
}
