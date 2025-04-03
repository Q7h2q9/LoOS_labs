.global _start

_start:
    # 调用getppid系统调用
    li.w    $a7, 173     # SYS_getppid 系统调用号
    syscall 0            # 执行系统调用
    # 返回值在a0中，直接作为退出码

