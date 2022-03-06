/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_PROC_H
#define _BRAMBLE_PROC_H

/* Times per cpu, in USER_HZ units, since boot.
 * See proc(5).
 */
#define CPU_NAME_SIZE 16
struct proc_cpu {
    char name[CPU_NAME_SIZE];  // cpu name (cpu, cpu0, cpu1, ...)
    int user;       // time spent in user mode
    int nice;       // time spent in user mode with low priority (nice)
    int system;     // time spent in system mode
    int idle;       // time spent in the idle stack
    int iowait;     // time waiting for I/O to complete (unreliable)
    int irq;        // time servicing interrupts
    int softirq;    // time servicing softirqs
    int steal;      // stolen time (in virtualized env)
    int guest;      // time spent running a virtual CPU for guests
    int guest_nice; // time spent running a niced guest
};

/* Get the overall CPU usage.
 * Returns 0 on success, -1 on failure.
 */
int proc_stat_get_cpu (struct proc_cpu *cpu);

/* Get the CPU usage for each CPU (up to count).
 * 'cpu' is a pre-allocated array of 'proc_cpu' structures.
 * Returns the number of structures filled, or -1 on error.
 */
int proc_stat_get_cpus (struct proc_cpu *cpu, size_t count);

/* Given two usage samples, compute the fraction "used" (between 0 and 1).
 * If one or both of the samples is all zeroes, return 0.
 */
double proc_stat_calc_cpu_usage (struct proc_cpu *sample1,
                                 struct proc_cpu *sample2);

#endif /* !_BRAMBLE_PROC */

// vi:ts=4 sw=4 expandtab
