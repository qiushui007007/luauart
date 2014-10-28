准备工作
opkg install luasocket_2.0.2-3_ar71xx.ipk
opkg install kmod-usb-serial-ch341_3.3.8-1_ar71xx.ipk --force-depends

测试操作如下:
1. 将 luauart.so, luaxucommon.so 拷贝到/usr/lib/lua, 无需修改文件属性
2. 将 BH4TDV.lua 拷贝到路由上自己的目录下(如我自己的为/xutest), 无需修改文件属性
3. lua /xutest/BH4TDV.lua
   运行命令看到结果如下:
--> Hello LUA, qiushui_007 test!
tx: 8, 01 03 90 01 00 02 B8 CB
rx: 9, 01 03 04 01 C6 00 E0 1A 7A
h1 = 45.4, t1 = 22.4
lewei50: reqbody = [{"Name":"T1", "Value":"22.4"}]
body:1
code:200
status:HTTP/1.1 200 OK

lewei50: reqbody = [{"Name":"H1", "Value":"45.4"}]
body:1
code:200
status:HTTP/1.1 200 OK

4. 剩下就是OP下的配置定时运行的事情了, 这个你们都懂的
