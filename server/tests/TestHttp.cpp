#include "protocol/http/HttpResponseParser.h"
#include "protocol/http/HttpRequestParser.h"
#include "log/Logger.h"

static Logger::ptr g_logger = LOG_NAME("system");
const char test_requst_data[] = "POST / HTTP/1.1\r\n"
    "HOST: www.baidu.com\r\n"
    "Content-Length: 11\r\n\r\n"
    "hello world";

void test_request()
{
    HttpRequestParser parser;
    std::string tmp = test_requst_data;
    size_t s = parser.execute(const_cast<char*>(&tmp[0]), tmp.size());

    LOG_INFO(g_logger) << "execute rt = " << s << ", has error: " << parser.hasError()
        << ", is finish: " << parser.isFinished() 
        << ", content length: " << parser.getContentLength();

    parser.getData()->dump(std::cout);
}

const char test_response_data[] = "HTTP/1.0 200 OK\r\n"
    "Accept-Ranges: bytes\r\n"
    "Cache-Control: no-cache\r\n"
    "Content-Length: 9508\r\n"
    "Content-Security-Policy: frame-ancestors 'self' https://chat.baidu.com https://fj-chat.baidu.com https://hba-chat.baidu.com https://hbe-chat.baidu.com https://njjs-chat.baidu.com https://nj-chat.baidu.com https://hna-chat.baidu.com https://hnb-chat.baidu.com;\r\n"
    "Content-Type: text/html\r\n\r\n"
    "Date: Thu, 27 Apr 2023 16:06:54 GMT\r\n"
    "P3p: CP=\" OTI DSP COR IVA OUR IND COM \"\r\n"
    "P3p: CP=\" OTI DSP COR IVA OUR IND COM \"\r\n"
    "Pragma: no-cache\r\n"
    "Server: BWS/1.1\r\n"
    "Set-Cookie: BAIDUID=DB9080021126EEF0B3EDC02F71A5065D:FG=1; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com\r\n"
    "Set-Cookie: BIDUPSID=DB9080021126EEF0B3EDC02F71A5065D; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com\r\n"
    "Vary: Accept-Encoding\r\n"
    "X-Ua-Compatible: IE=Edge,chrome=1\r\n"
    "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge,chrome=1\"><meta content=\"always\" name=\"referrer\"><meta name=\"description\" content=\"全球领先的中文搜索引擎、致力于让网民更便捷地获取信息，找到所求。百度超过千亿的中文网页数据库，可以瞬间找到相关的搜索结果。\"><link rel=\"shortcut icon\" href=\"//www.baidu.com/favicon.ico\" type=\"image/x-icon\"><link rel=\"search\" type=\"application/opensearchdescription+xml\" href=\"//www.baidu.com/content-search.xml\" title=\"百度搜索\"><title>百度一下，你就知道</title><style type=\"text/css\">body{margin:0;padding:0;text-align:center;background:#fff;height:100%}html{overflow-y:auto;color:#000;overflow:-moz-scrollbars;height:100%}body,input{font-size:12px;font-family:\"PingFang SC\",Arial,\"Microsoft YaHei\",sans-serif}a{text-decoration:none}a:hover{text-decoration:underline}img{border:0;-ms-interpolation-mode:bicubic}input{font-size:100%;border:0}body,form{position:relative;z-index:0}#wrapper{height:100%}#head_wrapper.s-ps-islite{padding-bottom:370px}#head_wrapper.s-ps-islite .s_form{position:relative;z-index:1}#head_wrapper.s-ps-islite .fm{position:absolute;bottom:0}#head_wrapper.s-ps-islite .s-p-top{position:absolute;bottom:40px;width:100%;height:181px}#head_wrapper.s-ps-islite #s_lg_img{position:static;margin:33px auto 0 auto;left:50%}#form{z-index:1}.s_form_wrapper{height:100%}#lh{margin:16px 0 5px;word-spacing:3px}.c-font-normal{font:13px/23px Arial,sans-serif}.c-color-t{color:#222}.c-btn,.c-btn:visited{color:#333!important}.c-btn{display:inline-block;overflow:hidden;font-family:inherit;font-weight:400;text-align:center;vertical-align:middle;outline:0;border:0;height:30px;width:80px;line-height:30px;font-size:13px;border-radius:6px;padding:0;background-color:#f5f5f6;cursor:pointer}.c-btn:hover{background-color:#315efb;color:#fff!important}a.c-btn{text-decoration:none}.c-btn-mini{height:24px;width:48px;line-height:24px}.c-btn-primary,.c-btn-primary:visited{color:#fff!important}.c-btn-primary{background-color:#4e6ef2}.c-btn-primary:hover{background-color:#315efb}a:active{color:#f60}#wrapper{position:relative;min-height:100%}#head{padding-bottom:100px;text-align:center}#wrapper{min-width:1250px;height:100%;min-height:600px}#head{position:relative;padding-bottom:0;height:100%;min-height:600px}.s_form_wrapper{height:100%}.quickdelete-wrap{position:relative}.tools{position:absolute;right:-75px}.s-isindex-wrap{position:relative}#head_wrapper.head_wrapper{width:auto}#head_wrapper{position:relative;height:40%;min-height:314px;max-height:510px;width:1000px;margin:0 auto}#head_wrapper .s-p-top{height:60%;min-height:185px;max-height:310px;position:relative;z-index:0;text-align:center}#head_wrapper input{outline:0;-webkit-appearance:none}#head_wrapper .s_btn_wr,#head_wrapper .s_ipt_wr{display:inline-block;zoom:1;background:0 0;vertical-align:top}#head_wrapper .s_ipt_wr{position:relative;width:546px}#head_wrapper .s_btn_wr{width:108px;height:44px;position:relative;z-index:2}#head_wrapper .s_ipt_wr:hover #kw{border-color:#a7aab5}#head_wrapper #kw{width:512px;height:16px;padding:12px 16px;font-size:16px;margin:0;vertical-align:top;outline:0;box-shadow:none;border-radius:10px 0 0 10px;border:2px solid #c4c7ce;background:#fff;color:#222;overflow:hidden;box-sizing:content-box}#head_wrapper #kw:focus{border-color:#4e6ef2!important;opacity:1}#head_wrapper .s_form{width:654px;height:100%;margin:0 auto;text-align:left;z-index:100}#head_wrapper .s_btn{cursor:pointer;width:108px;height:44px;line-height:45px;padding:0;background:0 0;background-color:#4e6ef2;border-radius:0 10px 10px 0;font-size:17px;color:#fff;box-shadow:none;font-weight:400;border:none;outline:0}#head_wrapper .s_btn:hover{background-color:#4662d9}#head_wrapper .s_btn:active{background-color:#4662d9}#head_wrapper .quickdelete-wrap{position:relative}#s_top_wrap{position:absolute;z-index:99;min-width:1000px;width:100%}.s-top-left{position:absolute;left:0;top:0;z-index:100;height:60px;padding-left:24px}.s-top-left .mnav{margin-right:31px;margin-top:19px;display:inline-block;position:relative}.s-top-left .mnav:hover .s-bri,.s-top-left a:hover{color:#315efb;text-decoration:none}.s-top-left .s-top-more-btn{padding-bottom:19px}.s-top-left .s-top-more-btn:hover .s-top-more{display:block}.s-top-right{position:absolute;right:0;top:0;z-index:100;height:60px;padding-right:24px}.s-top-right .s-top-right-text{margin-left:32px;margin-top:19px;display:inline-block;position:relative;vertical-align:top;cursor:pointer}.s-top-right .s-top-right-text:hover{color:#315efb}.s-top-right .s-top-login-btn{display:inline-block;margin-top:18px;margin-left:32px;font-size:13px}.s-top-right a:hover{text-decoration:none}#bottom_layer{width:100%;position:fixed;z-index:302;bottom:0;left:0;height:39px;padding-top:1px;overflow:hidden;zoom:1;margin:0;line-height:39px;background:#fff}#bottom_layer .lh{display:inline;margin-right:20px}#bottom_layer .lh:last-child{margin-left:-2px;margin-right:0}#bottom_layer .lh.activity{font-weight:700;text-decoration:underline}#bottom_layer a{font-size:12px;text-decoration:none}#bottom_layer .text-color{color:#bbb}#bottom_layer a:hover{color:#222}#bottom_layer .s-bottom-layer-content{text-align:center}</style></head><body></body></html>Connection closed by foreign host.";

void test_response()
{
    HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size());

    LOG_INFO(g_logger) << "execute rt = " << s << ", has error: " << parser.hasError()
    << ", is finish: " << parser.isFinished() 
    << ", content length: " << parser.getContentLength();

    parser.getData()->dump(std::cout);
}

int main(int argc, char** argv)
{
    test_request();

    std::cout << "=====================" << std::endl << std::endl << std::endl;

    test_response();
    return 0;
}