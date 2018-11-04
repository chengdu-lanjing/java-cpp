# 数组 #

## 创建 ##

java-cpp中，数组对应的类型为com_lanjing_cpp_common::Array&lt;E&gt;，其中E为元素类型。Array&lt;E&gt;本身继承自com_lanjing_cpp_common::Object类，遵循java-cpp的内存管理。

设计时要求Array类本身及其基类Object的数据同数组元素的数据共享一块连续内存内存，Array类的布局并不固定的，而是变长类。所以，Array不支持构造函数，而使用静态函数newInstance创建实例。如下（假设Direct3DVertex是用户预先定义的一种结构体）

    Ref<Array<Direct3DVertex>> vertexBuffer = Array<Direct3DVertex>::newInstance(100, ArrayElementType::C);
    
Ref&lt;Array&lt;Direct3DVertex&gt;&gt;是一种比较冗长的表达，因此java-cpp针对数组的引用定义了别名Arr&lt;E&gt; = Ref&lt;Array&lt;E&gt;&gt;，因此上述代码可简写为

    Arr<Direct3DVertex> vertexBuffer = Array<Direct3DVertex>::newInstance(100, ArrayElementType::C);

Array::newInstance的第一个参数为要创建数组的长度，不同于C/C++原始数组长度不能0，此长度非负即可。在Array&lt;E&gt;未使用其某些泛型特化版本时刻，具备第二个参数且其类型为枚举ArrayElementType，取值如下

1. CPP: 元素类型具备构建和析构。数组被创建时从前到后逐元素调用默认构造；数组被销毁时从后到前逐元素调用析构；数组被复制时逐元素调用拷贝构造。
2. C: 元素类型不具备构建和析构。数组被创建和销毁时无需逐元素调用构造和析构；数组被复制时采用memcpy以便于特定平台优化。
3. C_DEFAULT_AS_ZERO: 和C类似，只是数组创建时要将数据元素存储区域清0，此过程使用memeset以便于特定平台优化。

但是，很多时候，数组元素的类型自身即隐含了ArrayElementType的选择，所以Array&lt;E&gt;拥有两个模板特化

1. 当元素为bool, char(byte), short, int32_t, int64_t, float, double时，静态函数newInstance的第二个参数不再为ArrayElementType枚举，而是一个bool参数以表示是否在数组创建时将各元素清零。为了保持和Java的一致，该参数默认为true，仅当开发者需要明确地优化时，才需要手动将其设置为false
2. 当元素为对象引用Ref&lt;T&gt;时，静态函数newInstance根本就没有第二个参数，因为作为引用的元素必然拥有构造和析构。

即，如下：

    Arr<int> intArr = Array<int>::newInstance(100); // 默认会将所有元素设置为0, 和Java保持行为一致
    Arr<float> floatArr = Array<float>::newInstance(100, false); //开发人员明确表达不需要将所有元素预设为0，由他/她自己明确表示后续一定赋值
    Arr<Ref<Object>> objArr = Array<Ref<Object>>::newInstance(100); //对象数组，元素必然存在构造和析构，没得选择

由于在Java风格的开发中，数据元素为对象引是时一件非常常见的事，因此java-cpp定义别名RefArray&lt;E&gt; = Array&lt;Ref&lt;E&gt;&gt;以及RefArr&lt;E&gt; = Arr&lt;Ref&lt;E&gt;&gt;，因此，上面的最后一句代码可以简写为

    RefArr<Object> objArr = RefArray<Object>::newInstance(100);

## 下标 ##

引用Ref&lt;T&gt;在当T为数组类型时会启动一个模板偏特化版本，这种引用定义了operator[]以支持数组的下标语法，例如

    static Arr<int> createFibonacciArray(int size) {
        if (size < 2) {
            throw_new(IllegalArgumentException, "size must >= 2");
        }
        Arr<int> arr = Array<int>::newInstance(size);
        arr[0] = arr[1] = 1;
        for (int i = 2; i < arr.length(); i++) {
            arr[i] = arr[i - 2] + arr[i - 1];
        }
        return arr;
    }
    
注意：如果在编译环境中定义了DEBUG宏，下标操作会如同Java一样进行越界判断(越界异常为IllegalArgumentException，而非Java的ArrayIndexOutBoundException，也未定义该异常)；否则，下标操作会如同C/C++一样不做越界判断以提高性能。

## foreach循环 ##

引用Ref&lt;T&gt;在当T为数组类型时会启动一个模板偏特化版本，这种引用定义了begin和end函数以支持foreach循环语法，例如(引用上文定义的函数createFibonacciArray)

    Arr<int> arr = createFibonacciArray(1000);
    for (int &value : arr) {
        value = value * value;
    }

这样，数组中每个元素的值都被替换成了旧值的平方。

## 复制 ##

java-cpp支持数组全量复制和切片复制，先假设有一个旧数组arr(此处使用上文定义的函数createFibonacciArray)

    Arr<int> arr = createFibonacciArray(2000);

全量复制一个数组的做法如下

    Arr<int> clonedArr = arr->clone();
    
复制索引介于100到199之间的切片（即使第101个元素到第200个元素）的做法如下

    Arr<int> clonedArr = arr->clone(100, 200); //前闭后开
    
被复制的区域index >= 100 and index < 200，两个参数所表示的区间前闭后开，所以，第二个参数是200而非199。
    
另外，clone的第二个参数有一个特殊的例外值-1，等价于数组的长度，故

    Arr<int> clonedArr = arr->clone(1000, -1); //-1等价于arr.length()
    
表示复制从索引为1000的元素（第1001个元素）到末尾的所有元素。

## 兼容传统C/C++程序 ##

必须兼容经典的C/C++程序，数组对象提供了unsafe()方法返回这个兼容传统C/C++程序的指针，如下：

    Arr<int> arr = createFibonacciArray(1000);
    int *classicArr = arr->unsafe(); //classicArr为对传统C/C++程序可用的数组，很遗憾，数组长度信息丢失，需另外传递

----------
[<上一篇：功能性接口](./functional.md) | [首页](https://github.com/chengdu-lanjing/java-cpp)  |[下一篇：多线程>](./threading.md)
