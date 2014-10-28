--[[
lua的uart测试例程
#define PARMSET_9600                   0x00
#define PARMSET_19200                  0x02
#define PARMSET_57600                  0x04
#define PARMSET_115200                 0x06
#define PARMSET_2400                   0x08
#define COM1WR703N "/dev/ttyATH0"

#将RX和TX短接后, 测试结果
root@OpenWrt:/xutest# lua xuuart.lua
--> Hello LUA, qiushui_007 test!
tx: 8, 00 F2 F3 F4 05 06 07 08
rx: 8, 00 F2 F3 F4 05 06 07 08

]]--

print("--> Hello LUA, qiushui_007 test!")

function hex(s)		--显示字符串的HEX码
	local s1 = string.gsub(s, "(.)", function (x) return string.format("%02X ", string.byte(x)) end)
	return s1
end

-- load library
local uart = require "luauart"
--package.loadlib("./luauart.so", "luaopen_array")()

-- 底层库测试.
--device_name = "/dev/ttyATH0"
device_name = "/dev/ttyUSB0"
portnum = uart.open(device_name)
--print(portnum)

if portnum >= 0 then
	--uart.buad_set(portnum, 6)
	uart.buad_set(portnum, 0)
	uart.flush(portnum)
	--wifi温湿度, 必须>=200
	timeout_ms = 200
	uart.timeout(timeout_ms)

	--str1 = "1234567890"
	--str1 = string.char(0xf1, 0xf2, 0xf3, 0xf4, 0x05, 0x06, 0x07, 0x08)
	--str1 = string.char(0x00, 0xf2, 0xf3, 0xf4, 0x05, 0x06, 0x07, 0x08)
	--wifi温湿度, 读取湿度和温度
	str1 = string.char(0x01, 0x03, 0x90, 0x01, 0x00, 0x02, 0xb8, 0xcb)
	len = string.len(str1)
	print("tx: " .. len .. ", " .. hex(str1))
	len_ret, str_ret = uart.tx_rx(portnum, str1, len)
	if len_ret > 0 then
		print("rx: " .. len_ret .. ", " .. hex(str_ret))
	end

	--最后一定要close
	uart.close(portnum)

end
