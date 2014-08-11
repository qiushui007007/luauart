--[[
lua��uart��������

]]--

print("--> Hello LUA, qiushui_007 test!")

function hex(s)		--��ʾ�ַ�����HEX��
	local s1 = string.gsub(s, "(.)", function (x) return string.format("%02X ", string.byte(x)) end)
	return s1
end

-- load library
local uart = require "luauart"

-- �ײ�����.
--device_name = "/dev/ttyATH0"
device_name = "/dev/ttyUSB0"

-- ��USB-TTL����, ���ȡ��������Ч. ����3���������豸.
invalid_times = 0
while invalid_times < 3 do
	f_vo, pc_ret = uart.sharp2_get(device_name)
	-- �ο�2.3, 1V -- 200ug(0.2mg)
	if f_vo <= 3 then
		k =  200;
	end
	f_data = f_vo * k
	print("str = " .. pc_ret .. ", vo = " .. tostring(f_vo) .. ", PM2.5 = " .. tostring(f_data))

	if f_data == 0.0 then
		invalid_times = invalid_times + 1
		uart.delay_ms(500)
		print("invalid_times = " .. tostring(invalid_times))
		if invalid_times >= 3 then
			uart.reboot()
		end

	else
		invalid_times = 0
		break
	end

end

-- ��ȡ��ȷ����, �ϴ�����
if f_data > 0.0 then
	http = require("socket.http")
  local ltn12 = require "ltn12"

  -- lewei50
  local reqbody = "[{\"Name\":\"U1\", \"Value\":\"" .. tostring(f_data) .. "\"}]"
  local respbody = {}
  local body, code, headers, status = http.request {
      method = "POST",
      url = "http://www.lewei50.com/api/V1/gateway/UpdateSensors/01",
      source = ltn12.source.string(reqbody),
      headers = {
        ["userkey"] = "36be8ff22f794f1e8a0bee3336eef237",
        ["Content-Type"] = "application/x-www-form-urlencoded",
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
