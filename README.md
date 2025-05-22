# ICS2017 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System) in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to http://nju-ics.gitbooks.io/ics2017-programming-assignment/content/

To initialize, run
```bash
bash init.sh
```

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Nexus-am](https://github.com/NJU-ProjectN/nexus-am)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)

经 2025 年春南开大学计算机系统设计课程实践后，发现该项目完全可以在64位环境下运行，并不会出现太多前人提到的bug。实际遇到的问题只有两点：

- QEMU加载时报 "qemu_set_nonblock: Assertion `f == 0' failed." 错误；
- navy-apps 下提供的 libc 中 errno 存在多重定义。

当前仓库已经解决了这两个问题，如果在2025年春后南开的计算机系统设计课程的课程实验仍无改革，可直接使用本仓库在64位环境下进行实验。另外，
由于已经进行过 init 操作，所以项目提供的 init.sh 不会再导入环境变量，需手动在 shell 配置文件（如 ~/.bashrc, ~/.zshrc ）中添加环境变量导入：

```
export NEMU_HOME={ICS_HOME}/nemu
export AM_HOME={ICS_HOME}/nexus-am
export NAVY_HOME={ICS_HOME}/navy-apps
```

其中的 {ICS_HOME} 应当替换为 ics 项目的路径。
