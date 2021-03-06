/*  WMBubble dockapp 1.2 - Linux specific code
 *  Copyright 2000, 2001 timecop@japan.co.jp
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include "include/bubblemon.h"
#include "include/sys_include.h"

extern BubbleMonData bm;

/* returns current CPU load in percent, 0 to 100 */
int system_cpu(void) {
	unsigned int cpuload;
	u_int64_t load, total, oload, ototal;
	u_int64_t ab, ac, ad, ae;
	int i;
	FILE *stat;

	stat = fopen("/proc/stat", "r");
	fscanf(stat, "%*s %Ld %Ld %Ld %Ld", &ab, &ac, &ad, &ae);
	fclose(stat);

	/* Find out the CPU load */
	/* user + sys = load
	 * total = total */
	load = ab + ac + ad;	/* cpu.user + cpu.sys; */
	total = ab + ac + ad + ae;	/* cpu.total; */

	/* "i" is an index into a load history */
	i = bm.loadIndex;
	oload = bm.load[i];
	ototal = bm.total[i];

	bm.load[i] = load;
	bm.total[i] = total;
	bm.loadIndex = (i + 1) % bm.samples;

	/*
	  Because the load returned from libgtop is a value accumulated
	  over time, and not the current load, the current load percentage
	  is calculated as the extra amount of work that has been performed
	  since the last sample. yah, right, what the fuck does that mean?
	*/
	if (ototal == 0)		/* ototal == 0 means that this is the first time we get here */
		cpuload = 0;
	else if ((total - ototal) <= 0)
		cpuload = 100;
	else
		cpuload = (100 * (load - oload)) / (total - ototal);

	return cpuload;
}

void system_memory(void) {
	char *p;
	FILE *mem;

	/* put this in permanent storage instead of stack */
	static char shit[2048];

	/* we might as well get both swap and memory at the same time.
	 * sure beats opening the same file twice */
	mem = fopen("/proc/meminfo", "r");
	memset(shit, 0, sizeof(shit));
	fread(shit, 2048, 1, mem);
	p = strstr(shit, "MemTotal");
	if (p) {
		sscanf(p, "MemTotal:%Ld", &bm.mem_max);
		bm.mem_max <<= 10;

		p = strstr(p, "Active");
		if (p) {
			sscanf(p, "Active:%Ld", &bm.mem_used);
			bm.mem_used <<= 10;

			p = strstr(p, "SwapTotal");
			if (p) {
				sscanf(p, "SwapTotal:%Ld", &bm.swap_max);
				bm.swap_max <<= 10;

				p = strstr(p, "SwapFree");
				if (p) {
					sscanf(p, "SwapFree:%Ld", &bm.swap_used);
					bm.swap_used = bm.swap_max - (bm.swap_used << 10);
				}
			}
		}
	}
	fclose(mem);
}

void system_loadavg(void) {
	FILE *avg;
	avg = fopen("/proc/loadavg", "r");
	fscanf(avg, "%d.%d %d.%d %d.%d", &bm.loadavg[0].i, &bm.loadavg[0].f,
	       &bm.loadavg[1].i, &bm.loadavg[1].f,
	       &bm.loadavg[2].i, &bm.loadavg[2].f);
	fclose(avg);
}
