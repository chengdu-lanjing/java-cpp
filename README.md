# java-cpp是什么？ #
java-cpp是一个C++框架，让开发人员以Java的思维方式开发C++应用，从而来绕过C++的一系列陷阱，大幅降低开发难度，提高复杂项目的开发体验。此外，java-cpp还对部分JDK API和部分开源框架提供了类似实现。

# 如何运行demo？ #

 1. 进入linux或mac操作系统（目前暂不支持windows）
 2. 将本仓库git clone到本地，或下载到本地再解压。加入最终java-cpp根目录为${java-cpp-home}
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
1. [内存管理： 强引用，弱引用，对象复活](./doc/memory.md)
2. [异常：异常链，finally](./doc/exception.md)
3. [功能性接口：功能合并和拆分，事件支持](./doc/functional.md)
4. [数组](./doc/array.md)
5. [多线程：阻塞队列，线程池，调度器](./doc/threading.md)
6. [日志](./doc/logging.md)
7. [HTTP](./doc/http.md)
8. [数据库：统一的CDBC API，Sqlite3驱动](./doc/database.md)

# 版权 #

----------
陈涛，成都蓝景信息技术有限公司CTO
2018年11月4日
