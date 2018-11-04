#include <iostream>
#include <network/HttpClient.h>

using namespace std;
using namespace com_lanjing_cpp_common;
using namespace com_lanjing_cpp_common_network;

void printSomeHtmlCode(Ref<Response> response, int limit) {
    Arr<char> buf = Array<char>::newInstance(limit + 1);
    response->body()->stream().read(buf.unsafe(), limit);
    buf[limit] = '\0';
    const char *html = strstr(buf.unsafe(), "<html"); // Too much empty lines before <html> tags because of server page technology
    if (html != nullptr) {
        cout << html << endl;
    } else {
        cout << buf.unsafe() << endl;
    }
}

int main(int argc, char *argv[]) {
    try_ {
        Ref<HttpClient> httpClient = new_<HttpClient>();
        Ref<Response> response = httpClient
            ->newCall(
                /*
                 * You can also write the request like this:
                 *
                 *    Request::newBuilder()
                 *        ->get()
                 *        ->url("http://www.baidu.com/s?wd=WTF")
                 *        ->build()
                 *
                 * RequestBody is often used by other http methods such as POST, PUT, PATCH,
                 * but specially, FormBody can be used for http GET method too because
                 * the name value pairs of "FormBody" can be consider as the query string of the url automatically
                 */
                Request::newBuilder()
                    ->get(FormBody::newBuilder()->add("wd", "WTF")->build())
                    ->url("http://www.baidu.com/s")
                    ->build()
            )
            ->execute();
        printSomeHtmlCode(response, 1 * 1024);
    } catch_ (Exception, ex) {
        ex->printStackTrace(cerr);
    } end_try
    return 0;
}
