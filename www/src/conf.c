#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "conf.h"

cfg_t* get_conf()
{
	static cfg_bool_t persistance = cfg_false;
	static char *interface = NULL;
    static char *ip = NULL;
	static char *server_signature = NULL;
    static long int port = 8080;
	static long int childs = 10;
	

	/* Although the macro used to specify an integer option is called
	 * CFG_SIMPLE_INT(), it actually expects a long int. On a 64 bit system
	 * where ints are 32 bit and longs 64 bit (such as the x86-64 or amd64
	 * architectures), you will get weird effects if you use an int here.
	 *
	 * If you use the regular (non-"simple") options, ie CFG_INT() and use
	 * cfg_getint(), this is not a problem as the data types are implicitly
	 * cast.
	 */
	

	cfg_opt_t opts[] = {
		CFG_SIMPLE_BOOL("persistance", &persistance),
		CFG_SIMPLE_STR("interface", &interface),
		CFG_SIMPLE_STR("ip", &ip),
		CFG_SIMPLE_INT("port", &port),
		CFG_SIMPLE_INT("childs", &childs),
		CFG_SIMPLE_STR("server_signature", &server_signature),
		CFG_END()
	};
	cfg_t *cfg;

	/* Localize messages & types according to environment, since v2.9 */
#ifdef LC_MESSAGES
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
#endif

	/* set default value for the interface option */
	interface = strdup("Default");
    ip = strdup("Default");

	cfg = cfg_init(opts, 0);
	cfg_parse(cfg, "server.conf");

	cfg_print(cfg, stdout);
	
	
    return cfg;
	//return cfg;
}