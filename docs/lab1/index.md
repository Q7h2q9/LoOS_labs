# lab 1 系统调用
切换 lab 1分支
```shell
cd loos-lab-2025
git checkout lab1
```
## task 1
阅读 [loongarch ISA](https://docs.kernel.org/arch/loongarch/introduction.html)， 理解 loos 中 syscall 的实现方式。

在 core/syscall.c 实现一个简单的系统调用 sys_getppid，并在 scripts/gradelib/ 目录编写汇编代码 sys_test.s 实现从用户态调用该系统调用。


编译为可执行文件 sys_test 
```shell
toolchain/loongarch/gcc-13.2.0-loongarch64-linux-gnu-nw/bin/loongarch64-linux-gnu-as -o /tmp/sys_test.o scripts/gradelib/sys_test.s

toolchain/loongarch/gcc-13.2.0-loongarch64-linux-gnu-nw/bin/loongarch64-linux-gnu-ld -static -o /tmp/sys_test /tmp/sys_test.o

chmod +x /tmp/sys_test
```

挂载rootfs并复制测试程序(如果需要权限就使用 sudo 命令)
```shell
mount toolchain/loongarch/qemu/2k1000/rootfs.img /mnt
cp /tmp/sys_test /mnt/bin/
chmod +x /mnt/bin/sys_test
umount /mnt
```

执行测试程序
```shell
chmod +x scripts/gradelib/grade-lab1-sys

./scripts/gradelib/grade-lab1-sys 
```

## task 2
阅读 [龙芯2K1000LA处理器用户手册](https://loongson.cn/uploads/images/2022090113542571398.%E9%BE%99%E8%8A%AF2K1000LA%E5%A4%84%E7%90%86%E5%99%A8%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C.pdf) RTC相关文档，根据 rtc.h 提供的 API 实现 sys_clock_gettime 和 sys_gettimeofday 

执行测试程序
```shell
chmod +x scripts/gradelib/grade-lab1-rtc

./scripts/gradelib/grade-lab1-rtc
``` 