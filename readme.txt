׼������
opkg install luasocket_2.0.2-3_ar71xx.ipk
opkg install kmod-usb-serial-ch341_3.3.8-1_ar71xx.ipk --force-depends

���Բ�������:
1. �� luauart.so, luaxucommon.so ������/usr/lib/lua, �����޸��ļ�����
2. �� BH4TDV.lua ������·�����Լ���Ŀ¼��(�����Լ���Ϊ/xutest), �����޸��ļ�����
3. lua /xutest/BH4TDV.lua
   ����������������:
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

4. ʣ�¾���OP�µ����ö�ʱ���е�������, ������Ƕ�����
