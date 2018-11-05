/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

/**
 * 重要基础设施（需要C++11）
 *
 * 1、引用计数内存管理，包括对象和数组（下文有解释为什么不用std::shared_ptr）
 * 2、智能指针（下文有解释为什么不用std::shared_ptr）
 * 2、弱引用智能指针（既然已经不用std::shared_ptr，自然无法用std::weak_ptr）
 * 3、内存泄漏诊断（变模环境需要DEBUG宏）
 * 4、通用资源释放器
 * 5、具备文件名、代码行号、异常嵌套链及StackTrace的异常（这也是绝不使用std::exception的原因）
 *
 * @author 陈涛
 * 2016-11-15
 */

#include <assert.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <cxxabi.h>
#include <unistd.h>
#include <sys/time.h>
#include <atomic>
#include <map>
#include <list>

#ifdef __APPLE__
#define __noreturn _Noreturn
#else
#include <asm-generic/errno-base.h>
#ifndef __noreturn
#define __noreturn [[noreturn]]
#endif // !__noreturn
#endif // __APPLE__

/*
 * 注意
 * 必须使用如下的宏定义:
 *     new_, new_internal, throw_, throw_new, throw_new_internal, try_, catch_和end_try
 * 不得使用关键字:
 *     new, throw, try, catch
 */
#define new_ com_lanjing_cpp_common::newObject

#define new_internal(Class, ...) \
    com_lanjing_cpp_common::newInternalObject<Class>( \
            [=](void *__tmpNewInternalTarget) { \
                new(__tmpNewInternalTarget) Class(__VA_ARGS__); \
            } \
    )

#define throw_(ex) \
    { \
        auto __tmpExceptionToBeThrown = com_lanjing_cpp_common::Exception::unwrap(ex); \
        __tmpExceptionToBeThrown->retain(); \
        throw __tmpExceptionToBeThrown; \
    }

#define throw_new(Exception, ...) \
    com_lanjing_cpp_common::throwNewException<Exception>(__FILE__, __LINE__, __VA_ARGS__);

#define throw_new_internal(Exception, ...) \
    com_lanjing_cpp_common::throwNewInternalException<Exception>(\
            [=](void *__tmpNewInternalTarget) { \
                new(__tmpNewInternalTarget) Exception(__FILE__, __LINE__, __VA_ARGS__); \
            } \
    );

#define try_    \
    try {

#define catch_(Exception, ex) \
    } catch (Exception *ex) { \
        com_lanjing_cpp_common::Finalizer __catchedExceptionFinalizer([=]() { ex->release(); });

#define end_try \
    }


#define __concatenate(a, b)    a ## b
#define __declare_finalizer(name, suffix) \
    com_lanjing_cpp_common::Finalizer __concatenate(name, suffix)
#define defer \
    __declare_finalizer(__deferFinalizer, __LINE__)

#define exception_param_prefix \
    const char *fileName, int lineNumber
#define exception_arg_prefix \
    fileName, lineNumber


#define interface struct
#define interface_refcount() \
    public: \
        virtual void retain() override { com_lanjing_cpp_common::Object::retain(); } \
        virtual void release() override { com_lanjing_cpp_common::Object::release(); } \
    private:

#define extends public
#define implements virtual public
#define abstract

#define no_constructor_element_type_array(elementType) \
    template <> \
    class Array<elementType> : extends com_lanjing_cpp_common::_Array<elementType, Array<elementType>> { \
    public: \
        static com_lanjing_cpp_common::Ref<Array<elementType>> newInstance(int size, bool initializeAsZero = true) { \
            return com_lanjing_cpp_common::_Array<elementType, Array<elementType>>::newInstance( \
                    size, \
                    initializeAsZero ? \
                            com_lanjing_cpp_common::ArrayElementType::C_DEFAULT_AS_ZERO : \
                            com_lanjing_cpp_common::ArrayElementType::C \
            ); \
        } \
        static com_lanjing_cpp_common::Ref<Array<elementType>> of(const initializer_list<elementType> &list) { \
            return com_lanjing_cpp_common::_Array<elementType, Array<elementType>>::newInstance( \
                    list.size(), \
                    com_lanjing_cpp_common::ArrayElementType::C, \
                    list.begin() \
            ); \
        } \
    };

#define ref_implementation(T) \
    Ref() {} \
    Ref(T *p) : _Ref<T>(p) {} \
    Ref(const Ref<T> &right) : _Ref<T>(right) {} \
    Ref(const Ref<T> &&tmpRight) : _Ref<T>(tmpRight) {} \
    template <typename D> Ref(const Ref<D> &right) : _Ref<T>(right) {} \
    template <typename D> Ref(const Ref<D>&& tmpRight) : _Ref<T>(tmpRight) {} \
    Ref<T> &operator = (T *p) { \
        this->assign(p); \
        return *this; \
    } \
    Ref<T> &operator = (const Ref<T> &right) { \
        this->assign(right); \
        return *this; \
    } \
    Ref<T> &operator = (Ref<T> &&tmpRight) { \
        this->assign(tmpRight); \
        return *this; \
    } \
    template <typename D> Ref<T> &operator = (const Ref<D> &right) { \
        this->assign(right); \
        return *this; \
    } \
    template <typename D> Ref<T> &operator = (Ref<D> &&tmpRight) { \
        this->assign(tmpRight); \
        return *this; \
    } \
    template <typename X> \
    Ref<X> staticCast() const { \
        Ref<X> tmp = static_cast<X*>(this->p); \
        return tmp; \
    } \
    template <typename X> \
    Ref<X> dynamicCast() const { \
        Ref<X> tmp = dynamic_cast<X*>(this->p); \
        return tmp; \
    }

/**
 * 和C#相同: class关键字声明的为引用类型, struct关键字(interface宏除外)声明的为值类型
 *
 * 引用类型总是使用new在堆上分配, 并表现为一重指针(无论原始指针还是智能指针)
 *     目的: 对复杂数据结构避免拷贝,利于共享.
 *
 * 值类型绝不使用new, 而是直接声明成局部, 成员, 静态或全局变量,表现为原始类型自身或C++引用类型
 *     目的: 享受大量的C++语法糖.
 *
 *  [
 *  注:
 *
 *  1. 值类型不允许使用new, 然而有一种情况仍然会让值类型对象在堆上分配:
 *  当它作为另外一个引用类型的字段时, 就可以寄生在宿主对象内存区域的的一部分而存在于堆中.
 *  但是, 值类型一定不可能单独地存在于堆中.
 *
 *  2. 值类型应该尽量避免C++继承, 对于非指针类型而言,频繁的拷贝特别容易导致C++对象父类切片问题而出现bug;
 *  与此相反, 引用类型鼓励继承和多态.
 *  ]
 */
namespace com_lanjing_cpp_common {

    using namespace std;

    template <typename T> struct Ref;
    template <typename T, typename ...Args> Ref<T> newObject(Args &&...args);
    template <typename T> Ref<T> newInternalObject(function<void(void*)> constructor);
    template <typename E, typename ...Args> __noreturn void throwNewException(Args &&...);
    template <typename E> __noreturn void throwNewInternalException(function<void(void*)> constructor);

    struct LinuxErrors {
        static void handle(int err, const char *message = nullptr);
    };

    struct AtomicBoolean : public atomic<bool> {
    public:
        AtomicBoolean(bool initialValue = false) : atomic<bool>(initialValue) {}
        bool compareAndSet(bool expectedValue, bool newValue) {
            return this->compare_exchange_strong(expectedValue, newValue);
        }
        bool compareAndExchange(bool &expectedValue, bool newValue) {
            return this->compare_exchange_strong(expectedValue, newValue);
        }
    };

    struct AtomicInteger : public atomic<int> {
    public:
        AtomicInteger(int initialValue = 0) : atomic<int>(initialValue) {}
        bool compareAndSet(int expectedValue, int newValue) {
            return this->compare_exchange_strong(expectedValue, newValue);
        }
        bool compareAndExchange(int &expectedValue, int newValue) {
            return this->compare_exchange_strong(expectedValue, newValue);
        }
    };

    struct AtomicInteger64 : public atomic<int64_t> {
    public:
        AtomicInteger64(int64_t initialValue = 0) : atomic<int64_t>(initialValue) {}
        bool compareAndSet(int64_t expectedValue, int64_t newValue) {
            return this->compare_exchange_strong(expectedValue, newValue);
        }
        bool compareAndExchange(int64_t &expectedValue, int64_t newValue) {
            return this->compare_exchange_strong(expectedValue, newValue);
        }
    };

    // java.util.concurrent.lock.Lock
    struct Mutex {
    public:
        Mutex(bool reentrant = true) {
            if (reentrant) {
                pthread_mutexattr_t attr;
                LinuxErrors::handle(pthread_mutexattr_init(&attr));
                LinuxErrors::handle(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE));
                LinuxErrors::handle(pthread_mutex_init(&this->mtx, &attr), "Cannot initialize Mutex");
            } else {
                LinuxErrors::handle(pthread_mutex_init(&this->mtx, nullptr), "Cannot initialize Mutex");
            }
        }
        ~Mutex() {
            LinuxErrors::handle(pthread_mutex_destroy(&this->mtx), "Cannot destroy Mutex");
        }
        void enter() {
            LinuxErrors::handle(pthread_mutex_lock(&this->mtx), "Cannot enter Mutex");
        }
        void leave() {
            LinuxErrors::handle(pthread_mutex_unlock(&this->mtx), "Cannot leave Mutex");
        }
        struct Scope {
            Scope(Mutex &mutex) : parentMutex(nullptr) {
                mutex.enter();
                this->parentMutex = &mutex;
            }
            ~Scope() {
                Mutex *mutex = this->parentMutex;
                if (mutex != nullptr) {
                    this->parentMutex = nullptr;
                    mutex->leave();
                }
            }
        private:
            Mutex *parentMutex;
        };
    private:
        pthread_mutex_t mtx;
        friend class Condition;
    };

    // java.util.concurrent.lock.ReadWriteLock
    struct ReadWriteLock {
    public:
        ReadWriteLock(bool reentrant = true) : reentrant(reentrant) {
            LinuxErrors::handle(pthread_rwlock_init(&this->rwl, nullptr), "Cannot initialize ReadWriteLock");
        }
        ~ReadWriteLock() {
            LinuxErrors::handle(pthread_rwlock_destroy(&this->rwl), "Cannot destroy ReadWriteLock");
        }

    public:
        struct ReadingScope {
        public:
            ReadingScope(const ReadWriteLock &readWriteLock) : target(nullptr) {
                pthread_rwlock_t *tgt = const_cast<pthread_rwlock_t*>(&readWriteLock.rwl);
                LinuxErrors::handle(pthread_rwlock_rdlock(tgt), "Cannot acquire reading lock");
                this->target = tgt;
            }
            ~ReadingScope() {
                pthread_rwlock_t *rwlock = this->target;
                if (rwlock != nullptr) {
                    this->target = nullptr;
                    LinuxErrors::handle(pthread_rwlock_unlock(rwlock), "Cannot release reading lock");
                }
            }
        private:
            pthread_rwlock_t *target;
        };
        struct WritingScope {
        public:
            WritingScope(ReadWriteLock &readWriteLock) :parentLock(nullptr) {
                readWriteLock.lockWrite();
                this->parentLock = &readWriteLock;
            }
            ~WritingScope() {
                ReadWriteLock *readWriteLock = this->parentLock;
                if (readWriteLock != nullptr) {
                    this->parentLock = nullptr;
                    readWriteLock->unlockWrite();
                }
            }
        private:
            ReadWriteLock *parentLock;
        };
        struct PThreadId {
        public:
            PThreadId() : thread(0), nil(true) {}
            void set(pthread_t thread) {
                this->thread = thread;
                this->nil = false;
            }
            void clear() {
                this->nil = true;
            }
            bool equal(const pthread_t thread) {
                return !this->nil && pthread_equal(this->thread, thread);
            }
        private:
            volatile pthread_t thread;
            volatile bool nil;
        };

    private:
        void lockWrite() {
            int err = 0;
            pthread_t selfThread = pthread_self();
            if (!this->reentrant || !this->writingThreadId.equal(selfThread)) {
                err = pthread_rwlock_wrlock(&this->rwl);
            }
            if (this->reentrant && err == 0) {
                this->writingThreadId.set(selfThread);
                ++this->writingRecursiveDepth;
            }
            LinuxErrors::handle(err, "Cannot acquire writing lock");
        }
        void unlockWrite() {
            int err = 0;
            if (this->reentrant) {
                if (!this->writingThreadId.equal(pthread_self())) {
                    err = EPERM;
                } else if (--this->writingRecursiveDepth == 0) {
                    this->writingThreadId.clear();
                    err = pthread_rwlock_unlock(&this->rwl);
                }
            } else {
                err = pthread_rwlock_unlock(&this->rwl);
            }
            LinuxErrors::handle(err, "Cannot release writing lock");
        }
    private:
        pthread_rwlock_t rwl;
        bool reentrant;
        PThreadId writingThreadId;
        volatile int writingRecursiveDepth = 0;
        friend struct WritingScope;
    };

    // 资源释放器，不被直接使用
    // (无视异常，在析构时执行一个任意复杂的Lambda表达式，弥补标准C++不支持try/finally的遗憾)
    struct Finalizer {
    public:
        Finalizer(function<void()> handler) : handler(handler) {}
        ~Finalizer() { this->handler(); }
    private:
        function<void()> handler;
    };

    /**
     * 始祖接口
     */
    interface Interface {
        virtual ~Interface() {};
        virtual void retain() = 0;
        virtual void release() = 0;
    };

    /**
     * 始祖类
     *
     * std::shared_ptr虽然属于不侵入设计，但让原始指针和智能指针不再兼容, 
     * 导致将this指针逃逸出去很麻烦，即便小心地处理了,代码也不简洁,不易理解；
     * 所以，仍然使用COM和Objective-C的风格 的侵入式设计，
     * 所有设计意图为堆分配的类均需直接或间接从此类派生
     */
    class Object : implements Interface {

    public:
#ifdef DEBUG
        Object() : refCount(1) {
            memoryLeakMonitor().globalObjCount++;
        }
        virtual ~Object() {
            --memoryLeakMonitor().globalObjCount;
        }

#else
        Object() : refCount(1) {}
        virtual ~Object() {}
#endif //DEBUG

        virtual void retain() {
            int greaterThanOne = ++this->refCount;
            assert(greaterThanOne > 1);
        }
        virtual void release();
        virtual string toString() const {
            ostringstream builder;
            builder << className(this) << "@" << this;
            return builder.str();
        }

        /*
         * 拜type_info.name()不是真正的类名所赐，本函数得到真正的类名
         */
        static const string className(const type_info &typeInfo) {

            string value;
            {
                ReadWriteLock::ReadingScope readingScope(globalContext().classNameLock); //第一次锁（读锁）
                auto itr = globalContext().classNameMap.find(&typeInfo); //第一次访问
                if (itr != globalContext().classNameMap.end()) {
                    value = itr->second;
                }
            }

            if (value.length() == 0) { //第一次判断
                ReadWriteLock::WritingScope writingScope(globalContext().classNameLock); //第二次锁（写锁）
                auto itr = globalContext().classNameMap.find(&typeInfo); //第二次访问
                if (itr != globalContext().classNameMap.end()) {
                    value = itr->second;
                } else {
                     char *p = abi::__cxa_demangle(typeInfo.name(), nullptr, nullptr, nullptr);
                     value = p;
                     globalContext().classNameMap[&typeInfo] = value;
                     free(p);
                }
            }
            return value;
        }
        static string className(const Interface *obj) {
            return className(typeid(*obj));
        }
        static string className(Interface *obj) {
            return className(typeid(*obj));
        }

    public:

        /*
         * 引用类型,禁止拷贝和赋值
         */
        Object(const Object &) = delete;
        Object &operator = (const Object &) = delete;

        /*
         * 不得直接使用, 只能通过new_internal(Class, arg1, arg2, ..., argN)间接使用
         */
        void *operator new(size_t size, void *address) {
            return address;
        }
    private:
        /*
         * new运算符是私有的, 禁止使用原生的new Class(arg1, arg2, ..., argN)
         *
         * 如果目标类构造函数可见, 请使用
         * new_<Class>(arg1, arg2, ..., argN)
         * 请否则, 使用
         * new_internal(Class, arg1, arg2, ..., argN)
         */
        void *operator new(size_t size) {
            return ::operator new(size);
        }

        void willBeExported();

    protected:
        /*
         * 在构造函数之后被执行, 在C++的构造函数中自我调用没有多态性;
         * 而此方法中自我调用具备多态性, 此方法应该将this逃逸出去的操作
         */
        virtual void initialize() {}
        /*
         * 当对象被复活后执行, 绝大部分对象没机会执行此方法,
         * 除非finalize方法中this指针逃逸
         */
        virtual void resurrect() {}
        /*
         * 在析构函数之前被执行, 在C++的析构函数中自我调用没有多态性;
         * 而此方法中自我调用具备多态性, 此方法应该做一些非内存的释放操作
         *
         * 一般情况下, 该方法只会被执行一次.
         * 但如果配合对象复活机制时例外, 会反复执行知道对象无法复活为止
         * 这是和Java相比很大的一个区别
         */
        virtual void finalize() {}

    private:
        struct GlobalContext {
            Mutex weakRefMutex;
            map<const type_info*, string> classNameMap;
            ReadWriteLock classNameLock;
        };
        static GlobalContext &globalContext() {
            static GlobalContext uniqueInstance;
            return uniqueInstance;
        }
        void clearWeakReferences() {
            /*
             * 弱引用的清除必须在析构函数之前被调用
             * 原因：根据C++语言特性，~Object即将被执行的时候，派生类的析构早就被执行了，
             * 如果选择在~Object中清除所有弱引用，此时WeakRef仍然有机会取出指向~Object
             * 尚未被执行但派生类已然被析构的对象的强引用，后果不堪设想！
             */
            {
                // 作用域1，优化，先使用局部锁判断是否需要清除弱引用
                Mutex::Scope localScope(this->iwrMutex);
                if (this->invalidWeakRef.prev == &this->invalidWeakRef) {
                    return; //提前退出，避免后续操作全局锁以优化并发性
                }
            }

            {
                // 作用域2，清除所有弱引用
                Mutex::Scope globalScope(globalContext().weakRefMutex);
                _IWR *invalid = &this->invalidWeakRef;
                _IWR *iwr = invalid->next;
                while (iwr != invalid) {
                    _IWR *next = iwr->next;
                    static_cast<_WR*>(iwr)->target = nullptr;
                    static_cast<_WR*>(iwr)->prev = nullptr;
                    static_cast<_WR*>(iwr)->next = nullptr;
                    iwr = next;
                }
            }
        }

        struct _IWR { //Object内部的InvalidWeakRef，环形链表的头尾节点
        protected:
            _IWR *prev;
            _IWR *next;
            _IWR() {
                this->prev = this->next = this;
            }
            _IWR(int) : prev(nullptr), next(nullptr) {}
            friend class Object;
        };
        struct _WR : public _IWR { //真正WeakRef的非泛型基类
        public:
            ~_WR() {
                if (this->target) {
                    this->target = nullptr;
                    this->alone();
                }
            }
        protected:
            _WR(Object *target);
            void set(Object *target) {
                Object *old = this->target;
                if (old != target) {
                    if (old) {
                        this->alone();
                    }
                    this->target = target;
                    if (target) {
                        this->join(target->invalidWeakRef);
                    }
                }
            }
            Object *target;
        private:
            void join(_IWR &invalidWeakRef) {
                Mutex::Scope scope(globalContext().weakRefMutex);
                this->prev = invalidWeakRef.prev;
                this->next = &invalidWeakRef;
                this->prev->next = this;
                this->next->prev = this;
            }
            void alone() {
                Mutex::Scope scope(globalContext().weakRefMutex);
                this->prev->next = this->next;
                this->next->prev = this->prev;
            }
            friend class Object;
        };

#ifdef DEBUG
        struct MemoryLeakMonitor {
        public:
            ~MemoryLeakMonitor() {
                int goc = this->globalObjCount;
                if (goc == 0) {
                    cerr << "All the objects are deleted" << endl;
                } else {
                    cerr << goc << " object(s) is(are) still alive" << endl; //总泄漏数量（一定是完整信息）
                }

                //每个class类型各自的泄漏数量（有可能不是完整信息）
                Mutex::Scope scope(this->mutex);
                for (auto pair : this->retainedObjCountMap) {
                    cerr
                            << pair.second
                            << " '"
                            << pair.first
                            << "' object(s) is(are) still alive"
                            << endl;
                }
            }
            AtomicInteger globalObjCount;
            void retainAtFirst(const string &className) {
                Mutex::Scope scope(this->mutex);
                auto itr = this->retainedObjCountMap.find(className);
                if (itr == this->retainedObjCountMap.end()) {
                    this->retainedObjCountMap[className] = 1;
                } else {
                    itr->second++;
                }
            }
            void releaseAtLast(const string &className) {
                Mutex::Scope scope(this->mutex);
                auto itr = this->retainedObjCountMap.find(className);
                if (itr != this->retainedObjCountMap.end() && --itr->second == 0) {
                    this->retainedObjCountMap.erase(itr);
                }
            }

        private:
            map<string, int> retainedObjCountMap;
            Mutex mutex;
        };
        static MemoryLeakMonitor &memoryLeakMonitor() {
            static MemoryLeakMonitor staticInstance;
            return staticInstance;
        }
#endif //DEBUG

    private:
        AtomicInteger refCount;
        _IWR invalidWeakRef; //Object自身内置一个非法的弱引用，同其他合法的弱引用构成双向环链
        Mutex iwrMutex;

        friend struct Object::_WR;
        template <typename T> friend struct WeakRef;
        template <typename E, typename A> friend class _Array;
        template <typename T, typename ...Args> friend Ref<T> newObject(Args &&...);
        template <typename T> friend Ref<T> newInternalObject(function<void(void*)>);
        template <typename E, typename ...Args> __noreturn friend void throwNewException(Args &&...);
        template <typename E> __noreturn friend void throwNewInternalException(function<void(void*)>);
    };

    class Condition : extends Object {
    public:
        Condition(Mutex &mutex): mtx(&mutex.mtx) {
            LinuxErrors::handle(pthread_cond_init(&this->cond, nullptr));
        }
        virtual ~Condition() {
            pthread_cond_destroy(&this->cond);
        }
        void wait() {
            LinuxErrors::handle(pthread_cond_wait(&this->cond, this->mtx));
        }
        bool wait(time_t timeout) {
            struct timeval tv;
            gettimeofday(&tv,NULL);
            long sec = timeout / 1000;
            long nsec = (timeout % 1000) * 1000 * 1000;
            struct timespec ts = { tv.tv_sec + sec, tv.tv_usec * 1000 + nsec };
            return pthread_cond_timedwait(&this->cond, this->mtx, &ts) == 0;
        }
        void notify() {
            LinuxErrors::handle(pthread_cond_signal(&this->cond));
        }
        void notifyAll() {
            LinuxErrors::handle(pthread_cond_broadcast(&this->cond));
        }
    private:
        pthread_cond_t cond;
        pthread_mutex_t *mtx;
        friend struct Mutex;
    };

    template <typename T> //T必须为Interface或Object
    struct _Ref {
    public:
        ~_Ref() {
            T *p = this->p;
            if (p) {
                this->p = nullptr;
                p->release();
            }
        }
        T *get() const {
            return this->p;
        }
        T *operator ->() const;
        operator T*() const {
            return this->p;
        }
    protected:
        _Ref() {
            this->p = nullptr;
        }
        _Ref(T *p) {
            this->p = p;
            if (p) {
                p->retain();
            }
        }
        _Ref(const _Ref<T> &right) {
            T *p = right.p;
            this->p = p;
            if (p) {
                p->retain();
            }
        }
        _Ref(_Ref<T> &&tmpRight) {
            this->p = tmpRight.p;
            tmpRight.p = nullptr;
        }
        template <typename D> _Ref(const _Ref<D> &right) {
            T* p = right.get();
            this->p = p;
            if (p) {
                p->retain();
            }
        }
        template <typename D> _Ref(_Ref<D> &&tmpRight) {
            this->p = tmpRight.p;
            tmpRight.p = nullptr;
        }
        void assign(T *p) {
            T *op = this->p;
            if (op != p) {
                this->p = p;
                defer([=]{if (op) op->release();});
                if (p) p->retain();
            }
        }
        void assign(const _Ref<T> &right) {
            this->assign(right.p);
        }
        void assign(_Ref<T> &&tmpRight) {
            T *op = this->p;
            T *np = tmpRight.p;
            this->p = np;
            tmpRight.p = nullptr;
            if (op && op != np) {
                op->release();
            }
        }
        template <typename D> void assign(const _Ref<D> &right) {
            T *p = right.get();
            this->assign(p);
        }
        template <typename D> void assign(_Ref<D> &&tmpRight) {
            T *op = this->p;
            T *np = tmpRight.p;
            this->p = np;
            tmpRight.p = nullptr;
            if (op && op != np) {
                op->release();
            }
        }
        T *p;
    private:
        T *allocate(size_t size) {
            return this->p = reinterpret_cast<T*>(::operator new(size));
        }
        template <typename X> friend bool operator == (const _Ref<X> &, const _Ref<X> &);
        template <typename X> friend bool operator != (const _Ref<X> &, const _Ref<X> &);
        template <typename X> friend bool operator < (const _Ref<X> &, const _Ref<X> &);
        template <typename X> friend bool operator <= (const _Ref<X> &, const _Ref<X> &);
        template <typename X> friend bool operator > (const _Ref<X> &, const _Ref<X> &);
        template <typename X> friend bool operator >= (const _Ref<X> &, const _Ref<X> &);
        template <typename X> friend bool operator == (const _Ref<X> &, X*);
        template <typename X> friend bool operator != (const _Ref<X> &, X*);
        template <typename X> friend bool operator < (const _Ref<X> &, X*);
        template <typename X> friend bool operator <= (const _Ref<X> &, X*);
        template <typename X> friend bool operator > (const _Ref<X> &, X*);
        template <typename X> friend bool operator >= (const _Ref<X> &, X*);
        template <typename X> friend bool operator == (X*, const _Ref<X> &);
        template <typename X> friend bool operator != (X*, const _Ref<X> &);
        template <typename X> friend bool operator < (X*, const _Ref<X> &);
        template <typename X> friend bool operator <= (X*, const _Ref<X> &);
        template <typename X> friend bool operator > (X*, const _Ref<X> &);
        template <typename X> friend bool operator >= (X*, const _Ref<X> &);
        template <typename X> friend bool operator == (const _Ref<X> &, decltype(nullptr));
        template <typename X> friend bool operator != (const _Ref<X> &, decltype(nullptr));
        template <typename X> friend bool operator == (decltype(nullptr), const _Ref<X> &);
        template <typename X> friend bool operator != (decltype(nullptr), const _Ref<X> &);
        template <typename X, typename ...Args> friend Ref<X> newObject(Args &&...);
        template <typename X> friend Ref<X> newInternalObject(function<void(void*)>);
        template <typename E, typename ...Args> __noreturn friend void throwNewException(Args &&...);
        template <typename E> __noreturn friend void throwNewInternalException(function<void(void*)>);
        template <typename E, typename A> friend class _Array;
    };
    template <typename T> bool operator == (const _Ref<T> &a, const _Ref<T> &b) {
        return a.p == b.p;
    }
    template <typename T> bool operator != (const _Ref<T> &a, const _Ref<T> &b) {
        return a.p != b.p;
    }
    template <typename T> bool operator < (const _Ref<T> &a, const _Ref<T> &b) {
        return a.p < b.p;
    }
    template <typename T> bool operator <= (const _Ref<T> &a, const _Ref<T> &b) {
        return a.p <= b.p;
    }
    template <typename T> bool operator > (const _Ref<T> &a, const _Ref<T> &b) {
        return a.p > b.p;
    }
    template <typename T> bool operator >= (const _Ref<T> &a, const _Ref<T> &b) {
        return a.p >= b.p;
    }

    // These four functions compare smart pointer and raw pointer, they are unnecessary but useful for optimization
    template <typename T> bool operator == (const _Ref<T> &a, T *b) {
            return a.p == b;
    }
    template <typename T> bool operator != (const _Ref<T> &a, T *b) {
        return a.p != b;
    }
    template <typename T> bool operator < (const _Ref<T> &a, T *b) {
        return a.p < b;
    }
    template <typename T> bool operator <= (const _Ref<T> &a, T *b) {
        return a.p <= b;
    }
    template <typename T> bool operator > (const _Ref<T> &a, T *b) {
        return a.p > b;
    }
    template <typename T> bool operator >= (const _Ref<T> &a, T *b) {
        return a.p >= b;
    }
    template <typename T> bool operator == (T *a, const _Ref<T> &b) {
        return a == b.p;
    }
    template <typename T> bool operator != (T *a, const _Ref<T> &b) {
        return a != b.p;
    }
    template <typename T> bool operator < (T *a, const _Ref<T> &b) {
        return a < b.p;
    }
    template <typename T> bool operator <= (T *a, const _Ref<T> &b) {
        return a <= b.p;
    }
    template <typename T> bool operator > (T *a, const _Ref<T> &b) {
        return a > b.p;
    }
    template <typename T> bool operator >= (T *a, const _Ref<T> &b) {
        return a >= b.p;
    }
    template <typename T> bool operator == (const _Ref<T> &r, decltype(nullptr)) {
            return r.p == nullptr;
    }
    template <typename T> bool operator != (const _Ref<T> &r, decltype(nullptr)) {
        return r.p != nullptr;
    }
    template <typename T> bool operator == (decltype(nullptr), const _Ref<T> &r) {
        return nullptr == r.p;
    }
    template <typename T> bool operator != (decltype(nullptr), const _Ref<T> &r) {
        return nullptr != r.p;
    }

    // 强引用智能指针
    template <typename T> //T必须为Interface或Object
    struct Ref : public _Ref<T> {
    public:
        ref_implementation(T)
    };

    // 数组元素初始化方式
    enum ArrayElementType {
        /*
         * 数组元素为C++对象,
         * 1. 数组初始化时, 从头到尾调用各个元素的无参构造函数
         * 2. 数组复制时, 从头到尾调用各个元素的拷贝构造函数(Array::clone)或operator=(System::arraycopy)
         * 3. 数组析构时, 从尾到头调用各个元素的析构函数
         */
        CPP,
        C_DEFAULT_AS_ZERO, // 数组元素为C类型, 但是数组初始化时将各元素清0
        C // 数组元素为C类型且无任何额外附加操作
    };

    template <typename E> class Array;
    template <typename E> struct Ref<Array<E>>;
    template <typename E> class Array<Ref<Array<E>>>;
    template <> class Array<bool>;
    template <> class Array<char>;
    template <> class Array<short>;
    template <> class Array<int32_t>;
    template <> class Array<int64_t>;
    template <> class Array<float>;
    template <> class Array<double>;

    // 智能指针针对数组类型的模板偏特化.
    // 和Java/C#类似, 数组本身为引用类型, 即指针类型,
    // 但在C++指针无法像值一样便捷地支持一些语法糖, 例如[]下标和C++11的for(:)循环,
    // 所以对智能指针Ref<T>针对Array<E>类型模板参数进行偏特化, 来弥补这些功能
    template <typename E>
    struct Ref<Array<E>> : public _Ref<Array<E>> {

        ref_implementation(Array<E>)

        int length() const {
            Array<E> *p = this->p;
            return p ? p->length() : 0;
        }
        ArrayElementType elementType() const {
            Array<E> *p = this->p;
            return p ? p->elementType() : ArrayElementType::CPP;
        }
        E &operator[](int index) const;
        E *unsafe() const {
            Array<E> *p = this->p;
            return p ? p->unsafe() : nullptr;
        }
        E *begin() const {
            Array<E> *p = this->p;
            return p ? p->unsafe() : nullptr;
        }
        E *end() const {
            Array<E> *p = this->p;
            return p ? p->unsafe() + p->length() : nullptr;
        }
        operator E*() const { //高级数组 -> C/C++低级数组
            Array<E> *p = this->p;
            return p ? p->unsafe() : nullptr;
        }
    };

    template <typename T>
    struct WeakRef : public Object::_WR {
        WeakRef(T *target = nullptr) : Object::_WR(dynamic_cast<Object*>(target)) {}
        WeakRef(const Ref<T> &right) : Object::_WR(dynamic_cast<Object*>(right.get())) {}
        WeakRef(const WeakRef<T> &right) : Object::_WR(dynamic_cast<Object*>(right.target)) {}
        WeakRef<T> &operator = (T *target) {
            this->set(dynamic_cast<Object*>(target));
            return *this;
        }
        WeakRef<T> &operator = (const Ref<T> &right) {
            this->set(dynamic_cast<Object*>(right.get()));
            return *this;
        }
        WeakRef<T> &operator = (const WeakRef<T> &right) {
            this->set(dynamic_cast<Object*>(right.target));
            return *this;
        }
        Ref<T> get(bool validate = false) const;
        operator Ref<T>() const { //弱引用无法直接使用“->”，必须先转换成强类引用，这是隐式转换
            return this->get();
        }
        bool equals(const WeakRef<T> &weakRef) {
            return this->target == weakRef.target;
        }
        bool equals(const Ref<T> &ref) {
            return this->target == ref.get();
        }
        bool equals(T *p) {
            return this->target == p;
        }
    };

    class Exception : extends Object {
    public:
        Exception(const char *fileName, int lineNumber, const string &message = "", Ref<Exception> cause = nullptr)
            : fileName(fileName), lineNumber(lineNumber), message(message), cause(cause) {}
        const char *getFileName() const {
            return this->fileName;
        }
        int getLineNumber() const {
            return this->lineNumber;
        }
        string getMessage() const {
            return this->message;
        }
        const Ref<Exception> getCause() const {
            return this->cause;
        }
        // 打印整个异常链堆栈信息(C++ stream style)
        void printStackTrace(basic_ostream<char> &ostream = cerr) {
            ostream
            << className(this) << ": " << this->message << endl
            << "\tat " << this->fileName << ':' << this->lineNumber << endl;
            if (this->cause) {
                ostream << "Caused by: ";
                this->cause->printStackTrace(ostream);
            }
        }
        // 打印整个异常链堆栈信息(C file style)
        void printStackTrace(FILE *file) {
            fprintf(
                    file,
                    "%s: %s\n\tat %s:%d\n",
                    Object::className(this).c_str(), this->message.c_str(), this->fileName, this->lineNumber
            );
            if (this->cause) {
                fprintf(file, "Caused by: ");
                this->cause->printStackTrace(file);
            }
        }
        // 这两个boring方法供throw_宏使用
        template <typename E> static E *unwrap(Ref<E> ex) { return ex.get(); }
        template <typename E> static E *unwrap(E *ex) { return ex; }
    private:
        const char *fileName;
        int lineNumber;
        string message;
        Ref<Exception> cause;
    };

    class NullPointerException : extends Exception {
    public:
        NullPointerException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr) :
            Exception(exception_arg_prefix, message, cause) {
        }
    };

    class IllegalArgumentException : extends Exception {
    public:
        IllegalArgumentException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr) :
            Exception(exception_arg_prefix, message, cause) {
        }
    };

    class IllegalStateException : extends Exception {
    public:
        IllegalStateException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr) :
            Exception(exception_arg_prefix, message, cause) {}
    };

    class UnsupportedOperationException : extends Exception {
    public:
        UnsupportedOperationException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr) :
            Exception(exception_arg_prefix, message, cause) {}
    };

    class IOException : extends Exception {
    public:
        IOException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr) :
            Exception(exception_arg_prefix, message, cause) {}
    };

    class OSException : extends Exception {
    public:
        OSException(exception_param_prefix, int error, const string &message) :
            Exception(exception_arg_prefix, message, nullptr), error(error) {}
        int getError() const { return this->error; }
    private:
        int error;
    };

    class Semaphore : extends Object {
    public:
        Semaphore(int permits = 0) : permits(permits) {
            this->condition = new_<Condition>(this->mutex);
        }
        virtual ~Semaphore() {}
        void acquire(int permits = 1) {
            if (permits == 0) {
                return;
            }
            if (permits < 0) {
                throw_new(IllegalArgumentException, "argument cannot be negative number")
            }
            Mutex::Scope scope(this->mutex);
            while (this->permits < permits) {
                this->condition->wait();
            }
            this->permits -= permits;
        }
        bool tryAcquire(int permits, time_t timeout) {
            if (timeout < 0) {
                this->acquire(permits);
                return true;
            }
            if (permits == 0) {
                return true;
            }
            if (permits < 0) {
                throw_new(IllegalArgumentException, "argument cannot be negative number")
            }
            struct timeval tv;
            gettimeofday(&tv,NULL);
            time_t time =  tv.tv_sec * 1000 + tv.tv_usec / 1000;
            time_t endTime = time + timeout;
            Mutex::Scope scope(this->mutex);
            if (this->permits < permits) {
                while (true) {
                    this->condition->wait(endTime - time);
                    if (this->permits >= permits) {
                        break;
                    }
                    gettimeofday(&tv,NULL);
                    time_t time =  tv.tv_sec * 1000 + tv.tv_usec / 1000;
                    if (time >= endTime) {
                        return false;
                    }
                }
            }
            this->permits -= permits;
            return true;
        }
        void signal(int permits = 1) {
            if (permits == 0) {
                return;
            }
            if (permits < 0) {
                throw_new(IllegalArgumentException, "argument cannot be negative number")
            }
            Mutex::Scope scope(this->mutex);
            this->permits += permits;
            if (permits > 1) {
                this->condition->notifyAll();
            } else {
                this->condition->notify();
            }
        }
    private:
        Mutex mutex;
        int permits;
        Ref<Condition> condition;
    };

    // 引用计数数组，内存管理部分和实际数据部分共享一段内存，对象长度未知。
    template <typename E, typename A> //A extends _Array<E, T>
    class _Array : extends Object {
    public:
    public:
        virtual ~_Array() {
            if (this->eleType == ArrayElementType::CPP) {
                allocator<E> elementAllocator;
                E *p = this->unsafe();
                for (int i = this->size - 1; i >= 0; --i) {
                    elementAllocator.destroy(p + i);
                }
            }
        }
        int length() const {
            return this->size;
        }
        ArrayElementType elementType() const {
            return this->eleType;
        }
        E *unsafe() const {
            char *p = reinterpret_cast<char*>(const_cast<_Array<E, A>*>(this));
            return reinterpret_cast<E*>(p + sizeof(_Array<E, A>));
        }
        Ref<A> clone(int start = 0, int end = -1) const { //start闭end开
            if (end == -1) {
                end = this->size;
            }
            if (start > end) {
                throw_new(IllegalArgumentException, "start must <= end");
            }
            if (start < 0) {
                throw_new(IllegalArgumentException, "start is too small");
            }
            if (end > this->size) {
                throw_new(IllegalArgumentException, "end is too big");
            }
            int len = end - start;
            Ref<_Array<E, A>> arr = newInstance(
                    len,
                    this->eleType,
                    this->unsafe() + start
            );
            return static_cast<A*>(arr.get());
        }
        static Ref<A> newInstance(int size, ArrayElementType elementType) {
            return newInstance(size, elementType, nullptr);
        }
    protected:
        static Ref<A> newInstance(
                int size,
                ArrayElementType elementType,
                const E *src) {
            if (size < 0) {
                throw_new(IllegalArgumentException, "size must >= 0");
            }
            Ref<A> ref;
            A *p = ref.allocate(sizeof(_Array) + sizeof(E) * size);
            new(p) _Array<E, A>(size, elementType, src);
            p->willBeExported();
            return ref;
        };
    private:
        int size;
        ArrayElementType eleType;
        //带额外参数的new，不分配内存直接返回，仅仅给C++执行构造函数的机会
        void* operator new(size_t size, void *p) {
            return p;
        }
        _Array(int size, ArrayElementType elementType, const E *src) : size(size), eleType(elementType) {
            E *p = this->unsafe();
            switch (elementType) {
            case ArrayElementType::CPP:
                {
                    allocator<E> elementAllocator;
                    if (src) {
                        for (int i = 0; i < size; i++) {
                            elementAllocator.construct(p + i, src[i]);
                        }
                    } else {
                        for (int i = 0; i < size; i++) {
                            elementAllocator.construct(p + i);
                        }
                    }
                }
                break;
            case ArrayElementType::C_DEFAULT_AS_ZERO:
                if (src) {
                    memcpy(p, src, sizeof(E) * size);
                } else {
                    memset(p, 0, sizeof(E) * size);
                }
                break;
            case ArrayElementType::C:
                if (src) {
                    memcpy(p, src, sizeof(E) * size);
                }
                break;
            }
        }
    };

    template <typename E>
    class Array : extends _Array<E, Array<E>> {
    public:
        static Ref<Array<E>> of(const initializer_list<E> &list, ArrayElementType elementType) {
            return _Array<E, Array<E>>::newInstance(
                    list.size(),
                    elementType,
                    list.begin()
            );
        }
    };

    // 如果数组元素为智能指针, 元素一定需要构造, 故对这种情况进行模板偏特化
    template <typename E>
    class Array<Ref<E>> : extends _Array<Ref<E>, Array<Ref<E>>> {
    public:
        static Ref<Array<Ref<E>>> newInstance(int size) {
            return _Array<Ref<E>, Array<Ref<E>>>::newInstance(
                    size,
                    ArrayElementType::CPP
            );
        }
        static Ref<Array<Ref<E>>> of(const initializer_list<Ref<E>> &list) {
            return _Array<Ref<E>, Array<Ref<E>>>::newInstance(
                    list.size(),
                    ArrayElementType::CPP,
                    list.begin()
            );
        }
    };

    // 如果数组元素为基本类型, 元素一定不需构造, 故对这些情况进行模板全特化
    no_constructor_element_type_array(bool)
    no_constructor_element_type_array(char)
    no_constructor_element_type_array(short)
    no_constructor_element_type_array(int32_t)
    no_constructor_element_type_array(int64_t)
    no_constructor_element_type_array(float)
    no_constructor_element_type_array(double)

    // 如果数组元素为元素为也是数组, 那么元素一定是需要构造和析构的C++类型, 故对这种情况进行模板偏特化
    template <typename E>
    class Array<Ref<Array<E>>> : extends _Array<Ref<Array<E>>, Array<Ref<Array<E>>>> {
    public:
        static Ref<Array<Ref<Array<E>>>> newInstance(int size) {
            return _Array<Ref<Array<E>>, Array<Ref<Array<E>>>>::newInstance(
                    size,
                    ArrayElementType::CPP
            );
        }
        static Ref<Array<Ref<Array<E>>>> of(const initializer_list<Ref<Array<E>>> &list) {
            return _Array<Ref<Array<E>>, Array<Ref<Array<E>>>>::newInstance(
                    list.size(),
                    ArrayElementType::CPP,
                    list.begin()
            );
        }
    };

    // 为Object, Ref<T>和WeakRef<T>定义C++流输出(基于Object::toString)
    inline ostream &operator << (ostream &ostream, Object *obj) {
        if (!obj) {
            return ostream << "nullptr";
        }
        return ostream << static_cast<Object*>(obj)->toString();
    }
    template <typename T>
    ostream &operator << (ostream &out, const Ref<T> &ref) {
        if (ref == nullptr) {
            return out << "nullptr";
        }
        return out << ref->toString();
    }
    template <typename T>
    ostream &operator << (ostream &out, const WeakRef<T> &weakRef) {
        Ref<T> ref = weakRef.get();
        if (ref == nullptr) {
            return out << "WeakRef(nullptr)";
        }
        return out << "WeakRef(" << ref->toString() << ')';
    }
    inline ostream &operator << (ostream &out, Ref<Array<string>> strArr) {
        bool addSeparator = false;
        for (string &e : strArr) {
            if (addSeparator) {
                out << ", ";
            } else {
                addSeparator = true;
            }
            out << e;
        }
        return out;
    }

    template <typename T> // T must be Interface or Object
    class ThreadLocal : extends Object {
    public:
        ThreadLocal(Ref<T> initialValue = nullptr, bool weak = false);
        Ref<T> get() const;
        Ref<T> remove();
        Ref<T> set(Ref<T> value);
    private:
        Ref<ThreadLocal<Interface>> local;
    };

    template <>
    class ThreadLocal<Interface> : extends Object {
    private:
        struct GlobalController {
        public:
            GlobalController() {
                pthread_key_create(&this->key, free);
            }
            ~GlobalController() {
                pthread_key_delete(this->key);
            }

            map<ThreadLocal<Interface>*, Ref<Interface>> &strongMap() {
                return threadLocalData()->strongMap;
            }
            map<ThreadLocal<Interface>*, WeakRef<Interface>> &weakMap() {
                return threadLocalData()->weakMap;
            }
        private:
            struct ThreadLocalData {
                map<ThreadLocal<Interface>*, Ref<Interface>> strongMap;
                map<ThreadLocal<Interface>*, WeakRef<Interface>> weakMap;
            };
            ThreadLocalData *threadLocalData() {
                ThreadLocalData *data =
                        reinterpret_cast<ThreadLocalData*>(
                                pthread_getspecific(this->key)
                        );
                if (data == nullptr) {
                    data = new ThreadLocalData();
                    pthread_setspecific(this->key, data);
                }
                return data;
            }
            static void free(void *p) {
                if (p) {
                    delete reinterpret_cast<ThreadLocalData*>(p);
                }
            }
            pthread_key_t key;
        };
        // Key: ThreadLocal, Value: ThreadLocalValue
        static GlobalController &globalController() {
            static GlobalController instance;
            return instance;
        }
    public:
        ThreadLocal(Ref<Interface> initialValue = nullptr, bool weak = false) : weak(weak) {
            this->set(initialValue);
        }
        virtual ~ThreadLocal() {
            this->remove();
        }
        Ref<Interface> get() const{
            if (this->weak) {
                auto &weakMap = globalController().weakMap();
                auto itr = weakMap.find(const_cast<ThreadLocal<Interface>*>(this));
                if (itr == weakMap.end()) {
                    return nullptr;
                }
                Ref<Interface> ref = itr->second.get(false);
                if (ref == nullptr) {
                    weakMap.erase(itr);
                }
                return ref;
            }
            auto &strongMap = globalController().strongMap();
            auto itr = strongMap.find(const_cast<ThreadLocal<Interface>*>(this));
            if (itr == strongMap.end()) {
                return nullptr;
            }
            return itr->second;
        }
        Ref<Interface> remove() {
            if (this->weak) {
                auto &weakMap = globalController().weakMap();
                auto itr = weakMap.find(this);
                if (itr == weakMap.end()) {
                    return nullptr;
                }
                WeakRef<Interface> oldValue = itr->second;
                weakMap.erase(itr);
                return oldValue.get(false);
            }
            auto &strongMap = globalController().strongMap();
            auto itr = strongMap.find(this);
            if (itr == strongMap.end()) {
                return nullptr;
            }
            Ref<Interface> oldValue = itr->second;
            strongMap.erase(itr);
            return oldValue;
        }
        Ref<Interface> set(Ref<Interface> value) {
            if (this->weak) {
                auto &weakMap = globalController().weakMap();
                auto itr = weakMap.find(this);
                Ref<Interface> oldValue;
                if (itr != weakMap.end()) {
                    oldValue = itr->second.get(false);
                }
                WeakRef<Interface> weakRef = value;
                weakMap[this] = weakRef;
                return oldValue;
            }
            auto &strongMap = globalController().strongMap();
            auto itr = strongMap.find(this);
            Ref<Interface> oldValue;
            if (itr != strongMap.end()) {
                oldValue = itr->second;
            }
            strongMap[this] = value;
            return oldValue;
        }
    private:
        bool weak;
    };

    interface Closeable : implements Interface {
        virtual void close() = 0;
    };

    inline void LinuxErrors::handle(int err, const char *message) {
        if (err != 0) {
            throw_new(OSException, err, message != nullptr ? message : "OS error raised");
        }
    }

    inline void Object::willBeExported() {
#ifdef DEBUG
        string className = Object::className(this);
        memoryLeakMonitor().retainAtFirst(className);
#endif //DEBUG
        this->initialize();
    }

    inline void Object::release() {
        if (--this->refCount == 0) { //引用计数耗尽, 尝试释放对象
            ++this->refCount; //暂时提升引用计数, 防止后续finalize中触发对象的二次释放导致崩溃, 也为对象复活做准备
            defer([=]{
                if (--this->refCount == 0) { //如果在finalize执行后, 引用计数仍然为0, 真正释放对象, 不复活
                    defer([=]{
                        delete this;
                    });
#ifdef DEBUG
                    defer([=]{
                        string className = Object::className(this);
                        memoryLeakMonitor().releaseAtLast(className);
                    });
#endif //DEBUG
                    this->clearWeakReferences(); //清除弱引用
                } else {
                    this->resurrect();
                }
            });

            this->finalize(); //调用用户的finalize, 如果其中对引用计数的增加操作多于减少操作, 会导致对象复活
        }
    }

    inline Object::_WR::_WR(Object *target) : _IWR(0), target(target) {
        if (target) {
            if (target->refCount == 0) {
                throw_new(
                        IllegalArgumentException,
                        "The target has not been assigned to strong reference before"
                );
            }
            this->join(target->invalidWeakRef);
        }
    }

    template <typename T> T *_Ref<T>::operator ->() const {
        T *p = this->p;
        if (p == nullptr) {
            ostringstream builder;
            builder
            << "The current '"
            << Object::className(typeid(T)) <<
            " is nullptr so that '->' is not supported";
            throw_new(NullPointerException, builder.str().c_str());
        }
        return p;
    }

    template <typename E> E &Ref<Array<E>>::operator[](int index) const {
#ifdef DEBUG
        if (index < 0 || index >= this->length()) {
            throw_new(IllegalArgumentException, "Array index out of range");
        }
#endif //DEBUG
        return this->p->unsafe()[index];
    }

    template <typename T> Ref<T> WeakRef<T>::get(bool validate) const {
        Mutex::Scope scope(Object::globalContext().weakRefMutex);
        Ref<T> ref = dynamic_cast<T*>(this->target);
        if (validate && ref == nullptr) {
            ostringstream builder;
            builder
            << "No target of current WeakRef<"
            << Object::className(typeid(T))
            << '>';
            throw_new(IllegalStateException, builder.str().c_str())
        }
        return ref;
    }

    template <typename T> ThreadLocal<T>::ThreadLocal(Ref<T> initialValue, bool weak) {
        this->local = new_<ThreadLocal<Interface>>(initialValue, weak);
    }
    template <typename T> Ref<T> ThreadLocal<T>::get() const {
        Ref<Interface> ref = this->local->get();
        return ref.dynamicCast<T>();
    }
    template <typename T> Ref<T> ThreadLocal<T>::remove() {
        Ref<Interface> ref = this->local->remove().get();
        return ref.dynamicCast<T>();
    }
    template <typename T> Ref<T> ThreadLocal<T>::set(Ref<T> value) {
        Ref<Interface> ref = this->local->set(value).get();
        return ref.dynamicCast<T>();
    }

    /**
     * 这组方法在各种场景下, 取代带原生的new语句
     *
     * 和原生的new语句不同, 这组函数在确认对象的所有继承链上的构造函数都执行成功后
     * 将该对象标记成可以释放的, 此后Object::release才能释放此对象
     *
     * 在此之前, Object::release是不能释放对象的. 这样, 就允许用户在其构造函数内将this指针
     * 显式或隐式地转换为智能指针, 甚至让致谢智能指针逃逸出去.
     *
     * 否则, 引用计数就会因经过
     * 0 -> '构造内增加引用计数' -> '构造内减少引用计数' -> 0
     * 这一个过程, 在构造完成之前就被释放了而形成指针悬挂问题
     */
    template <typename T, typename ...Args> Ref<T> newObject(Args &&...args) {
        Ref<T> ref;
        T *p = ref.allocate(sizeof(T));
        new(p) T(args...);
        static_cast<Object*>(p)->willBeExported();
        return ref;
    }
    template <typename T> Ref<T> newInternalObject(function<void(void*)> constructor) {
        Ref<T> ref;
        T *p = ref.allocate(sizeof(T));
        constructor(p);
        static_cast<Object*>(p)->willBeExported();
        return ref;
    }
    template <typename E, typename ...Args> __noreturn void throwNewException(Args &&...args) {
        Ref<E> tmpRef;
        E *e = tmpRef.allocate(sizeof(E));
        new(e) E(args...);
        static_cast<Object*>(e)->willBeExported();
        tmpRef.p = nullptr;
        throw e;
    }
    template <typename E> __noreturn void throwNewInternalException(function<void(void*)> constructor) {
        Ref<E> tmpRef;
        E *e = tmpRef.allocate(sizeof(E));
        constructor(e);
        static_cast<Object*>(e)->willBeExported();
        tmpRef.p = nullptr;
        throw e;
    }

    template <typename E> using Arr = Ref<Array<E>>;
    template <typename E> using RefArray = Array<Ref<E>>;
    template <typename T> using RefArr = Arr<Ref<T>>;

    typedef char byte;
}
