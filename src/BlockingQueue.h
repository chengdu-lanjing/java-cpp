/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

#include "Common.h"
#include <list>

namespace com_lanjing_cpp_common {

    using namespace std;

    template <typename E> //E必须为Interface或Object
    interface BlockingQueue : Interface {
        virtual ~BlockingQueue() {}
        virtual void put(Ref<E> element) = 0;
        virtual bool offer(Ref<E> element, long timeout) = 0;
        virtual Ref<E> take() = 0;
        virtual Ref<E> poll(long timeout) = 0;
    };

    template <typename E>
    abstract class AbstractBlockingQueue : extends Object, implements BlockingQueue<E> {
    public:
        virtual ~AbstractBlockingQueue() {}

        virtual void put(Ref<E> element) override {
            if (element == nullptr) {
                throw_new(IllegalArgumentException, "element cannot be null");
            }

            Mutex::Scope scope(this->mutex);
            while (this->locklesslyIsFull()) {
                this->inCondition->wait();
            }
            this->locklesslyPush(element);
            this->outCondition->notify();
        }

        virtual bool offer(Ref<E> element, long timeout) override {
            if (element == nullptr) {
                throw_new(IllegalArgumentException, "element cannot be null");
            }

            Mutex::Scope scope(this->mutex);
            if (this->locklesslyIsFull() &&
                    !this->inCondition->wait(timeout) &&
                    this->locklesslyIsFull()) {
                return false;
            }
            this->locklesslyPush(element);
            this->outCondition->notify();
            return true;
        }

        virtual Ref<E> take() override {
            Mutex::Scope scope(this->mutex);
            while (this->locklesslyIsEmpty()) {
                this->outCondition->wait();
            }
            Ref<E> element = this->locklesslyPoll();
            this->inCondition->notify();
            return element;
        }

        virtual Ref<E> poll(long timeout) override {
            Mutex::Scope scope(this->mutex);
            if (this->locklesslyIsEmpty() &&
                    !this->outCondition->wait(timeout) &&
                    !this->locklesslyIsEmpty()) {
                return nullptr;
            }
            Ref<E> element = this->locklesslyPoll();
            this->inCondition->notify();
            return element;
        }

    protected:
        AbstractBlockingQueue() {
            this->inCondition = new_<Condition>(this->mutex);
            this->outCondition = new_<Condition>(this->mutex);
        }

        virtual bool locklesslyIsEmpty() = 0;

        virtual bool locklesslyIsFull() = 0;

        virtual void locklesslyPush(Ref<E> element) = 0;

        virtual Ref<E> locklesslyPoll() = 0;

    private:
        Mutex mutex;
        Ref<Condition> inCondition;
        Ref<Condition> outCondition;

        interface_refcount()
    };

    template <typename E>
    class ArrayBlockingQueue : extends AbstractBlockingQueue<E> {
    public:
        ArrayBlockingQueue(int capacity) {
            if (capacity < 2) {
                throw_new(IllegalArgumentException, "capacity cannot be less than 2");
            }
            this->elements = RefArray<E>::newInstance(capacity);
        }
        virtual ~ArrayBlockingQueue() {}

    protected:

        virtual bool locklesslyIsEmpty() override {
            return this->in == this->out;
        }

        virtual bool locklesslyIsFull() override {
            return (this->in + 1) % this->elements.length() == this->out;
        }

        virtual void locklesslyPush(Ref<E> element) override {
            this->elements[this->in] = element;
            this->in = (this->in + 1) % this->elements.length();
        }

        virtual Ref<E> locklesslyPoll() override {
            Ref<E> element = this->elements[this->out];
            this->elements[this->out] = nullptr;
            this->out = (this->out + 1) % this->elements.length();
            return element;
        }

    private:
        RefArr<E> elements;
        int in = 0;
        int out = 0;
    };

    template <typename E>
    class LinkedBlockingQueue : public AbstractBlockingQueue<E> {

    public:
        virtual ~LinkedBlockingQueue() {}

    protected:

        virtual bool locklesslyIsEmpty() override {
            return this->elements.empty();
        }

        virtual bool locklesslyIsFull() override {
            return false;
        }

        virtual void locklesslyPush(Ref<E> element) override {
            this->elements.push_back(element);
        }

        virtual Ref<E> locklesslyPoll() override {
            Ref<E> front = this->elements.front();
            this->elements.pop_front();
            return front;
        }
    private:
        list<Ref<E>> elements;
    };
}
