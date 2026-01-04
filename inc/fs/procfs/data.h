
#ifndef INC_FS_PROCFS_DATA_H
#define INC_FS_PROCFS_DATA_H

struct proc;

int get_data_cmdline(struct proc *proc, char *buffer, int max);
int get_data_stat(struct proc *proc, char *buffer, int max);
int get_data_statm(struct proc *proc, char *buffer, int max);

int get_data_mounts(struct proc *proc, char *buffer, int max);

#endif
