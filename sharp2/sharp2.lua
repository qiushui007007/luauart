--[[
lua的WIFI温湿度测试例程, 如下地址可以查看
http://www.lewei50.com/u/g/2375
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


function hex(s)		--显示字符串的HEX码
	local s1 = string.gsub(s, "(.)", function (x) return string.format("%02X ", string.byte(x)) end)
	return s1
end

---
function http_lewei50(deviceid, data)
	local gateid = "01"
	local userkey = "36be8ff22f794f1e8a0bee3336eef237"

  local reqbody = "[{\"Name\":\"" .. deviceid .. "\", \"Value\":\"" .. tostring(data) .. "\"}]"
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

function http_lewei50_2(deviceid, data, deviceid1, data1)
	local gateid = "01"
	local userkey = "36be8ff22f794f1e8a0bee3336eef237"

  --local reqbody = "[{\"Name\":\"" .. deviceid .. "\", \"Value\":\"" .. tostring(data) .. "\"}]"
  local reqbody = "[{\"Name\":\"" .. deviceid .. "\", \"Value\":\"" .. tostring(data) .. "\"}, {\"Name\":\"" .. deviceid1 .. "\", \"Value\":\"" .. tostring(data1) .. "\"}]"
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

--- read sharp2 ------------------------------------------------------------------------
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

	local str_data = "PM2.5 = " .. tostring(f_data) .. ", str = " .. pc_ret .. ", vo = " .. tostring(f_vo)
	print(str_data)

	return f_data
end

f_u1 = sharp2_get()

t_min = 0.0
t_max = 560.0
if f_u1 > t_min and f_u1 < t_max then
	http_lewei50("U1", f_u1)
end

