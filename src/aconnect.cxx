/*
 * connect / disconnect two subscriber ports
 *   ver.0.1.3
 *
 * Copyright (C) 1999 Takashi Iwai
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#define ACONNECT_GUI

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

#ifdef ACONNECT_GUI
#include "MyWindow.hxx"
extern MyWindow* patchbay;
#endif

static void error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
	va_list arg;

	if (err == ENOENT)	/* Ignore those misleading "warnings" */
		return;
	va_start(arg, fmt);
	fprintf(stderr, "ALSA lib %s:%i:(%s) ", file, line, function);
	vfprintf(stderr, fmt, arg);
	if (err)
		fprintf(stderr, ": %s", snd_strerror(err));
	putc('\n', stderr);
	va_end(arg);
}

static void usage(void)
{
	fprintf(stderr, "aconnect - ALSA sequencer connection manager\n");
	fprintf(stderr, "Copyright (C) 1999-2000 Takashi Iwai\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, " * Connection/disconnection betwen two ports\n");
	fprintf(stderr, "   aconnect [-options] sender receiver\n");
	fprintf(stderr, "     sender, receiver = client:port pair\n");
	fprintf(stderr, "     -d,--disconnect     disconnect\n");
	fprintf(stderr, "     -e,--exclusive      exclusive connection\n");
	fprintf(stderr, "     -r,--real #         convert real-time-stamp on queue\n");
	fprintf(stderr, "     -t,--tick #         convert tick-time-stamp on queue\n");
	fprintf(stderr, " * List connected ports (no subscription action)\n");
	fprintf(stderr, "   aconnect -i|-o [-options]\n");
	fprintf(stderr, "     -i,--input          list input (readable) ports\n");
	fprintf(stderr, "     -o,--output         list output (writable) ports\n");
	fprintf(stderr, "     -l,--list           list current connections of each port\n");
	fprintf(stderr, " * Remove all exported connections\n");
	fprintf(stderr, "     -x, --removeall\n");
}

/*
 * check permission (capability) of specified port
 */

#define LIST_INPUT	1
#define LIST_OUTPUT	2

#define perm_ok(pinfo,bits) ((snd_seq_port_info_get_capability(pinfo) & (bits)) == (bits))

static int check_permission(snd_seq_port_info_t *pinfo, int perm)
{
	if (perm) {
		if (perm & LIST_INPUT) {
			if (perm_ok(pinfo, SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ))
				goto __ok;
		}
		if (perm & LIST_OUTPUT) {
			if (perm_ok(pinfo, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE))
				goto __ok;
		}
		return 0;
	}
 __ok:
	if (snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_NO_EXPORT)
		return 0;
	return 1;
}

/*
 * list subscribers of specified type
 */
static void list_each_subs(snd_seq_t *seq, snd_seq_query_subscribe_t *subs, int type, const char *msg)
{
	int count = 0;
#ifdef ACONNECT_GUI
	snd_seq_query_subscribe_set_type(subs, (snd_seq_query_subs_type_t) type);
#else
	snd_seq_query_subscribe_set_type(subs, type);
#endif
	snd_seq_query_subscribe_set_index(subs, 0);
	while (snd_seq_query_port_subscribers(seq, subs) >= 0) {
		const snd_seq_addr_t *addr;
#ifndef ACONNECT_GUI
		if (count++ == 0)
			printf("\t%s: ", msg);
		else
			printf(", ");
#endif
		addr = snd_seq_query_subscribe_get_addr(subs);
#ifdef ACONNECT_GUI
		if (patchbay->GetCurType()==kInput && type==SND_SEQ_QUERY_SUBS_WRITE && patchbay->GetCurPort()) 
		{
			Connector* output = 
				patchbay->FindOutput(addr->client, addr->port);
			if (output)
			{
				patchbay->MyWindow::Connect(output,patchbay->GetCurPort()->Input());
			}
		}
#else
		printf("%d:%d", addr->client, addr->port);
#endif
#ifndef ACONNECT_GUI
		if (snd_seq_query_subscribe_get_exclusive(subs))
			printf("[ex]");
		if (snd_seq_query_subscribe_get_time_update(subs))
			printf("[%s:%d]",
			       (snd_seq_query_subscribe_get_time_real(subs) ? "real" : "tick"),
			       snd_seq_query_subscribe_get_queue(subs));
#endif
		snd_seq_query_subscribe_set_index(subs, snd_seq_query_subscribe_get_index(subs) + 1);
	}
#ifndef ACONNECT_GUI
	if (count > 0)
		printf("\n");
#endif
}

/*
 * list subscribers
 */
static void list_subscribers(snd_seq_t *seq, const snd_seq_addr_t *addr)
{
	snd_seq_query_subscribe_t *subs;
	snd_seq_query_subscribe_alloca(&subs);
	snd_seq_query_subscribe_set_root(subs, addr);
	list_each_subs(seq, subs, SND_SEQ_QUERY_SUBS_READ, "Connecting To");
	list_each_subs(seq, subs, SND_SEQ_QUERY_SUBS_WRITE, "Connected From");
}

/*
 * search all ports
 */
typedef void (*action_func_t)(snd_seq_t *seq, snd_seq_client_info_t *cinfo, snd_seq_port_info_t *pinfo, int count);

static void do_search_port(snd_seq_t *seq, int perm, action_func_t do_action)
{
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	int count;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(seq, cinfo) >= 0) {
		/* reset query info */
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		count = 0;
		while (snd_seq_query_next_port(seq, pinfo) >= 0) {
			if (check_permission(pinfo, perm)) {
				do_action(seq, cinfo, pinfo, count);
				count++;
			}
		}
	}
}


static void print_port(snd_seq_t *seq, snd_seq_client_info_t *cinfo,
		       snd_seq_port_info_t *pinfo, int count)
{
	if (! count) {
#ifdef ACONNECT_GUI
		patchbay->AddClient(cinfo);
#else
		printf("client %d: '%s' [type=%s]\n",
		       snd_seq_client_info_get_client(cinfo),
		       snd_seq_client_info_get_name(cinfo),
		       (snd_seq_client_info_get_type(cinfo) == SND_SEQ_USER_CLIENT ? "user" : "kernel"));
#endif
	}
#ifdef ACONNECT_GUI
	patchbay->AddPort(pinfo);
#else
	printf("  %3d '%-16s' %d\n",
	       snd_seq_port_info_get_port(pinfo),
	       snd_seq_port_info_get_name(pinfo),
	       snd_seq_port_info_get_type(pinfo)
				 );

	printf("  %3d '%-16s'\n",
	       snd_seq_port_info_get_port(pinfo),
	       snd_seq_port_info_get_name(pinfo));
#endif
}

static void print_port_and_subs(snd_seq_t *seq, snd_seq_client_info_t *cinfo,
				snd_seq_port_info_t *pinfo, int count)
{
	print_port(seq, cinfo, pinfo, count);
	list_subscribers(seq, snd_seq_port_info_get_addr(pinfo));
}


/*
 * remove all (exported) connections
 */
static void remove_connection(snd_seq_t *seq, snd_seq_client_info_t *cinfo,
			      snd_seq_port_info_t *pinfo, int count)
{
	snd_seq_query_subscribe_t *query;

	snd_seq_query_subscribe_alloca(&query);
	snd_seq_query_subscribe_set_root(query, snd_seq_port_info_get_addr(pinfo));

	snd_seq_query_subscribe_set_type(query, SND_SEQ_QUERY_SUBS_READ);
	snd_seq_query_subscribe_set_index(query, 0);
	for (; snd_seq_query_port_subscribers(seq, query) >= 0;
	     snd_seq_query_subscribe_set_index(query, snd_seq_query_subscribe_get_index(query) + 1)) {
		snd_seq_port_info_t *port;
		snd_seq_port_subscribe_t *subs;
		const snd_seq_addr_t *sender = snd_seq_query_subscribe_get_root(query);
		const snd_seq_addr_t *dest = snd_seq_query_subscribe_get_addr(query);
		snd_seq_port_info_alloca(&port);
		if (snd_seq_get_any_port_info(seq, dest->client, dest->port, port) < 0)
			continue;
		if (!(snd_seq_port_info_get_capability(port) & SND_SEQ_PORT_CAP_SUBS_WRITE))
			continue;
		if (snd_seq_port_info_get_capability(port) & SND_SEQ_PORT_CAP_NO_EXPORT)
			continue;
		snd_seq_port_subscribe_alloca(&subs);
		snd_seq_port_subscribe_set_queue(subs, snd_seq_query_subscribe_get_queue(query));
		snd_seq_port_subscribe_set_sender(subs, sender);
		snd_seq_port_subscribe_set_dest(subs, dest);
		snd_seq_unsubscribe_port(seq, subs);
	}

	snd_seq_query_subscribe_set_type(query, SND_SEQ_QUERY_SUBS_WRITE);
	snd_seq_query_subscribe_set_index(query, 0);
	for (; snd_seq_query_port_subscribers(seq, query) >= 0;
	     snd_seq_query_subscribe_set_index(query, snd_seq_query_subscribe_get_index(query) + 1)) {
		snd_seq_port_info_t *port;
		snd_seq_port_subscribe_t *subs;
		const snd_seq_addr_t *dest = snd_seq_query_subscribe_get_root(query);
		const snd_seq_addr_t *sender = snd_seq_query_subscribe_get_addr(query);
		snd_seq_port_info_alloca(&port);
		if (snd_seq_get_any_port_info(seq, sender->client, sender->port, port) < 0)
			continue;
		if (!(snd_seq_port_info_get_capability(port) & SND_SEQ_PORT_CAP_SUBS_READ))
			continue;
		if (snd_seq_port_info_get_capability(port) & SND_SEQ_PORT_CAP_NO_EXPORT)
			continue;
		snd_seq_port_subscribe_alloca(&subs);
		snd_seq_port_subscribe_set_queue(subs, snd_seq_query_subscribe_get_queue(query));
		snd_seq_port_subscribe_set_sender(subs, sender);
		snd_seq_port_subscribe_set_dest(subs, dest);
		snd_seq_unsubscribe_port(seq, subs);
	}
}

static void remove_all_connections(snd_seq_t *seq)
{
	do_search_port(seq, 0, remove_connection);
}


/*
 * main..
 */

enum {
	SUBSCRIBE, UNSUBSCRIBE, LIST, REMOVE_ALL
};

static struct option long_option[] = {
	{"disconnect", 0, NULL, 'd'},
	{"input", 0, NULL, 'i'},
	{"output", 0, NULL, 'o'},
	{"real", 1, NULL, 'r'},
	{"tick", 1, NULL, 't'},
	{"exclusive", 0, NULL, 'e'},
	{"list", 0, NULL, 'l'},
	{"removeall", 0, NULL, 'x'},
	{NULL, 0, NULL, 0},
};

#ifdef ACONNECT_GUI
int aconnect_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	int c;
	snd_seq_t *seq;
	int queue = 0, convert_time = 0, convert_real = 0, exclusive = 0;
	int command = SUBSCRIBE;
	int list_perm = 0;
	int client;
	int list_subs = 0;
	snd_seq_port_subscribe_t *subs;
	snd_seq_addr_t sender, dest;

	optind = 0;

	while ((c = getopt_long(argc, argv, "dior:t:elx", long_option, NULL)) != -1) {
		switch (c) {
		case 'd':
			command = UNSUBSCRIBE;
			break;
		case 'i':
			command = LIST;
			list_perm |= LIST_INPUT;
			break;
		case 'o':
			command = LIST;
			list_perm |= LIST_OUTPUT;
			break;
		case 'e':
			exclusive = 1;
			break;
		case 'r':
			queue = atoi(optarg);
			convert_time = 1;
			convert_real = 1;
			break;
		case 't':
			queue = atoi(optarg);
			convert_time = 1;
			convert_real = 0;
			break;
		case 'l':
			list_subs = 1;
			break;
		case 'x':
			command = REMOVE_ALL;
			break;
		default:
			usage();
			exit(1);
		}
	}

#ifdef ACONNECT_GUI
	seq = patchbay->GetHandle();
#else
	if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		fprintf(stderr, "can't open sequencer\n");
		return 1;
	}
#endif
	
	snd_lib_error_set_handler(error_handler);

	switch (command) {
	case LIST:
		do_search_port(seq, list_perm,
			       list_subs ? print_port_and_subs : print_port);
#ifndef ACONNECT_GUI
		snd_seq_close(seq);
#endif
		return 0;
	case REMOVE_ALL:
		remove_all_connections(seq);
#ifndef ACONNECT_GUI
		snd_seq_close(seq);
#endif
		return 0;
	}

	/* connection or disconnection */

	if (optind + 2 > argc) {
#ifndef ACONNECT_GUI
		snd_seq_close(seq);
#endif
		usage();
		exit(1);
	}

	if ((client = snd_seq_client_id(seq)) < 0) {
#ifndef ACONNECT_GUI
		snd_seq_close(seq);
#endif
		fprintf(stderr, "can't get client id\n");
		return 1;
	}

	/* set client info */
	if (snd_seq_set_client_name(seq, "ALSA Connector") < 0) {
#ifndef ACONNECT_GUI
		snd_seq_close(seq);
#endif
		fprintf(stderr, "can't set client info\n");
		return 1;
	}

	/* set subscription */
	if (snd_seq_parse_address(seq, &sender, argv[optind]) < 0) {
		fprintf(stderr, "invalid sender address %s\n", argv[optind]);
		return 1;
	}
	if (snd_seq_parse_address(seq, &dest, argv[optind + 1]) < 0) {
		fprintf(stderr, "invalid destination address %s\n", argv[optind + 1]);
		return 1;
	}
	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	snd_seq_port_subscribe_set_queue(subs, queue);
	snd_seq_port_subscribe_set_exclusive(subs, exclusive);
	snd_seq_port_subscribe_set_time_update(subs, convert_time);
	snd_seq_port_subscribe_set_time_real(subs, convert_real);

	if (command == UNSUBSCRIBE) {
		if (snd_seq_get_port_subscription(seq, subs) < 0) {
#ifndef ACONNECT_GUI
			snd_seq_close(seq);
#endif		
			fprintf(stderr, "No subscription is found\n");
			return 1;
		}
		if (snd_seq_unsubscribe_port(seq, subs) < 0) {
#ifndef ACONNECT_GUI
			snd_seq_close(seq);
#endif
			fprintf(stderr, "Disconnection failed (%s)\n", snd_strerror(errno));
			return 1;
		}
	} else {
		if (snd_seq_get_port_subscription(seq, subs) == 0) {
#ifndef ACONNECT_GUI
			snd_seq_close(seq);
#endif
			fprintf(stderr, "Connection is already subscribed\n");
			return 1;
		}
		if (snd_seq_subscribe_port(seq, subs) < 0) {
#ifndef ACONNECT_GUI
			snd_seq_close(seq);
#endif
			fprintf(stderr, "Connection failed (%s)\n", snd_strerror(errno));
			return 1;
		}
	}

#ifndef ACONNECT_GUI
	snd_seq_close(seq);
#endif
	return 0;
}
