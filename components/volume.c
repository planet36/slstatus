/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* percentages will be clamped to 99 */
//#define MAX_PCT_99

#if defined(__OpenBSD__) | defined(__FreeBSD__)
	#include <poll.h>
	#include <sndio.h>
	#include <stdlib.h>
	#include <sys/queue.h>

	struct control {
		LIST_ENTRY(control)	next;
		unsigned int		addr;
	#define CTRL_NONE	0
	#define CTRL_LEVEL	1
	#define CTRL_MUTE	2
		unsigned int		type;
		unsigned int		maxval;
		unsigned int		val;
	};

	static LIST_HEAD(, control) controls = LIST_HEAD_INITIALIZER(controls);
	static struct pollfd *pfds;
	static struct sioctl_hdl *hdl;

	/*
	 * Call-back to obtain the description of all audio controls.
	 */
	static void
	ondesc(void *unused, struct sioctl_desc *desc, int val)
	{
		struct control *c, *ctmp;
		unsigned int type = CTRL_NONE;

		if (desc == NULL)
			return;

		/* Delete existing audio control with the same address. */
		LIST_FOREACH_SAFE(c, &controls, next, ctmp) {
			if (desc->addr == c->addr) {
				LIST_REMOVE(c, next);
				free(c);
				break;
			}
		}

		/* Only match output.level and output.mute audio controls. */
		if (desc->group[0] != 0 ||
		    strcmp(desc->node0.name, "output") != 0)
			return;
		if (desc->type == SIOCTL_NUM &&
		    strcmp(desc->func, "level") == 0)
			type = CTRL_LEVEL;
		else if (desc->type == SIOCTL_SW &&
			 strcmp(desc->func, "mute") == 0)
			type = CTRL_MUTE;
		else
			return;

		c = malloc(sizeof(struct control));
		if (c == NULL) {
			warnx("sndio: failed to allocate audio control");
			return;
		}

		c->addr = desc->addr;
		c->type = type;
		c->maxval = desc->maxval;
		c->val = val;
		LIST_INSERT_HEAD(&controls, c, next);
	}

	/*
	 * Call-back invoked whenever an audio control changes.
	 */
	static void
	onval(void *unused, unsigned int addr, unsigned int val)
	{
		struct control *c;

		LIST_FOREACH(c, &controls, next) {
			if (c->addr == addr)
				break;
		}
		c->val = val;
	}

	static void
	cleanup(void)
	{
		struct control *c;

		if (hdl) {
			sioctl_close(hdl);
			hdl = NULL;
		}

		free(pfds);
		pfds = NULL;

		while (!LIST_EMPTY(&controls)) {
			c = LIST_FIRST(&controls);
			LIST_REMOVE(c, next);
			free(c);
		}
	}

	static int
	init(void)
	{
		hdl = sioctl_open(SIO_DEVANY, SIOCTL_READ, 0);
		if (hdl == NULL) {
			warnx("sndio: cannot open device");
			goto failed;
		}

		if (!sioctl_ondesc(hdl, ondesc, NULL)) {
			warnx("sndio: cannot set control description call-back");
			goto failed;
		}

		if (!sioctl_onval(hdl, onval, NULL)) {
			warnx("sndio: cannot set control values call-back");
			goto failed;
		}

		pfds = calloc(sioctl_nfds(hdl), sizeof(struct pollfd));
		if (pfds == NULL) {
			warnx("sndio: cannot allocate pollfd structures");
			goto failed;
		}

		return 1;
	failed:
		cleanup();
		return 0;
	}

	const char *
	vol_perc(const char *unused)
	{
		static int initialized;
		struct control *c;
		int n, v, value;

		if (!initialized)
			initialized = init();

		if (hdl == NULL)
			return NULL;

		n = sioctl_pollfd(hdl, pfds, POLLIN);
		if (n > 0) {
			n = poll(pfds, n, 0);
			if (n > 0) {
				if (sioctl_revents(hdl, pfds) & POLLHUP) {
					warnx("sndio: disconnected");
					cleanup();
					initialized = 0;
					return NULL;
				}
			}
		}

		value = 100;
		LIST_FOREACH(c, &controls, next) {
			if (c->type == CTRL_MUTE && c->val == 1)
				value = 0;
			else if (c->type == CTRL_LEVEL) {
				v = (c->val * 100 + c->maxval / 2) / c->maxval;
				/* For multiple channels return the minimum. */
				if (v < value)
					value = v;
			}
		}

#ifdef MAX_PCT_99
		if (value > 99)
			value = 99;
#endif

		return bprintf("%d", value);
	}
#else
	#include <sys/soundcard.h>

	const char *
	vol_perc(const char *card)
	{
		size_t i;
		int v, afd, devmask;
		char *vnames[] = SOUND_DEVICE_NAMES;

		if ((afd = open(card, O_RDONLY | O_NONBLOCK)) < 0) {
			warn("open '%s'", card);
			return NULL;
		}

		if (ioctl(afd, (int)SOUND_MIXER_READ_DEVMASK, &devmask) < 0) {
			warn("ioctl 'SOUND_MIXER_READ_DEVMASK'");
			(void)close(afd);
			return NULL;
		}
		for (i = 0; i < LEN(vnames); i++) {
			if (devmask & (1 << i) && !strcmp("vol", vnames[i])) {
				if (ioctl(afd, MIXER_READ(i), &v) < 0) {
					warn("ioctl 'MIXER_READ(%ld)'", i);
					(void)close(afd);
					return NULL;
				}
			}
		}

		(void)close(afd);
		v &= 0xFF;

#ifdef MAX_PCT_99
		if (v > 99)
			v = 99;
#endif

		return bprintf("%d", v);
	}
#endif
