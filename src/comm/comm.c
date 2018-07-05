/*
 * $Id: comm.c 1019 2007-02-15 00:52:41Z zsuzsu $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

/* *INDENT-OFF* */
#include <sys/types.h>
#if	!defined(WIN32)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/telnet.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <sys/wait.h>
#else
#	include <winsock.h>
#	include <sys/timeb.h>
#endif

#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdarg.h>   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(SUNOS) || defined(SVR4) || defined(LINUX)
#	include <crypt.h>
#endif

#include "merc.h"
#include "debug.h"
#include "db/cmd.h"
#include "interp.h"
#include "quest.h"
#include "update.h"
#include "ban.h"
#include "charset.h"
#include "resolver.h"
#include "olc/olc.h"
#include "comm_info.h"
#include "comm_colors.h"
#include "db/lang.h"
#include "pdata.h"
#include "stats.h"
#include "fixes.h"

bool class_ok(CHAR_DATA *ch , int class);

 struct codepage {
	char* name;
	unsigned char* from;
	unsigned char* to;
};

struct codepage codepages[] = {
/*	{ "koi8-r",		koi8_koi8,	koi8_koi8	},
 *	{ "alt (cp866)",	alt_koi8,	koi8_alt	},
 *	{ "win (cp1251)",	win_koi8,	koi8_win	},
 *	{ "iso (ISO-8859-5)",	iso_koi8,	koi8_iso	},
 	{ "mac",		mac_koi8,	koi8_mac	},
	*/{ "Welcome to the World of Abatoir, `Tales of the Realm'",	koi8_koi8,	koi8_vola },
};
#define NCODEPAGES (sizeof(codepages) / sizeof(struct codepage))

/*
 * Socket and TCP/IP stuff.
 */

#if defined (WIN32)
#include <winsock.h>

void    gettimeofday    args( ( struct timeval *tp, void *tzp ) );

/*  Definitions for the TELNET protocol. Copied from telnet.h */

#define IAC		255	/* interpret as command: */
#define DONT		254	/* you are not to use option */
#define DO		253	/* please, you use option */
#define WONT		252	/* I won't use option */
#define WILL		251	/* I will use option */
#define SB		250	/* interpret as subnegotiation */
#define GA		249	/* you may reverse the line */
#define EL		248	/* erase the current line */
#define EC		247	/* erase the current character */
#define AYT		246	/* are you there */
#define AO		245	/* abort output--but let prog finish */
#define IP		244	/* interrupt process--permanently */
#define BREAK		243	/* break */
#define DM		242	/* data mark--for connect. cleaning */
#define NOP		241	/* nop */
#define SE		240	/* end sub negotiation */
#define EOR		239	/* end of record (transparent mode) */
#define SYNCH		242	/* for telfunc calls */

#define TELOPT_ECHO	1	/* echo */
#endif

char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
char 	go_ahead_str	[] = { IAC, GA, '\0' };

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[26];
time_t		    current_time;	/* time of this pulse */	
int                 iNumPlayers = 0; /* The number of players on */

extern int		max_on;

int	init_socket		(int port);
void	process_who		(int port);
void	init_descriptor		(int control);
void	close_descriptor	(DESCRIPTOR_DATA *d);
bool	read_from_descriptor	(DESCRIPTOR_DATA *d);
bool	write_to_descriptor	(int desc, char *txt, uint length);
void	game_loop_unix		(void);
#if !defined(WIN32)
void	resolv_done		(void);
#endif

/*
 * Other local functions (OS-independent).
 */
bool	check_reconnect		(DESCRIPTOR_DATA *d, const char *name,
				 bool fConn);
bool	check_playing		(DESCRIPTOR_DATA *d, const char *name);
int	main			(int argc, char **argv);
void	nanny			(DESCRIPTOR_DATA *d, const char *argument);
bool	process_output		(DESCRIPTOR_DATA *d, bool fPrompt);
void	read_from_buffer	(DESCRIPTOR_DATA *d);
void	stop_idling		(CHAR_DATA *ch);
void    bust_a_prompt           (CHAR_DATA *ch);
void 	log_area_popularity	(void);

varr 	control_sockets = { sizeof(int), 2 };
varr	info_sockets = { sizeof(int), 2 };
varr	info_trusted = { sizeof(struct in_addr), 2 };

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-p port...] [-i port...]\n"
			"Where:\n"
			"\t-p -- listen port\n"
			"\t-i -- info service port\n",
		get_filename(name));
	exit(1);
}

#define GETINT(v, i) (*(int*) VARR_GET(v, i))

static void open_sockets(varr *v, const char *logm)
{
	int i, j;

	for (i = 0, j = 0; i < v->nused; i++) {
		int port = GETINT(v, i);
		int sock;
		if ((sock = init_socket(port)) < 0)
			continue;
		log_printf(logm, port);
		GETINT(v, j++) = sock;
	}
	v->nused = j;
}

void close_sockets(varr *v)
{
	int i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
	}
}

int main(int argc, char **argv)
{
	struct timeval now_time;
	int ch;
	int check_info;

#if defined WIN32
	WORD	wVersionRequested = MAKEWORD(1, 1);
	WSADATA	wsaData;
	int err;
#endif
	/*
	 * Memory debugging if needed.
	 */
#if defined(MALLOC_DEBUG)
	malloc_debug(2);
#endif

	setlocale(LC_ALL, "");

	/*
	 * Init time.
	 */
	gettimeofday(&now_time, NULL);
	current_time 	= (time_t) now_time.tv_sec;
	strnzcpy(str_boot_time, sizeof(str_boot_time), strtime(current_time));

	/*
	 * Run the game.
	 */
	
#if defined (WIN32)
	srand((unsigned) time(NULL));
	err = WSAStartup(wVersionRequested, &wsaData); 
	if (err) {
		log_printf("winsock.dll: %s", strerror(errno));
		exit(1);
	}
#else
	resolver_init();
#endif

	debug_init();
	boot_db_system();

	if (argc > 1) {
		/*
		 * command line parameters override configuration settings
		 */
		control_sockets.nused = 0;
		info_sockets.nused = 0;

		opterr = 0;
		while ((ch = getopt(argc, argv, "p:i:")) != -1) {
			int *p;

			switch (ch) {
			case 'p':
				if (!is_number(optarg))
					usage(argv[0]);
				p = varr_enew(&control_sockets);
				*p = atoi(optarg);
				break;

			case 'i':
				if (!is_number(optarg))
					usage(argv[0]);
				p = varr_enew(&info_sockets);
				*p = atoi(optarg);
				break;

			default:
				usage(argv[0]);
			}
		}
		argc -= optind;
		argv += optind;
	}

	if (!control_sockets.nused) {
		log_printf("no control sockets defined");
		exit(1);
	}
	check_info = (!!info_sockets.nused);

	boot_db();

	open_sockets(&control_sockets, "now listening for user logins on port %d");
	open_sockets(&info_sockets, "info service started on port %d");

	if (!control_sockets.nused) {
		log_printf("no control sockets could be opened.");
		exit(1);
	}

	if (check_info && !info_sockets.nused) {
		log_printf("no info service sockets could be opened.");
		exit(1);
	}

	game_loop_unix();

	close_sockets(&control_sockets);
	close_sockets(&info_sockets);

#if defined (WIN32)
	WSACleanup();
#else
	resolver_done();
#endif
	log_area_popularity();

	/*
	 * That's all, folks.
	 */
	log("Normal termination of game.");
	return 0;
}

int init_socket(int port)
{
	static struct sockaddr_in sa_zero;
	struct sockaddr_in sa;
	struct linger ld;
	int x = 1;
	int fd;

#if defined (WIN32)
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
#else
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
		log_printf("init_socket(%d): socket: %s",
			   port, strerror(errno));
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		       (char *) &x, sizeof(x)) < 0) {
		log_printf("init_socket(%d): setsockopt: SO_REUSEADDR: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	ld.l_onoff  = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
		       (char *) &ld, sizeof(ld)) < 0) {
		log_printf("init_socket(%d): setsockopt: SO_LINGER: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	sa		= sa_zero;
#if !defined (WIN32)
	sa.sin_family   = AF_INET;
#else
	sa.sin_family   = PF_INET;
#endif
	sa.sin_port	= htons(port);

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		log_printf("init_socket(%d): bind: %s", port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	if (listen(fd, 3) < 0) {
		log_printf("init_socket(%d): listen: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	return fd;
}

static void add_fds(varr *v, fd_set *in_set, int *maxdesc)
{
	int i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
		FD_SET(fd, in_set);
		if (*maxdesc < fd) *maxdesc = fd;
	}
}

static void check_fds(varr *v, fd_set *in_set, void (*new_conn_cb)(int))
{
	int i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
		if (FD_ISSET(fd, in_set))
			new_conn_cb(fd);
	}
}

void game_loop_unix(void)
{
	static struct timeval null_time;
	struct timeval last_time;
 
	gettimeofday(&last_time, NULL);
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while (!merc_down) {
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		DESCRIPTOR_DATA *d;
		INFO_DESC *id;
		INFO_DESC *id_next;
		int maxdesc;

#if defined(MALLOC_DEBUG)
		if (malloc_verify() != 1)
			abort();
#endif

		/*
		 * Poll all active descriptors.
		 */
		FD_ZERO(&in_set );
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);

		maxdesc = 0;
		add_fds(&control_sockets, &in_set, &maxdesc);
		add_fds(&info_sockets, &in_set, &maxdesc);

#if !defined (WIN32)
		FD_SET(fileno(rfin), &in_set);
		maxdesc = UMAX(maxdesc, fileno(rfin));
#endif

		for (d = descriptor_list; d; d = d->next) {
			maxdesc = UMAX(maxdesc, d->descriptor);
			FD_SET(d->descriptor, &in_set );
			FD_SET(d->descriptor, &out_set);
			FD_SET(d->descriptor, &exc_set);
		}

		for (id = id_list; id; id = id->next) {
			maxdesc = UMAX(maxdesc, id->fd);
			FD_SET(id->fd, &in_set);
		}

		if (select(maxdesc+1,
			   &in_set, &out_set, &exc_set, &null_time) < 0) {
			log_printf("game_loop: select: %s", strerror(errno));
			exit(1);
		}

#if !defined (WIN32)
		if (FD_ISSET(fileno(rfin), &in_set))
			resolv_done();
#endif

		check_fds(&control_sockets, &in_set, init_descriptor);
		check_fds(&info_sockets, &in_set, info_newconn);

		for (id = id_list; id; id = id_next) {
			id_next = id->next;

			if (FD_ISSET(id->fd, &in_set))
				info_process_cmd(id);
		}

		/*
		 * Kick out the freaky folks.
		 */
		for (d = descriptor_list; d; d = d_next) {
			d_next = d->next;   
			if (FD_ISSET(d->descriptor, &exc_set)) {
				FD_CLR(d->descriptor, &in_set );
				FD_CLR(d->descriptor, &out_set);
				if (d->character && d->character->level > 1)
					save_char_obj(d->character, FALSE);
				d->outtop = 0;
				close_descriptor(d);
			}
		}

		/*
		 * Process input.
		 */
		for (d = descriptor_list; d != NULL; d = d_next) {
			d_next		= d->next;
			d->fcommand	= FALSE;

			if (FD_ISSET(d->descriptor, &in_set)) {
				if (d->character != NULL)
					d->character->timer = 0;

				if (!read_from_descriptor(d)) {
					FD_CLR(d->descriptor, &out_set);
					if (d->character != NULL
					&&  d->character->level > 1)
						save_char_obj(d->character,
							      FALSE);
					d->outtop = 0;
					close_descriptor(d);
					continue;
				}
			}

			if (d->character != NULL && d->character->daze > 0)
				--d->character->daze;

			if (d->character != NULL && d->character->wait > 0) {
				--d->character->wait;
				continue;
			}

			read_from_buffer(d);
			if (d->incomm[0] != '\0') {
				d->fcommand = TRUE;
				stop_idling(d->character);

				if (d->showstr_point)
					show_string(d, d->incomm);
				else if (d->pString)
					string_add(d->character, d->incomm);
				else if (d->connected == CON_PLAYING) {
					if (!run_olc_editor(d))
			    			substitute_alias(d, d->incomm);
				}
				else
					nanny(d, d->incomm);

				if (d->connected != CON_RESOLV)
					d->incomm[0]	= '\0';
			}
		}

		/*
		 * Autonomous game motion.
		 */
		update_handler();

		/*
		 * Output.
		 */
		for (d = descriptor_list; d != NULL; d = d_next) {
			d_next = d->next;

			if ((d->fcommand || d->outtop > 0)
			&&  FD_ISSET(d->descriptor, &out_set)) {
				if (!process_output(d, TRUE)) {
					if (d->character != NULL
					&&  d->character->level > 1)
						save_char_obj(d->character, FALSE);
					d->outtop = 0;
					close_descriptor(d);
				}
			}
		}

	/*
	 * Synchronize to a clock.
	 * Sleep(last_time + 1/PULSE_PER_SCD - now).
	 * Careful here of signed versus unsigned arithmetic.
	 */
#if !defined (WIN32)
	 {
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday(&now_time, NULL);
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SCD;
	    secDelta	= ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
	    while (usecDelta < 0) {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while (usecDelta >= 1000000) {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if (select(0, NULL, NULL, NULL, &stall_time) < 0) {
		    log_printf("game_loop: select: stall: %s", strerror(errno));
		    exit(1);
		}
	    }
	}
#else
	{
	    int times_up;
	    int nappy_time;
	    struct _timeb start_time;
	    struct _timeb end_time;
	    _ftime( &start_time );
	    times_up = 0;

	    while( times_up == 0 )
	    {
			_ftime( &end_time );
			if ( ( nappy_time = (int) ( 1000 * (double) ( ( end_time.time - start_time.time ) +
				       ( (double) ( end_time.millitm - start_time.millitm ) /
					1000.0 ) ) ) ) >= (double)( 1000 / PULSE_PER_SCD ) )
			  times_up = 1;
		else
		{
		    Sleep( (int) ( (double) ( 1000 / PULSE_PER_SECOND ) -
				  (double) nappy_time ) );
		    times_up = 1;
		}
	  }
	}
#endif
		gettimeofday(&last_time, NULL);
		current_time = (time_t) last_time.tv_sec;
	}
}

static void cp_print(DESCRIPTOR_DATA* d)
{
/*	char buf[MAX_STRING_LENGTH];
	int i;

	write_to_buffer(d, "\n\r", 0);
	for (i = 0; i < NCODEPAGES; i++) {
		snprintf(buf, sizeof(buf), "%s%d. %s",
			 i ? " " : "", i+1, codepages[i].name);
		write_to_buffer(d, buf, 0);
	}*/ 
	write_to_buffer(d, 
	"\n\rPlease enter the name you wish to be called by in this realm: ", 0);
} 

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

void init_descriptor(int control)
{
	DESCRIPTOR_DATA *dnew;
	struct sockaddr_in sock;
	int desc;
	socklen_t size;
	HELP_DATA *greeting;

	size = sizeof(sock);
	getsockname(control, (struct sockaddr *) &sock, &size);
	if ((desc = accept(control, (struct sockaddr *) &sock, &size)) < 0) {
		log_printf("init_descriptor: accept: %s", strerror(errno));
		return;
	}

#if !defined (WIN32)
	if (fcntl(desc, F_SETFL, FNDELAY) < 0) {
		log_printf("init_descriptor: fcntl: FNDELAY: %s",
			   strerror(errno));
		return;
	}
#endif

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor();

	dnew->descriptor	= desc;
	dnew->connected		= CON_GET_NAME;
	dnew->showstr_head	= NULL;
	dnew->showstr_point	= NULL;
	dnew->pString		= NULL;
	dnew->olced		= NULL;
	dnew->pEdit		= NULL;
	dnew->pEdit2		= NULL;
	dnew->outsize		= 2000;
	dnew->outbuf		= malloc(dnew->outsize);
	dnew->wait_for_se	= 0;
	dnew->codepage		= codepages; 
	dnew->host		= NULL;

	size = sizeof(sock);
	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
		log_printf("init_descriptor: getpeername: %s",
			   strerror(errno));
		return;
	}
#if defined (WIN32)
	else {
		/* Copying from ROM 2.4b6 */
		int addr;
		struct hostent *from;

		addr = ntohl(sock.sin_addr.s_addr);
		from = gethostbyaddr((char *) &sock.sin_addr,
				     sizeof(sock.sin_addr), AF_INET);
		dnew->host = str_dup(from ? from->h_name : "unknown");
	}
#endif

	log_printf("sock.sinaddr: %s", inet_ntoa(sock.sin_addr));

	dnew->next		= descriptor_list;
	descriptor_list		= dnew;

	/*
	 * Send the greeting.
	 */
	if ((greeting = help_lookup(1, "GREETING"))) {
		char buf[MAX_STRING_LENGTH];
		parse_colors(mlstr_mval(greeting->text), buf, sizeof(buf),
			     FORMAT_DUMB);
		write_to_buffer(dnew, buf + (buf[0] == '.'), 0);
	}
	cp_print(dnew);
}

void close_descriptor(DESCRIPTOR_DATA *dclose)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d;

	if (dclose->outtop > 0)
		process_output(dclose, FALSE);

	if (dclose->snoop_by != NULL) 
		write_to_buffer(dclose->snoop_by,
				"Your victim has left the game.\n\r", 0);

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->snoop_by == dclose)
			d->snoop_by = NULL;

	if ((ch = dclose->character) != NULL) {
		log_printf("Closing link to %s.", ch->name);
		if (dclose->connected == CON_PLAYING) {

			if (!IS_IMMORTAL(ch)
			&& ch->in_room->area->clan != CLAN_FREEMAN
			&& ch->clan != ch->in_room->area->clan)
				ch->in_room->area->interlopers--;

			act("$n has drifted into the astral plane.", 
				ch, NULL, NULL, TO_ROOM);
			wiznet("Net death has claimed $N.", ch, NULL,
			       WIZ_LINKS, 0, 
			       (ch->invis_level > ch->incog_level) ? ch->invis_level
			       					   : ch->incog_level);
			if (ch->pcdata->pdata)
				ch->pcdata->pdata->online = FALSE;
			ch->desc = NULL;
		}
		else
	    		free_char(dclose->character);
	}

	if (d_next == dclose)
		d_next = d_next->next;   

	if (dclose == descriptor_list)
		descriptor_list = descriptor_list->next;
	else {
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d && d->next != dclose; d = d->next)
			;
		if (d != NULL)
			d->next = dclose->next;
		else
			bug("Close_socket: dclose not found.", 0);
	}

#if !defined( WIN32 )
	close(dclose->descriptor);
#else
	closesocket(dclose->descriptor);
#endif
	free_descriptor(dclose);
}

bool read_from_descriptor(DESCRIPTOR_DATA *d)
{
	int iOld;
	int iStart;
	unsigned char *p, *q;

	/* 
	 * Hold horses if pending command already
	 */
	if (d->incomm[0] != '\0')
		return TRUE;

	/* Check for overflow. */
	iOld = iStart = strlen(d->inbuf);
	if (iStart >= sizeof(d->inbuf) - 10) {
		log_printf("%s input overflow!", d->host);
		write_to_descriptor(d->descriptor,
				    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
		return FALSE;
	}

	for (; ;) {
		int nRead;

#if !defined (WIN32)
	nRead = read( d->descriptor, d->inbuf + iStart,
		     sizeof( d->inbuf ) - 10 - iStart );
#else
	nRead = recv( d->descriptor, d->inbuf + iStart,
		     sizeof( d->inbuf ) - 10 - iStart, 0 );
#endif
		if (nRead > 0) {
			iStart += nRead;
			if (d->inbuf[iStart-1] == '\n'
			||  d->inbuf[iStart-1] == '\r')
				break;
		}
		else if (nRead == 0) {
			log("EOF encountered on read.");
			return FALSE;
			break;
		}
#if !defined (WIN32)
		else if (errno == EWOULDBLOCK)
			break;
#else
        else if ( WSAGetLastError() == WSAEWOULDBLOCK)
	    break;
#endif
		else {
			log_printf("read_from_descriptor: %s", strerror(errno));
			return FALSE;
		}
	}

	d->inbuf[iStart] = '\0';
	if (iOld == iStart)
		return TRUE;

	for (p = ((unsigned char *) d->inbuf+iOld); *p;) {
		unsigned char *r;

		if (*p != IAC
		||  (d->connected == CON_PLAYING &&
		     d->character &&
		     IS_SET(d->character->comm, COMM_NOTELNET))) {
			p++;
			continue;
		}

		if (d->wait_for_se)
			goto wse;

		switch (p[1]) {
		case DONT:
		case DO:
		case WONT:
		case WILL:
			q = p+3;
			break;

		wse:
		case SB:   
			q = (unsigned char *) strchr((char *)p, SE);
			if (q == NULL) {
				q = (unsigned char *) strchr((char *)p, '\0');
				d->wait_for_se = 1; 
			}
			else {
				q++; 
				d->wait_for_se = 0; 
			}
			break;

		case IAC:
			memmove(p, p+1, strlen((char *)p));
			p++;
			continue;
			/* NOTREACHED */

		default:
			q = p+2;
			break;
		}
		if ((r = (unsigned char *) strchr((char *)p, '\0')) < q)
			q = r;
		memmove(p, q, strlen((char *) q)+1);
	} 

	return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(DESCRIPTOR_DATA *d)
{
	int i, j, k;

	/*
	 * Hold horses if pending command already.
	 */
	if (d->incomm[0] != '\0')
		return;

	/*
	 * Look for at least one new line.
	 */
	for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
		if (d->inbuf[i] == '\0')
			return;

	/*
	 * Canonical input processing.
	 */
	for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (k >= MAX_INPUT_LENGTH - 2) {
			write_to_descriptor(d->descriptor,
					    "Line too long.\n\r", 0);

			/* skip the rest of the line */
			for (; d->inbuf[i] != '\0'; i++)
				if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
					break;

			d->inbuf[i]   = '\n';
			d->inbuf[i+1] = '\0';
			break;
		}

		if (d->inbuf[i] == '\b' && k > 0)
			--k;
		else if ((unsigned)d->inbuf[i] >= ' ')
			d->incomm[k++] =
			 d->codepage->from[(unsigned char) d->inbuf[i]];
	}

	/*
	 * Finish off the line.
	 */
	if (k == 0)
		d->incomm[k++] = ' ';
	d->incomm[k] = '\0';

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */
	if (k > 1 || d->incomm[0] == '!') {
		if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast))
			d->repeat = 0;
		else {
			CHAR_DATA *ch = d->original ? d->original :
						      d->character;
			if (ch && ++d->repeat >= 100) {
				char buf[MAX_STRING_LENGTH];

				log_printf("%s input spamming!", d->host);
				snprintf(buf, sizeof(buf),
					 "Inlast:[%s] Incomm:[%s]!",
					 d->inlast, d->incomm);
					
				wiznet("SPAM SPAM SPAM $N spamming, and OUT!",
					ch, NULL, WIZ_SPAM, 0, ch->level);
				wiznet("[$N]'s $t!",
					ch, buf, WIZ_SPAM, 0, ch->level);

				write_to_descriptor(d->descriptor, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
				d->repeat = 0;
				if (d->showstr_point) {
					if (d->showstr_head) {
						free_string(d->showstr_head);
						d->showstr_head = NULL;
					}
					d->showstr_point = NULL;
				}
				if (d->pString) {
					free_string(*d->pString);
					*d->pString = d->backup;
					d->pString = NULL;
				}
				strnzcpy(d->incomm, sizeof(d->incomm), "quit");
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if (d->incomm[0] == '!')
		strnzcpy(d->incomm, sizeof(d->incomm), d->inlast);
	else
		strnzcpy(d->inlast, sizeof(d->inlast), d->incomm);

	/*
	 * Shift the input buffer.
	 */
	while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		i++;
	for (j = 0; (d->inbuf[j] = d->inbuf[i+j]) != '\0'; j++)
		;
}

/*
 * Low level output function.
 */
void battle_prompt(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int percent;
	char* msg;
 
        if (victim->max_hit > 0)
		percent = victim->hit * 100 / victim->max_hit;
        else
		percent = -1;
  
        if (percent >= 100)
		msg = "{Cis in perfect health{x.";
        else if (percent >= 90)
		msg = "{bhas a few scratches{x.";
        else if (percent >= 75)
		msg = "{Bhas some small but disgusting cuts{x.";
        else if (percent >= 50)
		msg = "{Gis covered with bleeding wounds{x.";
        else if (percent >= 30)
		msg = "{Yis gushing blood{x.";
        else if (percent >= 15)
		msg = "{Mis writhing in agony{x.";
        else if (percent >= 0)
		msg = "{Ris convulsing on the ground{x.";
        else
		msg = "{Ris nearly dead{x.";

	act_puts("$N $t", ch, msg, victim, TO_CHAR | ACT_TRANS, POS_DEAD);
}

/*
 * Some specials added by KIO 
 */
bool process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
	extern bool merc_down;
	bool ga = FALSE;

	/*
	 * Bust a prompt.
	 */
	if (!merc_down) {
		CHAR_DATA *ch = d->character;

		if (d->showstr_point) {
			write_to_buffer(d, "[Hit Return to continue]\n\r", 0);
			ga = TRUE;
		}
		else if (fPrompt && d->connected == CON_PLAYING) {
			if (d->pString) {
				write_to_buffer(d, "  > ", 0);
				ga = TRUE;
			}
			else if (ch) {
				CHAR_DATA *victim;

				/* battle prompt */
				if ((victim = ch->fighting) != NULL
				&&  can_see(ch,victim))
					battle_prompt(ch, victim);

				if (!IS_SET(ch->comm, COMM_COMPACT))
					write_to_buffer(d, "\n\r", 2);

				if (IS_SET(ch->comm, COMM_PROMPT))
					bust_a_prompt(d->character);
				ga = TRUE;
			}
		}

		if (ch && ga && !IS_SET(ch->comm, COMM_TELNET_GA))
			ga = FALSE;
	}

	/*
	 * Short-circuit if nothing to write.
	 */
	if (d->outtop == 0)
		return TRUE;

	/*
	 * Snoop-o-rama.
	 */
	if (d->snoop_by) {
		if (d->character)
			write_to_buffer(d->snoop_by, d->character->name, 0);
		write_to_buffer(d->snoop_by, "> ", 2);
		write_to_buffer(d->snoop_by, d->outbuf, d->outtop);
	}

	/*
	 * OS-dependent output.
	 */
	if (!write_to_descriptor(d->descriptor, d->outbuf, d->outtop)) {
		d->outtop = 0;
		return FALSE;
	}
	else {
		if (ga)
			write_to_descriptor(d->descriptor, go_ahead_str, 0);
		d->outtop = 0;
		return TRUE;
	}
}

void percent_hp(CHAR_DATA *ch, char buf[MAX_STRING_LENGTH])
{
	if (ch->hit >= 0)
		snprintf(buf, sizeof(buf), "%d%%",
			 ((100 * ch->hit) / UMAX(1,ch->max_hit)));
	else
		strnzcpy(buf, sizeof(buf), "BAD!");
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * bust
 */
void bust_a_prompt(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	const char *str;
	const char *i = "";
	char *point;
	CHAR_DATA *victim;
	EXIT_DATA *pexit;
	OBJ_DATA *obj = NULL;
	bool found;
	const char *dir_name[] = {"N","E","S","W","U","D"};
	int door;
 
	if (IS_SET(ch->comm, COMM_AFK)) {
		char_printf(ch, "{c<AFK>{x %s", ch->prefix);
		return;
	}

	point = buf;
	str = ch->prompt;
	if (IS_NULLSTR(str))
		str = DEFAULT_PROMPT;

	while (*str != '\0') {
		if (*str != '%') {
			*point++ = *str++;
			continue;
		}

		switch(*++str) {
		default:
	        	i = "";
			break;

		/* alignment */
		case 'a':
			i = IS_GOOD(ch) ? "good" :
			    IS_EVIL(ch) ? "evil" :
			    "neutral";
			break;

		/* encumberance */
		case 'c':
			snprintf(buf2, sizeof(buf2), "%d%%", 
				ENCUMBERANCE(ch));
			i = buf2;
			break;

		/* exits */
		case 'e':
			found = FALSE;
			buf2[0] = '\0';
			for (door = 0; door < 6; door++)
				if ((pexit = ch->in_room->exit[door])
				&&  pexit->to_room.r
				&&  can_see_room(ch, pexit->to_room.r)
				&&  check_blind_raw(ch)
				&&  (!IS_SET(pexit->exit_info, EX_CLOSED) ||
				     IS_IMMORTAL(ch))) {
					found = TRUE;
					strnzcat(buf2, sizeof(buf2),
						 dir_name[door]);
					if (IS_SET(pexit->exit_info, EX_CLOSED))
						strnzcat(buf2, sizeof(buf2),
							 "*");
				}
			if (buf2[0])
				strnzcat(buf2, sizeof(buf2), " ");
			i = buf2;
			break;

		/* IMM edit mode */
		case 'E':
			i = OLCED(ch) ? OLCED(ch)->name : str_empty;
			if (!IS_NULLSTR(i)) {
				snprintf(buf2, sizeof(buf2), "%s", i);
				i = buf2;
			}
			break;

		/* gold */
		case 'g':
			snprintf(buf2, sizeof(buf2), "%d", ch->gold);
			i = buf2;
			break;

		/* hp */
		case 'h':
			snprintf(buf2, sizeof(buf2), "%d", ch->hit);
			i = buf2;
			break;

		/* max hp */
		case 'H':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_hit);
			i = buf2;
			break;

		/* IMM incog level */
		case 'I':
			if (ch->incog_level) {
				snprintf(buf2, sizeof(buf2), "%d",
					 ch->incog_level);
				i = buf2;
			}
			else
				i = "";
			break;

		/* mana */
		case 'm':
			snprintf(buf2, sizeof(buf2), "%d", ch->mana);
			i = buf2;
			break;

		/* max mana */
		case 'M':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_mana);
			i = buf2;
			break;

		/* name */
		case 'n':
			i = ch->name;
			break;

		/* hp % other */
		case 'o':
			if ((victim = ch->fighting) != NULL) {
				if (can_see(ch, victim)) {
					percent_hp(victim, buf2);
					i = buf2;
				}
				else
					i = "???";
			}
			else
				i = "None";
			break;

		/* darkness */
		case 'r':
			if (ch->in_room)
				i = (check_blind_raw(ch) && !room_is_dark(ch)) ?
				     mlstr_cval(ch->in_room->name, ch) :
				     "darkness";
			else
				i = "";
			break;

		/* IMM room*/
		case 'R':
			if (IS_IMMORTAL(ch) && ch->in_room) {
				snprintf(buf2, sizeof(buf2), "%d",
					 ch->in_room->vnum);
				i = buf2;
			}
			else
				i = "";
			break;

		/* silver */
		case 's':
			snprintf(buf2, sizeof(buf2), "%d", ch->silver);
			i = buf2;
			break;

		/* sex */
		case 'S':
			i = ch->sex == SEX_MALE ? "Male" :
			    ch->sex == SEX_FEMALE ? "Female" :
			    "None";
			break;

		/* time */
		case 't':
			snprintf(buf2, sizeof(buf2),
				 "%d%s", (time_info.hour % 12 == 0) ? 
				 12 : time_info.hour % 12, 
				 time_info.hour >= 12 ? "pm" : "am");
			i = buf2;
			break;

		/* terrain */	
		case 'T': 
			switch (ch->in_room->sector_type) {
			case SECT_INSIDE:
				snprintf(buf2, sizeof(buf2), "{DI{x");
				i = buf2;
				break;
			case SECT_CITY:
				snprintf(buf2, sizeof(buf2), "{YC{x");
				i = buf2;
				break;
			case SECT_FIELD:
				snprintf(buf2, sizeof(buf2), "{GG{x");
				i = buf2;
				break;
			case SECT_FOREST:
				snprintf(buf2, sizeof(buf2), "{gF{x");
				i = buf2;
				break;
			case SECT_HILLS:
				snprintf(buf2, sizeof(buf2), "{yH{x");
				i = buf2;
				break;
			case SECT_MOUNTAIN:
				snprintf(buf2, sizeof(buf2), "{yM{x");
				i = buf2;
				break;
			case SECT_WATER_SWIM:
				snprintf(buf2, sizeof(buf2), "{bW{x");
				i = buf2;
				break;
			case SECT_WATER_NOSWIM:
				snprintf(buf2, sizeof(buf2), "{bW{x");
				i = buf2;
				break;
			case SECT_UNUSED:
				snprintf(buf2, sizeof(buf2), "-");
				i = buf2;
				break;
			case SECT_AIR:
				snprintf(buf2, sizeof(buf2), "{CA{x");
				i = buf2;
				break;
			case SECT_DESERT:
				snprintf(buf2, sizeof(buf2), "{YD{x");
				i = buf2;
				break;
			case SECT_ARCTIC:
				snprintf(buf2, sizeof(buf2), "{WS{x");
				i = buf2;
				break;
			case SECT_ROAD:
				snprintf(buf2, sizeof(buf2), "{DR{x");
				i = buf2;
				break;

			default:
				char_printf(ch, "?");
				break;
			}
			break;

		/* move */
		case 'v':
			snprintf(buf2, sizeof(buf2), "%d", ch->move);
			i = buf2;
			break;

		/* max move */
		case 'V':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_move);
			i = buf2;
			break;

		/* IMM wizi level */
		case 'W':
			if (ch->invis_level) {
				snprintf(buf2, sizeof(buf2), "%d",
					 ch->invis_level);
				i = buf2;
			}
			else
				i = "";
			break;

		/* exp */
		case 'x':
			if (ch->level >= LEVEL_HERO) {
				obj = get_augment_obj(ch);
				if (obj)
					snprintf(buf2, sizeof(buf2), "%d", 
						obj->augment->exp);
				else
					snprintf(buf2, sizeof(buf2), "-");
			}
			else {
				snprintf(buf2, sizeof(buf2), "%d", ch->exp);
			}
			i = buf2;
			break;

		/* tnl */
		case 'X':
			if (!IS_NPC(ch)) {
				if (ch->level >= LEVEL_HERO) {
					obj = get_augment_obj(ch);
					if (obj)
						snprintf(buf2, sizeof(buf2), "%d", 
							augment_tnl(obj));
					else
						snprintf(buf2, sizeof(buf2), "-");
				}
				else
					snprintf(buf2, sizeof(buf2), "%d",
						 exp_to_level(ch));
				i = buf2;
			}
			else
				i = "";
			break;

		/* your hp %*/
		case 'y':
			percent_hp(ch, buf2);
			i = buf2;
			break;

		/* IMM area name */
		case 'z':
			if (IS_IMMORTAL(ch) && ch->in_room != NULL)
				i = ch->in_room->area->name;
			else
				i = "";
			break;

		case '%':
			i = "%%";
			break;

		}
		++str;
		while((*point = *i) != '\0')
			++point, ++i;
	}

	*point = '\0';
	send_to_char(buf, ch);

	if (ch->prefix[0] != '\0')
		write_to_buffer(ch->desc, ch->prefix, 0);
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, uint length)
{
	uint size;
	int i;
	bool noiac = (d->connected == CON_PLAYING &&
		      d->character != NULL &&
		      IS_SET(d->character->comm, COMM_NOIAC));

	if (d->connected == CON_DUMMY)
		return;
	
	/*
	 * Find length in case caller didn't.
	 */
	if (length <= 0)
		length = strlen(txt);

	/*
	 * Adjust size in case of IACs (they will be doubled)
	 */
	size = length;
	if (!noiac)
		for (i = 0; i < length; i++)
			if (d->codepage->to[(unsigned char) txt[i]] == IAC)
				size++;

	/*
	 * Initial \n\r if needed.
	 */
	if (d->outtop == 0
	&&  !d->fcommand
	&&  (!d->character || !IS_SET(d->character->comm, COMM_TELNET_GA))) {
		d->outbuf[0]	= '\n';
		d->outbuf[1]	= '\r';
		d->outtop	= 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
	while (d->outtop + size >= d->outsize) {
		char *outbuf;

		if (d->outsize >= 524288) {
			LOG("**** BUG: Buffer overflow. Closing.\n");
			close_descriptor(d);
			return;
 		}
		outbuf = malloc(2 * d->outsize);
		strncpy(outbuf, d->outbuf, d->outtop);
		free(d->outbuf);
		d->outbuf = outbuf;
		d->outsize *= 2;
	}

	/*
	 * Copy.
	 */
	while (length--) {
		unsigned char c;

		c = d->codepage->to[(unsigned char) *txt++];
		d->outbuf[d->outtop] = c;
		if (c == IAC) {
			if (noiac)
				d->outbuf[d->outtop] = IAC_REPL;
			else 
				d->outbuf[++d->outtop] = IAC;
		}
		d->outtop++;
	}
	return;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor(int desc, char *txt, uint length)
{
	uint iStart;
	uint nWrite;
	uint nBlock;

	if (!length)
		length = strlen(txt);

	for (iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);
#if !defined( WIN32 )
		if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
#else
		if ((nWrite = send(desc, txt + iStart, nBlock, 0)) < 0) {
#endif
			log_printf("write_to_descriptor: %s", strerror(errno));
			return FALSE;
		}
	} 
	return TRUE;
}

int search_sockets(DESCRIPTOR_DATA *inp)
{
	DESCRIPTOR_DATA *d;

	if (IS_IMMORTAL(inp->character))
		return 0;

	for(d = descriptor_list; d; d = d->next) {
		if(!strcmp(inp->host, d->host)) {
			if (d->character && inp->character
			&&  !strcmp(inp->character->name, d->character->name)) 
				continue;
			return 1;
		}
	}
	return 0;
}
  
flag32_t align_restrict(CHAR_DATA *ch);
void get_alignment(CHAR_DATA *ch, const char *argument);
flag32_t ethos_restrict(CHAR_DATA *ch);
void get_ethos(CHAR_DATA *ch, const char *argument);

void advance(CHAR_DATA *victim, int level);

static void print_hometown(CHAR_DATA *ch)
{
	race_t *r;
	class_t *cl;
	int htn;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata
	||  (cl = class_lookup(ch->class)) == NULL) {
		char_puts("You should create your character anew.\n", ch);
		close_descriptor(ch->desc);
		return;
	}

	if ((htn = hometown_permanent(ch)) >= 0) {
		ch->hometown = htn;
		char_printf(ch, "\nYour hometown is %s, permanently.\n"
				"[Hit Return to continue]\n",
			    hometown_name(htn));

/* XXX */
		ch->desc->connected = CON_CREATE_DONE;
		return;
	}

	char_puts("\n", ch);
	do_help(ch, "HOMETOWN");
	hometown_print_avail(ch);
	char_puts("? ", ch);
	ch->desc->connected = CON_PICK_HOMETOWN;
}

/* *INDENT-ON* */
/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(DESCRIPTOR_DATA * d, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d_old, *d_next;
	char buf1[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	char *pwdnew;
	int iClass, race, i, j;
	int nextquest = 0;
	struct sockaddr_in sock;
	socklen_t size;
	race_t *r;
	int count = 0;
	const char *statfmt = 
			"Str:{c%s{x Int:{c%s{x Wis:{c%s{x"
			" Dex:{c%s{x  Con:{c%s{x  Cha:{c%s{x\n"
			"Accept (Y/N/?)? ";
	int k;

	while (isspace(*argument))
		argument++;

	ch = d->character;

	switch (d->connected) {
	default:
		bug("Nanny: bad d->connected %d.", d->connected);
		close_descriptor(d);
		return;

/*case CON_GET_CODEPAGE: {
		int num;

		if (argument[0] == '\0') {
			close_descriptor(d);
			return;
		}

		if (argument[1] != '\0'
		||  (num = argument[0] - '1') < 0
		||  num >= NCODEPAGES) {
			cp_print(d);
			break;
		}

		d->codepage = codepages+num;
		log_printf("'%s' codepage selected", d->codepage->name);
		d->connected = CON_GET_NAME;
		write_to_buffer(d, "By which name do you wish to be known? ", 0);
		break;
	}
*/
	case CON_GET_NAME:
		if (argument[0] == '\0') {
			close_descriptor(d);
			return;
		}

		if (!pc_name_ok(argument)) {
			write_to_buffer(d, "Illegal name, try another.\n\r"
					"Name: ", 0);
			return;
		}

		if (load_char_obj(d, argument))
			ch = d->character;
		else {
			ch = d->character = new_char_obj(argument);
			ch->desc = d;
			ch->acct_flags |= ACCT_NEW;
		}

		if (d->host == NULL) {
			size = sizeof(sock);
			if (getpeername(d->descriptor,
					(struct sockaddr *) &sock, &size) < 0)
				d->host = str_dup("(unknown)");
			else {
#if defined (WIN32)
				printf("%s@%s\n", ch->name,
				       inet_ntoa(sock.sin_addr));
#else
				fprintf(rfout, "%s@%s\n",
					ch->name, inet_ntoa(sock.sin_addr));
#endif
				d->connected = CON_RESOLV;
/* wait until sock.sin_addr gets resolved */
				break;
			}
		}

		/* FALLTHRU */

	case CON_RESOLV:
		if (d->host == NULL)
			break;

		/*
		 * Swiftest: I added the following to ban sites.  I don't
		 * endorse banning of sites, but Copper has few descriptors now
		 * and some people from certain sites keep abusing access by
		 * using automated 'autodialers' and leaving connections hanging.
		 *
		 * Furey: added suffix check by request of Nickel of HiddenWorlds.
		 */
		if (check_ban(d->host, BAN_ALL)) {
			write_to_buffer(d,
					"Your site has been banned from this mud.\n\r",
					0);
			close_descriptor(d);
			return;
		}

		if (!IS_IMMORTAL(ch)) {
			if (check_ban(d->host, BAN_PLAYER)) {
				write_to_buffer(d,
						"Your site has been banned for players.\n\r",
						0);
				close_descriptor(d);
				return;
			}
#undef NO_PLAYING_TWICE
#ifdef NO_PLAYING_TWICE
			if (search_sockets(d)) {
				write_to_buffer(d,
						"Playing twice is restricted...\n\r",
						0);
				close_descriptor(d);
				return;
			}
#endif
			if (iNumPlayers > MAX_OLDIES
			    && !IS_SET(ch->acct_flags, ACCT_NEW)) {
				snprintf(buf, sizeof(buf),
					 "\nThere are currently %i players mudding out of a maximum of %i.\n"
					 "Please try again soon.\n",
					 iNumPlayers - 1, MAX_OLDIES);
				write_to_buffer(d, buf, 0);
				close_descriptor(d);
				return;
			}
			if (iNumPlayers > MAX_NEWBIES
			    && IS_SET(ch->acct_flags, ACCT_NEW)) {
				snprintf(buf, sizeof(buf),
					 "\nThere are currently %i players mudding.\n"
					 "New player creation is limited to when there are less than %i players.\n"
					 "Please try again soon.\n",
					 iNumPlayers - 1, MAX_NEWBIES);
				write_to_buffer(d, buf, 0);
				close_descriptor(d);
				return;
			}
		}

		if (IS_SET(ch->state_flags, STATE_DENY)) {
			log_printf("Denying access to %s@%s.", argument,
				   d->host);
			write_to_buffer(d, "You are denied access.\n\r", 0);
			close_descriptor(d);
			return;
		}

		if (check_reconnect(d, argument, FALSE))
			REMOVE_BIT(ch->acct_flags, ACCT_NEW);
		else if (wizlock && !IS_HERO(ch)) {
			write_to_buffer(d, "The game is wizlocked.\n\r", 0);
			close_descriptor(d);
			return;
		}

		if (!IS_SET(ch->acct_flags, ACCT_NEW)) {
			/* Old player */
			write_to_descriptor(d->descriptor, echo_off_str, 0);
			write_to_buffer(d, "Password: ", 0);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		} else {
			/* New player */
			if (newlock) {
				write_to_buffer(d, "The game is newlocked.\n\r",
						0);
				close_descriptor(d);
				return;
			}

			if (check_ban(d->host, BAN_NEWBIES)) {
				write_to_buffer(d,
						"New players are not allowed from your site.\n\r",
						0);
				close_descriptor(d);
				return;
			}

			do_help(ch, "NAME");
			write_to_buffer(d,
					"Does your name fit the above criteria (Y/N)? ",
					0);
			d->connected = CON_CONFIRM_NEW_NAME;
			return;
		}
		break;

/* RT code for breaking link */
	case CON_BREAK_CONNECT:
		switch (*argument) {
		case 'y':
		case 'Y':
			for (d_old = descriptor_list; d_old; d_old = d_next) {
				CHAR_DATA *rch;

				d_next = d_old->next;
				if (d_old == d || d_old->character == NULL)
					continue;

				rch = d_old->original ? d_old->original :
				    d_old->character;
				if (str_cmp(ch->name, rch->name))
					continue;

				if (d_old->original)
					do_return(d_old->character, str_empty);
				close_descriptor(d_old);
			}

			if (check_reconnect(d, ch->name, TRUE))
				return;
			write_to_buffer(d, "Reconnect attempt failed.\n\r", 0);

			/* FALLTHRU */

		case 'n':
		case 'N':
			write_to_buffer(d, "Name: ", 0);
			if (d->character != NULL) {
				free_char(d->character);
				d->character = NULL;
			}
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Y or N? ", 0);
			break;
		}
		break;

	case CON_CONFIRM_NEW_NAME:
		switch (*argument) {
		case 'y':
		case 'Y':
			snprintf(buf, sizeof(buf),
				 "New character.\n\r"
				 "Give me a password for %s: ", ch->name);
			write_to_buffer(d, buf, 0);
			write_to_descriptor(d->descriptor, echo_off_str, 0);
			d->connected = CON_GET_NEW_PASSWORD;
			break;

		case 'n':
		case 'N':
			write_to_buffer(d, "Ok, what IS it, then? ", 0);
			free_char(d->character);
			d->character = NULL;
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Yes or No? ", 0);
			break;
		}
		break;

	case CON_GET_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n\r", 2);
#endif

		if (strlen(argument) < 5) {
			write_to_buffer(d,
					"Password must be at least five characters long.\n\rPassword: ",
					0);
			return;
		}

		pwdnew = crypt(argument, ch->name);
		free_string(ch->pcdata->pwd);
		ch->pcdata->pwd = str_dup(pwdnew);
		write_to_buffer(d, "Please retype password: ", 0);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer(d, "\n\r", 2);
#endif

		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
			write_to_buffer(d, "Passwords don't match.\n\r"
					"Retype password: ", 0);
			d->connected = CON_GET_NEW_PASSWORD;
			return;
		}

		write_to_descriptor(d->descriptor, (char *) echo_on_str, 0);
		d->connected = CON_ANSI_DETECTOR;
		write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r", 0);
		break;

	case CON_ANSI_DETECTOR:
		switch (argument[0]) {
		default:
			write_to_buffer(d,
					"Does your client understand ANSI color(Y/N)? ",
					0);
			break;
		case 'y':
		case 'Y':
			SET_BIT(ch->comm, COMM_COLOR);
			if (CHAR_CREATE_ALLOW_NEWBIES)
				d->connected = CON_NEWBIE_DETECTOR;
			else if (CHAR_CREATE_ALLOW_TRUE_LIFERS)
				d->connected = CON_TRUE_LIFER;
			else
				d->connected = CON_GET_NEW_RACE;
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			break;
		case 'n':
		case 'N':
			if (CHAR_CREATE_ALLOW_NEWBIES)
				d->connected = CON_NEWBIE_DETECTOR;
			else if (CHAR_CREATE_ALLOW_TRUE_LIFERS)
				d->connected = CON_TRUE_LIFER;
			else
				d->connected = CON_GET_NEW_RACE;
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			break;
		}
		break;

	case CON_NEWBIE_DETECTOR:
		switch (argument[0]) {
		case 'Y':
		case 'y':
			char_puts
			    ("\n{D-{w={W[{x Welcome to {CLegends {x& {CLore{x! {W]{w={D-{x\n"
			     "{Y*{x remember to ask questions on the '{Mnewbie{x' channel.\n",
			     ch);
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			d->connected = CON_NEWBIE_PROTECTION;
			SET_BIT(ch->acct_flags, ACCT_NEW);
			REMOVE_BIT(ch->channels, CHAN_OOC);

			return;
			break;
		case 'N':
		case 'n':
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			REMOVE_BIT(ch->acct_flags, ACCT_NEW);
			d->connected = CON_TRUE_LIFER;
			return;
			break;

		default:
			write_to_buffer(d,
					"-=[ Is this your first time visiting our Legends & Lore? ]=-\r\n",
					0);
		}
		write_to_buffer(d,
				"\n\rAre you completely new to the realm? (Y/N): ",
				0);
		break;

	case CON_NEWBIE_PROTECTION:
		switch (argument[0]) {
		case 'H':
		case 'h':
		case '?':
			do_help(ch, "1.newbie protection");
			break;
		case 'Y':
		case 'y':
			write_to_buffer(d, "\n\rWise Choice!"
					"\n\rYou are now invulnerable to PlayerKilling.",
					0);
			SET_BIT(ch->acct_flags, ACCT_NEWBIE);
			REMOVE_BIT(ch->channels, CHAN_OOC);
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			d->connected = CON_GET_NEW_RACE;
			return;
			break;
		case 'N':
		case 'n':
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			d->connected = CON_GET_NEW_RACE;
			return;
			break;

		default:
			write_to_buffer(d, "-=[ Explore Mode? ]=-\r\n", 0);
			do_help(ch, "newbie protection intro");
		}
		write_to_buffer(d,
				"\n\rWould you like to be protected (Y/N/?): ",
				0);
		break;

	case CON_TRUE_LIFER:
		switch (argument[0]) {
		case 'H':
		case 'h':
		case '?':
			do_help(ch, "true lifer");
			break;
		case 'Y':
		case 'y':
			write_to_buffer(d,
					"\n\rGood luck . . . you'll need it!",
					0);
			SET_BIT(ch->acct_flags, ACCT_TRUE_LIFER);
			DEBUG(DEBUG_CHAR_CREATE,
				"new character: %s has chosen the path of a True Lifer",
				ch->name);
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			d->connected = CON_GET_NEW_RACE;
			return;
			break;
		case 'N':
		case 'n':
			if (CHAR_CREATE_ONLY_TRUE_LIFERS) {
				write_to_buffer(d,
					"\n\rSorry, mate, only True Lifers are allowed.", 0);
				write_to_buffer(d,
						"\n\rGood luck . . . you'll need it!",
						0);
				SET_BIT(ch->acct_flags, ACCT_TRUE_LIFER);
				DEBUG(DEBUG_CHAR_CREATE,
					"new character: %s didn't choose, but is a True Lifer",
					ch->name);
			}
			write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",
					0);
			d->connected = CON_GET_NEW_RACE;
			return;
			break;

		default:
			write_to_buffer(d, "-=[ True Lifer? ]=-\r\n", 0);
			do_help(ch, "true life intro");
		}
		write_to_buffer(d,
				"\n\rOne life, one death, that's the deal.  You in (Y/N/?): ",
				0);
		break;

	case CON_GET_NEW_RACE:
		one_argument(argument, arg, sizeof(arg));

		if (*arg == '\0') {
			write_to_buffer(d,
					"Legends & Lore is home for the following races:\n\r",
					0);
			do_help(ch, "RACETABLE");
			write_to_buffer(d,
					"('help <race>' for more information)\r\n"
					"What is your race? ", 0);
			break;
		} else if (!str_cmp(arg, "help")) {
			argument = one_argument(argument, arg, sizeof(arg));
			if (argument[0] == '\0') {
				write_to_buffer(d,
						"Legends & Lore is the home for the following races:\n\r",
						0);
				do_help(ch, "RACETABLE");
			} else {
				do_help(ch, argument);
			}
			write_to_buffer(d,
					"('help <race>' for more information)\r\n"
					"What is your race? ", 0);
			break;
		}

		race = rn_lookup(argument);
		r = RACE(race);

		if (race < 1 || !r->pcdata) {
			write_to_buffer(d, "That is not a valid race.\n\r", 0);
			write_to_buffer(d,
					"The following races are available:\n\r  ",
					0);
			for (race = 1; race < races.nused; race++) {
				r = RACE(race);
				if (!r->pcdata)
					break;
				if (race == 8 || race == 14)
					write_to_buffer(d, "\n\r  ", 0);
				write_to_buffer(d, "(", 0);
				write_to_buffer(d, r->name, 0);
				write_to_buffer(d, ") ", 0);
			}
			write_to_buffer(d, "\n\r", 0);
			write_to_buffer(d,
					"('help <race>' for more information)\r\n"
					"What is your race? ", 0);
			break;
		}

		SET_ORG_RACE(ch, race);
		ch->race = race;
		for (i = 0; i < MAX_STATS; i++)
			ch->mod_stat[i] = 0;

		/* Add race stat modifiers 
		   for (i = 0; i < MAX_STATS; i++)
		   ch->mod_stat[i] += r->pcdata->stats[i];      */

		/* Add race modifiers */
		ch->max_hit += r->pcdata->hp_bonus;
		ch->hit = ch->max_hit;
		ch->max_mana += r->pcdata->mana_bonus;
		ch->mana = ch->max_mana;
		ch->practice = r->pcdata->prac_bonus;

		ch->affected_by = ch->affected_by | r->aff;
		ch->imm_flags = ch->imm_flags | r->imm;
		ch->res_flags = ch->res_flags | r->res;
		ch->vuln_flags = ch->vuln_flags | r->vuln;
		ch->form = r->form;
		ch->parts = r->parts;

		/* add cost */
		ch->pcdata->points = r->pcdata->points;
		ch->size = r->pcdata->size;

		write_to_buffer(d, "What is your sex (M/F)? ", 0);
		d->connected = CON_GET_NEW_SEX;
		break;

	case CON_GET_NEW_SEX:
		switch (argument[0]) {
		case 'm':
		case 'M':
			ch->sex = SEX_MALE;
			ch->pcdata->true_sex = SEX_MALE;
			break;
		case 'f':
		case 'F':
			ch->sex = SEX_FEMALE;
			ch->pcdata->true_sex = SEX_FEMALE;
			break;
		default:
			write_to_buffer(d,
					"That's not a sex.\n\rWhat IS your sex? ",
					0);
			return;
		}

		do_help(ch, "class help");

		strnzcpy(buf, sizeof(buf), "Select a class:\n\r[ ");
		snprintf(buf1, sizeof(buf), "  (Continuing:) ");

		count = 0;
		for (iClass = 0; iClass < classes.nused; iClass++) {
			if (class_ok(ch, iClass)) {
				if (count++ < 7) {
					strnzcat(buf, sizeof(buf),
						 CLASS(iClass)->name);
					strnzcat(buf, sizeof(buf), " ");
				} else {
					count += 100;
					strnzcat(buf1, sizeof(buf),
						 CLASS(iClass)->name);
					strnzcat(buf1, sizeof(buf), " ");
				}
			}
		}
		if (count > 100) {
			strnzcat(buf, sizeof(buf), "\n\r");
			strnzcat(buf1, sizeof(buf1), "]:\n\r");
		} else
			strnzcat(buf, sizeof(buf), "]\n\r");

		write_to_buffer(d, buf, 0);
		if (count > 100)
			write_to_buffer(d, buf1, 0);

		write_to_buffer(d,
				"('help <class>' for more information)\r\nWhat is your class? ",
				0);
		d->connected = CON_GET_NEW_CLASS;
		break;

	case CON_GET_NEW_CLASS:
		iClass = cn_lookup(argument);
		argument = one_argument(argument, arg, sizeof(arg));

		if (!str_cmp(arg, "help")) {
			if (argument[0] == '\0')
				do_help(ch, "class help");
			else
				do_help(ch, argument);
			write_to_buffer(d,
					"('help <class>' for more information)\r\nWhat is your class? ",
					0);
			return;
		}

		if (iClass == -1) {
			write_to_buffer(d,
					"That's not a class.\n\rWhat IS your class? ",
					0);
			return;
		}

		if (iClass == CLASS_THIEF) {
			write_to_buffer(d,
					"That class is currently broken and not much fun to play.\n\rChoose again: ",
					0);
			return;
		}
		if (iClass == CLASS_ROGUE) {
			write_to_buffer(d,
					"That class is curently under development.\n\rChoose again: ",
					0);
			return;
		}

		if (!class_ok(ch, iClass)) {
			if (IS_NEWBIE(ch)) {
				write_to_buffer(d,
						"That class is not available for your race or sex"
						" or because it is advanced.\n\rChoose again: ",
						0);
			} else {
				write_to_buffer(d,
						"That class is not available for your race or sex.\n\r"
						"Choose again: ", 0);
			}
			return;
		}

		ch->class = iClass;
		ch->pcdata->points += CLASS(iClass)->points;
		act("You are now $t.", ch, CLASS(iClass)->name, NULL, TO_CHAR);

		for (i = 0; i < MAX_STATS; i++) {
			ch->perm_stat[i] =
			    race_lookup(ch->pcdata->race)->pcdata->stats[i];
			if (CHAR_CREATE_CLASS_STAT_BONUS)
				ch->perm_stat[i] += CLASS(ch->class)->stats[i];
		}

		do_help(ch, "stats");
		char_puts("\nNow rolling your stats.\n", ch);
		char_puts("\nEveryone starts with the same stat total, you're choosing the allocation.\n", ch);

		if (STAT_TRAIN_RATE > 0)
			char_puts("You don't get many trains, so choose well.\n",ch);
		else
			char_puts("{Y*{x After you are born, stats can only be augmented by {mmagic{x.\n",ch);

		char_printf(ch, statfmt,
			get_stat_alias(ch, STAT_STR),
			get_stat_alias(ch, STAT_INT),
			get_stat_alias(ch, STAT_WIS),
			get_stat_alias(ch, STAT_DEX),
			get_stat_alias(ch, STAT_CON),
			get_stat_alias(ch, STAT_CHA));

		DEBUG(DEBUG_CHAR_ROLLS,
		    "ROLL: %s (%s/%s): Str:%d  Int:%d  Wis:%d  Dex:%d  Con:%d  Cha:%d Lck:%d",
		     ch->name, 
		     race_name(ch->race),
		     class_name(ch),
		     ch->perm_stat[STAT_STR],
		     ch->perm_stat[STAT_INT], 
		     ch->perm_stat[STAT_WIS],
		     ch->perm_stat[STAT_DEX], 
		     ch->perm_stat[STAT_CON],
		     ch->perm_stat[STAT_CHA],
		     ch->perm_stat[STAT_LCK]);
		     
	     	d->connected = CON_ACCEPT_STATS;

		break;

	case CON_ACCEPT_STATS:

		switch (argument[0]) {
		case 'H':
		case 'h':
		case '?':
			one_argument(argument, arg2, sizeof(arg2));
			if (arg2[0] != '\0') {
				do_help(ch, arg2);
			}
			else
				do_help(ch, "stats");
			break;

		case 'y':
		case 'Y':
			for (i = 0; i < MAX_STATS; i++)
				ch->mod_stat[i] = 0;
			DEBUG(DEBUG_CHAR_CREATE,
			    "newchar %s (%s/%s): Str:%d  Int:%d  Wis:%d  Dex:%d  Con:%d  Cha:%d Lck:%d",
			     ch->name, 
			     race_name(ch->race),
			     class_name(ch),
			     ch->perm_stat[STAT_STR],
			     ch->perm_stat[STAT_INT], 
			     ch->perm_stat[STAT_WIS],
			     ch->perm_stat[STAT_DEX], 
			     ch->perm_stat[STAT_CON],
			     ch->perm_stat[STAT_CHA],
			     ch->perm_stat[STAT_LCK]
			     );
			char_puts("\n", ch);
			char_puts("[Press Enter to continue]", ch);
			d->connected = CON_GET_ALIGNMENT;
			break;

		case 'n':
		case 'N':
			for (i = 0; i < MAX_STATS; i++) {
				ch->perm_stat[i] =
				    race_lookup(ch->pcdata->race)->pcdata->
				    stats[i];
				if (CHAR_CREATE_CLASS_STAT_BONUS)
					ch->perm_stat[i] += CLASS(ch->class)->stats[i];
			}

			for (i = 0; i < 100; i++) {
				j = number_range(0, MAX_STATS - 1);
				k = number_range(0, MAX_STATS - 1);
				if ((ch->perm_stat[j] < (RACE(ch->race)->pcdata->stats[j] +STAT_RANDOM_VARIANCE))
				&& (ch->perm_stat[k] > (RACE(ch->race)->pcdata->stats[k] -STAT_RANDOM_VARIANCE))) {
					ch->perm_stat[j]++;
					ch->perm_stat[k]--;
				}
			}
			char_printf(ch, statfmt,
				 get_stat_alias(ch, STAT_STR),
				 get_stat_alias(ch, STAT_INT),
				 get_stat_alias(ch, STAT_WIS),
				 get_stat_alias(ch, STAT_DEX),
				 get_stat_alias(ch, STAT_CON),
				 get_stat_alias(ch, STAT_CHA));

			break;

		default:
			write_to_buffer(d, "Please answer (Y/N/?): ", 0);
			break;
		}

		DEBUG(DEBUG_CHAR_ROLLS,
		    "ROLL: %s (%s/%s): Str:%d  Int:%d  Wis:%d  Dex:%d  Con:%d  Cha:%d Lck:%d",
		     ch->name, 
		     race_name(ch->race),
		     class_name(ch),
		     ch->perm_stat[STAT_STR],
		     ch->perm_stat[STAT_INT], 
		     ch->perm_stat[STAT_WIS],
		     ch->perm_stat[STAT_DEX], 
		     ch->perm_stat[STAT_CON],
		     ch->perm_stat[STAT_CHA],
		     ch->perm_stat[STAT_LCK]);
		break;


	case CON_GET_ALIGNMENT:
		get_alignment(ch, argument);
		if (d->connected == CON_GET_ETHOS)
			get_ethos(ch, NULL);
		break;

	case CON_GET_ETHOS:
		get_ethos(ch, argument);
		break;

	case CON_PICK_HOMETOWN:{
			int htn;

			if (IS_NEWBIE(ch)) {
				if (ch->class != CLASS_NECROMANCER
				    && ch->class != CLASS_VAMPIRE) {
					ch->hometown = htn_lookup("Midgaard");
					write_to_buffer(d,
							"Since you are new here, your hometown"
							" has been automagically set to Midgaard.\n\r",
							0);
				} else {
					ch->hometown =
					    htn_lookup("Old Midgaard");
					write_to_buffer(d,
							"Your hometown has been automagically set to "
							" Old Midgaard.\n\r",
							0);
				}
				write_to_buffer(d,
						"[Hit Return to Continue]\n\r",
						0);
				d->connected = CON_CREATE_DONE;
				break;
			} else if (argument[0] == '\0'
				   || (htn = htn_lookup(argument)) < 0
				   || hometown_restrict(HOMETOWN(htn), ch)) {
				char_puts("That's not a valid hometown.\n", ch);
				print_hometown(ch);
				return;
			}

			ch->hometown = htn;
			char_printf(ch, "\nNow your hometown is %s.\n"
				    "[Hit Return to continue]\n",
				    hometown_name(htn));
			d->connected = CON_CREATE_DONE;
			break;
		}

	case CON_CREATE_DONE:
		if (IS_NEWBIE(ch)) {
			log_printf("%s@%s newbie player.", ch->name, d->host);
		} else
			log_printf("%s@%s new player.", ch->name, d->host);
		write_to_buffer(d, "\n\r", 2);
		do_help(ch, "motd");
		char_puts("[Press Enter to continue]", ch);
		ch->lines = PAGELEN;
		d->connected = CON_READ_MOTD;
		break;

	case CON_GET_OLD_PASSWORD:
		write_to_buffer(d, "\n\r", 2);

		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
			write_to_buffer(d, "Wrong password.\n\r", 0);
			log_printf("Wrong password by %s@%s",
				   ch->name, d->host);
			if (ch->endur == 2)
				close_descriptor(d);
			else {
				write_to_descriptor(d->descriptor,
						    (char *) echo_off_str, 0);
				write_to_buffer(d, "Password: ", 0);
				d->connected = CON_GET_OLD_PASSWORD;
				ch->endur++;
			}
			return;
		}

		if (ch->pcdata->pwd[0] == '\0') {
			write_to_buffer(d, "Warning! Null password!\n\r"
					"Type 'password null <new password>'"
					" to fix.\n\r", 0);
		}

		write_to_descriptor(d->descriptor, (char *) echo_on_str, 0);

		if (check_playing(d, ch->name)
		    || check_reconnect(d, ch->name, TRUE))
			return;

		log_printf("%s@%s has connected.", ch->name, d->host);
		d->connected = CON_READ_IMOTD;

		/* FALL THRU */

	case CON_READ_IMOTD:
		write_to_buffer(d, "\n\r", 2);

		if (IS_SET(ch->acct_flags, ACCT_NEWBIE))
			do_help(ch, "1.newbie");
		else
			do_help(ch, "motd");

		write_to_buffer(d, "\n\r", 2);

		if (IS_IMMORTAL(ch))
			do_help(ch, "imotd");

		d->connected = CON_READ_MOTD;

		/* FALL THRU */

	case CON_READ_MOTD:
		update_skills(ch);
		write_to_buffer(d,
				"\n\rWelcome to Legend & Lore. Enjoy!!...\n\r",
				0);
		ch->next = char_list;
		char_list = ch;
		if (!char_list_lastpc)
			char_list_lastpc = ch;
		d->connected = CON_PLAYING;
		{
			int count;
			FILE *max_on_file;
			int tmp = 0;
			count = 0;
			for (d = descriptor_list; d != NULL; d = d->next)
				if (d->connected == CON_PLAYING)
					count++;
			max_on = UMAX(count, max_on);
			if ((max_on_file = dfopen(TMP_PATH, MAXON_FILE, "r"))) {
				fscanf(max_on_file, "%d", &tmp);
				fclose(max_on_file);
			}
			if (tmp < max_on
			    && (max_on_file =
				dfopen(TMP_PATH, MAXON_FILE, "w"))) {
				fprintf(max_on_file, "%d", max_on);
				log("Global max_on changed.");
				fclose(max_on_file);
			}
		}

		if (ch->level == 0) {
			ch->max_mana += (ch->perm_stat[STAT_INT] * 2/3
				+ ch->perm_stat[STAT_WIS]/2);
			ch->max_move += (ch->perm_stat[STAT_CON]/5
				+ ch->perm_stat[STAT_DEX]/5) * 5;
		}

		reset_char(ch);

		/* quest code */
		nextquest = -abs(ch->pcdata->questtime);
		quest_cancel(ch);
		ch->pcdata->questtime = nextquest;
		/* !quest code */

		snprintf(buf, sizeof(buf),
			 " login: {W$N{x - %s",
			 (ch->desc) 
			 ? ((ch->pcdata->fake_ip)
			 	? ch->pcdata->fake_ip : ch->desc->host)
			 : "<no descriptor>");
		wiznet(buf, ch, NULL, WIZ_LOGINS, 0, ch->level);

		for (i = 0; i < MAX_STATS; i++) {
			int max_stat = get_max_train(ch, i);

			if (ch->perm_stat[i] > max_stat) {
				ch->train += ch->perm_stat[i] - max_stat;
				ch->perm_stat[i] = max_stat;
			}
		}

		if (!IS_IMMORTAL(ch)
		    && ch->gold > 1000 && ch->gold - 1000 / 2 > 0) {
			char_printf(ch, "You are taxed %d gold to pay for"
				    " the Mayor's bar tab.\n\r",
				    (ch->gold - 1000) / 2);
			ch->gold -= (ch->gold - 1000) / 2;
		}

		if (!IS_IMMORTAL(ch)) {
			for (i = 2; exp_for_level(ch, i) < ch->exp; i++);

			if (i < ch->level) {
				int con;
				int wis;
				int inte;
				int dex;

				con = ch->perm_stat[STAT_CON];
				wis = ch->perm_stat[STAT_WIS];
				inte = ch->perm_stat[STAT_INT];
				dex = ch->perm_stat[STAT_DEX];
				ch->perm_stat[STAT_CON] =
				    get_max_train(ch, STAT_CON);
				ch->perm_stat[STAT_WIS] =
				    get_max_train(ch, STAT_WIS);
				ch->perm_stat[STAT_INT] =
				    get_max_train(ch, STAT_INT);
				ch->perm_stat[STAT_DEX] =
				    get_max_train(ch, STAT_DEX);
				do_remove(ch, "all");
				advance(ch, i - 1);
				ch->perm_stat[STAT_CON] = con;
				ch->perm_stat[STAT_WIS] = wis;
				ch->perm_stat[STAT_INT] = inte;
				ch->perm_stat[STAT_DEX] = dex;
			}
		}

		if (ch->level == 0) {
			OBJ_DATA *wield;
			OBJ_INDEX_DATA *map;

			ch->level = 1;
			ch->exp = base_exp(ch);
			ch->hit = ch->max_hit;
			ch->mana = ch->max_mana;
			ch->move = ch->max_move;
			/*ch->train = 3; */
			ch->practice += 5;
			ch->pcdata->death = 0;

			set_title(ch, title_lookup(ch));

			do_outfit(ch, str_empty);

			obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP), 0),
				    ch);
			obj_to_char(create_obj
				    (get_obj_index(OBJ_VNUM_NMAP1), 0), ch);
			obj_to_char(create_obj
				    (get_obj_index(OBJ_VNUM_NMAP2), 0), ch);
			obj_to_char(create_obj
				    (get_obj_index(OBJ_VNUM_MUSHROOM), 0), ch);
			obj_to_char(create_obj
				    (get_obj_index(OBJ_VNUM_MUSHROOM), 0), ch);
			obj_to_char(create_obj
				    (get_obj_index(OBJ_VNUM_MUSHROOM), 0), ch);
			obj_to_char(create_obj
				    (get_obj_index(OBJ_VNUM_SCHOOL_FLASK), 0),
				    ch);
			obj_to_char(create_obj
				    (get_obj_index(OBJ_VNUM_NEWBIE_GUIDE), 0),
				    ch);

			if ((map = get_map(ch)) != NULL)
				obj_to_char(create_obj(map, 0), ch);

			if ((wield = get_eq_char(ch, WEAR_WIELD)))
				set_skill_raw(ch, get_weapon_sn(wield),
					      40, FALSE);

			char_puts("\n", ch);
			do_help(ch, "NEWBIE INFO");
			char_puts("\n", ch);
			char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));

		} else {
			CHAR_DATA *pet;
			ROOM_INDEX_DATA *to_room;

			if (ch->in_room
			    && (room_is_private(ch->in_room) ||
				(ch->in_room->area->clan &&
				 ch->in_room->area->clan != ch->clan)))
				ch->in_room = NULL;

			if (ch->in_room)
				to_room = ch->in_room;
			else if (IS_IMMORTAL(ch))
				to_room = get_room_index(ROOM_VNUM_CHAT);
			else
				to_room = get_room_index(ROOM_VNUM_TEMPLE);

			pet = ch->pet;
			act("$N emerges from the astral plane.",
			    to_room->people, NULL, ch, TO_ALL);
			char_to_room(ch, to_room);

			if (pet) {
				act("$N emerges following $S master.",
				    to_room->people, NULL, pet, TO_ROOM);
				char_to_room(pet, to_room);

				/*expire if no clan item to support them*/
				if (!clan_item_ok(pet->pIndexData->clan)) {
					act("$n withers away as the source of"
					" $s existance wanes.",
					pet, NULL, NULL, TO_ROOM);
				}
			}
		}

		sync_ch_pdata(ch);
		ch_update_usage(ch->pcdata->pdata, FALSE);

		if (!JUST_KILLED(ch)) {
			do_look(ch, "auto");
			do_unread(ch, "login");
		}

		if (IS_IMMORTAL(ch)) {
			char_puts("\n", ch);
			doprintf(do_anoncolor, ch, "set");
		}

		if (clan_remove_outcast(ch))
			save_char_obj(ch, FALSE);

		/* very inefficent, but it works */
		if (ch->clan != CLAN_NONE && !clan_item_ok(ch->clan))
			clan_items_remove(ch, clan_lookup(ch->clan));

		/* welcome newbies */
		if (ch->level < 2
		    && (IS_SET(ch->acct_flags, ACCT_NEW) || IS_NEWBIE(ch))) {
			DESCRIPTOR_DATA *dt;
			for (dt = descriptor_list; dt; dt = dt->next)
				if (dt->connected == CON_PLAYING) {
					char_printf(dt->character,
						    "{W[{mN{ce{mw{cb{mi{ce{W]{x:"
						    " {MEveryone give warm welcome to"
						    " our newest player, {Y%s{M!{x\n",
						    ch->name);
				}
			char_puts
			    ("{gNote{x: you must be level 2 to save your character.\n",
			     ch);
		}

		fix_1_v2_hp_dr(ch);
		save_char_obj(ch, FALSE);

		break;
	}
}
/* *INDENT-OFF* */

/*
 * look for link-dead player to reconnect.
 *
 * when fConn == FALSE then
 * simple copy password for newly [re]connected character
 * authentication
 *
 * otherwise reconnect attempt is made
 */
bool check_reconnect(DESCRIPTOR_DATA *d, const char *name, bool fConn)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d2;

	if (!fConn) {
		for (d2 = descriptor_list; d2; d2 = d2->next) {
			if (d2 == d)
				continue;
			ch = d2->original ? d2->original : d2->character;
			if (ch && !str_cmp(d->character->name, ch->name)) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_qdup(ch->pcdata->pwd);
				return TRUE;
			}
		}
	}

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch->next) {
		if ((!fConn || ch->desc == NULL)
		&&  !str_cmp(d->character->name, ch->name)) {
			if (!fConn) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_qdup(ch->pcdata->pwd);
			}
			else {
				free_char(d->character);
				d->character	= ch;
				ch->desc	= d;
				ch->timer	= 0;
				char_puts("Reconnecting. Type replay to see missed tells.\n", ch);
				act("$n emerges from the astral plane.",
				    ch, NULL, NULL, TO_ROOM);

				log_printf("%s@%s reconnected.",
					   ch->name, d->host);
				wiznet("$N grasps the fullness of $S link.",
				       ch, NULL, WIZ_LINKS, 0, 
				       (ch->invis_level > ch->incog_level) 
				       ? ch->invis_level
				       : ch->incog_level);
				d->connected = CON_PLAYING;
				sync_ch_pdata(ch);

				if (!IS_IMMORTAL(ch)
				&& ch->in_room->area->clan != CLAN_FREEMAN
				&& ch->clan != ch->in_room->area->clan)
					ch->in_room->area->interlopers++;

			}
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Check if already playing.
 */
bool check_playing(DESCRIPTOR_DATA *d, const char *name)
{
	DESCRIPTOR_DATA *dold;

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		&&  dold->character != NULL
/*		&&  dold->connected != CON_GET_CODEPAGE */
		&&  dold->connected != CON_GET_NAME
		&&  dold->connected != CON_RESOLV
		&&  dold->connected != CON_GET_OLD_PASSWORD
		&&  !str_cmp(name, dold->original ?  dold->original->name :
						     dold->character->name)) {
			write_to_buffer(d, "That character is already playing.\n\r",0);
			write_to_buffer(d, "Do you wish to connect anyway (Y/N)?",0);
			d->connected = CON_BREAK_CONNECT;
			return TRUE;
		}
	}

	return FALSE;
}

void stop_idling(CHAR_DATA *ch)
{
	if (ch == NULL
	||  ch->desc == NULL
	||  ch->desc->connected != CON_PLAYING
	||  !ch->was_in_room
	||  ch->in_room->vnum != ROOM_VNUM_LIMBO)
		return;

	ch->timer = 0;
	char_from_room(ch);
	act("$N has returned from the void.",
	    ch->was_in_room->people, NULL, ch, TO_ALL);
	char_to_room(ch, ch->was_in_room);
	ch->was_in_room	= NULL;
}

void char_puts(const char *txt, CHAR_DATA *ch)
{
	send_to_char(GETMSG(txt, ch->lang), ch);
}

void char_printf(CHAR_DATA *ch, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), GETMSG(format, ch->lang), ap);
	va_end(ap);
	send_to_char(buf, ch);
}

/*
 * Write to one char.
 */
void send_to_char(const char *txt, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH*8];

	if (txt == NULL || ch->desc == NULL)
		return;

	parse_colors(txt, buf, sizeof(buf), OUTPUT_FORMAT(ch));
	write_to_buffer(ch->desc, buf, 0);
}

/*
 * Send a page to one char.
 */
void page_to_char(const char *txt, CHAR_DATA *ch)
{
	if (txt == NULL || ch->desc == NULL)
		return; /* ben yazdim ibrahim */

	if (ch->lines == 0) {
		send_to_char(txt, ch);
		return;
	}
	
	ch->desc->showstr_head = str_dup(txt);
	ch->desc->showstr_point = ch->desc->showstr_head;
	show_string(ch->desc, str_empty);
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
	char buffer[4*MAX_STRING_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char *scan;
	int lines = 0;
	int show_lines;

	one_argument(input, buf, sizeof(buf));
	if (buf[0] != '\0') {
		if (d->showstr_head) {
			free_string(d->showstr_head);
			d->showstr_head = NULL;
		}
		d->showstr_point  = NULL;
		return;
	}

	if (d->character)
		show_lines = d->character->lines;
	else
		show_lines = 0;

	for (scan = buffer; scan - buffer < sizeof(buffer)-2;
						scan++, d->showstr_point++) {
		/*
		 * simple copy if not eos and not eol
		 */
		if ((*scan = *d->showstr_point) && (*scan) != '\n') 
			continue;

		/*
		 * bamf out buffer if we reached eos or show_lines limit
		 */
		if (!*scan || (show_lines > 0 && ++lines >= show_lines)) {
			const char *chk;

			if (*scan)
				*++scan = '\0';
			send_to_char(buffer, d->character);

			for (chk = d->showstr_point; isspace(*chk); chk++)
				;
			if (!*chk) {
				if (d->showstr_head) {
					free_string(d->showstr_head);
					d->showstr_head = NULL;
				}
				d->showstr_point  = NULL;
			}
			return;
		}
	}
}

void log_area_popularity(void)
{
	FILE *fp;
	AREA_DATA *area;
	extern AREA_DATA *area_first;

	if ((fp = dfopen(TMP_PATH, AREASTAT_FILE, "w")) == NULL)
		return;
	fprintf(fp,"\nBooted %sArea popularity statistics (in char * ticks)\n",
	        str_boot_time);

	for (area = area_first; area != NULL; area = area->next)
		if (area->count >= 5000000)
			fprintf(fp,"%-60s overflow\n",area->name);
		else
			fprintf(fp,"%-60s %u\n",area->name,area->count);

	fclose(fp);
}

/* class ok ******************************************************
 * checks to see if the class is okay for the current defined character
 */
bool class_ok(CHAR_DATA *ch, int class)
{
	race_t *r;
	class_t *cl;

	if ((cl = class_lookup(class)) == NULL
	||  (r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata)
		return FALSE;

	if (IS_NEWBIE(ch) && class == CLASS_SAMURAI)
		return FALSE;

	if (rclass_lookup(r, cl->name) == NULL
	||  (cl->restrict_sex >= 0 && cl->restrict_sex != ch->sex))
		return FALSE;

	if (cl->restrict_align 
	&& r->pcdata->restrict_align
	&& !(r->pcdata->restrict_align & cl->restrict_align))
		return FALSE;

	if (cl->restrict_ethos
	&& r->pcdata->restrict_ethos
	&& !(r->pcdata->restrict_ethos & cl->restrict_ethos))
		return FALSE;

	return TRUE;
}

flag32_t align_restrict(CHAR_DATA *ch)
{
	race_t *r;
	flag32_t align_restrict = RA_NONE;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata)
		return RA_NONE;

	if (IS_SET(r->pcdata->restrict_align, RA_GOOD)
	&&  IS_SET(CLASS(ch->class)->restrict_align, RA_GOOD))
		align_restrict |= RA_GOOD;

	if (IS_SET(r->pcdata->restrict_align, RA_NEUTRAL)
	&&  IS_SET(CLASS(ch->class)->restrict_align, RA_NEUTRAL))
		align_restrict |= RA_NEUTRAL;

	if (IS_SET(r->pcdata->restrict_align, RA_EVIL)
	&&  IS_SET(CLASS(ch->class)->restrict_align, RA_EVIL))
		align_restrict |= RA_EVIL;

	return align_restrict;
}

void get_alignment (CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d = ch->desc;
	flag32_t allowed_align = align_restrict(ch);
	bool align_set = FALSE;

	if (allowed_align == RA_GOOD) {
		ch->alignment = 1000; 
		write_to_buffer(d, "You are compelled to follow the path of Good.\n\r",0);
		align_set = TRUE;
	}
	else if (allowed_align == RA_NEUTRAL) {
		ch->alignment = 0; 
		write_to_buffer(d, "You are compelled to follow the path of Neutrality.\n\r",0);
		align_set = TRUE;
	}
	else if (allowed_align == RA_EVIL) {
		ch->alignment = -1000; 
		write_to_buffer(d, "You are compelled to follow the path of Evil.\n\r",0);
		align_set = TRUE;
	}

	if (!align_set && argument != NULL) {
		switch(argument[0]) {
			case 'g' : case 'G' : 
				if (IS_SET(allowed_align, RA_GOOD)) {
					ch->alignment = 1000; 
					write_to_buffer(d, "Now your character is good.\n\r",0);
					align_set = TRUE;
				}
				else {
					write_to_buffer(d,"That's not a valid alignment.\n\r",0);
				}
				break;
			case 'n' : case 'N' : 
				if (IS_SET(allowed_align, RA_NEUTRAL)) {
					ch->alignment = 0; 
					write_to_buffer(d, "Now your character is neutral.\n\r",0);
					align_set = TRUE;
				}
				else {
					write_to_buffer(d,"That's not a valid alignment.\n\r",0);
				}
				break;
			case 'e' : case 'E' : 
				if (IS_SET(allowed_align, RA_EVIL)) {
					ch->alignment = -1000; 
					write_to_buffer(d, "Now your character is evil.\n\r",0);
					align_set = TRUE;
				}
				else {
					write_to_buffer(d,"That's not a valid alignment.\n\r",0);
				}
				break;
			case '?' : case 'h': case 'H':
				do_help(ch, "'alignment'");
				break;
		}
	}

	if (!align_set) {
		write_to_buffer(d, "\n\rChoose your alignment (", 0);
		if (IS_SET(allowed_align, RA_GOOD))
			write_to_buffer(d, "Good ", 0);
		if (IS_SET(allowed_align, RA_NEUTRAL))
			write_to_buffer(d, "Neutral ", 0);
		if (IS_SET(allowed_align, RA_EVIL))
			write_to_buffer(d, "Evil", 0);
		write_to_buffer(d, ")[?]: ", 0);
	}
	else {
		write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r", 0);
		if (CHAR_CREATE_ETHOS)
			d->connected = CON_GET_ETHOS;
		else
			d->connected = CON_PICK_HOMETOWN;
	}
}
	
flag32_t ethos_restrict(CHAR_DATA *ch)
{
	race_t *r;
	flag32_t ethos_restrict = RA_NONE;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata)
		return RA_NONE;

	if (IS_SET(r->pcdata->restrict_ethos, RE_LAWFUL)
	&&  IS_SET(CLASS(ch->class)->restrict_ethos, RE_LAWFUL))
		ethos_restrict |= RE_LAWFUL;

	if (IS_SET(r->pcdata->restrict_ethos, RE_NEUTRAL)
	&&  IS_SET(CLASS(ch->class)->restrict_ethos, RE_NEUTRAL))
		ethos_restrict |= RE_NEUTRAL;

	if (IS_SET(r->pcdata->restrict_ethos, RE_CHAOTIC)
	&&  IS_SET(CLASS(ch->class)->restrict_ethos, RE_CHAOTIC))
		ethos_restrict |= RE_CHAOTIC;

	return ethos_restrict;
}

void get_ethos (CHAR_DATA *ch, const char *argument) 
{
	DESCRIPTOR_DATA *d = ch->desc;
	flag32_t allowed_ethos = ethos_restrict(ch);
	bool ethos_set = FALSE;

	if (allowed_ethos == RE_LAWFUL) {
		ch->ethos = ETHOS_LAWFUL; 
		write_to_buffer(d, "You are compelled to follow the Law.\n\r",0);
		ethos_set = TRUE;
	}
	else if (allowed_ethos == RE_NEUTRAL) {
		ch->ethos = ETHOS_NEUTRAL; 
		write_to_buffer(d, "You are compelled to be be ambigious toward the Law.\n\r",0);
		ethos_set = TRUE;
	}
	else if (allowed_ethos == RE_CHAOTIC) {
		ch->ethos = ETHOS_CHAOTIC; 
		write_to_buffer(d, "You are compelled to have contempt for the Law.\n\r",0);
		ethos_set = TRUE;
	}

	if (!ethos_set && argument != NULL) {
		switch(argument[0]) {
			case 'l' : case 'L' : 
				if (IS_SET(allowed_ethos, RE_LAWFUL)) {
					ch->ethos = ETHOS_LAWFUL; 
					write_to_buffer(d, "You follow the Law.\n\r",0);
					ethos_set = TRUE;
				}
				else {
					write_to_buffer(d,"That's not a valid ethos.\n\r",0);
				}
				break;
			case 'n' : case 'N' : 
				if (IS_SET(allowed_ethos, RE_NEUTRAL)) {
					ch->ethos = ETHOS_NEUTRAL; 
					write_to_buffer(d, "You are ambigious toward the Law.\n\r",0);
					ethos_set = TRUE;
				}
				else {
					write_to_buffer(d,"That's not a valid ethos.\n\r",0);
				}
				break;
			case 'c' : case 'C' : 
				if (IS_SET(allowed_ethos, RE_CHAOTIC)) {
					ch->ethos = ETHOS_CHAOTIC; 
					write_to_buffer(d, "You have contempt for the Law.\n\r",0);
					ethos_set = TRUE;
				}
				else {
					write_to_buffer(d,"That's not a valid ethos.\n\r",0);
				}
				break;
			case '?' : case 'h': case 'H':
				do_help(ch, "'ethos'");
				break;
		}
	}

	if (!ethos_set) {
		write_to_buffer(d, "\n\rChoose your ethos (", 0);
		if (IS_SET(allowed_ethos, RE_LAWFUL))
			write_to_buffer(d, "Lawful ", 0);
		if (IS_SET(allowed_ethos, RE_NEUTRAL))
			write_to_buffer(d, "Neutral ", 0);
		if (IS_SET(allowed_ethos, RE_CHAOTIC))
			write_to_buffer(d, "Chaotic", 0);
		write_to_buffer(d, ")[?]: ", 0);
	}
	else {
		write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r", 0);
		d->connected = CON_PICK_HOMETOWN;
	}
}

#if !defined (WIN32)
void resolv_done()
{
	char *host;
	char buf[MAX_STRING_LENGTH];
	char *p;
	DESCRIPTOR_DATA *d;

	while (fgets(buf, sizeof(buf), rfin)) {
		if ((p = strchr(buf, '\n')) == NULL) {
			log_printf("rfin: line too long, skipping to '\\n'");
			while(fgetc(rfin) != '\n')
				;
			continue;
		}
		*p = '\0';

		if ((host = strchr(buf, '@')) == NULL)
			continue;
		*host++ = '\0';

		log_printf("resolv_done: %s@%s", buf, host);

		for (d = descriptor_list; d; d = d->next) {
			if (d->host
			||  d->character == NULL
			||  str_cmp(buf, d->character->name))
				continue;
			d->host = str_dup(host);
		}
	}
}
#endif

/* Windows 95 and Windows NT support functions (copied from Envy) */
#if defined (WIN32)
void gettimeofday (struct timeval *tp, void *tzp)
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

