/**
 * gwsocket.c -- An rfc6455-complaint Web Socket Server
 *    _______       _______            __        __
 *   / ____/ |     / / ___/____  _____/ /_____  / /_
 *  / / __ | | /| / /\__ \/ __ \/ ___/ //_/ _ \/ __/
 * / /_/ / | |/ |/ /___/ / /_/ / /__/ ,< /  __/ /_
 * \____/  |__/|__//____/\____/\___/_/|_|\___/\__/
 *
 * The MIT License (MIT)
 * Copyright (c) 2009-2020 Gerardo Orellana <hello @ goaccess.io>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>

#include "gwsocket.h"
#include "log.h"
#include "websocket.h"
#include "xmalloc.h"

#if HAVE_CONFIG_H
#include <config.h>
#endif

static WSServer *server = NULL;
static WSConfig gsconfig = { 0 };

/* *INDENT-OFF* */
static char short_options[] = "p:Vh";
static struct option long_opts[] = {
  {"port"           , required_argument , 0 , 'p' } ,
  {"addr"           , required_argument , 0 ,  0  } ,
  {"echo-mode"      , no_argument       , 0 ,  0  } ,
  {"max-frame-size" , required_argument , 0 ,  0  } ,
  {"origin"         , required_argument , 0 ,  0  } ,
  {"pipein"         , required_argument , 0 ,  0  } ,
  {"pipeout"        , required_argument , 0 ,  0  } ,
  {"std"            , no_argument       , 0 ,  0  } ,
  {"stdin"          , no_argument       , 0 ,  0  } ,
  {"stdout"         , no_argument       , 0 ,  0  } ,
#if HAVE_LIBSSL
  {"ssl-cert"       , required_argument , 0 ,  0  } ,
  {"ssl-key"        , required_argument , 0 ,  0  } ,
#endif
  {"access-log"     , required_argument , 0 ,  0  } ,
  {"strict"         , no_argument       , 0 ,  0  } ,
  {"version"        , no_argument       , 0 , 'V' } ,
  {"help"           , no_argument       , 0 , 'h' } ,
  {0, 0, 0, 0}
};

/* Command line help. */
static void
cmd_help (void)
{
  printf ("\nGWSocket - %s\n\n", GW_VERSION);

  printf (
  "Usage: "
  "gwsocket [ options ... ] -p [--addr][--origin][...]\n"
  "The following options can also be supplied to the command:\n\n"
  ""
  "  -p --port=<port>         - Specifies the port to bind.\n"
  "  -h --help                - This help.\n"
  "  -V --version             - Display version information and exit.\n"
  "  --access-log=<path/file> - Specifies the path/file for the access log.\n"
  "  --addr=<addr>            - Specify an IP address to bind to.\n"
  "  --echo-mode              - Echo all received messages.\n"
  "  --max-frame-size=<bytes> - Maximum size of a websocket frame. This\n"
  "                             includes received frames from the client\n"
  "                             and messages through the named pipe.\n"
  "  --origin=<origin>        - Ensure clients send the specified origin\n"
  "                             header upon the WebSocket handshake.\n"
  "  --pipein=<path/file>     - Creates a named pipe (FIFO) that reads\n"
  "                             from on the given path/file.\n"
  "  --pipeout=<path/file>    - Creates a named pipe (FIFO) that writes\n"
  "                             to on the given path/file.\n"
  "  --std                    - Enable --stdin and --stdout.\n"
  "  --stdin                  - Send stdin to the websocket.\n"
  "  --stdout                 - Send received websocket data to stdout.\n"
  "  --strict                 - Parse messages using strict mode. See\n"
  "                             man page for more details.\n"
  "  --ssl-cert=<cert.crt>    - Path to SSL certificate.\n"
  "  --ssl-key=<priv.key>     - Path to SSL private key.\n"
  "\n"
  "See the man page for more information `man gwsocket`.\n\n"
  "For more details visit: http://gwsocket.io\n"
  "gwsocket Copyright (C) 2020 by Gerardo Orellana"
  "\n\n"
  );
  ws_stop (server);
  exit (EXIT_FAILURE);
}
/* *INDENT-ON* */

static void
handle_signal_action (int sig_number) {
  if (sig_number == SIGINT) {
    printf ("SIGINT caught!\n");
    /* if it fails to write, force stop */
    if ((write (server->self_pipe[1], "x", 1)) == -1 && errno != EAGAIN)
      ws_stop (server);
  } else if (sig_number == SIGPIPE) {
    printf ("SIGPIPE caught!\n");
  }
}

static int
setup_signals (void) {
  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = handle_signal_action;
  if (sigaction (SIGINT, &sa, 0) != 0) {
    perror ("sigaction()");
    return -1;
  }
  if (sigaction (SIGPIPE, &sa, 0) != 0) {
    perror ("sigaction()");
    return -1;
  }
  return 0;
}

static int
onopen (WSPipeOut * pipeout, WSClient * client) {
  uint32_t hsize = sizeof (uint32_t) * 3;
  char *hdr = calloc (hsize, sizeof (char));
  char *ptr = hdr;

  ptr += pack_uint32 (ptr, client->listener);
  ptr += pack_uint32 (ptr, 0x10);
  ptr += pack_uint32 (ptr, INET6_ADDRSTRLEN);

  ws_write_fifo (pipeout, hdr, hsize);
  ws_write_fifo (pipeout, client->remote_ip, INET6_ADDRSTRLEN);
  free (hdr);

  return 0;
}

static int
onclose (WSPipeOut * pipeout, WSClient * client) {
  uint32_t hsize = sizeof (uint32_t) * 3;
  char *hdr = calloc (hsize, sizeof (char));
  char *ptr = hdr;

  ptr += pack_uint32 (ptr, client->listener);
  ptr += pack_uint32 (ptr, 0x08);
  ptr += pack_uint32 (ptr, 0);

  ws_write_fifo (pipeout, hdr, hsize);
  free (hdr);

  return 0;
}

static int
onmessage (WSPipeOut * pipeout, WSClient * client) {
  WSMessage **msg = &client->message;
  uint32_t hsize = sizeof (uint32_t) * 3;
  char *hdr = NULL, *ptr = NULL;

  hdr = calloc (hsize, sizeof (char));
  ptr = hdr;
  ptr += pack_uint32 (ptr, client->listener);
  ptr += pack_uint32 (ptr, (*msg)->opcode);
  ptr += pack_uint32 (ptr, (*msg)->payloadsz);

  ws_write_fifo (pipeout, hdr, hsize);
  ws_write_fifo (pipeout, (*msg)->payload, (*msg)->payloadsz);
  free (hdr);

  return 0;
}

static void
parse_long_opt (const char *name, const char *oarg) {
  if (!strcmp ("addr", name))
    gsconfig.host = oarg;
  if (!strcmp ("echo-mode", name))
    gsconfig.echomode = 1;
  if (!strcmp ("max-frame-size", name))
    gsconfig.max_frm_size = atoi (oarg);
  if (!strcmp ("origin", name))
    gsconfig.origin = oarg;
  if (!strcmp ("pipein", name))
    gsconfig.pipein = oarg;
  if (!strcmp ("pipeout", name))
    gsconfig.pipeout = oarg;
  if (!strcmp ("strict", name))
    gsconfig.strict = 1;
  if (!strcmp ("access-log", name))
    gsconfig.accesslog = oarg;
#if HAVE_LIBSSL
  if (!strcmp ("ssl-cert", name))
    gsconfig.sslcert = oarg;
  if (!strcmp ("ssl-key", name))
    gsconfig.sslkey = oarg;
#endif
  if (!strcmp ("std", name)) {
    gsconfig.use_stdin = 1;
    gsconfig.use_stdout = 1;
  }
  if (!strcmp ("stdin", name))
    gsconfig.use_stdin = 1;
  if (!strcmp ("stdout", name))
    gsconfig.use_stdout = 1;
}

static void
set_server_opts (void) {
  if (gsconfig.host)
    ws_set_config_host (gsconfig.host);
  if (gsconfig.port)
    ws_set_config_port (gsconfig.port);
  if (gsconfig.echomode)
    ws_set_config_echomode (1);
  if (gsconfig.max_frm_size)
    ws_set_config_frame_size (gsconfig.max_frm_size);
  if (gsconfig.origin)
    ws_set_config_origin (gsconfig.origin);
  if (gsconfig.pipein)
    ws_set_config_pipein (gsconfig.pipein);
  if (gsconfig.pipeout)
    ws_set_config_pipeout (gsconfig.pipeout);
  if (gsconfig.strict)
    ws_set_config_strict (1);
  if (gsconfig.accesslog)
    ws_set_config_accesslog (gsconfig.accesslog);
#if HAVE_LIBSSL
  if (gsconfig.sslcert)
    ws_set_config_sslcert (gsconfig.sslcert);
  if (gsconfig.sslkey)
    ws_set_config_sslkey (gsconfig.sslkey);
#endif
  if (gsconfig.use_stdin && gsconfig.use_stdout) {
    ws_set_config_stdin (1);
    ws_set_config_stdout (1);
  }
  if (gsconfig.use_stdin)
    ws_set_config_stdin (1);
  if (gsconfig.use_stdout)
    ws_set_config_stdout (1);
}

/* Read the user's supplied command line options. */
static int
read_option_args (int argc, char **argv) {
  int o, idx = 0;

  while ((o = getopt_long (argc, argv, short_options, long_opts, &idx)) >= 0) {
    if (-1 == o || EOF == o)
      break;
    switch (o) {
    case 'p':
      gsconfig.port = optarg;
      break;
    case 'h':
      cmd_help ();
      return 1;
    case 'V':
      fprintf (stdout, "GWSocket %s\n", GW_VERSION);
      return 1;
    case 0:
      parse_long_opt (long_opts[idx].name, optarg);
      break;
    case '?':
      return 1;
    default:
      return 1;
    }
  }

  for (idx = optind; idx < argc; idx++)
    cmd_help ();

  return 0;
}

static void
set_self_pipe (void) {
  /* Initialize self pipe. */
  if (pipe (server->self_pipe) == -1)
    FATAL ("Unable to create pipe: %s.", strerror (errno));

  /* make the read and write pipe non-blocking */
  set_nonblocking (server->self_pipe[0]);
  set_nonblocking (server->self_pipe[1]);
}

int
main (int argc, char **argv) {
  if (read_option_args (argc, argv))
    exit (EXIT_FAILURE);

  if ((server = ws_init ("0.0.0.0", "7890", set_server_opts)) == NULL) {
    perror ("Error during ws_init.\n");
    exit (EXIT_FAILURE);
  }
  /* callbacks */
  server->onclose = onclose;
  server->onmessage = onmessage;
  server->onopen = onopen;

  set_self_pipe ();
  if (setup_signals () != 0)
    exit (EXIT_FAILURE);

  ws_start (server);
  ws_stop (server);

  return EXIT_SUCCESS;
}
