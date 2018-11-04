/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

#include "../Functional.h"
#include "../Auxiliary.h"
#include <ctime>
#include <iomanip>
#include <thread>
#include <vector>
#include <set>
#include <map>
#include <fstream>

#define declare_logger(className) \
private: \
    static com_lanjing_cpp_common_log::Logger &logger() { \
        static com_lanjing_cpp_common_log::Logger logger = \
                com_lanjing_cpp_common_log::Logger::of<className>(); \
        return logger; \
    }

namespace com_lanjing_cpp_common_log {

    using namespace com_lanjing_cpp_common;

    enum LogLevel {
        DBG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    class Configuration;

    struct Logger {
    public:
        static Logger of(const char *tag) {
            return Logger(tag);
        };
        template <typename C> static Logger of() {
            return Logger(Object::className(typeid(C)));
        }

        void debug(const char *message) {
            this->log(LogLevel::DBG, message);
        }
        void debug(Ref<Exception> exception, const char *message) {
            this->log(LogLevel::DBG, exception, message);
        }
        template <typename ...Args> void debug(const char *message, Args ...args) {
            this->log(LogLevel::DBG, message, args...);
        }
        template <typename ...Args> void debug(
                Ref<Exception> exception,
                const char *message,
                Args ...args) {
            this->log(LogLevel::DBG, exception, message, args...);
        }

        void info(const char *message) {
            this->log(LogLevel::INFO, message);
        }
        void info(Ref<Exception> exception, const char *message) {
            this->log(LogLevel::INFO, exception, message);
        }
        template <typename ...Args> void info(const char *message, Args ...args) {
            this->log(LogLevel::INFO, message, args...);
        }
        template <typename ...Args> void info(
                Ref<Exception> exception,
                const char *message,
                Args ...args) {
            this->log(LogLevel::INFO, exception, message, args...);
        }

        void warn(const char *message) {
            this->log(LogLevel::WARN, message);
        }
        void warn(Ref<Exception> exception, const char *message) {
            this->log(LogLevel::WARN, exception, message);
        }
        template <typename ...Args> void warn(const char *message, Args ...args) {
            this->log(LogLevel::WARN, message, args...);
        }
        template <typename ...Args> void warn(
                Ref<Exception> exception,
                const char *message,
                Args ...args) {
            this->log(LogLevel::WARN, exception, message, args...);
        }

        void error(const char *message) {
            this->log(LogLevel::ERROR, message);
        }
        void error(Ref<Exception> exception, const char *message) {
            this->log(LogLevel::ERROR, exception, message);
        }
        template <typename ...Args> void error(const char *message, Args ...args) {
            this->log(LogLevel::ERROR, message, args...);
        }
        template <typename ...Args> void error(
                Ref<Exception> exception,
                const char *message,
                Args ...args) {
            this->log(LogLevel::ERROR, exception, message, args...);
        }

        void fatal(const char *message) {
            this->log(LogLevel::FATAL, message);
        }
        void fatal(Ref<Exception> exception, const char *message) {
            this->log(LogLevel::FATAL, exception, message);
        }
        template <typename ...Args> void fatal(const char *message, Args ...args) {
            this->log(LogLevel::FATAL, message, args...);
        }
        template <typename ...Args> void fatal(
                Ref<Exception> exception,
                const char *message,
                Args ...args) {
            this->log(LogLevel::FATAL, exception, message, args...);
        }

        void log(LogLevel level, const char *message) {
            this->log(level, static_cast<Exception*>(nullptr), message);
        }
        void log(LogLevel level, Ref<Exception> exception, const char *message);
        template <typename ...Args> void log(LogLevel level, const char *message, Args ...args) {
            log(level, static_cast<Exception*>(nullptr), message, args...);
        }
        template <typename ...Args> void log(
                LogLevel level,
                Ref<Exception> exception,
                const char *message,
                Args ...args);

    private:
        static string resolveTrimMarginChar(const string &message, char trimMarginChar) {
            bool trimRequired = false;
            for (char ch : message) {
                if (ch == trimMarginChar) {
                    trimRequired = true;
                    break;
                }
            }
            if (!trimRequired) {
                return message;
            }
            ostringstream trimedBuilder;
            ostringstream wsBuilder;
            for (char ch : message) {
                bool whitespace = ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
                if (whitespace) {
                    wsBuilder << ch;
                } else {
                    if (ch != trimMarginChar) {
                        trimedBuilder << wsBuilder.str() << ch;
                    }
                    wsBuilder.str("");
                }
            }
            return trimedBuilder.str();
        }
        template <typename A, typename ...MA>
        static void resolveArguments(ostream &out, const char *messageCharArr, A arg, MA ...moreArgs) {
            const char *placeHolder = messageCharArr;
            while (*placeHolder) {
                if (placeHolder[0] == '{' && placeHolder[1] == '}') {
                    break;
                }
                placeHolder++;
            }
            for (const char *ch = messageCharArr; ch < placeHolder; ch++) {
                out << *ch;
            }
            out << arg;
            const char *moreMessage = *placeHolder ? placeHolder + 2 : placeHolder;
            resolveArguments(out, moreMessage, moreArgs...);
        }
        static void resolveArguments(ostream &out, const char *messageCharArr) {
            out << messageCharArr;
        }

    private:
        Logger(const string &tag): tag(tag) {}
        string tag;
    };

    interface Layout : implements Interface {
        virtual string header() const { return ""; }
        virtual string footer() const { return ""; }
        virtual string text(
                const string &tag,
                LogLevel level,
                const string &message,
                Ref<Exception> ex) const = 0;
    };

    interface Appender : implements Interface {

        virtual Nullable<LogLevel> getMinLevel() const = 0;

        virtual Nullable<LogLevel> getMaxLevel() const = 0;

        virtual void append(const string &text) = 0;

        /**
         * Returns the layout of this appender.
         *
         * If appender does not have its own layout,
         * uses the layout of configuration,
         *
         * If the configuration does not have its own layout,
         * look up the layout from the parent configuration to root configuration
         *
         * If no layout can be found in all the objects, SimpleLayout will be used
         */
        virtual Ref<Layout> getLayout() const = 0;

        virtual void addLayoutWillChangeListener(Ref<RefConsumer<Appender>> listener) = 0;
        virtual void removeLayoutWillChangeListener(Ref<RefConsumer<Appender>> listener) = 0;

        virtual void addIOTargetWillChangeListener(Ref<RefConsumer<Appender>> listener) = 0;
        virtual void removeIOTargetWillChangeListener(Ref<RefConsumer<Appender>> listener) = 0;
    };

    // 书写PatternLayout太麻烦了,先写个SimpleLayout将就吧
    class SimpleLayout : extends Object, implements Layout {
    public:
        virtual string text(
                const string &tag,
                LogLevel level,
                const string &message,
                Ref<Exception> ex) const override {
            time_t nowTime = time(nullptr);
            tm* nowTm = localtime(&nowTime);
            ostringstream oss;
            oss
            << (1900 + nowTm->tm_year)
            << '-'
            << (nowTm->tm_mon + 1)
            << '-'
            << nowTm->tm_mday
            << ' '
            << nowTm->tm_hour
            << ':'
            << nowTm->tm_min
            << ':'
            << nowTm->tm_sec
            << ' '
            << tag
            << ' ';
            switch (level) {
            case LogLevel::DBG:
                oss << "-DEBUG";
                break;
            case LogLevel::INFO:
                oss << "- INFO";
                break;
            case LogLevel::WARN:
                oss << "- WARN";
                break;
            case LogLevel::ERROR:
                oss << "-ERROR";
                break;
            case LogLevel::FATAL:
                oss << "-FATAL";
                break;
            default:
                break;
            }
            oss << ": " << message << endl;
            if (ex != nullptr) {
                ex->printStackTrace(oss);
            }
            return oss.str();
        }
    };

    class XmlLayout : extends Object, implements Layout {
    public:
        virtual string header() const override {
            return "<?xml version = \"1.0\"?>\n<log-records>";
        }
        virtual string footer() const override {
            return "</log-records>";
        }
        virtual string text(
                const string &tag,
                LogLevel level,
                const string &message,
                Ref<Exception> ex) const override {
            ostringstream oss;
            oss << "\t<log-record>\n\t\t<timestamp>";
            appendTimestamp(oss);
            oss << "</timestamp>\n\t\t<tag>";
            appendContent(oss, tag);
            oss << "</tag>\n\t\t<level>" << levelString(level) << "</level>\n\t\t<message>";
            appendContent(oss, message);
            oss << "</message>\n";
            if (ex != nullptr) {
                oss << "\t\t<exception>\n";
                appendException(oss, ex, 3);
                oss << "\t\t</exception>\n";
            }
            oss << "\t</log-record>\n";
            return oss.str();
        }
    private:
        static void appendTimestamp(ostream &out) {
            time_t nowTime = time(nullptr);
            tm* nowTm = localtime(&nowTime);
            ostringstream oss;
            out
            << (1900 + nowTm->tm_year)
            << '-'
            << (nowTm->tm_mon + 1)
            << '-'
            << nowTm->tm_mday
            << ' '
            << nowTm->tm_hour
            << ':'
            << nowTm->tm_min
            << ':'
            << nowTm->tm_sec;
        }
        static void appendContent(ostream &out, const string& content) {
            for (char ch : content) {
                if (ch == '<') {
                    out << "&lt;";
                } else if(ch == '>') {
                    out << "&gt;";
                } if (ch == '&') {
                    out << "&amp;";
                } else {
                    out << ch;
                }
            }
        }
        static const char *levelString(LogLevel level) {
            switch (level) {
            case LogLevel::DBG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARN:
                return "WARN";
            case LogLevel::ERROR:
                return "ERROR";
            case LogLevel::FATAL:
                return "FATAL";
            }
            return "--UNKNOWN--";
        }
        static void appendException(ostream &out, Ref<Exception> ex, int indent) {
            appendTabs(out, indent);
            out << "<fileName>" << ex->getFileName() << "<fileName>\n";
            appendTabs(out, indent);
            out << "<lineNumber>" << ex->getLineNumber() << "<lineNumber>\n";
            appendTabs(out, indent);
            out << "<className>" << Object::className(ex) << "</className>\n";
            appendTabs(out, indent);
            out << "<message>";
            appendContent(out, ex->getMessage());
            out << "</message>\n";
            if (ex->getCause() != nullptr) {
                appendTabs(out, indent);
                out << "<caused-by>\n";
                appendException(out, ex->getCause(), indent + 1);
                appendTabs(out, indent);
                out << "</caused-by>\n";
            }
        }
        static void appendTabs(ostream &out, int indent) {
            for (int i = indent; i > 0; --i) {
                out << '\t';
            }
        }
    };

    template <typename A> // A extends AbstractAppender<A>
    abstract class AbstractAppender : extends Object, implements Appender {
    public:
        virtual Ref<Layout> getLayout() const override {
            return this->layout;
        }
        Ref<A> setLayout(Ref<Layout> layout) {
            {
                ReadWriteLock::ReadingScope readingScope(this->readWriteLock);
                if (this->layout == layout) {
                    return static_cast<A*>(this);
                }
            }

            {
                ReadWriteLock::WritingScope writingScope(this->readWriteLock);
                if (this->layout != layout) {
                    this->layout = layout;
                }
                if (this->layoutWillChangedListener != nullptr) {
                    this->layoutWillChangedListener(this);
                }
                return static_cast<A*>(this);
            }
        }
        Ref<A> setMinLevel(Nullable<LogLevel> minLevel = nullptr) {
            ReadWriteLock::WritingScope writingScope(this->readWriteLock);
            this->minLevel = minLevel;
            return static_cast<A*>(this);
        }
        Ref<A> setMaxLevel(Nullable<LogLevel> maxLevel = nullptr) {
            ReadWriteLock::WritingScope writingScope(this->readWriteLock);
            this->maxLevel = maxLevel;
            return static_cast<A*>(this);
        }
        virtual Nullable<LogLevel> getMinLevel() const override {
            ReadWriteLock::ReadingScope readingScope(this->readWriteLock);
            return this->minLevel;
        }
        virtual Nullable<LogLevel> getMaxLevel() const override {
            ReadWriteLock::ReadingScope readingScope(this->readWriteLock);
            return this->maxLevel;
        }
        virtual void addLayoutWillChangeListener(Ref<RefConsumer<Appender>> listener) override {
            ReadWriteLock::WritingScope writingScope(this->readWriteLock);
            this->layoutWillChangedListener += listener;
        }
        virtual void removeLayoutWillChangeListener(Ref<RefConsumer<Appender>> listener) override {
            ReadWriteLock::WritingScope writingScope(this->readWriteLock);
            this->layoutWillChangedListener -= listener;
        }
        virtual void addIOTargetWillChangeListener(Ref<RefConsumer<Appender>> listener) override {
            ReadWriteLock::WritingScope writingScope(this->readWriteLock);
            this->ioTargetWillChangedListener += listener;
        }
        virtual void removeIOTargetWillChangeListener(Ref<RefConsumer<Appender>> listener) override {
            ReadWriteLock::WritingScope writingScope(this->readWriteLock);
            this->ioTargetWillChangedListener -= listener;
        }
    protected:
        AbstractAppender() {}
        ReadWriteLock readWriteLock;
        Ref<Layout> layout;
        Ref<RefConsumer<Appender>> layoutWillChangedListener;
        Ref<RefConsumer<Appender>> ioTargetWillChangedListener;
        Nullable<LogLevel> minLevel;
        Nullable<LogLevel> maxLevel;
        interface_refcount()
    };

    class ConsoleAppender : extends AbstractAppender<ConsoleAppender> {
    public:
        ConsoleAppender(ostream &console = clog) : console(&console) {}
        Ref<ConsoleAppender> setConsole(ostream &console) {
            this->console = &console;
            return this;
        }
        virtual void append(const string &text) override {
            (*this->console) << text;
        }
    private:
        ostream *console;
    };

    class FileAppender : extends AbstractAppender<FileAppender> {
    public:
        FileAppender(const string &path, bool appendMode = false)
                : path(path), appendMode(appendMode), stream(nullptr) {}
        virtual void append(const string &text) override {
            this->getStream() << text;
        }
        void flush() {
            Mutex::Scope scope(mutex);
            if (this->stream != nullptr) {
                this->stream->flush();
            }
        }
    protected:
        virtual void finalize() override {
            this->closeStream();
        }
        ofstream &getStream() {
            if (stream == nullptr) {
                Mutex::Scope scope(mutex);
                if (stream == nullptr) {
                    stream = new ofstream(path, ios::out | (appendMode ? ios::app : ios::trunc));
                }
            }
            return *stream;
        }
        Ref<FileAppender> setPath(const string &path) {
            Mutex::Scope scope(this->mutex);
            if (this->path != path) {
                this->path = path;
                this->closeStream();
            }
            return this;
        }
        Ref<FileAppender> setAppendMode(bool appendMode) {
            Mutex::Scope scope(this->mutex);
            if (this->appendMode != appendMode) {
                this->appendMode = appendMode;
                this->closeStream();
            }
            return this;
        }
    private:
        void closeStream() {
            ofstream *s = this->stream;
            if (s != nullptr) {
                this->stream = nullptr;
                delete s;
            }
        }
    private:
        Mutex mutex;
        string path;
        bool appendMode;
        ofstream *stream;
    };

    class RootConfiguration;

    class Configuration : extends Object {
    public:
        static Ref<RootConfiguration> root();
        static Ref<Configuration> of(const char *tagPrefix);
        static Ref<Configuration> of(const string &tagPrefix);
        Ref<Configuration> setLevel(Nullable<LogLevel> optionalLevel) {
            ReadWriteLock::WritingScope writingScope(this->rwl);
            if (this->declaredLevel != optionalLevel) {
                this->declaredLevel = optionalLevel;
                this->sharedState->modify();
            }
            return this;
        }
        Ref<Configuration> setLayout(Ref<Layout> layout) {
            ReadWriteLock::WritingScope writingScope(this->rwl);
            if (this->declaredLayout != layout) {
                this->declaredLayout = layout;
                this->sharedState->modify();
            }
            return this;
        }
        Ref<Configuration> clearAppenders() {
            ReadWriteLock::WritingScope writingScope(this->rwl);
            if (!this->declaredAppenders.empty()) {
                this->declaredAppenders.clear();
                this->sharedState->modify();
            }
            return this;
        }
        Ref<Configuration> addAppender(Ref<Appender> appender) {
            if (appender != nullptr) {
                ReadWriteLock::WritingScope writingScope(this->rwl);
                this->declaredAppenders.push_back(appender);
                this->sharedState->modify();
            }
            return this;
        }
        bool isEnabled(LogLevel level) {
            return this->getConfiguredInfo()->level <= level;
        }
        void log(
                const string &tag,
                LogLevel level,
                const string &message,
                Ref<Exception> ex) {
            Ref<ConfiguredInfo> info = this->getConfiguredInfo();
            if (info->level <= level) {
                map<Ref<Layout>, string> cacheMap;
                for (auto appender : info->appenders) {
                    Ref<Layout> appliedLayout = appender->getLayout();
                    if (appliedLayout == nullptr) {
                        appliedLayout = info->layout;
                    }
                    this->sharedState->useAppender(appender, appliedLayout);
                    Nullable<LogLevel> minLevel = appender->getMinLevel();
                    Nullable<LogLevel> maxLevel = appender->getMaxLevel();
                    if ((minLevel == nullptr || level >= minLevel) &&
                            (maxLevel == nullptr || level <= maxLevel)) {
                        string text = layoutText(
                                appliedLayout,
                                tag,
                                level,
                                message,
                                ex,
                                cacheMap);
                        appender->append(text);
                    }
                }
            }
        }
    protected:
        class SharedState : extends Object {
        public:
            void modify() {
                while (true) {
                    if (++this->versionAtomic != 0) {
                        break;
                    }
                }
            }
            int version() {
                return this->versionAtomic;
            }
            void useAppender(Ref<Appender> appender, Ref<Layout> layout) {
                {
                    ReadWriteLock::ReadingScope readingScope(this->rwl);
                    auto itr = this->workingAppenderMap.find(appender);
                    if (itr != this->workingAppenderMap.end() && itr->second == layout) {
                        return;
                    }
                }
                {
                    ReadWriteLock::WritingScope writingScope(this->rwl);
                    auto itr = this->workingAppenderMap.find(appender);
                    if (itr == this->workingAppenderMap.end()) {
                        appender->append(layout->header());
                        appender->append("\n");
                        this->workingAppenderMap[appender] = layout;
                        appender->addLayoutWillChangeListener(
                                RefConsumer<Appender>::weakOf(this, &SharedState::unuseAppender)
                        );
                        appender->addIOTargetWillChangeListener(
                                RefConsumer<Appender>::weakOf(this, &SharedState::unuseAppender)
                        );
                    } else if (layout != itr->second) {
                        appender->append(itr->second->footer());
                        appender->append("\n");
                        appender->append(layout->header());
                        appender->append("\n");
                        this->workingAppenderMap[appender] = layout;
                    }
                }
            }
            void unuseAppender(Ref<Appender> appender) {
                {
                    ReadWriteLock::ReadingScope readingScope(this->rwl);
                    if (this->workingAppenderMap.find(appender) == this->workingAppenderMap.end()) {
                        return;
                    }
                }
                {
                    ReadWriteLock::WritingScope writingScope(this->rwl);
                    auto itr = this->workingAppenderMap.find(appender);
                    if (itr != this->workingAppenderMap.end()) {
                        Ref<Layout> layout = itr->second;
                        this->workingAppenderMap.erase(itr);
                        appender->append(layout->footer());
                    }
                }
            }
        protected:
            virtual void finalize() {
                ReadWriteLock::WritingScope writingScope(this->rwl);
                for (auto &pair : this->workingAppenderMap) {
                    pair.first->append(pair.second->footer());
                    pair.first->append("\n");
                }
            }
        private:
            AtomicInteger versionAtomic;
            map<Ref<Appender>, Ref<Layout>> workingAppenderMap;
            ReadWriteLock rwl;
        };
        Configuration(Ref<SharedState> sharedState, const string &name)
            : sharedState(sharedState), version(0), name(name) {}
        Ref<Configuration> child(const string &tag, bool autoCreate = false) {
            vector<string> names = splitTag(tag);
            Ref<Configuration> configuration = this;
            for (auto name : names) {
                Ref<Configuration> tmpConfiguration = configuration->directChild(name, autoCreate);
                if (tmpConfiguration == nullptr) {
                    break;
                }
                configuration = tmpConfiguration;
            }
            return configuration;
        }
    private:
        class ConfiguredInfo : extends Object {
        public:
            ConfiguredInfo(
                    Ref<ConfiguredInfo> parentInfo,
                    Nullable<LogLevel> declaredLogLevel,
                    Ref<Layout> declaredLayout,
                    const vector<Ref<Appender>> &declaredAppenders) {
                if (declaredLogLevel != nullptr) {
                    this->level = declaredLogLevel;
                } else if (parentInfo != nullptr) {
                    this->level = parentInfo->level;
                } else {
                    this->level = LogLevel::INFO;
                }
                if (declaredLayout != nullptr) {
                    this->layout = declaredLayout;
                } else if (parentInfo != nullptr) {
                    this->layout = parentInfo->layout;
                } else {
                    this->layout = defaultLayout();
                }
                set<Ref<Appender>> appendSet;
                if (!declaredAppenders.empty()) {
                    for (auto appender : declaredAppenders) {
                        if (appendSet.insert(appender).second) {
                            this->appenders.push_back(appender);
                        }
                    }
                } else if (parentInfo != nullptr) {
                    for (auto appender : parentInfo->appenders) {
                        if (appendSet.insert(appender).second) {
                            this->appenders.push_back(appender);
                        }
                    }
                }
                if (this->appenders.empty()) {
                    this->appenders.push_back(defaultAppender());
                }
            }
            LogLevel level;
            Ref<Layout> layout;
            vector<Ref<Appender>> appenders;
            static Ref<Layout> defaultLayout() {
                static Ref<Layout> instance = new_<SimpleLayout>();
                return instance;
            }
            static Ref<Appender> defaultAppender() {
                static Ref<Appender> instance = new_<ConsoleAppender>();
                return instance;
            }
        };
    private:
        Ref<Configuration> directChild(const string &name, bool autoCreate) {
            Ref<Configuration> child;
            {
                ReadWriteLock::ReadingScope readingScope(this->rwl);
                auto itr = this->childConfigurationMap.find(name);
                if (itr != this->childConfigurationMap.end()) {
                    child = this->childConfigurationMap[name];
                }
            }
            if (child == nullptr) {
                ReadWriteLock::WritingScope writingScope(this->rwl);
                child = new_internal(Configuration, this->sharedState, name);
                this->childConfigurationMap[name] = child;
                child->parentConfigurationRef = this;
            }
            return child;
        }
        Ref<ConfiguredInfo> getConfiguredInfo() {
            Ref<ConfiguredInfo> info;
            Ref<Configuration> parentConfiguration;
            bool refresh = this->sharedState->version() != this->version;
            {
                ReadWriteLock::ReadingScope readingScope(this->rwl);
                parentConfiguration = this->parentConfigurationRef.get();
                if (!refresh) {
                    info = this->configuredInfo;
                }
            }
            if (info == nullptr) {
                Ref<ConfiguredInfo> parentInfo;
                if (parentConfiguration != nullptr) {
                    parentInfo = parentConfiguration->getConfiguredInfo();
                }
                {
                    ReadWriteLock::WritingScope writingScope(this->rwl);
                    if (refresh) {
#ifdef DEBUG
                        if (this->configuredInfo != nullptr) {
                            clog
                            << "The computed configured info of log configuration '"
                            << this->name
                            << "' must be refresh because the configuration has been changed";
                        }
#endif //DEBUG
                    } else {
                        info = this->configuredInfo;
                    }
                    if (info == nullptr) {
                        info = new_<ConfiguredInfo>(
                                parentInfo,
                                this->declaredLevel,
                                this->declaredLayout,
                                this->declaredAppenders
                        );
                        this->configuredInfo = info;
                        this->version = this->sharedState->version();
                    }
                }
            }
            return info;
        }
        static vector<string> splitTag(const string &tag) {
            vector<string> vec;
            ostringstream builder;
            int size = tag.size();
            for (int i = 0; i < size; i++) {
                char c = tag[i];
                switch (c) {
                case ':':
                    if (i < size - 1 && tag[i + 1] == ':') {
                        collectStringToVector(builder, vec);
                        i++;
                    }
                    break;
                case '_':
                case '.':
                    collectStringToVector(builder, vec);
                    break;
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    break;
                default:
                    builder << c;
                    break;
                }
            }
            collectStringToVector(builder, vec);
            return vec;
        }
        static void collectStringToVector(ostringstream &stream, vector<string> &vec) {
            string str = stream.str();
            if (!str.empty()) {
                vec.push_back(str);
            }
            stream.str("");
        }
        static string layoutText(
                Ref<Layout> layout,
                const string &tag,
                LogLevel level,
                const string &message,
                Ref<Exception> ex,
                map<Ref<Layout>, string> &cacheMap) {
            auto itr = cacheMap.find(layout);
            if (itr == cacheMap.end()) {
                string text = layout->text(tag, level, message, ex);
                cacheMap[layout] = text;
                return text;
            }
            return cacheMap[layout];
        }
    private:
        Ref<SharedState> sharedState;
        int version;
        string name;
        ReadWriteLock rwl;
        map<string, Ref<Configuration>> childConfigurationMap;
        WeakRef<Configuration> parentConfigurationRef;
        Nullable<LogLevel> declaredLevel;
        Ref<Layout> declaredLayout;
        vector<Ref<Appender>> declaredAppenders;
        Ref<ConfiguredInfo> configuredInfo;
    };

    class RootConfiguration : extends Configuration {
    public:
        char getTrimMarginChar() const {
            return this->trimMarginChar;
        }
        Ref<RootConfiguration> setTrimMarginChar(char trimMarginChar) {
            this->trimMarginChar = trimMarginChar;
            return this;
        }
    private:
        RootConfiguration() : Configuration(new_<SharedState>(), ""), trimMarginChar('|') {}
        char trimMarginChar;
        friend class Configuration;
    };

    inline Ref<RootConfiguration> Configuration::root() {
        static Ref<RootConfiguration> instance = new_internal(RootConfiguration);
        return instance;
    }

    inline Ref<Configuration> Configuration::of(const char *tagPrefix) {
        if (tagPrefix == nullptr || tagPrefix[0] == '\0') {
            return Configuration::root();
        }
        return Configuration::root()->child(tagPrefix, true);
    }

    inline Ref<Configuration> Configuration::of(const string &tagPrefix) {
        if (tagPrefix.empty()) {
            return Configuration::root();
        }
        return Configuration::root()->child(tagPrefix, true);
    }

    inline void Logger::log(LogLevel level, Ref<Exception> exception, const char *message) {
        Ref<Configuration> configuration = Configuration::of(this->tag);
        if (configuration->isEnabled(level)) {
            string resolvedMessage = this->resolveTrimMarginChar(
                    message,
                    Configuration::root()->getTrimMarginChar()
            );
            configuration->log(tag, level, resolvedMessage, exception);
        }
    }

    template <typename ...Args> void Logger::log(
            LogLevel level,
            Ref<Exception> exception,
            const char *message,
            Args ...args) {
        Ref<Configuration> configuration = Configuration::of(this->tag);
        if (configuration->isEnabled(level)) {
            string resolvedMessage = this->resolveTrimMarginChar(
                    message,
                    Configuration::root()->getTrimMarginChar()
            );
            ostringstream oss;
            this->resolveArguments(oss, resolvedMessage.c_str(), args...);
            resolvedMessage = oss.str();
            Configuration::of(this->tag)->log(tag, level, resolvedMessage, exception);
        }
    }
}
