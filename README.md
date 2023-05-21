# UIConnect

#### 项目简介

    UIConnect 是一款简易HTTP服务器框架，基于reactor及one thread one loop模型，
    用户只需继承业务处理类，重写业务处理方法，并在主loop启动前进行url注册即可搭建简单HTTP服务器。

    项目主要分为日志模块，配置模块，线程模块，网络模块，http模块，定时器模块
    在dev分支中还额外封装有协程及调度模块。

    其中：
    日志模块支持流式和格式化写入；
    配置模块基于yaml-cpp完成；
    网络模块主要针对socket及epoll进行封装，并将socket的生命周期进行分层，主要分为：
        TcpChannel层： 进行epoll事件控制；
        TcpConnection层：连接及分包处理，socket的消息缓存使用链式存储代替连续存储，支持动态扩容
        TcpSession层：消息转发及生命周期控制，比如超时控制，读写速度控制等；
    http模块基于mongrel库封装了http请求及响应，并也继承了tcp的分层策略，支持url的模糊及精准匹配；
    定时器模块基于std::set并重写比较器，实现大顶堆模式，定时任务基于过期时间进行升序排列；

#### 使用说明
    适用于Linux操作系统，编译此项目需要g++编译器，CMake项目构建工具及BOOST库，
    项目所引入的第三方库文件在server/external中，链接文件在./lib中。

#### 示例说明
    在sever/examples路径中有http_server的示例代码，
    用户只需继承HttpProcessor类，重写handleHttpRequest方法即可完成业务处理，
    然后使用Dispatcher进行url注册即可。
