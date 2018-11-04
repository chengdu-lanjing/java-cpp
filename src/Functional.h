/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once
#include "Common.h"
#include <functional>

namespace com_lanjing_cpp_common {

    using namespace std;

    interface Runnable;

    template <typename ...Args> interface Consumer;

    template <typename T> interface Supplier;

    template <typename> interface Function; //未定义, 仅仅为了支持另外一个Function<R(Args...)>类的语法
    template <typename R, typename ...Args> interface Function<R(Args...)>;


    template <typename T> using RefConsumer = Consumer<Ref<T>>;



    template <> struct Ref<Runnable>;
    template <typename ...Args> struct Ref<Consumer<Args...>>;
    template <typename T> struct Ref<Supplier<T>>;
    template <typename R, typename ...Args> struct Ref<Function<R(Args...)>>;


    template <>
    struct Ref<Runnable> : public _Ref<Runnable> {
        ref_implementation(Runnable)
        Ref<Runnable> &operator += (Ref<Runnable> right);
        Ref<Runnable> &operator -= (Ref<Runnable> right);
        void operator()() const;
    };
    Ref<Runnable> operator + (Ref<Runnable> a, Ref<Runnable> b);
    Ref<Runnable> operator - (Ref<Runnable> a, Ref<Runnable> b);

    interface Runnable : implements Interface {
    public:

        virtual void run() = 0;

        static Ref<Runnable> of(function<void()> lambda) {
            class Wrapper : extends Object, implements Runnable {
            public:
                Wrapper(function<void()> lambda) : lambda(lambda) {}
                virtual void run() override {
                    this->lambda();
                }
            private:
                function<void()> lambda;
                interface_refcount()
            };
            return new_<Wrapper>(lambda);
        }

        template <typename O>
        static Ref<Runnable> of(Ref<O> owner, void(O::*method)()) {
            return makeByInstanceMethod(owner, method, false);
        }

        template <typename O>
        static Ref<Runnable> of(O *owner, void(O::*method)()) {
            return makeByInstanceMethod(Ref<O>(owner), method, false);
        }

        template <typename O>
        static Ref<Runnable> weakOf(Ref<O> owner, void(O::*method)()) {
            return makeByInstanceMethod(owner, method, true);
        }

        template <typename O>
        static Ref<Runnable> weakOf(O *owner, void(O::*method)()) {
            return makeByInstanceMethod(Ref<O>(owner), method, true);
        }

        virtual ~Runnable() {}

    private:
        template <typename O>
        static Ref<Runnable> makeByInstanceMethod(Ref<O> owner, void(O::*method)(), bool weak) {
            class Wrapper : extends Object, implements Runnable {
            public:
                Wrapper(Ref<O> owner, void(O::*method)(), bool weak) : method(method) {
                    if (method == nullptr) {
                        throw_new(IllegalArgumentException, "owner cannot be nullptr");
                    }
                    if (weak) {
                        this->weakRef = owner;
                    } else {
                        this->strongRef = owner;
                    }
                }
                virtual void run() override {
                    if (this->strongRef != nullptr) {
                        (this->strongRef.get()->*this->method)();
                    } else {
                        Ref<O> snapshotRef = this->weakRef.get(false);
                        if (snapshotRef != nullptr) {
                            (snapshotRef.get()->*this->method)();
                        }
                    }
                }
            private:
                Ref<O> strongRef;
                WeakRef<O> weakRef;
                void(O::*method)();
                interface_refcount()
            };
            return new_<Wrapper>(owner, method, weak);
        }
    };


    template <typename ...Args>
    struct Ref<Consumer<Args...>> : public _Ref<Consumer<Args...>> {
        ref_implementation(Consumer<Args...>)
        Ref<Consumer<Args...>> &operator += (Ref<Consumer<Args...>> right);
        Ref<Consumer<Args...>> &operator -= (Ref<Consumer<Args...>> right);
        void operator()(Args ...args) const;
    };
    template <typename ...Args> Ref<Consumer<Args...>> operator + (Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b);
    template <typename ...Args> Ref<Consumer<Args...>> operator - (Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b);

    template <typename ...Args>
    interface Consumer : implements Interface {
    public:

        virtual void accept(Args ...args) = 0;

        static Ref<Consumer<Args...>> of(function<void(Args...)> lambda) {
            class Wrapper : extends Object, implements Consumer<Args...> {
            public:
                Wrapper(function<void(Args...)> lambda) : lambda(lambda) {}
                virtual void accept(Args... args) override {
                    this->lambda(args...);
                }
            private:
                function<void(Args...)> lambda;
                interface_refcount()
            };
            return new_<Wrapper>(lambda);
        }

        template <typename O>
        static Ref<Consumer<Args...>> of(Ref<O> owner, void(O::*method)(Args...)) {
            return makeByInstanceMethod(owner, method, false);
        }

        template <typename O>
        static Ref<Consumer<Args...>> of(O *owner, void(O::*method)(Args...)) {
            return makeByInstanceMethod(Ref<O>(owner), method, false);
        }

        template <typename O>
        static Ref<Consumer<Args...>> weakOf(Ref<O> owner, void(O::*method)(Args...)) {
            return makeByInstanceMethod(owner, method, true);
        }

        template <typename O>
        static Ref<Consumer<Args...>> weakOf(O *owner, void(O::*method)(Args...)) {
            return makeByInstanceMethod(Ref<O>(owner), method, true);
        }

        virtual ~Consumer() {}

    private:
        template <typename O>
        static Ref<Consumer<Args...>> makeByInstanceMethod(Ref<O> owner, void(O::*method)(Args...), bool weak) {
            class Wrapper : extends Object, implements Consumer<Args...> {
            public:
                Wrapper(Ref<O> owner, void(O::*method)(Args...), bool weak) : method(method) {
                    if (method == nullptr) {
                        throw_new(IllegalArgumentException, "owner cannot be nullptr");
                    }
                    if (weak) {
                        this->weakRef = owner;
                    } else {
                        this->strongRef = owner;
                    }
                }
                virtual void accept(Args... args) override {
                    if (this->strongRef != nullptr) {
                        (this->strongRef.get()->*this->method)(args...);
                    } else {
                        Ref<O> snapshotRef = this->weakRef.get(false);
                        if (snapshotRef != nullptr) {
                            (snapshotRef.get()->*this->method)(args...);
                        }
                    }
                }
            private:
                Ref<O> strongRef;
                WeakRef<O> weakRef;
                void(O::*method)(Args...);
                interface_refcount()
            };
            return new_<Wrapper>(owner, method, weak);
        }
    };


    template <typename T>
    struct Ref<Supplier<T>> : public _Ref<Supplier<T>> {
    public:
        Ref<Supplier<T>> &operator += (Ref<Supplier<T>> right);
        Ref<Supplier<T>> &operator -= (Ref<Supplier<T>> right);
        T operator()() const;
    private:
        typedef Supplier<T> _SelfType;
    public:
        ref_implementation(_SelfType)
    };
    template <typename T> Ref<Supplier<T>> operator + (Ref<Supplier<T>> a, Ref<Supplier<T>> b);
    template <typename T> Ref<Supplier<T>> operator - (Ref<Supplier<T>> a, Ref<Supplier<T>> b);

    template <typename T>
    interface Supplier : implements Interface {
    public:

        virtual T get() = 0;

        static Ref<Supplier<T>> of(function<T()> lambda) {
            class Wrapper : extends Object, implements Supplier<T> {
            public:
                Wrapper(function<T()> lambda) : lambda(lambda) {}
                virtual T get() override {
                    return this->lambda();
                }
            private:
                function<T()> lambda;
                interface_refcount()
            };
            return new_<Wrapper>(lambda);
        }

        template <typename O>
        static Ref<Supplier<T>> of(Ref<O> owner, void(O::*method)()) {
            class Wrapper : extends Object, implements Supplier<T> {
            public:
                Wrapper(Ref<O> owner, void(O::*method)()) : strongRef(owner), method(method) {
                    if (method == nullptr) {
                        throw_new(IllegalArgumentException, "owner cannot be nullptr");
                    }
                }
                virtual T get() override {
                    return (this->strongRef.get()->*this->method)();
                }
            private:
                Ref<O> strongRef;
                T(O::*method)();
                interface_refcount()
            };
            return new_<Wrapper>(owner, method);
        }

        template <typename O>
        static Ref<Supplier<T>> of(O *owner, void(O::*method)()) {
            return of(Ref<O>(owner), method);
        }

        template <typename O>
        static Ref<Supplier<T>> weakOf(Ref<O> owner, void(O::*method)(), const T &defaultValue) {
            class Wrapper : extends Object, implements Supplier<T> {
            public:
                Wrapper(Ref<O> owner, void(O::*method)(), const T &defaultValue) :
                    weakRef(owner), method(method), defaultValue(defaultValue) {
                    if (method == nullptr) {
                        throw_new(IllegalArgumentException, "owner cannot be nullptr");
                    }
                }
                virtual T get() override {
                    Ref<O> snapshotRef = this->weakRef.get(false);
                    if (snapshotRef != nullptr) {
                        return (snapshotRef.get()->*this->method)();
                    }
                    return this->defaultValue;
                }
            private:
                WeakRef<O> weakRef;
                T(O::*method)();
                T defaultValue;
                interface_refcount()
            };
            return new_<Wrapper>(owner, method, defaultValue);
        }

        template <typename O>
        static Ref<Supplier<T>> weakOf(O *owner, void(O::*method)(), const T &defaultValue) {
            return of(Ref<O>(owner), method, defaultValue);
        }

        virtual ~Supplier() {}
    };


    template <typename R, typename ...Args>
    struct Ref<Function<R(Args...)>> : public _Ref<Function<R(Args...)>> {
    public:
        Ref<Function<R(Args...)>> &operator += (Ref<Function<R(Args...)>> right);
        Ref<Function<R(Args...)>> &operator -= (Ref<Function<R(Args...)>> right);
        R operator()(Args ...args) const;
    private:
        typedef Function<R(Args...)> _SelfType;
    public:
        ref_implementation(_SelfType)
    };
    template <typename R, typename ...Args> Ref<Function<R(Args...)>> operator + (Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b);
    template <typename R, typename ...Args> Ref<Function<R(Args...)>> operator - (Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b);

    template <typename R, typename ...Args>
    interface Function<R(Args...)> : implements Interface {
    public:

        virtual R apply(Args ...args) = 0;

        static Ref<Function<R(Args...)>> of(function<R(Args...)> lambda) {
            class Wrapper : extends Object, implements Function<R(Args...)> {
            public:
                Wrapper(function<R(Args...)> lambda) : lambda(lambda) {}
                virtual R apply(Args... args) override {
                    return this->lambda(args...);
                }
            private:
                function<R(Args...)> lambda;
                interface_refcount()
            };
            return new_<Wrapper>(lambda);
        }

        template <typename O>
        static Ref<Function<R(Args...)>> of(Ref<O> owner, void(O::*method)(Args...)) {
            class Wrapper : extends Object, implements Function<R(Args...)> {
            public:
                Wrapper(Ref<O> owner, void(O::*method)(Args...)) : strongRef(owner), method(method) {
                    if (method == nullptr) {
                        throw_new(IllegalArgumentException, "owner cannot be nullptr");
                    }
                }
                virtual R apply(Args... args) override {
                    return (this->strongRef.get()->*this->method)(args...);
                }
            private:
                Ref<O> strongRef;
                void(O::*method)(Args...);
                interface_refcount()
            };
            return new_<Wrapper>(owner, method);
        }

        template <typename O>
        static Ref<Function<R(Args...)>> of(O *owner, void(O::*method)(Args...)) {
            return of(Ref<O>(owner), method);
        }

        template <typename O>
        static Ref<Function<R(Args...)>> weakOf(Ref<O> owner, void(O::*method)(Args...), const R &defaultValue) {
            class Wrapper : extends Object, implements Function<R(Args...)> {
            public:
                Wrapper(Ref<O> owner, void(O::*method)(Args...), const R &defaultValue) :
                    weakRef(owner), method(method), defaultValue(defaultValue) {
                    if (method == nullptr) {
                        throw_new(IllegalArgumentException, "owner cannot be nullptr");
                    }
                }
                virtual R apply(Args... args) override {
                    Ref<O> snapshotRef = this->weakRef.get(false);
                    if (snapshotRef != nullptr) {
                        return (snapshotRef.get()->*this->method)(args...);
                    }
                    return this->defaultValue;
                }
            private:
                WeakRef<O> weakRef;
                void(O::*method)(Args...);
                R defaultValue;
                interface_refcount()
            };
            return new_<Wrapper>(owner, method, defaultValue);
        }

        template <typename O>
        static Ref<Function<R(Args...)>> weakOf(O *owner, void(O::*method)(Args...), const R &defaultValue) {
            return of(Ref<O>(owner), method, defaultValue);
        }

        virtual ~Function() {}
    };


    struct Functions {
    public:
        static Ref<Runnable> combine(Ref<Runnable> a, Ref<Runnable> b);
        static Ref<Runnable> remove(Ref<Runnable> a, Ref<Runnable> b);

        template <typename ...Args>
        static Ref<Consumer<Args...>> combine(Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b);
        template <typename ...Args>
        static Ref<Consumer<Args...>> remove(Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b);

        template <typename T>
        static Ref<Supplier<T>> combine(Ref<Supplier<T>> a, Ref<Supplier<T>> b);
        template <typename T>
        static Ref<Supplier<T>> remove(Ref<Supplier<T>> a, Ref<Supplier<T>> b);

        template <typename R, typename ...Args>
        static Ref<Function<R(Args...)>> combine(Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b);
        template <typename R, typename ...Args>
        static Ref<Function<R(Args...)>> remove(Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b);

    private:
        Functions();

        template <typename I>
        abstract class AbstractCombinedInterface : extends Object, implements I {
        public:
            static Ref<I> combine(
                    function<Ref<AbstractCombinedInterface<I>>(Ref<I>, Ref<I>)> combinedInstanceFactory,
                    Ref<I> a,
                    Ref<I> b) {
                if (a == nullptr) {
                    return b;
                }
                if (b == nullptr) {
                    return a;
                }
                return combinedInstanceFactory(a, b);
            }
            static Ref<I> remove(
                    function<Ref<AbstractCombinedInterface<I>>(Ref<I>, Ref<I>)> combinedInstanceFactory,
                    Ref<I> a,
                    Ref<I> b) {
                if (a == nullptr) {
                    return nullptr;
                }
                if (b == nullptr) {
                    return a;
                }
                list<Ref<I>> removingInstances;
                forEach(b, [&](Ref<I> instance){
                    removingInstances.push_back(instance);
                });
                Ref<I> resultInstance = nullptr;
                forEach(a, [&](Ref<I> instance){
                    bool removed = false;
                    for (auto
                            itr = removingInstances.begin(),
                            endItr = removingInstances.end();
                            itr != endItr;
                            itr++) {
                        if (instance == *itr) {
                            removingInstances.erase(itr);
                            removed = true;
                            break;
                        }
                    }
                    if (!removed) {
                        resultInstance = combine(combinedInstanceFactory, resultInstance, instance);
                    }
                });
                return resultInstance;
            }
        protected:
            AbstractCombinedInterface(Ref<I> self, Ref<I> next)
                : self(self), next(next) {
                if (self == nullptr) {
                    throw_new(IllegalArgumentException, "self cannot be null");
                }
                if (next == nullptr) {
                    throw_new(IllegalArgumentException, "next cannot be null");
                }
            }
        private:
            static void forEach(Ref<I> instance, function<void(Ref<I>)> consumer) {

                Ref<AbstractCombinedInterface<I>> combinedInstance =
                        dynamic_cast<AbstractCombinedInterface<I>*>(instance.get());
                // Why does "instance.dynamicCast<AbstractCombinedInterface<I>>()" report error?

                if (combinedInstance != nullptr) {
                    forEach(combinedInstance->self, consumer);
                    forEach(combinedInstance->next, consumer);
                } else if (instance != nullptr) {
                    consumer(instance);
                }
            }
        protected:
            Ref<I> self;
            Ref<I> next;
            interface_refcount()
        };

        class CombinedRunnable : extends AbstractCombinedInterface<Runnable> {
        public:
            CombinedRunnable(Ref<Runnable> a, Ref<Runnable> b)
                    : AbstractCombinedInterface<Runnable>(a, b) {}
            virtual void run() override {
                this->self->run();
                this->next->run();
            }
        };

        template <typename ...Args>
        class CombinedConsumer : extends AbstractCombinedInterface<Consumer<Args...>> {
        public:
            CombinedConsumer(Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b)
                    : AbstractCombinedInterface<Consumer<Args...>>(a, b) {}
            virtual void accept(Args ...args) override {
                this->self->accept(args...);
                this->next->accept(args...);
            }
        };

        template <typename T>
        class CombinedSupplier : extends AbstractCombinedInterface<Supplier<T>> {
        public:
            CombinedSupplier(Ref<Supplier<T>> a, Ref<Supplier<T>> b)
                    : AbstractCombinedInterface<Supplier<T>>(a, b) {}
            virtual T get() override {
                this->self->get();
                return this->next->get();
            }
        };

        template <typename> class CombinedFunction; //未定义

        template <typename R, typename ...Args>
        class CombinedFunction<R(Args...)> : extends AbstractCombinedInterface<Function<R(Args...)>> {
        public:
            CombinedFunction(Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b)
                    : AbstractCombinedInterface<Function<R(Args...)>>(a, b) {}
            virtual R apply(Args ...args) override {
                this->self->apply(args...);
                return this->next->apply(args...);
            }
        };
    };

    inline Ref<Runnable> Functions::combine(Ref<Runnable> a, Ref<Runnable> b) {
        return CombinedRunnable::combine(
                [&](Ref<Runnable> a, Ref<Runnable> b) {
                    return new_internal(CombinedRunnable, a, b);
                },
                a,
                b
        );
    }

    inline Ref<Runnable> Functions::remove(Ref<Runnable> a, Ref<Runnable> b) {
        return CombinedRunnable::remove(
                [&](Ref<Runnable> a, Ref<Runnable> b) {
                    return new_internal(CombinedRunnable, a, b);
                },
                a,
                b
        );
    }

    template <typename ...Args>
    Ref<Consumer<Args...>> Functions::combine(Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b) {
        return CombinedConsumer<Args...>::combine(
                [&](Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b) {
                    return new_internal(CombinedConsumer<Args...>, a, b);
                },
                a,
                b
        );
    }

    template <typename ...Args>
    Ref<Consumer<Args...>> Functions::remove(Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b) {
        return CombinedConsumer<Args...>::remove(
                [&](Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b) {
                    return new_internal(CombinedConsumer<Args...>, a, b);
                },
                a,
                b
        );
    }

    template <typename T>
    Ref<Supplier<T>> Functions::combine(Ref<Supplier<T>> a, Ref<Supplier<T>> b) {
        return CombinedSupplier<T>::combine(
                [&](Ref<Supplier<T>> a, Ref<Supplier<T>> b) {
                    return newInternalObject<CombinedSupplier<T>>([=](void *address){
                        new(address) CombinedSupplier<T>(a, b);
                    });
                },
                a,
                b
        );
    }

    template <typename T>
    Ref<Supplier<T>> Functions::remove(Ref<Supplier<T>> a, Ref<Supplier<T>> b) {
        return CombinedSupplier<T>::remove(
                [&](Ref<Supplier<T>> a, Ref<Supplier<T>> b) {
                    return newInternalObject<CombinedSupplier<T>>([=](void *address){
                        new(address) CombinedSupplier<T>(a, b);
                    });
                },
                a,
                b
        );
    }

    template <typename R, typename ...Args>
    Ref<Function<R(Args...)>> Functions::combine(Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b) {
        return CombinedFunction<R(Args...)>::combine(
                [&](Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b) {
                    return newInternalObject<CombinedFunction<R(Args...)>>([=](void *address){
                        new(address) CombinedFunction<R(Args...)>(a, b);
                    });
                },
                a,
                b
        );
    }

    template <typename R, typename ...Args>
    Ref<Function<R(Args...)>> Functions::remove(Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b) {
        return CombinedFunction<R(Args...)>::remove(
                [&](Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b) {
                    return newInternalObject<CombinedFunction<R(Args...)>>([=](void *address){
                        new(address) CombinedFunction<R(Args...)>(a, b);
                    });
                },
                a,
                b
        );
    }

    inline Ref<Runnable>& Ref<Runnable>::operator += (Ref<Runnable> right) {
        return *this = *this + right;
    }
    inline Ref<Runnable>& Ref<Runnable>::operator -= (Ref<Runnable> right) {
        return *this = *this - right;
    }
    inline void Ref<Runnable>::operator()() const {
        return this->get()->run();
    }
    inline Ref<Runnable> operator + (Ref<Runnable> a, Ref<Runnable> b) {
        return Functions::combine(a, b);
    }
    inline Ref<Runnable> operator - (Ref<Runnable> a, Ref<Runnable> b) {
        return Functions::remove(a, b);
    }

    template <typename ...Args>
    Ref<Consumer<Args...>> &Ref<Consumer<Args...>>::operator += (Ref<Consumer<Args...>> right) {
        return *this = *this + right;
    }
    template <typename ...Args>
    Ref<Consumer<Args...>> &Ref<Consumer<Args...>>::operator -= (Ref<Consumer<Args...>> right) {
        return *this = *this - right;
    }
    template <typename ...Args>
    void Ref<Consumer<Args...>>::operator()(Args ...args) const {
        this->get()->accept(args...);
    }
    template <typename ...Args>
    Ref<Consumer<Args...>> operator + (Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b) {
        return Functions::combine(a, b);
    }
    template <typename ...Args>
    Ref<Consumer<Args...>> operator - (Ref<Consumer<Args...>> a, Ref<Consumer<Args...>> b) {
        return Functions::remove(a, b);
    }

    template <typename T>
    Ref<Supplier<T>> &Ref<Supplier<T>>::operator += (Ref<Supplier<T>> right) {
        return *this = *this + right;
    }
    template <typename T>
    Ref<Supplier<T>> &Ref<Supplier<T>>::operator -= (Ref<Supplier<T>> right) {
        return *this = *this - right;
    }
    template <typename T>
    T Ref<Supplier<T>>::operator()() const {
        return this->get()->get();
    }
    template <typename T>
    Ref<Supplier<T>> operator + (Ref<Supplier<T>> a, Ref<Supplier<T>> b) {
        return Functions::combine(a, b);
    }
    template <typename T>
    Ref<Supplier<T>> operator - (Ref<Supplier<T>> a, Ref<Supplier<T>> b) {
        return Functions::remove(a, b);
    }

    template <typename R, typename ...Args>
    Ref<Function<R(Args...)>> &Ref<Function<R(Args...)>>::operator += (Ref<Function<R(Args...)>> right) {
        return *this = *this + right;
    }
    template <typename R, typename ...Args>
    Ref<Function<R(Args...)>> &Ref<Function<R(Args...)>>::operator -= (Ref<Function<R(Args...)>> right) {
        return *this = *this - right;
    }
    template <typename R, typename ...Args>
    R Ref<Function<R(Args...)>>::operator()(Args ...args) const {
        return this->get()->apply(args...);
    }
    template <typename R, typename ...Args>
    Ref<Function<R(Args...)>> operator + (Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b) {
        return Functions::combine(a, b);
    }
    template <typename R, typename ...Args>
    Ref<Function<R(Args...)>> operator - (Ref<Function<R(Args...)>> a, Ref<Function<R(Args...)>> b) {
        return Functions::remove(a, b);
    }
}
