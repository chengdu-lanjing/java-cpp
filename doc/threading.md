# 多线程 #

多线程部分挑了一小部分JDK来实现，由于JDK API本身用法不用多说。此处仅给出简要描述，更多的内容请参考附带的demo。

## 阻塞队列 #

BlockingQueue.h提供用于生产者-消费者模型的阻塞队列，提供如下三个重要类型：
1. com_lanjing_cpp_common::BlockingQueue&lt;E&gt;接口: 对应java.util.concurrent.BlockingQueue&lt;E&gt;
2. com_lanjing_cpp_common::ArrayBlockingQueue&lt;E&gt;类: 对应java.util.concurrent.ArrayBlockingQueue&lt;E&gt;
3. com_lanjing_cpp_common::LinkedBlockingQueue&lt;E&gt;类: 对应java.util.concurrent.LinkedBlockingQueue&lt;E&gt;

## 线程池 ##

ExecutorService.h提供了com_lanjing_cpp_common::ExecutorService类，充当java.util.concurrent.ExecutorService接口的一个简化实现。

## 调度器 ##

ExecutorService.h提供了com_lanjing_cpp_common::ScheduledExecutorService类，充当java.util.concurrent.ScheduledExecutorService接口的一个简化实现。


----------
[<上一篇：数组](./array.md) | [首页](../README.md) | [下一篇：日志>](./logging.md)
