# 异常 #

## 异常链 ##

在实际开发中，上层模块往往会对下层模块的抛出的异常进行包装和转换。以java语言为例，JDBC层面数据库相关的异常必然是SQLException，但是几乎上层框架（如Hibernate、myBatis、kotlin标配的Exposed等）的开发者都会把底层的SQLException异常进行“捕获->转换->再抛”，从这些框架中得到异常早非SQLException, 而是HibernateExeption这类东西了。同理，当开发者再结合springframework使用这些上层框架时，springframework还会对异常再转换一次，最终开发者得到从DataAccessException派生的各种springframework定义的异常。

对于支持结构化异常处理的语言而言，针对异常的“捕获->转换->再抛”是非常普遍的一种行为。原因是多方面的：或许因开发者在利弊平衡中并不认可Java的受查异常而信奉非受查异常；或许因开发者觉得上游依赖太多或者某个上游依赖的异常类型设计过于繁杂而想对暴露出去的异常种类进行一次瘦身；或许因开发者认为某个上游依赖的异常类型设计过于单一但异常参数信息过于复杂而要对异常种类进行一次分类扩充并降低每种异常的参数复杂度；或许开发者觉得遵循同一个技术规范约定而编写插件的不同提供商提供的异常设计千奇百怪互不兼容而想对异常进行一次统一；等等，抑或是多种原因的综合。总之，对异常的“捕获->转换->再抛”正常得如同家常便饭一般。

以Java为代表的大部分现代语言，其异常最大的魅力之处在于“捕获->转换->再抛”过程中，新异常并没有完全取代旧异常，而是包含了旧异常的引用，并且可以使用java.lang.Throwable.getCause()方法从新异常取出旧异常，通过这种方式，一次异常事故中，各层次软件模块所抛出的异常就如形成了一个单向链表，这个链表就是异常链，异常链打印在日志中虽然有点庞大有点耗费日志空间，但可以让调查事故的人顺藤(caused by )摸瓜地看个每个模块出了什么问题，相对而言降低了调查成本。

为了让不同的异常对象能传递成一条理论上长度无限的单项链表，异常就必须是引用类型的，而非值类型的。所以java-cpp提供了com_lanjing_cpp_common::Exception而不使用std::exception。

定义一个新异常的方法如下：

    using namespace com_lanjing_cpp_common;
    class FooException : extends Exception {
    public:
        FooException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr)
            : Exception(exception_arg_prefix, message, cause) {}
    };
    
例子中的宏exception_param_prefix和exception_arg_prefix分别从形参和实参的角度代表着抛出异常的源码的文件名和行号，不用细究，纪律化地遵循即可。后续参数自己随意设计，但作为最普通的、不需要额外参数的异常，至少应该基于一个message加一个可选的cause构建。

## 特有语法 ##

既要实现引用类型的异常及其内存自动管理机制，又要保住结构化异常处理最珍贵的品质（如果先捕获基异常再捕获子异常会导致编译报错）。绞尽脑汁后发现，原始的throw, try, catch关键字绝对无法满足要求，所以创建了一套新的宏语法，来取代这些关键字。

1. 抛出新异常
   抛出新异常分为两种情况
   - 如果异常的构造是public的，请使用宏thorw_new
   - 如果异常的构造不是public的，请使用宏throw_new_internal
   
   例子如下

        throw_new(MyException, arg1, arg2, ..., argN);
        throw_new_internal(MyException, arg1, arg2, ..., argN);
        
    此处的arg1...argN就是异常类中exception_param_prefix之后你自定义的参数，并不包含exception_param_prefix所代表的源码文件名和行号，因为它们是由这两个宏自动给予的。
   
2. 捕获异常
    请使用宏try_, catch_和end_try，用法如下

        try_
            你的业务代码
        catch_ (AException, ex)
            你的错误处理代码
        catch_ (BException, ex)
            你的错误处理代码
        end_try
        
    或许，对某些开发者而言，C系列语言代码块没有花括弧会觉得抓狂（本人就是这种开发者），可以添加冗余的花括号，如下：
    
        try_ {
            你的业务代码
        } catch_ (AException, ex) {
            你的错误处理代码
        } catch_ (BException, ex) {
            你的错误处理代码
        } end_try // 注意：使用带花括符这种冗余写法时，忘记end_try并不会带来视觉上的违和感，切记别忘了。
        
3. 重新抛出旧异常

    有时候需要先捕获异常，做一些处理后，然后再原封不动地重新抛出去，请使用throw_宏，用法如下
    
        前文略
        catch_ (MyException, ex) {
            某些重要的处理逻辑，比如，数据库回滚
            throw_(ex); //必要处理结束后，将旧异常原封不动地重新抛出去
        }
        后续略

## 应对C++不支持finally ##

令人费解的是，除特定厂商推出的C++变种外，标准C++并不支持finally。为了弥补这个缺陷，java-cpp引入swift风格的defer宏，此宏接受一个lambda参数并保证在当前作用域结束时自动调用该lambda。例如：

    {
        setup();
        defer([] { teardown(); });
        test();
    }
    
defer保证在最后结尾处右花括号处一定会执行teardown()，无论test是否异常。

唯一的限制是，如果要在同一作用域中多次使用defer宏，就必须写到不同的行中去，否则编译报错。


----------

[<前一篇：内存管理](./memory.md) | [首页](..) | [下一篇：功能性接口>](./functional.md)

