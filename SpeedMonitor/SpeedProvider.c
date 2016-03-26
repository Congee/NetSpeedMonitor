#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "SpeedProvider.h"

static bool is_interface_active(int fd, char *ifname) {
	struct ifmediareq media = {0};
	strncpy(media.ifm_name, ifname, IFNAMSIZ);
	if (ioctl(fd, SIOCGIFMEDIA, (caddr_t)&media) < 0)
		return false;

	return media.ifm_status & IFM_AVALID && media.ifm_status & IFM_ACTIVE;
}

__unused
static char *active_interface() {
	// finds the active interface by establishing a udp socket
	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		perror("socket()");
		return NULL;
	}

	struct ifaddrs *ifap; // will be initilized by getifaddrs().
    if (getifaddrs(&ifap)) {
		perror("getifaddrs()");
		exit(1);
	}

	struct ifaddrs *ifa = ifap;
	for (; ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_data &&
			is_interface_active(fd, ifa->ifa_name) && 
			ifa->ifa_addr->sa_family == AF_LINK)
		{
			break;
		}

	size_t len = strlen(ifa->ifa_name);
	char *if_name = malloc(len);
	memcpy(if_name, ifa->ifa_name, len);

	close(fd);
	freeifaddrs(ifap);
	return if_name;
}

static int ifcount() {
	// sysctl net.link.generic.system.ifcount
	int count = 0;
	int name[5] = {CTL_NET, PF_LINK, NETLINK_GENERIC, IFMIB_SYSTEM, IFMIB_IFCOUNT};
	sysctl(name, 5, &count, &(size_t){sizeof(count)}, NULL, 0);
	return count;
}

/* [TODO]: rx rate, netstat/if.c +770 - 2016-01-22 04:33P */
void fill_interface_data(struct ifmibdata *ifmib) {
    // [FIX]: should establish socket connection only ONCE
	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		perror("socket()");
		exit(1);
	}
    
    // man ifmib
	int row = 1; // the index of interfaces returned by ifcount() starts with 1
	int ifcounts = ifcount();
	int name[6] = {CTL_NET, PF_LINK, NETLINK_GENERIC}; /* Common OID prefix */
	name[3] = IFMIB_IFDATA;
	name[5] = IFDATA_GENERAL;

	for (; row < ifcounts; ++row) {
		name[4] = row;
		if (sysctl(name, 6, ifmib, &(size_t){sizeof(*ifmib)}, NULL, 0) == -1)
			perror("sysctl()");

		if (is_interface_active(fd, ifmib->ifmd_name))
			break;
	}
	close(fd);
}

static const char *suffixes[] = {
	"bytes",
	"KiB",
	"MiB",
	"GiB",
	"TiB",
	"PiB",
	"EiB",
	"ZiB",
	"YiB"
};

void humanize_digit(long double number, struct human_readble_string *string) {
	unsigned int base = 1024;
	unsigned int max = 9999; // 4 digits at most
	unsigned int count;

	for (count = 0; number > max; count++)
		number /= base;

	string->number = number;
	string->suffix = (char *)suffixes[count];
}
