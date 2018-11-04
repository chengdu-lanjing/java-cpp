/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

#include <map>
#include <list>
#include <sstream>
#include "../Common.h"

namespace com_lanjing_cpp_common_database {

    using namespace com_lanjing_cpp_common;
    using namespace std;

    class SQLException : extends Exception {
    public:
        SQLException(exception_param_prefix, const string &message = "", Exception *cause = nullptr)
            : Exception(exception_arg_prefix, message, cause), errorCode(0) {
            cerr << message << endl;
        }
        SQLException(exception_param_prefix, int errorCode, const string &message = "", Exception *cause = nullptr)
            : Exception(exception_arg_prefix, message, cause), errorCode(errorCode) {
            cerr << message << endl;
        }
        int getErrorCode() const {
            return this->errorCode;
        }
    private:
        int errorCode;
    };

    interface ResultSet : implements Closeable {
        virtual bool next() = 0;
        virtual int getInt(int col) = 0;
        virtual __int64_t getInt64(int col) = 0;
        virtual double getDouble(int col) = 0;
        virtual string getString(int col) = 0;
        virtual void getString(int col, char *target) {
            string str = this->getString(col);
            strcpy(target, str.c_str());
        }
    };

    interface Statement : implements Closeable {
        virtual Ref<Statement> set(int index, int value) = 0;
        virtual Ref<Statement> set(const char *name, int value) = 0;
        virtual Ref<Statement> set(int index, __int64_t value) = 0;
        virtual Ref<Statement> set(const char *name, __int64_t value) = 0;
        virtual Ref<Statement> set(int index, double value) = 0;
        virtual Ref<Statement> set(const char *name, double value) = 0;
        virtual Ref<Statement> set(int index, const char *value) = 0;
        virtual Ref<Statement> set(const char *name, const char *value) = 0;
        virtual Ref<Statement> set(int index, const string &value) {
            return this->set(index, value.c_str());
        }
        virtual Ref<Statement> set(const char *name, const string &value) {
            return this->set(name, value.c_str());
        }
        virtual Ref<ResultSet> executeQuery() = 0;
        virtual int executeUpdate() = 0;
    };

    interface Connection : implements Closeable {
        virtual void beginTransaction() = 0;
        virtual void commitTransaction() = 0;
        virtual void rollbackTransaction() = 0;
        virtual Ref<Statement> preparedStatement(const char *sql) = 0;
        struct TransactionScope {
        public:
            TransactionScope(Ref<Connection> con) : succ(false) {
                if (con == nullptr) {
                    throw_new(IllegalArgumentException, "connection must be specified");
                }
                con->beginTransaction();
                this->con = con;
            }
            void success() {
                this->succ = true;
            }
            ~TransactionScope() {
                if (this->succ) {
                    con->commitTransaction();
                } else {
                    con->rollbackTransaction();
                }
            }
        private:
            Ref<Connection> con;
            bool succ;
        };
    };

    class Driver : extends Object {
    public:
        virtual bool acceptsURL(const char *url) = 0;
        virtual Ref<Connection> connect(const char *url, const map<string, string> &properties) = 0;
    protected:
        virtual void initialize() override;
    };

    struct DriverManager {
    public:
        static Ref<Connection> getConnection(const char *url) {
            map<string, string> properties;
            return getConnection(url, properties);
        }
        static Ref<Connection> getConnection(const char *url, const map<string, string> &properties) {
            ReadWriteLock::ReadingScope readingScope(driverMapRwl());
            Ref<Driver> matchedDriver;
            for (auto pair : driverMap()) {
                Ref<Driver> &driver = pair.second;
                if (driver->acceptsURL(url)) {
                    matchedDriver = driver;
                    break;
                }
            }
            if (matchedDriver == nullptr) {
                ostringstream messageBuilder;
                messageBuilder << "The url '" << url << "' cannot be accepted by any driver";
                throw_new(SQLException, messageBuilder.str().c_str());
            }
            return matchedDriver->connect(url, properties);
        }
    private:
        static void registerDriver(Ref<Driver> driver) {
            if (driver == nullptr) {
                throw_new(IllegalArgumentException, "driver must be specified");
            }
            ReadWriteLock::WritingScope writingScope(driverMapRwl());
            string driverClassName = Object::className(driver);
            driverMap()[driverClassName] = driver;
        }
        static map<string, Ref<Driver>> &driverMap() {
            static map<string, Ref<Driver>> map;
            return map;
        }
        static ReadWriteLock &driverMapRwl() {
            static ReadWriteLock rwl;
            return rwl;
        }
        friend class Driver;
    };

    inline void Driver::initialize() {
        DriverManager::registerDriver(this);
    }
}

namespace com_lanjing_cpp_common_database_spi {

    using namespace com_lanjing_cpp_common_database;

    class AbstractDbObject : extends Object, implements Closeable {
    public:
        virtual ~AbstractDbObject() {}
        virtual void close() override {
            this->doClose();
        }
    protected:
        AbstractDbObject(Ref<AbstractDbObject> parent = nullptr) : parentRef(parent) {
            this->childLocal = new_<ThreadLocal<AbstractDbObject>>(nullptr, true);
        }
        virtual void initialize() override final {
            this->doOpen();
        }
        virtual void finalize() override final {
            this->close();
        }
        virtual void onOpen() = 0;
        virtual void onClose() = 0;
        void checkState() {
            if (this->closed) {
                ostringstream builder;
                builder << "The " << Object::className(this) << " has been closed";
                throw_new(IllegalStateException, builder.str().c_str());
            }
        }
        Ref<AbstractDbObject> getParent() {
            return this->parentRef.get(false);
        }
        Ref<AbstractDbObject> getChild() {
            return this->childLocal->get();
        }
    private:
        void doOpen() {
            Ref<AbstractDbObject> parent = this->getParent();
            if (parent != nullptr) {
                parent->setChild(this);
            }
            this->onOpen();
        }
        void doClose() {
            if (this->closed.compareAndSet(false, true)) {
                defer([=]{
                    Ref<AbstractDbObject> parent = this->getParent();
                    if (parent != nullptr) {
                        parent->setChild(nullptr);
                    }
                });
                defer([=]{
                    this->onClose();
                });
                Ref<AbstractDbObject> child = this->getChild();
                if (child != nullptr) {
                    child->setParent(nullptr);
                    child->close();
                }
            }
        }
        void setChild(Ref<AbstractDbObject> child) {
            if (child != nullptr) {
                this->childLocal->set(child);
            } else {
                this->childLocal->remove();
            }
        }
        void setParent(Ref<AbstractDbObject> parent) {
            this->parentRef = parent;
        }
    private:
        AtomicBoolean closed;
        WeakRef<AbstractDbObject> parentRef;
        Ref<ThreadLocal<AbstractDbObject>> childLocal;
    };

    abstract class AbstractConnection : extends AbstractDbObject, implements Connection {
    public:
        virtual void beginTransaction() override final {
            this->checkState();
            if (!transaction.compareAndSet(false, true)) {
                throw_new(
                    SQLException,
                    "Cannot begin new transaction because the current connection is already in transaction"
                );
            }
            if (this->getChild() != nullptr) {
                throw_new(
                    SQLException,
                    "Cannot begin new transaction because the connection has unclosed statement"
                );
            }
        }
        virtual void commitTransaction() override final {
            this->checkState();
            if (!transaction.compareAndSet(true, false)) {
                throw_new(
                    SQLException,
                    "Cannot commit the transaction because the current connection isn't already in transaction"
                );
            }
            if (this->getChild() != nullptr) {
                throw_new(
                    SQLException,
                    "Cannot commit the transaction because the connection has unclosed statement"
                );
            }
        }
        virtual void rollbackTransaction() override final {
            this->checkState();
            if (!transaction.compareAndSet(true, false)) {
                throw_new(
                    SQLException,
                    "Cannot rollback the transaction because the current connection isn't already in transaction"
                );
            }
            if (this->getChild() != nullptr) {
                throw_new(
                    SQLException,
                    "Cannot rollback the transaction because the connection has unclosed statement"
                );
            }
        }
        virtual void close() override {
            AbstractDbObject::close();
        }
    protected:
        AbstractConnection(const char *url, const map<string, string> &properties) :
            url(url), properties(properties) {}
        virtual void onBeginTransaction() = 0;
        virtual void onCommitTransaction() = 0;
        virtual void onRollbackTransaction() = 0;
    protected:
        string url;
        map<string, string> properties;
    private:
        AtomicBoolean transaction;
        interface_refcount()
    };

    abstract class AbstractStatement : extends AbstractDbObject, implements Statement {
    public:
        virtual void close() override {
            AbstractDbObject::close();
        }
    protected:
        AbstractStatement(Ref<AbstractConnection> con, const char *sql) :
            AbstractDbObject(con), sql(sql) {}
        string sql;
        interface_refcount()
    };

    abstract class AbstractResultSet : extends AbstractDbObject, implements ResultSet {
    public:
        virtual void close() override {
            AbstractDbObject::close();
        }
    protected:
        AbstractResultSet(Ref<AbstractStatement> statement) :
            AbstractDbObject(statement) {}
        interface_refcount()
    };

    abstract class AbstractWrapperConnection : extends Object, implements Connection {
    public:
        virtual void beginTransaction() override {
            this->targetConnection->beginTransaction();
        }
        virtual void commitTransaction() override {
            this->targetConnection->commitTransaction();
        }
        virtual void rollbackTransaction() override {
            this->targetConnection->rollbackTransaction();
        }
        virtual Ref<Statement> preparedStatement(const char *sql) override {
            return this->targetConnection->preparedStatement(sql);
        }
        virtual void close() override {
            this->targetConnection->close();
        }
    protected:
        AbstractWrapperConnection(Ref<Connection> targetConnection, bool closeWhenFinalized) :
            targetConnection(targetConnection), closeWhenFinalized(closeWhenFinalized) {}
        virtual void finalize() override {
            if (this->closeWhenFinalized) {
                this->close();
            }
        }
        virtual Ref<Connection> getTargetConnection() const {
            return this->targetConnection;
        }
    private:
        Ref<Connection> targetConnection;
        bool closeWhenFinalized;
        interface_refcount()
    };

    abstract class AbstractWrapperStatement : extends Object, implements Statement {
    public:
        virtual Ref<Statement> set(int index, int value) override {
            this->targetStatement->set(index, value);
            return this;
        }
        virtual Ref<Statement> set(const char *name, int value) override {
            this->targetStatement->set(name, value);
            return this;
        }
        virtual Ref<Statement> set(int index, __int64_t value) override {
            this->targetStatement->set(index, value);
            return this;
        }
        virtual Ref<Statement> set(const char *name, __int64_t value) override {
            this->targetStatement->set(name, value);
            return this;
        }
        virtual Ref<Statement> set(int index, double value) override {
            this->targetStatement->set(index, value);
            return this;
        }
        virtual Ref<Statement> set(const char *name, double value) override {
            this->targetStatement->set(name, value);
            return this;
        }
        virtual Ref<Statement> set(int index, const char *value) override {
            this->targetStatement->set(index, value);
            return this;
        }
        virtual Ref<Statement> set(const char *name, const char *value) override {
            this->targetStatement->set(name, value);
            return this;
        }
        virtual Ref<ResultSet> executeQuery() override {
            return this->targetStatement->executeQuery();
        }
        virtual int executeUpdate() override {
            return this->targetStatement->executeUpdate();
        }
        virtual void close() override {
            this->targetStatement->close();
        }
    protected:
        AbstractWrapperStatement(Ref<Statement> targetStatement, bool closeWhenFinalized) :
            targetStatement(targetStatement), closeWhenFinalized(closeWhenFinalized) {}
        virtual void finalize() override {
            if (this->closeWhenFinalized) {
                this->close();
            }
        }
        virtual Ref<Statement> getTargetConnection() const {
            return this->targetStatement;
        }
    private:
        Ref<Statement> targetStatement;
        bool closeWhenFinalized;
        interface_refcount()
    };
}
