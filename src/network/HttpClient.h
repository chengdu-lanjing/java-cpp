/*
 * 本框架版权归"成都蓝景信息技术有限公司所有", 更多细节请参见LICENSE文件
 *
 * 本框架提供以Java思维来开发C++应用程序的能力, 并对本公司相关项目需要用到的JDK和开源框架的API给出类似实现
 *
 * @author 陈涛
 */
#pragma once

#include "../Auxiliary.h"
#include <curl/curl.h>
#include <vector>

namespace com_lanjing_cpp_common_network {

    using namespace com_lanjing_cpp_common;

    class Request;
    interface RequestBody;
    class FormBody;
    class Call;
    class Response;
    interface ResponseBody;

    class HttpException : extends Exception {
    public:
        HttpException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr)
            : Exception(exception_arg_prefix, message, cause) {}
    };

    class HttpClient : extends Object {
    public:
        Ref<Call> newCall(Ref<Request> request);
    };

    class Call : extends Object {
    public:
        Ref<Response> execute();
    private:
        static size_t readCallback(void *ptr, size_t size, size_t nmemb, void *userData);
        static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *userData);
    private:
        static void throwIfNecessary(CURLcode curlCode, const char *message = nullptr) {
            if (curlCode != CURLE_OK) {
                ostringstream messageBuilder;
                if (message != nullptr && message[0] != '\0') {
                    messageBuilder << message << ", " << curl_easy_strerror(curlCode);
                }
                throw_new(HttpException, messageBuilder.str().c_str());
            }
        }
    private:
        Call(Ref<Request> request);
        struct GlobalController {
            GlobalController() {
                CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
                throwIfNecessary(code, "Cannot initialize curl");
            }
            ~GlobalController() {
                curl_global_cleanup();
            }
            CURL *initCurl() {
                return curl_easy_init();
            }
            void cleanupCurl(CURL *curl) {
                curl_easy_cleanup(curl);
            }
        };
        static GlobalController &globalController() {
            static GlobalController instance;
            return instance;
        }
        class BodyWrapper : extends Object {
        public:
            BodyWrapper(Ref<RequestBody> body) : body(body) {}
            const char *contentType() const;
            int contentLength() const;
            int writeTo(void *ptr, int limit);
        private:
            Ref<RequestBody> body;
            int sourceOffset = 0;
        };
    private:
        Ref<Request> request;
        Ref<Response> response;
        CURL *curl = nullptr;
        curl_slist *slist = nullptr;
        Ref<BodyWrapper> bodyWrapper;
        friend class HttpClient;
        friend class FormBody;
    };

    class Request : extends Object {
    public:
        class Builder;
        static Ref<Builder> newBuilder() {
            return new_internal(Builder);
        }
        const string &getMethod() const {
            return this->method;
        }
        Ref<RequestBody> body() const {
            return this->bdy;
        }
        string getUrl() const {
            return this->url;
        }
        int getHeaderCount() const {
            return this->headers.size();
        }
        string getHeaderName(int index) const {
            return this->headers[index].name;
        }
        string getHeaderValue(int index) const {
            return this->headers[index].value;
        }
        time_t getConnectTimeout() const {
            return this->connectTimeout;
        }
        time_t getTimeout() const {
            return this->timeout;
        }
    private:
        Request() {}
        void header(const char *name, const char *value, bool replace) {
            if (!name || !name[0] || !value || !value[0]) {
                throw_new(IllegalArgumentException, "both name and value must be specified");
            }
            if (replace) {
                auto itr = this->headers.begin();
                while (itr != this->headers.end()) {
                    if (itr->name == name) {
                        this->headers.erase(itr);
                    } else {
                        itr++;
                    }
                }
            }
            this->headers.push_back({name, value});
        }
    private:
        struct Header { string name; string value; };
    public:
        class Builder : extends Object {
        public:
            Ref<Request> build() {
                Ref<Request> request = this->request;
                if (request == nullptr) {
                    throw_new(IllegalStateException, "build can only be invoked once");
                }
                this->request = nullptr;
                return request;
            }
            Ref<Builder> get(Ref<FormBody> body = nullptr) {
                this->request->method = "GET";
                this->request->bdy = body;
                return this;
            }
            Ref<Builder> post(Ref<RequestBody> body) {
                if (body == nullptr) {
                    throw_new(IllegalArgumentException, "body must be specified for HTTP post");
                }
                this->request->method = "POST";
                this->request->bdy = body;
                return this;
            }
            Ref<Builder> put(Ref<RequestBody> body) {
                if (body == nullptr) {
                    throw_new(IllegalArgumentException, "body must be specified for HTTP put");
                }
                this->request->method = "PUT";
                this->request->bdy = body;
                return this;
            }
            Ref<Builder> url(const char *url) {
                this->request->url = url;
                return this;
            }
            Ref<Builder> connectTimeout(long timeout) {
                this->request->connectTimeout = timeout;
                return this;
            }
            Ref<Builder> timeout(long timeout) {
                this->request->timeout = timeout;
                return this;
            }
            Ref<Builder> header(const char *name, const char *value) {
                this->request->header(name, value, true);
                return this;
            }
            Ref<Builder> addHeader(const char *name, const char *value) {
                this->request->header(name, value, false);
                return this;
            }
        protected:
            virtual void initialize() override {
                this->request = new_internal(Request);
            }
        private:
            Builder() {}
        private:
            Ref<Request> request;
            friend class Request;
        };
    private:
        string method;
        Ref<RequestBody> bdy;
        string url;
        vector<Header> headers;
        time_t connectTimeout = 3000;
        time_t timeout = 3000;
        friend class Builder;
        friend class FormBody;
        friend class Call;
    };

    interface RequestBody : implements Interface {

        virtual const char *contentType() const = 0;
        virtual int contentLength() const = 0;
        virtual int writeTo(int sourceOffset, byte *target, int targetLen) const = 0;

        static Ref<RequestBody> create(const char *contentType, string text, int length = -1) {
            if (length == -1) {
                length = text.length();
            }
            Arr<char> arr = Array<char>::newInstance(length);
            System::arraycopy(text.c_str(), arr.unsafe(), length, false);
            return create(contentType, arr, length);
        }

        template <typename E = byte>
        static Ref<RequestBody> create(const char *contentType, Arr<E> arr, int length = -1) {
            class Impl : extends Object, implements RequestBody {
            public:
                Impl(const char *contentType, Arr<E> arr, int length)
                        : theContentType(contentType), arr(arr) {
                    if (arr.elementType() == ArrayElementType::CPP) {
                        throw_new(IllegalArgumentException, "array element type cannot be c++ type");
                    }
                    if (length < -1) {
                        throw_new(IllegalArgumentException, "index cannot be less than -1");
                    }
                    if (length == -1) {
                        this->length = arr.length();
                    } else {
                        this->length = Math::min(length, arr.length());
                    }
                }
                virtual const char *contentType() const override {
                    return this->theContentType.c_str();
                }
                virtual int contentLength() const override {
                    return this->length * sizeof(E);
                }
                virtual int writeTo(int sourceOffset, byte *target, int targetLen) const override {
                    int len = Math::min(this->contentLength() - sourceOffset, targetLen);
                    if (len <= 0) {
                        return 0;
                    }
                    System::arraycopy(
                            reinterpret_cast<byte*>(this->arr.unsafe()) + sourceOffset,
                            target,
                            len,
                            false
                    );
                    return len;
                }
            private:
                string theContentType;
                Arr<byte> arr;
                int length;
            };
            return new_internal(Impl, contentType, arr, length);
        }
    };

    class FormBody : extends Object, implements RequestBody {
    public:
        class Builder : extends Object {
        public:
            Ref<Builder> add(const char *name, int value) {
                ostringstream builder;
                builder << value;
                return this->add(name, builder.str().c_str());
            }
            Ref<Builder> add(const char *name, __int64_t value) {
                ostringstream builder;
                builder << value;
                return this->add(name, builder.str().c_str());
            }
            Ref<Builder> add(const char *name, const char *value) {
                char *escapedName = nullptr;
                char *escapedValue = nullptr;
                defer([=]{
                    globalEscaper().freeEscaped(escapedName);
                    globalEscaper().freeEscaped(escapedValue);
                });
                if (this->builder.tellp()) {
                    this->builder << '&';
                }
                escapedName = globalEscaper().escape(name);
                escapedValue = globalEscaper().escape(value);
                this->builder << escapedName << '=' << escapedValue;\
                return this;
            }
            Ref<FormBody> build() {
                int len = this->builder.tellp();
                Arr<byte> arr = Array<byte>::newInstance(len, false);
                this->builder.seekg(0);
                this->builder.read(arr.unsafe(), len);
                return new_internal(FormBody, arr);
            }
        private:
            Builder() {}
            stringstream builder;
            friend class FormBody;
        };
    public:
        static Ref<Builder> newBuilder() {
            return new_internal(Builder);
        }
        virtual const char *contentType() const override {
            return "application/x-www-form-urlencoded";
        }
        virtual int contentLength() const override {
            return this->content.length();
        }
        virtual int writeTo(int sourceOffset, byte *target, int targetLen) const override {
            int len = Math::min(this->content.length() - sourceOffset, targetLen);
            System::arraycopy(
                    this->content.unsafe() + sourceOffset,
                    target,
                    targetLen,
                    false
            );
            return len;
        }
    private:
        struct GlobalEscaper {
            GlobalEscaper() {
                this->sharedCurl = initCurl();
            }
            ~GlobalEscaper() {
                CURL *curl = this->sharedCurl;
                if (curl != nullptr) {
                    this->sharedCurl = nullptr;
                    cleanupCurl(this->sharedCurl);
                }
            }
            char *escape(const char *value) {
                Mutex::Scope(this->mutex);
                return curl_easy_escape(this->sharedCurl, value, 0);
            }
            void freeEscaped(char *escaped) {
                if (escaped) {
                    curl_free(escaped);
                }
            }
            CURL *sharedCurl;
            Mutex mutex;
        };
        static GlobalEscaper &globalEscaper() {
            static GlobalEscaper instance;
            return instance;
        }
    private:
        FormBody(Arr<byte> content) : content(content) {}
        friend class Builder;
        Arr<byte> content;
    private:
        static CURL *initCurl() {
            return Call::globalController().initCurl();
        }
        static void cleanupCurl(CURL *curl) {
            Call::globalController().cleanupCurl(curl);
        }
        friend class GlobalEscaper;
    };

    interface ResponseBody : implements Interface {
        virtual int contentLength() const = 0;
        virtual basic_istream<char> &stream() = 0;
        virtual string str() const = 0;
    };

    class Response : extends Object {
    public:
        int code() const {
            return this->cd;
        }
        Ref<ResponseBody> body() const {
            return this->bdy;
        }

    private:
        Response() {
            this->bdy = new_internal(BodyImpl);
        }

    private:
        class BodyImpl : extends Object, implements ResponseBody {
        public:
            stringstream builder;
            virtual int contentLength() const override {
                return this->builder.str().length();
            }
            virtual basic_istream<char> &stream() override {
                return this->builder;
            }
            virtual string str() const override {
                return this->builder.str();
            }
            interface_refcount()
            friend class Call;
        };
        Ref<BodyImpl> bdy;
        int cd = 404;
        friend class Call;
    };



    inline Ref<Call> HttpClient::newCall(Ref<Request> request) {
        return new_internal(Call, request);
    }

    inline Call::Call(Ref<Request> request) : request(request) {
        this->response = new_internal(Response);
    }

    inline Ref<Response> Call::execute() {

        CURLcode code;
        defer([=] {
            this->bodyWrapper = nullptr;
            curl_slist *slist = this->slist;
            if (slist != nullptr) {
                this->slist = nullptr;
                curl_slist_free_all(slist);
            }
            CURL *curl = this->curl;
            if (curl != nullptr) {
                this->curl = nullptr;
                curl_easy_cleanup(curl);
            }
        });

        this->curl = globalController().initCurl();

        if (this->request->body() != nullptr) {
            const string &method = this->request->getMethod();
            if (method == "POST" || method == "PUT") {
                this->bodyWrapper = new_internal(BodyWrapper, this->request->body());
                code = curl_easy_setopt(
                        this->curl,
                        CURLOPT_READFUNCTION,
                        readCallback);
                throwIfNecessary(code, "Cannot set request body");
                code = curl_easy_setopt(
                        this->curl,
                        CURLOPT_READDATA,
                        this);
                throwIfNecessary(code, "Cannot set request body");
            }
            if (method == "POST") {
                code = curl_easy_setopt(
                        this->curl,
                        CURLOPT_POSTFIELDSIZE,
                        this->bodyWrapper->contentLength());
                throwIfNecessary(code, "Cannot set post data size");
            }
        }

        string url = this->request->getUrl();
        if (this->request->getMethod() == "GET" && this->request->body() != nullptr) {
            int lastSeparatorIndex = -1;
            for (int i = url.length() - 1;i >= 0; --i) {
                char c = url[i];
                if (c == '?' || c == '&') {
                    lastSeparatorIndex = i;
                    break;
                }
            }
            ostringstream builder;
            builder << url;
            if (lastSeparatorIndex == -1) {
                builder << '?';
            } else if (lastSeparatorIndex < (int)url.length() - 1) {
                builder << '&';
            }
            char buf[64];
            int sourceOffset = 0;
            while (true) {
                int len = this->request->body()->writeTo(
                        sourceOffset, buf, sizeof(buf)/sizeof(buf[0]) - 1
                );
                if (len <= 0) {
                    break;
                }
                sourceOffset += len;
                buf[len] = '\0';
                builder << buf;
            }
            this->bodyWrapper = nullptr;
            code = curl_easy_setopt(
                    this->curl,
                    CURLOPT_URL,
                    builder.str().c_str());
            throwIfNecessary(code, "Cannot set url");
        } else {
            code = curl_easy_setopt(
                    this->curl,
                    CURLOPT_URL,
                    url.c_str());
            throwIfNecessary(code, "Cannot set url");
        }

        if (this->request->getMethod() == "POST") {
            code = curl_easy_setopt(this->curl, CURLOPT_POST, 1);
            throwIfNecessary(code, "Cannot set post method");
        } else if (this->request->getMethod() == "PUT") {
            code = curl_easy_setopt(this->curl, CURLOPT_PUT, 1);
            throwIfNecessary(code, "Cannot set put method");
        } else if (this->request->getMethod() == "DELETE") {
            code = curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            throwIfNecessary(code, "Cannot set delete method");
        }

        bool additionalContentType = this->bodyWrapper != nullptr;
        int headerCount = this->request->getHeaderCount();
        for (int i = 0; i < headerCount; i++) {
            ostringstream builder;
            builder << this->request->getHeaderName(i) << ':' << this->request->getHeaderValue(i);
            this->slist = curl_slist_append(this->slist, builder.str().c_str());
            if (additionalContentType && this->request->getHeaderName(i) == "Content-Type") {
                additionalContentType = false;
            }
        }
        if (additionalContentType) {
            ostringstream builder;
            builder << "Content-Type" << ':' << this->bodyWrapper->contentType();
            this->slist = curl_slist_append(this->slist, builder.str().c_str());
        }
        if (this->slist != nullptr) {
            code = curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, this->slist);
            throwIfNecessary(code, "Cannot set headers");
        }

        if (this->request->getConnectTimeout() > 0) {
            code = curl_easy_setopt(
                    this->curl,
                    CURLOPT_CONNECTTIMEOUT_MS,
                    this->request->getConnectTimeout());
            throwIfNecessary(code, "Cannot set connect timeout");
        }

        if (this->request->getTimeout() > 0) {
            code = curl_easy_setopt(
                    this->curl,
                    CURLOPT_TIMEOUT_MS,
                    this->request->getTimeout());
            throwIfNecessary(code, "Cannot set timeout");
        }

        code = curl_easy_setopt(
                this->curl,
                CURLOPT_WRITEFUNCTION,
                writeCallback);
        throwIfNecessary(code, "Cannot set request write function");
        code = curl_easy_setopt(
                this->curl,
                CURLOPT_WRITEDATA,
                this);
        throwIfNecessary(code, "Cannot set request write data");

        code = curl_easy_perform(this->curl);
        throwIfNecessary(code, "Cannot set request write data");

        code = curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &response->cd);
        throwIfNecessary(code, "Cannot set request write data");

        return this->response;
    }

    inline size_t Call::readCallback(void *ptr, size_t size, size_t nmemb, void *userData) {
        Ref<Call> call = reinterpret_cast<Call*>(userData);
        return call->bodyWrapper->writeTo(ptr, size * nmemb);
    }

    inline size_t Call::writeCallback(void *ptr, size_t size, size_t nmemb, void *userData) {
        Ref<Call> call = reinterpret_cast<Call*>(userData);
        int len = size * nmemb;
        call->response->bdy->builder.write(reinterpret_cast<char*>(ptr), len);
        return len;
    }

    inline const char *Call::BodyWrapper::contentType() const {
        return this->body->contentType();
    }

    inline int Call::BodyWrapper::contentLength() const {
        return this->body->contentLength();
    }

    inline int Call::BodyWrapper::writeTo(void *ptr, int limit) {
        int len = this->body->writeTo(
                this->sourceOffset,
                reinterpret_cast<byte*>(ptr),
                limit
        );
        this->sourceOffset += len;
        return len;
    }
}

