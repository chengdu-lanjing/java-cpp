# java-cpp是什么？ #

java-cpp是一个C++框架，让开发人员以Java的思维方式开发C++应用，从而来绕过C++的一系列陷阱，大幅降低开发难度，提高复杂项目的开发体验。此外，java-cpp还对部分JDK API和部分开源框架提供了类似实现。

*事实上，java-cpp的诞生源自公司内部的一个移动项目，最初我们开发了一个Android应用，该应用有一系列复杂的模块，和Android平台关联性不大，但内部逻辑和算法比较复杂，具备较高的移植价值和可行性。后来公司要开发iOS版本，面对复杂度很高的这些模块，没有人愿意用Swift再开发一次，因为这样会导致产品线分裂，日后维护、改进、升级和测试都要做两次。最终我决定开发java-cpp框架，支持以Java的思维来开发C++应用，随后将Android项目中这些希望被移植的模块机械地翻译成了C++形成了共享库，该共享库同时支撑起了项目的Android版本和iOS版本，并没有导致产品代码出现分裂，同时和Java一脉相承的代码风格也不会导致可维护性问题，即便维护的人C++经验不太丰富。对这个项目而言这是最好的技术选择。现在这个开源版本是将公司业务无关的部分剥离出来，其稳定性和可靠性已经在实际项目中经过了大量的验证和测试。*

# 如何运行demo？ #

 1. 进入linux或mac操作系统（目前暂不支持windows）
 2. 将本仓库git clone到本地，或下载到本地再解压。假设最终java-cpp根目录为${java-cpp-home}
 3. cd {java-cpp-home}/scripts
 4. ./all_demos.sh
 5. 根据命令行打印的菜单列表，输入选择项再回车。可以选择运行单个demo，一系列demo，或所有的demo

 如果执行所有的demo，能看到类似的[输出结果](./doc/all_demos_result.txt)
 
**注意事项**
1. 如果要运行HTTP相关的demo，需先安装[curl](https://curl.haxx.se/)。
2. 如果要运行sqlite3相关的demo，需先安装[sqlite3](https://www.sqlite.org/index.html)。

# 如何开发自己的应用？ #
1. 本仓库src目录下的头文件提供了所有的功能，并无相应的cpp文件或编译后的so库， 将此目录加入C++编译环境的包含路径列表即可。
2. 在C++连接器参数中添加"-lpthread"；如果使用了HTTP相关功能, 请添加"-lcurl"(需先安装[curl](https://curl.haxx.se/))；如果使用了sqlite相关的功能， 请添加"-lsqlite3"(需先安装[sqlite3](https://www.sqlite.org/index.html))。
 
# 文档目录 #
1. [内存管理： 强引用，弱引用，内存泄漏，对象复活](./doc/memory.md)
2. [异常：异常链，finally](./doc/exception.md)
3. [功能性接口：功能合并和拆分，事件支持](./doc/functional.md)
4. [数组](./doc/array.md)
5. [多线程：阻塞队列，线程池，调度器](./doc/threading.md)
6. [日志](./doc/logging.md)
7. [HTTP](./doc/http.md)
8. [数据库：统一的CDBC API，Sqlite3驱动](./doc/database.md)

# 版权 #
本框架归成都蓝景信息技术有限（Chengdu Lanjing Data&information Technology Co.,L）公司所有，协议为MIT，请阅读[LICENSE](./LICENSE)以了解更多

----------
陈涛，成都蓝景信息技术有限公司CTO

2018年11月4日
