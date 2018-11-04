# 日志 #

为了方便程序运维和跟踪，java-cpp提供了一套slf4j接口的简化版本，于头文件log/Logger.h中。

## 配置 ##

配置由com_lanjing_cpp_common_log::Configuration提供，多个Configuration对象形成一颗配置树。静态方法root()可以得到树的根节点，例如

    Ref<RootConfiguration> rootCfg = Configuration::root();
    
而静态方法of可以得到树的叶节点，例如

    Ref<Configuration> cfg = Configuration::of("com_yourcompany_yourproject_yourmodule");
    
of方法的参数为一字符串，通常是代码中类的全名或名字空间的全名。java-cpp会尝试以‘.’、‘_’或‘::’为分隔符将此字符串分割为多段，然后以这些段为路径获取子树节点，如果不存在则创建。也就是说

    Ref<Configuration> cg = Configuration::of("com.yourcompany.yourproject.yourmodule");
    
或

    Ref<Configuration> cfg = Configuration::of("com::yourcompany::yourproject::yourmodule");

会返回和上面代码一样的对象。

再来看一例

    Ref<Configuration> projectCfg = Configuration::of("com::yourcompany::yourproject");
    Ref<Configuration> moduleCfg = Configuration::of("com::yourcompany::yourproject::yourModule");
    Ref<Configuration> classCfg = Configuration::of("com::yourcompany::yourproject::YourClass");
    
不难看出moduleCfg是projectCfg的子节点，反过来projectCfg是moduleCfg的父节点；classCfg是moduleCfg的子节点，反过来moduleCfg是classCfg的父节点。

日志中提供两个关键接口com_lanjing_cpp_common_log::Layout接口和com_lanjing_cpp_common_log::Appender接口，前者关心输出什么样式的字符，后心关心输出到哪里, 本日志API规定

1. 配置树的每个节点可以持有0到1个Layout。
2. 配置树的每个节点可以持有0到无穷个Appender(将日志同时向多个IO文件分发输出)。
3. 每个Appender自身也可以持有0到1个Layout。
4. 如果某个配置节点持有了一些Appender(只要Appender数量非0即可)，则使用自己持有的Appender；否则，递归查找上级节点直到某个上级节点持有自己的Appender为止；如果连根节点都没持有Appender，则使用默认的Appender。
5. 对于每个Appender而言，输出日志时先参考自己的Layout; 如果自己没有Layout，则参考自己所在的配置节点的Layout; 如果配配置节点仍然没有持有Layout，则递归查找上级节点直到某个上级节点持有自己的Layout为止；如果最终连根节点都没持有任何Layout，则使用系统默认的Layout。

应用程序的日志打印操作一定先映射成一个配置树节点，根据此配置节点决定如何输出日志，所以配置树的设置比较重要，但对于如何使用代码为配置树任意的Configuration对象指定Appender和Layout以及如何为Appender指定Layout，配套demo中有代码演示，不再赘述。

## 日志使用 ##

请使用declare_logger宏定义日志对象，如下

    class MyBusinessProcessor : extends Object {
        delcare_logger(MyBusinessProcessor)
        
    public: //declare_logger宏会把语义上下文切换为private:
        ...TODO...
    };
    
delcare_logger定义个一个私有静态方法logger()用于返回和当前类相关的日志对象。当前所在类的代码可以基于其debug, info, warn, error, fatal方法打印不同严重程度的日志，比如

    logger().debug("Hello world");

可以使用slf4j风格的变参，将参数嵌入日志消息中

    logger().info("小朋友已经学会了{} + {} = {}", 5, 7, 12);
    
也可以将异常的stack trace打印进入日志文件，为了让消息参数和异常的并存，API风格和slf4j略有差异，异常参数改到了消息参数之前。如下

    前文略
    catch_ (Exception, ex) {
        logger().warn(
            ex, //和slf4j不同，异常参数在消息参数之前，而非之后 
            "很不幸，小朋友还没有学会{} + {} = {}", 
            5, 
            7, 
            12);
    }
    后续略
    
最后，考虑到较长的文本日志和代码要勤于换行之间的矛盾，吸取了kotlin的trimMargin功能，例如
    
    logger().info(
        "长长长长长长长长长长长长长长长长长      \
        |长长长长长长长长长长长长长长长长长    \
        |长长长长长长长长长长长长长长长长长       \
        |长长长长长长长长长长长长长长长长长    \
        |长长长长长长长长长长长长长长长长长"
    );

这段消息比较长，用了C/C++特有的转义符完成了多行字符串的书写，为了保持代码美观，换行后的字符串也进行了缩进而非顶格书写，但是这样引入了大量的空白字符。

为了解决此问题，字符串从第二行开始均为'|'开头，这套日志API保证，从此'|'开始往前的所有空白（包括'|'本身）都会被自动忽略，因此最终打印的日志没有空白(如果真需要个别空白，请写在'|'之后)。


----------

[<上一篇：多线程](./threading.md) | [首页](../) | [下一篇：HTTP>](./http.md)
