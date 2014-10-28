--[[
乐为物联的dongle的 LUA测试例程, 如下地址可以查看
http://www.lewei50.com/u/g/2375

设备总结: CH340+STC+DHT22, CP2102+sharp2, DS9490+DS18B20,

准备工作
opkg install luasocket_2.0.2-3_ar71xx.ipk
opkg install kmod-usb-serial-ch341_3.3.8-1_ar71xx.ipk --force-depends
opkg install kmod-usb-serial-cp210x_3.3.8-1_ar71xx.ipk --force-depends

测试操作如下:
1. 拷贝 luauart.so, luaxucommon.so, owlua.so -> /usr/lib/lua, 无需修改文件属性
2. 拷贝 BH4TDV.lua -> 路由上自己的目录下(如我自己的为/xutest), 无需修改文件属性
3. lua /xutest/BH4TDV.lua

版本Log
V1.0: 2014-10-28, 经过深入的科学发展观而形成.

]]--

print("--> Hello LUA, qiushui_007 test!")
-- load library
local xucommon = require "luaxucommon"
local uart = require "luauart"
local http = require("socket.http")
local ltn12 = require "ltn12"

str_ip = xucommon.get_local_ip("auto")

to_id = "qiushui_007@foxmail.com"
function mail_tx(to_id, subject, content)
	xucommon.mail_tx(to_id, subject, content)
end

function delay_ms(delay1)
	xucommon.delay_ms(delay1)
end

function crc8_get(arg)
	crc = xucommon.crc8_get(arg)
	return crc
end

function crc16_get(arg)
	crc = xucommon.crc16_get(arg)
	return crc
end

function crc32_get(arg)
	crc = xucommon.crc32_get(arg)
	return crc
end

function hex(s)		--显示字符串的HEX码
	local s1 = string.gsub(s, "(.)", function (x) return string.format("%02X ", string.byte(x)) end)
	return s1
end

---
function http_lewei50_request(reqbody)
	local gateid = "01"
	local userkey = "36be8ff22f794f1e8a0bee3336eef237"

  --local reqbody = "[{\"Name\":\"" .. deviceid .. "\", \"Value\":\"" .. tostring(data) .. "\"}]"
  local respbody = {}
  local body, code, headers, status = http.request {
      method = "POST",
      url = "http://www.lewei50.com/api/V1/gateway/UpdateSensors/" .. gateid,
      source = ltn12.source.string(reqbody),
      headers = {
        ["userkey"] = userkey,
        --["Content-Type"] = "application/x-www-form-urlencoded",
        ["content-length"] = string.len(reqbody),
        ["Connection"] = "Close"
      },
      sink = ltn12.sink.table(respbody)
  }

  print("lewei50: " .. "reqbody = " .. reqbody)
  print('body:' .. tostring(body))
  print('code:' .. tostring(code))
  print('status:' .. tostring(status) .. "\n")
end

function http_lewei50(deviceid, data)
  local reqbody = "[{\"Name\":\"" .. deviceid .. "\", \"Value\":\"" .. tostring(data) .. "\"}]"
  http_lewei50_request(reqbody)
end

function http_lewei50_2(deviceid, data, deviceid1, data1)
  local reqbody = "[{\"Name\":\"" .. deviceid .. "\", \"Value\":\"" .. tostring(data) .. "\"}, {\"Name\":\"" .. deviceid1 .. "\", \"Value\":\"" .. tostring(data1) .. "\"}]"
  http_lewei50_request(reqbody)
end

--- 读取BH4TDV之温湿度 start -------------------------------------------------------------------------
function bh4tdv_get()
	--device_name = "/dev/ttyATH0"
	local device_name = "/dev/ttyUSB0"

	local portnum = uart.open(device_name)
	--print(portnum)

	-- 先赋特定值
	f_h1 = 0.0
	f_t1 = 0.0

	if portnum >= 0 then
		uart.buad_set(portnum, 0)
		uart.flush(portnum)
		--必须>=200
		timeout_ms = 200
		uart.timeout(timeout_ms)

		--读取湿度和温度
		str1 = string.char(0x01, 0x03, 0x90, 0x01, 0x00, 0x02, 0xb8, 0xcb)
		len = string.len(str1)
		print("tx(" .. len .. "): " .. hex(str1))
		len_ret, str_ret = uart.tx_rx(portnum, str1, len)
		if len_ret == 9 then
			print("rx(" .. len_ret .. "): " .. hex(str_ret))
			--01 03 04 01 DD 00 F9 AB B7
			--- 判断CRC16
			--str_tmp = string.char(0x31, 0x32, 0x33, 0x34, 0x35, 0x36)
			crc16 = crc16_get(str_ret)
			print("crc16 = " .. tostring(crc16) .. ", 0x" .. string.format("%04X", crc16))

			f_h1 = 0.0
			f_t1 = 0.0
			if crc16 == 0x00 then
				h1_h = string.byte(str_ret, 4)
				h1_l = string.byte(str_ret, 5)
				f_h1 = (h1_h * 256 + h1_l) / 10

				t1_h = string.byte(str_ret, 6)
				t1_l = string.byte(str_ret, 7)
				f_t1 = (t1_h * 256 + t1_l) / 10
			end
			print("h1 = " .. tostring(f_h1) .. ", t1 = " .. tostring(f_t1) )

		end

		--最后一定要close
		uart.close(portnum)
	end

	return f_h1, f_t1
end

f_h1, f_t1 = bh4tdv_get()
http_lewei50_2("T1", f_t1, "H1", f_h1)

--- 温度的最小和最大值
t_min = 10.0
t_max = 35.0
if f_t1 < t_min or f_t1 > t_max then
  local str_time = os.date("%Y") .. "-" .. os.date("%m") .. "-" .. os.date("%d") .. " " .. os.date("%X")
	local content = str_time .. ", " .. str_ip .. ", " .. tostring(f_t1) .. "\nwww.lewei50.com/u/g/2375"
	subject = "BH4DTV read error"
	mail_tx(to_id, subject, content)
end
--- 读取BH4TDV之温湿度 end -------------------------------------------------------------------------

--- 同一网关不能更新太快, 否则可能封IP
delay_ms(1000)

--- 读取DS18B20的温度 start -------------------------------------------------------------------------
function ds18b20_get()
	local owlua = require "owlua"

	f_t2 = 0.0
	local device_name = "USB"
	local portnum = owlua.open(device_name)
	if portnum > 0 then
		f_t2 = owlua.ds18b20_get_one(portnum)
		print("T2 = " .. tostring(f_t2))
		owlua.close(portnum)
	end

	return f_t2
end

f_t2 = ds18b20_get()
http_lewei50("T2", f_t2)

--- 差值大于特定范围则报告
f_mark = 2.0
--f_t2 = f_t1 - f_mark - 0.1  --only test
f_diff = f_t1 - f_t2
if f_diff < -f_mark or f_diff > f_mark then
	local str_time = os.date("%Y") .. "-" .. os.date("%m") .. "-" .. os.date("%d") .. " " .. os.date("%X")
	local content = str_time .. ", " .. str_ip .. ", BH4TDV: " .. tostring(f_t1) .. ", 18B20: " .. tostring(f_t2)
	print(content)
	subject = "BH4DTV diff bigger than mark"
	mail_tx(to_id, subject, content)
end
--- 读取DS18B20的温度 End -------------------------------------------------------------------------

delay_ms(1000)

--- read sharp2 start ------------------------------------------------------------------------
function sharp2_get()
	local device_name = "/dev/ttyUSB1"

	f_vo, pc_ret = uart.sharp2_get(device_name)
	if f_vo > 0.0 then
		if f_vo <= 3 then
				k =  200
		end
		f_data = f_vo * k
	else
		f_data = 0.0
	end

	str_data = "PM2.5 = " .. tostring(f_data) .. ", str = " .. pc_ret .. ", vo = " .. tostring(f_vo)
	print(str_data)

	return f_data, str_data
end

f_u1, str_u1 = sharp2_get()

t_min = 0.0
t_max = 560.0
--f_u1 = 0.0	-- only test!!!
if f_u1 > t_min and f_u1 < t_max then
	http_lewei50("U1", f_u1)

else
	local str_time = os.date("%Y") .. "-" .. os.date("%m") .. "-" .. os.date("%d") .. " " .. os.date("%X")
	local content = str_time .. ", " .. str_ip .. ", " .. str_u1
	print(content)
	subject = "sharp2 data error"
	mail_tx(to_id, subject, content)

end
--- read sharp2 end ------------------------------------------------------------------------