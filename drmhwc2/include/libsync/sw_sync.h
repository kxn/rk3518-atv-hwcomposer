/* Minimal sw_sync userspace shim for RK BSP kernels (CONFIG_SW_SYNC=y).
 * This vendor build cannot use the legacy exported libsync/sw_sync.h API.
 * Match the RK kernel UAPI; do not depend on debugfs permissions. */
#ifndef RK_SW_SYNC_H
#define RK_SW_SYNC_H

#include <linux/ioctl.h>
#include <linux/types.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define SW_SYNC_IOC_MAGIC 'W'

struct sw_sync_create_fence_data {
	__u32	value;
	char	name[32];
	__s32	fence;
};

static_assert(sizeof(sw_sync_create_fence_data) == 40, "sw_sync ioctl size");
static_assert(offsetof(sw_sync_create_fence_data, fence) == 36, "sw_sync fence offset");

/* bionic does not expose _IOWR via linux/ioctl.h; encode the ioctl number
 * directly (dir<<30 | size<<16 | type<<8 | nr). struct size = 40 bytes. */
#define SW_SYNC_IOC_CREATE_FENCE	((3u << 30) | (40u << 16) | ('W' << 8) | 0)
#define SW_SYNC_IOC_INC			((1u << 30) | (4u << 16) | ('W' << 8) | 1)

static inline int sw_sync_timeline_create(void)
{
	/* Prefer the device node: debugfs can be root-only on production images. */
	int fd = open("/dev/sw_sync", O_RDWR | O_CLOEXEC);
	if (fd < 0)
		fd = open("/sys/kernel/debug/sync/sw_sync", O_RDWR | O_CLOEXEC);
	return fd;
}

static inline int sw_sync_fence_create(int fd, const char *name, unsigned int value)
{
	struct sw_sync_create_fence_data data = {};
	int err;
	data.value = value;
	strncpy(data.name, name ? name : "rkfence", sizeof(data.name) - 1);
	data.name[sizeof(data.name) - 1] = 0;
	err = ioctl(fd, SW_SYNC_IOC_CREATE_FENCE, &data);
	return err < 0 ? err : data.fence;
}

static inline int sw_sync_timeline_inc(int fd, unsigned int count)
{
	return ioctl(fd, SW_SYNC_IOC_INC, &count);
}

#endif
