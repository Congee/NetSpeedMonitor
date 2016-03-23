#ifndef SPEEDPROVIDER_H
#define SPEEDPROVIDER_H

#include <net/if.h>
#include <net/if_media.h>
#include <net/if_mib.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <net/if_mib.h>

struct human_readble_string {
	long double number;
	char *suffix;
};

void fill_interface_data(struct ifmibdata *ifmib);
void humanize_digit(long double number, struct human_readble_string *string);

#endif /* end of include guard: SPEEDPROVIDER_H */
