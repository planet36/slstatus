/* See LICENSE file for copyright and license details. */
#pragma once

/* interval between updates (in ms) */
const unsigned int interval = 2000;

/* text to show if no value can be retrieved */
static const char unknown_str[] = "n/a";

/* maximum output string length */
#define MAXLEN 2048

/*
 * function            description                     argument (example)
 *
 * battery_meter       battery meter, unicode          battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_perc        battery percentage              battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_remaining   battery remaining HH:MM         battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_state       battery charging state          battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * clockdiff           elapsed time between calls      NULL
 * clocktime           high-resolution clock           NULL
 * counter             integer counter of samples      NULL
 * cpu_cmeter          cpu usage meter, ascii          NULL
 * cpu_freq            cpu frequency in MHz            NULL
 * cpu_hist            cpu usage history, unicode      NULL
 * cpu_meter           cpu usage meter, unicode        NULL
 * cpu_perc            cpu usage in percent            NULL
 * datetime            date and time                   format string (%F %T)
 * disk_free           free disk space in <SI>B        mountpoint path (/)
 * disk_meter          disk usage meter, unicode       mountpoint path (/)
 * disk_perc           disk usage in percent           mountpoint path (/)
 * disk_total          total disk space in <SI>B       mountpoint path (/)
 * disk_used           used disk space in <SI>B        mountpoint path (/)
 * entropy             available entropy               NULL
 * gid                 GID of current user             NULL
 * hostname            hostname                        NULL
 * ipv4                IPv4 address                    interface name (eth0)
 * ipv6                IPv6 address                    interface name (eth0)
 * kernel_release      `uname -r`                      NULL
 * keyboard_indicators caps/num lock indicators        format string (c?n?)
 *                                                     see keyboard_indicators.c
 * keymap              layout (variant) of current     NULL
 *                     keymap
 * load_avg            load average                    NULL
 * netspeed_rx         receive network speed           interface name (wlan0)
 * netspeed_tx         transfer network speed          interface name (wlan0)
 * num_files           number of files in a directory  path
 *                                                     (/home/foo/Inbox/cur)
 * ram_free            free memory in <SI>B            NULL
 * ram_hist            memory usage history, unicode   NULL
 * ram_meter           memory usage meter, unicode     NULL
 * ram_perc            memory usage in percent         NULL
 * ram_total           total memory size in <SI>B      NULL
 * ram_used            used memory in <SI>B            NULL
 * run_command         custom shell command            command (echo foo)
 * separator           string to echo                  NULL
 * swap_free           free swap in <SI>B              NULL
 * swap_hist           swap usage history, unicode     NULL
 * swap_meter          swap usage meter, unicode       NULL
 * swap_perc           swap usage in percent           NULL
 * swap_total          total swap size in <SI>B        NULL
 * swap_used           used swap in <SI>B              NULL
 * temp                temperature in degree celsius   sensor file
 *                                                     (/sys/class/thermal/...)
 *                                                     NULL on OpenBSD
 *                                                     thermal zone on FreeBSD
 *                                                     (tz0, tz1, etc.)
 * uid                 UID of current user             NULL
 * up                  interface is running            interface name (eth0)
 * uptime              system uptime                   NULL
 * username            username of current user        NULL
 * vol_perc            OSS/ALSA volume in percent      mixer file (/dev/mixer)
 *                                                     NULL on OpenBSD/FreeBSD
 * wifi_essid          WiFi ESSID                      interface name (wlan0)
 * wifi_perc           WiFi signal in percent          interface name (wlan0)
 *
 *
 * <SI> is a decimal or binary SI prefix.
 */
static const struct component components[] = {
	/* function format          argument */

	{ cpu_perc    , "%2s%%"    , NULL          } ,
	{ separator   , " ┆ "      , NULL          } ,
	{ ram_used    , "%7sB"     , NULL          } ,
	{ separator   , " ┆ "      , NULL          } ,
	{ netspeed_rx , "%7sB/s ▼" , "enp0s3"      } ,
	{ separator   , " ┆ "      , NULL          } ,
	{ netspeed_tx , "%7sB/s ▲" , "enp0s3"      } ,
	{ separator   , " ┆ "      , NULL          } ,
	//{ datetime    , "%s"       , "%a %b %d %T %Z" } ,
	//{ run_command , "%s"       , "TZ=America/New_York date +\"%a %b %d %T %Z\"" },
	{ datetime    , "%s"       , "%a %b %d %R" } ,

};
