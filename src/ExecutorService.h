/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

#include "Functional.h"
#include "BlockingQueue.h"
#include "Auxiliary.h"

namespace com_lanjing_cpp_common {

    class ExecutorService : extends Object {
    public:
        ExecutorService(int threadCount = 1, bool giveupPendingTasksAfterShutdown = true) {
            if (threadCount < 1) {
                throw_new(IllegalArgumentException, "threadCount cannot be less than 1");
            }
            this->sharedService = new_<SharedService>(threadCount, giveupPendingTasksAfterShutdown);
        }
        virtual ~ExecutorService() {
            this->sharedService->shutdown();
        }
        bool isShutdown() const {
            return this->sharedService->isShutdown();
        }
        void shutdown() {
            this->sharedService->shutdown();
        }
        void execute(Ref<Runnable> runnable) {
            this->sharedService->execute(runnable);
        }
#ifdef DEBUG
        static int threadCount();
#endif //DEBUG
    private:
        /*
         * SharedService被调用者线程和线程池线程共享，用户线程的release操作并不能导致析构被执行!
         *
         * 但是,外部的ExecutorService包装类却不同, 它被不被线程池线程所共享，用户线程的release操作很容易引起析构被执行,
         * 从而间接导致SharedService的shutdown被执行。这种内外两层设计非常重要！
         */
        class SharedService : extends Object {
        public:
            SharedService(int threadCount, bool giveupPendingTasksAfterShutdown) {
                this->giveupPendingTasksAfterShutdown = giveupPendingTasksAfterShutdown;
                this->runnableQueue = new_<LinkedBlockingQueue<Runnable>>();
                this->semaphore = new_<Semaphore>();
                this->threads = Array<pthread_t>::newInstance(
                        threadCount,
                        ArrayElementType::C
                );
                for (int i = 0; i < threadCount; i++) {
                    this->retain();
                    int error = pthread_create(
                            &this->threads[i],
                            nullptr,
                            threadProc,
                            this
                    );
                    if (error != 0) {
                        this->release();
                        this->shutdown(i);
                        ostringstream oss;
                        oss << "Cannot create thread-" << i;
                        throw_new(OSException, error, oss.str().c_str());
                    }
                }
            }

            virtual ~SharedService() {}

            void execute(Ref<Runnable> runnable) {
                if (!this->closed) {
                    this->runnableQueue->put(runnable);
                }
            }

            bool isShutdown() const {
                return this->closed;
            }

            void shutdown() {
                this->shutdown(this->threads.length());
            }

#ifdef DEBUG
            static AtomicInteger &threadCount() {
                static AtomicInteger instance;
                return instance;
            }
#endif //DEBUG

        private:
            void shutdown(int count) {
                if (this->closed.compareAndSet(false, true)) {
                    for (int i = count; i > 0; --i) {
                        this->runnableQueue->put(nilRunnable());
                    }
                    this->semaphore->acquire(count);
                }
            }

            static void *threadProc(void *data) {
#ifdef DEBUG
                ++threadCount();
#endif //DEBUG
                SharedService *service =
                        reinterpret_cast<SharedService*>(data);
                defer([=]() {
                    service->semaphore->signal();
                    service->release();
#ifdef DEBUG
                    --threadCount();
#endif //DEBUG
                });
                service->threadRun();
                return nullptr;
            }

            void threadRun() {
                while (!this->closed || !this->giveupPendingTasksAfterShutdown) {
                    try_ {
                        Ref<Runnable> runnable = this->runnableQueue->take();
                        if (runnable == nilRunnable()) {
                            return;
                        }
                        runnable();
                    } catch_(Exception, ex) {
                        ex->printStackTrace();
                    } end_try
                }
            }

        private:
            bool giveupPendingTasksAfterShutdown;
            AtomicBoolean closed;
            Ref<Semaphore> semaphore;
            Arr<pthread_t> threads;
            Ref<BlockingQueue<Runnable>> runnableQueue;
        };
    private:
        Ref<SharedService> sharedService;

        // 在iOS系统中,pthread_cancel只能保证pthread_cleanup的执行, 无法保证线程栈上C++对象的析构的执行
        // 故,不使用pthread_cancel退出线程,而使用此特殊任务保证执行线程退出
        static Ref<Runnable> &nilRunnable() {
            static Ref<Runnable> instance = Runnable::of([=]{});
            return instance;
        }
    };

#ifdef DEBUG
    inline int ExecutorService::threadCount() {
        return SharedService::threadCount();
    }
#endif //DEBUG

    interface ScheduledFuture : implements Interface {
        virtual void cancel() = 0;
    };

    class ScheduledExecutorService : extends ExecutorService {
    public:
        ScheduledExecutorService(int threadCount = 1, bool giveupPendingTasksAfterShutdown = true) :
            ExecutorService(threadCount, giveupPendingTasksAfterShutdown) {}
        virtual ~ScheduledExecutorService() {}

    public:

        Ref<ScheduledFuture> schedule(Ref<Runnable> runnable, time_t delayMillis) {
            Ref<SimpleRunnableWrapper> wrapper = new_<SimpleRunnableWrapper>(runnable);
            scheduledController().addTask(
                    new_<Task>(this, wrapper, System::currentTimeMillis() + delayMillis)
            );
            return wrapper;
        }

        Ref<ScheduledFuture> scheduleAtFixedRate(Ref<Runnable> runnable, time_t initialDelayMillis, time_t peroidMillis) {
            Ref<FixedRateRunnableWrapper> wrapper =
                    new_<FixedRateRunnableWrapper>(this, runnable, peroidMillis);
            scheduledController().addTask(
                    new_<Task>(this, wrapper, System::currentTimeMillis() + initialDelayMillis)
            );
            return wrapper;
        }

        Ref<ScheduledFuture> scheduleWithFixedDelay(Ref<Runnable> runnable, time_t initialDelayMillis, time_t delayMillis) {
            Ref<FixedDelayRunnableWrapper> wrapper =
                    new_<FixedDelayRunnableWrapper>(this, runnable, delayMillis);
            scheduledController().addTask(
                    new_<Task>(this, wrapper, System::currentTimeMillis() + initialDelayMillis)
            );
            return wrapper;
        }

    private:
        class Task : extends Object {
        public:
            Task(
                    Ref<ScheduledExecutorService> owner,
                    Ref<Runnable> runnable,
                    time_t time) :
                        owner(owner),
                        runnable(runnable),
                        time(time) {}
            WeakRef<ScheduledExecutorService> owner;
            Ref<Runnable> runnable;
            time_t time;
        };
        class SimpleRunnableWrapper : extends Object, implements Runnable, implements ScheduledFuture {
        public:
            SimpleRunnableWrapper(Ref<Runnable> target) : target(target) {}
            virtual ~SimpleRunnableWrapper() {}
            virtual void run() override {
                if (!this->cancelled) {
                    this->target();
                }
            }
            virtual void cancel() override {
                this->cancelled.compareAndSet(false, true);
            }
        private:
            Ref<Runnable> target;
            AtomicBoolean cancelled;
            interface_refcount()
        };
        class AbstractFixedValueRunnableWrapper : extends Object, implements Runnable, implements ScheduledFuture {
        public:
            AbstractFixedValueRunnableWrapper(
                    Ref<ScheduledExecutorService> owner,
                    Ref<Runnable> target,
                    time_t fixedValue) :
                        owner(owner),
                        target(target),
                        fixedValue(fixedValue) {}
            virtual ~AbstractFixedValueRunnableWrapper() {}
            virtual void cancel() override {
                this->cancelled.compareAndSet(false, true);
            }
        protected:
            WeakRef<ScheduledExecutorService> owner;
            Ref<Runnable> target;
            time_t fixedValue;
            AtomicBoolean cancelled;
            interface_refcount()
        };
        class FixedRateRunnableWrapper : extends AbstractFixedValueRunnableWrapper {
        public:
            FixedRateRunnableWrapper(Ref<ScheduledExecutorService> owner, Ref<Runnable> target, time_t fixedRate) :
                AbstractFixedValueRunnableWrapper(owner, target, fixedRate) {}
            virtual ~FixedRateRunnableWrapper() {}
            virtual void run() override {
                if (!this->cancelled) {
                    Ref<ScheduledExecutorService> owner = this->owner.get();
                    if (owner) {
                        owner->schedule(this, this->fixedValue);
                    }
                    this->target();
                }
            }
        };
        class FixedDelayRunnableWrapper : extends AbstractFixedValueRunnableWrapper {
        public:
            FixedDelayRunnableWrapper(
                    Ref<ScheduledExecutorService> owner,
                    Ref<Runnable> target,
                    time_t fixedRate) :
                AbstractFixedValueRunnableWrapper(owner, target, fixedRate) {}
            virtual ~FixedDelayRunnableWrapper() {}
            virtual void run() override {
                if (!this->cancelled) {
                    defer([=]{
                        Ref<ScheduledExecutorService> owner = this->owner.get();
                        if (owner != nullptr) {
                            owner->schedule(this, this->fixedValue);
                        }
                    });
                    this->target();
                }
            }
        };
        struct ScheduledController {
        public:
            ScheduledController() {
                this->condition = new_<Condition>(this->mutex);
                this->nilTask = new_<Task>(nullptr, nullptr, 0);
                pthread_create(
                        &this->thread,
                        nullptr,
                        threadProc,
                        this
                );
            }
            ~ScheduledController() {
                if (this->closed.compareAndSet(false, true)) {
                    this->addTask(this->nilTask);
                    pthread_join(this->thread, nullptr);
                }
            }
            void addTask(Ref<Task> task) {
                Mutex::Scope scope(this->mutex);
                auto iterator = this->taskMap.find(task->time);
                if (iterator == this->taskMap.end()) {
                    list<Ref<Task>> list;
                    list.push_back(task);
                    this->taskMap[task->time] = list;
                } else {
                    iterator->second.push_back(task);
                }
                this->condition->notify();
            }
        private:
            static void *threadProc(void *data) {
                static_cast<ScheduledController*>(data)->threadProc();
                return nullptr;
            }
            void threadProc() {
                while (!this->closed) {
                    Ref<Task> task = this->fetchTask();
                    if (task == nullptr) {
                        break;
                    }
                    Ref<ScheduledExecutorService> owner = task->owner.get();
                    if (owner) {
                        owner->execute(task->runnable);
                    }
                }
            }
            Ref<Task> fetchTask() {
                while (!this->closed) {
                    Mutex::Scope scope(this->mutex);
                    while (this->taskMap.empty()) {
                        this->condition->wait();
                    }
                    list<Ref<Task>> &taskList = this->taskMap.begin()->second;
                    Ref<Task> task = taskList.front();
                    if (task == this->nilTask) {
                        return nullptr;
                    }
                    time_t sleepMillis = task->time - System::currentTimeMillis();
                    if (sleepMillis > 0) {
                        this->condition->wait(sleepMillis);
                    } else {
                        taskList.pop_front();
                        if (taskList.empty()) {
                            this->taskMap.erase(this->taskMap.begin());
                        }
                        return task;
                    }
                }
                return nullptr;
            }
        private:
            AtomicBoolean closed;
            pthread_t thread;
            map<time_t, list<Ref<Task>>> taskMap;
            Mutex mutex;
            Ref<Condition> condition;
            Ref<Task> nilTask;
        };
        static ScheduledController &scheduledController() {
            static ScheduledController instance;
            return instance;
        }
    };
}
