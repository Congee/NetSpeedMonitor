#include <stdio.h>
#include <sys/sockio.h>
#include <unistd.h>
#include <net/if.h>
#include <net/if_media.h>
#include <net/if_mib.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <ifaddrs.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

static unsigned int interval = 1;

static bool is_interface_active(int fd, char *ifname) {
	struct ifmediareq media = {0};
	strncpy(media.ifm_name, ifname, IFNAMSIZ);
	if (ioctl(fd, SIOCGIFMEDIA, (caddr_t)&media) < 0)
		return false;

	return media.ifm_status & IFM_AVALID && media.ifm_status & IFM_ACTIVE;
}

static char *active_interface() {
	struct ifaddrs *ifap;
	
	int fd; // finds the active interface by establishing a udp socket
	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
		perror("socket()");

	getifaddrs(&ifap);
	struct ifaddrs *ifa = ifap;
	for (; ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_data &&
			is_interface_active(fd, ifa->ifa_name) && 
			ifa->ifa_addr->sa_family == AF_LINK)
		{
			break;
		}

	close(fd);
	freeifaddrs(ifap);
	return ifa->ifa_name;
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
	int fd;
	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
		perror("socket()");

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

struct human_readble_string {
	long double number;
	char *suffix;
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

#if 0
int main(void) {
	struct ifmibdata ifmib;
	struct if_data64 ifdata = {0};
	struct human_readble_string string = {0, NULL};

	while (true) {
		//memset(&string, 0, sizeof(string));
		fill_interface_data(&ifmib);
		size_t rx_bytes = ifmib.ifmd_data.ifi_ibytes - ifdata.ifi_ibytes;
		size_t tx_bytes = ifmib.ifmd_data.ifi_obytes - ifdata.ifi_obytes;

		humanize_digit(rx_bytes, &string);
		printf("%.1Lf %s", string.number, string.suffix);
		putchar('\t');
		humanize_digit(tx_bytes, &string);
		printf("%.1Lf %s", string.number, string.suffix);
		putchar('\n');

		sleep(interval);
		ifdata = ifmib.ifmd_data;
	}
	return 0;
}
#endif