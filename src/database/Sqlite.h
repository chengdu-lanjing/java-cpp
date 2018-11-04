/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

#include <sqlite3.h>
#include "Database.h"
#include "../log/Logger.h"
#include <unistd.h>
#include <chrono>
#include <thread>

namespace com_lanjing_cpp_common_database {

    using namespace std;
    using namespace com_lanjing_cpp_common;
    using namespace com_lanjing_cpp_common_database_spi;

    class Sqlite3Driver : extends Driver {
    private:
        class StatementImpl;
        class ResultSetImpl;
        class ConnectionImpl : extends AbstractConnection {
            declare_logger(ConnectionImpl)
        public:
            ConnectionImpl(const char *url, const map<string, string> &properties) :
                AbstractConnection(url, properties), db(nullptr) {}
            virtual void onBeginTransaction() override {
                this->preparedStatement("begin")->executeUpdate();
            }
            virtual void onCommitTransaction() override {
                this->preparedStatement("commit")->executeUpdate();
            }
            virtual void onRollbackTransaction() override {
                this->preparedStatement("rollback")->executeUpdate();
            }
            virtual Ref<Statement> preparedStatement(const char *sql) override;
        protected:
            virtual void onOpen() override {
                const char *dbFilePath = this->url.c_str() + strlen(urlPrefix());
                if (dbFilePath[0] == '\0') {
                    throw_new(SQLException, "sqlite requires file path")
                }
                int ret = sqlite3_open_v2(
                    dbFilePath,
                    &db,
                    SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                    nullptr
                );
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                    << "Cannot open the database connection '"
                    << this->url
                    << "' because "
                    << sqlite3_errmsg(this->db);
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                ret = sqlite3_busy_handler(this->db, busyHandler, nullptr);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                    << "Cannot open the database connection '"
                    << this->url
                    << "' because busy handler cannot be configured";
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
            }
            virtual void onClose() override {
                int ret = sqlite3_close_v2(this->db);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder << "Cannot close the database connection '" << this->url << '\'';
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
            }
        private:
            static int busyHandler(void *data, int retryCount) {
                int sleepMillis = Math::min((retryCount + 1) * 100, 3000);
                logger().warn(
                        "sqlite is busy(retried count: {}), retry after {} milliseconds",
                        retryCount,
                        sleepMillis);
                this_thread::sleep_for(chrono::milliseconds(sleepMillis));
                return 1;
            }
            sqlite3 *db;
            friend class StatementImpl;
        };
        class StatementImpl : extends AbstractStatement {
        public:
            StatementImpl(Ref<ConnectionImpl> con, const char *sql) :
                AbstractStatement(con, sql), stmt(nullptr) {}
            virtual Ref<Statement> set(int index, int value) override {
                this->checkState();
                int ret = sqlite3_bind_int(this->stmt, index, value);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter["
                        << index
                        << "] of the statement '"
                        << this->sql <<
                        "' to be "
                        << value;
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<Statement> set(const char *name, int value) override {
                this->checkState();
                int ret = sqlite3_bind_int(this->stmt, this->indexOf(name), value);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter['"
                        << name
                        << "'] of the statement '"
                        << this->sql <<
                        "' to be "
                        << value;
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<Statement> set(int index, __int64_t value) override {
                this->checkState();
                int ret = sqlite3_bind_int64(this->stmt, index, value);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter["
                        << index
                        << "] of the statement '"
                        << this->sql <<
                        "' to be "
                        << value;
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<Statement> set(const char *name, __int64_t value) override {
                this->checkState();
                int ret = sqlite3_bind_int64(this->stmt, this->indexOf(name), value);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter['"
                        << name
                        << "'] of the statement '"
                        << this->sql <<
                        "' to be "
                        << value;
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<Statement> set(int index, double value) override {
                this->checkState();
                int ret = sqlite3_bind_double(this->stmt, index, value);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter["
                        << index
                        << "] of the statement '"
                        << this->sql <<
                        "' to be "
                        << value;
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<Statement> set(const char *name, double value) override {
                this->checkState();
                int ret = sqlite3_bind_double(this->stmt, this->indexOf(name), value);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter['"
                        << name
                        << "'] of the statement '"
                        << this->sql <<
                        "' to be "
                        << value;
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<Statement> set(int index, const char *value) override {
                this->checkState();
                int ret = sqlite3_bind_text(this->stmt, index, value, -1, nullptr);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter["
                        << index
                        << "] of the statement '"
                        << this->sql <<
                        "' to be '"
                        << value
                        << '\'';
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<Statement> set(const char *name, const char *value) override {
                this->checkState();
                int ret = sqlite3_bind_text(this->stmt, this->indexOf(name), value, -1, nullptr);
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot set the parameter['"
                        << name
                        << "'] of the statement '"
                        << this->sql <<
                        "' to be '"
                        << value
                        << '\'';
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
                return this;
            }
            virtual Ref<ResultSet> executeQuery() override;
            virtual int executeUpdate() override {
                this->checkState();
                Ref<ConnectionImpl> con = static_cast<ConnectionImpl*>(this->getParent().get());
                if (con == nullptr) {
                    throw_new(IllegalStateException, "The connection of this statement is closed");
                }
                sqlite3_step(this->stmt);
                return sqlite3_changes(con->db);
            }
        protected:
            virtual void onOpen() override {
                Ref<ConnectionImpl> con = static_cast<ConnectionImpl*>(this->getParent().get());
                int ret = sqlite3_prepare_v2(
                    con->db,
                    this->sql.c_str(),
                    -1,
                    &stmt,
                    nullptr
                );
                if (ret != SQLITE_OK) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "Cannot open the statement for the sql' "
                        << this->sql
                        << ", the sqlite error message is: "
                        << sqlite3_errmsg(con->db);
                    throw_new(SQLException, ret, messageBuilder.str().c_str());
                }
            }
            virtual void onClose() override {
                sqlite3_stmt *st = this->stmt;
                if (st != nullptr) {
                    this->stmt = nullptr;
                    int ret = sqlite3_finalize(st);
                    if (ret != SQLITE_OK) {
                        Ref<ConnectionImpl> con = static_cast<ConnectionImpl*>(this->getParent().get());
                        ostringstream messageBuilder;
                        messageBuilder
                            << "Cannot close the statement for the sql '"
                            << this->sql
                            << "', the reason is: "
                            << sqlite3_errmsg(con->db);
                        throw_new(SQLException, ret, messageBuilder.str().c_str());
                    }
                }
            }
        private:
            int indexOf(const char *name) {
                int index = sqlite3_bind_parameter_index(this->stmt, name);
                if (index == 0) {
                    ostringstream messageBuilder;
                    messageBuilder
                        << "There is no parameter whose name is '"
                        << name
                        << "' in the sql '"
                        << this->sql
                        << '\'';
                    throw_new(IllegalArgumentException, messageBuilder.str().c_str());
                }
                return index;
            }
        private:
            sqlite3_stmt *stmt;
            friend class ResultSetImpl;
        };
        class ResultSetImpl : extends AbstractResultSet {
        public:
            ResultSetImpl(Ref<StatementImpl> stmt) :AbstractResultSet(stmt), stmt(nullptr) {}
            virtual bool next() override {
                this->checkState();
                return sqlite3_step(this->stmt) == SQLITE_ROW;
            }
            virtual int getInt(int col) override {
                this->checkState();
                return sqlite3_column_int(stmt, col - 1);
            }
            virtual __int64_t getInt64(int col) override {
                this->checkState();
                return sqlite3_column_int64(stmt, col - 1);
            }
            virtual double getDouble(int col) override {
                this->checkState();
                return sqlite3_column_double(stmt, col - 1);
            }
            virtual string getString(int col) override {
                this->checkState();
                return reinterpret_cast<const char*>(sqlite3_column_text(stmt, col - 1));
            }
        protected:
            virtual void onOpen() override {
                Ref<StatementImpl> stmt = static_cast<StatementImpl*>(this->getParent().get());
                this->stmt = stmt->stmt;
            }
            virtual void onClose() override {
                // Do nothing
            }
            sqlite3_stmt *stmt;
        };
    public:
        virtual bool acceptsURL(const char *url) override {
            size_t prefixLen = strlen(urlPrefix());
            if (strlen(url) < prefixLen) {
                return false;
            }
            return memcmp(url, urlPrefix(), prefixLen) == 0;
        }
        virtual Ref<Connection> connect(const char *url, const map<string, string> &properties) override {
            return new_<ConnectionImpl>(url, properties);
        }
    private:
        static const char *urlPrefix() {
            return "cdbc:sqlite:";
        }
    };

    inline Ref<Statement> Sqlite3Driver::ConnectionImpl::preparedStatement(const char *sql) {
        this->checkState();
        return new_<StatementImpl>(this, sql);
    }
    inline Ref<ResultSet> Sqlite3Driver::StatementImpl::executeQuery() {
        this->checkState();
        return new_<ResultSetImpl>(this);
    }
}
