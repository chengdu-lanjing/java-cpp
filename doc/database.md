# 数据库之CDBC #

java-cpp提供了一套类似于JDBC简化版的API，用于统一不同数据库的操作方式，简称CDBC。与JDBC要求数据库连接字符串以"jdbc:"开头类似，CDBC要求数据库连接字符串以"cdbc:"开头。这套API定义在头文件database/Database.h中。

java-app对CDBC API给出了一个针对[sqlite3](https://www.sqlite.org/index.html)的驱动实现，由于该驱动封装了[sqlite3](https://www.sqlite.org/index.html)的C接口，使用前请务必先安装[sqlite3](https://www.sqlite.org/index.html)。这个封装兼驱动实现在头文件database/Sqlite.h中。

database/Database.h文件除了定义了CDBC接口的API外，其com_lanjing_cpp_common_database_spi名字空间空间中也定义了一些SPI，想要为其它数据库实现CDBC驱动的开发者基于其成果继续开发会比自己重头开发便捷一点。

关于CDBC该如何使用，请参见配套demo。


----------

[<上一篇：HTTP](./http.md) | [首页](../)
