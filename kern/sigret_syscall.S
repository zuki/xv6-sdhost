.global execute_sigret_syscall_start
.global execute_sigret_syscall_end

execute_sigret_syscall_start:
# 139 = SYS_rt_sigreturn
    mov x8, #139
    svc #0

execute_sigret_syscall_end:
