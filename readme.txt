�汾Log
V1.1: 2014-10-28, ��������Ŀ�ѧ��չ�۶��γ�.

��Ϊ������dongle�� LUA��������, ���µ�ַ���Բ鿴
http://www.lewei50.com/u/g/2375

�豸�ܽ�: CH340+STC+DHT22, CP2102+sharp2, DS9490+DS18B20,

׼������
opkg install luasocket_2.0.2-3_ar71xx.ipk
opkg install kmod-usb-serial-ch341_3.3.8-1_ar71xx.ipk --force-depends
opkg install kmod-usb-serial-cp210x_3.3.8-1_ar71xx.ipk --force-depends

���Բ�������:
1. ���� luauart.so, luaxucommon.so, owlua.so -> /usr/lib/lua, �����޸��ļ�����
2. ���� lewei_dongle.lua -> ·�����Լ���Ŀ¼��(�����Լ���Ϊ/xutest), �����޸��ļ�����
3. lua /xutest/lewei_dongle.lua
   ����������������:
--> Hello LUA, qiushui_007 test!
tx(8): 01 03 90 01 00 02 B8 CB
rx(9): 01 03 04 01 BF 00 BC CB 9A
crc16 = 0, 0x0000
h1 = 44.7, t1 = 18.8
lewei50: reqbody = [{"Name":"T1", "Value":"18.8"}, {"Name":"H1", "Value":"44.7"}]
body:1
code:200
status:HTTP/1.1 200 OK

, ds18b20 =  18.5,  65.3
T2 = 18.5
lewei50: reqbody = [{"Name":"T2", "Value":"18.5"}]
body:1
code:200
status:HTTP/1.1 200 OK

PM2.5 = 220.703125, str = AA00E2006E50FF, vo = 1.103515625
lewei50: reqbody = [{"Name":"U1", "Value":"220.703125"}]
body:1
code:200
status:HTTP/1.1 200 OK

4. ʣ�¾���OP�µ����ö�ʱ���е�������, ������Ƕ�����
