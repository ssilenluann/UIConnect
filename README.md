# UIConnect

#### 介绍
    UIConnect 是一款建议Http服务器框架，适用于Linux操作系统，编译及运行此项目需要g++编译器，
    CMake项目构建工具及BOOST库，
    项目所引入的第三方库文件在server/external中，链接文件在./lib中。

#### 使用说明
    在examples路径中有http_server的示例代码，
    用户只需继承HttpProcessor类，重写handleHttpRequest方法即可完成业务处理，
    然后使用Dispatcher进行url注册即可
