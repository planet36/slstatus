/* See LICENSE file for copyright and license details. */
#pragma once

/* battery */
const char *battery_meter(const char *);
const char *battery_perc(const char *);
const char *battery_remaining(const char *);
const char *battery_state(const char *);

/* clocktime */
const char *clockdiff(const char *unused);
const char *clocktime(const char *unused);

/* counter */
const char *counter(const char *unused);

/* cpu */
const char *cpu_cmeter(const char *unused);
const char *cpu_freq(const char *unused);
const char *cpu_hist(const char *unused);
const char *cpu_meter(const char *unused);
const char *cpu_perc(const char *unused);

/* datetime */
const char *datetime(const char *fmt);

/* disk */
const char *disk_free(const char *path);
const char *disk_meter(const char *path);
const char *disk_perc(const char *path);
const char *disk_total(const char *path);
const char *disk_used(const char *path);

/* entropy */
const char *entropy(const char *unused);

/* hostname */
const char *hostname(const char *unused);

/* ip */
const char *ipv4(const char *interface);
const char *ipv6(const char *interface);

/* kernel_release */
const char *kernel_release(const char *unused);

/* keyboard_indicators */
const char *keyboard_indicators(const char *fmt);

/* keymap */
const char *keymap(const char *unused);

/* load_avg */
const char *load_avg(const char *unused);

/* netspeeds */
const char *netspeed_rx(const char *interface);
const char *netspeed_tx(const char *interface);

/* num_files */
const char *num_files(const char *path);

/* ram */
const char *ram_free(const char *unused);
const char *ram_hist(const char *unused);
const char *ram_meter(const char *unused);
const char *ram_perc(const char *unused);
const char *ram_total(const char *unused);
const char *ram_used(const char *unused);

/* run_command */
const char *run_command(const char *cmd);

/* separator */
const char *separator(const char *separator);

/* swap */
const char *swap_free(const char *unused);
const char *swap_hist(const char *unused);
const char *swap_meter(const char *unused);
const char *swap_perc(const char *unused);
const char *swap_total(const char *unused);
const char *swap_used(const char *unused);

/* temperature */
const char *temp(const char *);

/* uptime */
const char *uptime(const char *unused);

/* user */
const char *gid(const char *unused);
const char *uid(const char *unused);
const char *username(const char *unused);

/* volume */
const char *vol_perc(const char *card);

/* wifi */
const char *wifi_essid(const char *interface);
const char *wifi_perc(const char *interface);
