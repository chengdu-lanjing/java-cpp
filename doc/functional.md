# 功能性接口 #

在绝大部分编程语言中，将一段代码表述为变量是一个很重要的功能，对于程序库的提供者而言，往往需要用在库本身已知的固有的代码中嵌入用户提供的未知代码片段（如事件回调）。实现这种目的的方法有很多，C/C++的函数指针、.NET的委托、大多数编程语言支持的lambda表达式。

java-cpp采用和Java一样的约定，视仅有一个方法的接口为Functional interface. 但和Java既允许也需要开发人员定义自己的Functional interface不同，由于C++支持强大的泛型变参，java-cpp不允许也不需要让用户定义自己的Functional interface，故在头文件Functional.h中，定义了4个内置的Functional interface以满足用户任意的需求。

1. 无输入且无输出，请使用接口com_lanjing_cpp_common::Runnable。
2. 有输入但无输出，请使用接口com_lanjing_cpp_common::Consumer&lt;ArgType1, ArgType2, ..., ArgTypeN&gt;，其中N >= 1。特别值得一提的是，如果只有一个泛型参数且这个参数为引用，可以使用别名RefConsumer&lt;T&gt; = Consumer&lt;Ref&lt;T&gt;&gt;。
3. 无输入但有输出，请使用接口com_lanjing_cpp_common::Supplier&lt;ReturnType&gt;。
4. 既有输入也有输出，请使用接口com_lanjing_cpp_common::Function&lt;ReturnType(ArgType1, ArgType2, ... ArgTypeN)&gt;，其中N >= 1。

这4个接口各自唯一的方法的名称分别为run、consume、get和apply，这点和Java一样。但实际上开发人员无需在意它们，Ref&lt;T&gt;在泛型参数为者这四个功能性接口之一时，提供了模板特化，其中支持了operator()。即，调用接口的方法的代码可以直接写作“引用变量(arg1, arg2, ..., argN)”而不必写作"引用变量->方法名(arg1, arg2, ..., argN)"。

## 构建功能接口方式 ##

### 基于全局函数或静态函数 ###

    static int sum(int a, int b) {
        return a + b;
    }
    Ref<Function<int(int, int)>> function = Function<int(int, int)>::of(sum);
    
### 基于lambda表达式构建 ###

    Ref<Function<int(int, int)>> function = Function<int(int, int)>::of([](int a, int b) {
        return a + b;
    });
    
### 基于对象的非静态成员函数构建 ###

先写一个类，注意其成员sum函数并不是静态的，这是一个对象状态相关的实例成员函数。

    class Handler: extends Object {
    public:
        Handler(const string &name) : name(name) {}
        void sum(int a, int b) {
            cout << "Invoke method 'sum' of the Handler '" << this->name << "'" << endl;
            return a + b;
        }
    private:
        string name;
    };
    
如果要被创建的功能性接口实例持有响应对象的强引用，请如下：

    Ref<Handler> handler = new_<Handler>("Default handler");
    Ref<Functional<int(int, int)>> function = Functional<int(int, int)>::of(handler, &Handler::sum);

如果要被创建的功能性接口实例持有响应对象的弱引用，请如下：

    Ref<Handler> handler = new_<Handler>("Default handler");
    Ref<Functional<int(int, int)>> function = Functional<int(int, int)>::weakof(handler, &Handler::sum);

## 类似.NET委托的功能合并和拆分能力 ##

Ref&lt;T&gt;在泛型参数为者这四个功能性接口之一时，提供了模板特化，支持+、-、+=和-=行为。

多个同类型的功能性接口实例允许通过“+”运算符返回一个新的合并接口

    Ref<Consumer<string>> consumer1 = ...;
    Ref<Consumer<string>> consumer2 = ...;
    Ref<Consumer<string>> consumer = consumer1 + consumer2;
    
调用合并后接口实例的方法等价于循环地调用所有原始接口的相应方法。

这种操作行为合并的能力建议用在没有返回值的Runnable和Consumer&lt;ArgType1, ArgType2, ..., ArgTypeN&gt;上； 但如果非要用在Suppiler&lt;ReturnType&gt;和Consumer&lt;ReturnType(ArgType1, ArgType2, ..., ArgTypeN)&gt;这种有返回值的接口上，合并出的新接口实例的相关方法的返回值为参与合并的最后一个原始接口中对应方法实例的返回值。

类似地，使用“-”，可以从一个合并而成的接口实例中剔除某些原始的接口实例部分，剔除后的剩余部分也用新接口实例的方式返回。如果所有原始接口都被剔除干净了，相减结果为nullptr。

注意：功能性接口实例一定是只读的，无论“+”所代表的功能合并，还是“-”所代表的功能拆分，都不会更改原始的接口实例，而是返回全新的实例。

这种功能的合并和拆分能力在事件开发时极其有用，更多详情请参见相关demo。

----------

[<上一篇：异常](./exception.md) | [首页](./README.md) | [下一篇: 数组>](./array.md)
