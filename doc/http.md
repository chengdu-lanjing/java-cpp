# HTTP #

java-cpp对[curl](https://curl.haxx.se/)的HTTP客户端进行了类似OkHttp风格的封装，所以使用此功能前请务必先安装[curl](https://curl.haxx.se/)。

API位于network/HttpClient.h中, 这部分功能比较简单，且配套demo有演示，不再赘述，仅声明一点

com_lanjing_cpp_common_network::RequestBody常常作为POST, PUT, PATCH等http方法向服务端上传图像，XML, json等流数据的手段，但其用于表示简单name-value配对集合的派生类com_lanjing_cpp_common_network::FormBody的对象却可以作为get方法的可选参数，此时，FormBody内部的的name-value配对集合会自动转移并拼接在url末尾，免除用户转义和拼接之苦。更多详细信息，请参见demo


----------

[<上一篇：日志](./logging.md) | [首页](https://github.com/chengdu-lanjing/java-cpp) | [下一篇：数据库之CDBC>](./database.md)
