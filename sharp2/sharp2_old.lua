--[[
lua的sharp2测试例程

]]--

print("--> Hello LUA, qiushui_007 test!")

-- load library
local uart = require "luauart"
local xucommon = require "luaxucommon"

function hex(s)		--显示字符串的HEX码
	local s1 = string.gsub(s, "(.)", function (x) return string.format("%02X ", string.byte(x)) end)
	return s1
end

-- 底层库测试.
--device_name = "/dev/ttyATH0"
device_name = "/dev/ttyUSB1"

-- 若USB-TTL当机, 则读取的数据无效. 超过N次则重启设备.
invalid_times = 0
retry_times = 10
while invalid_times < retry_times do
	f_vo, pc_ret = uart.sharp2_get(device_name)

	if f_vo < 0.0 then
		invalid_times = invalid_times + 1
		xucommon.delay_ms(1000)
		print("invalid_times = " .. tostring(invalid_times))

		-- 异常时发送Email以方便统计
		str_time = os.date("%Y") .. "-" .. os.date("%m") .. "-" .. os.date("%d") .. " " .. os.date("%X")
		str_ip = xucommon.get_local_ip("auto")

		if invalid_times < retry_times then
			to_id = "qiushui_007@foxmail.com"
			subject = "sharp2 read error"
			content = str_time .. ", local_ip: " .. str_ip .. "\nwww.lewei50.com/u/g/2375" .. "\nwww.yeelink.net/devices/4420"
			xucommon.mail_tx(to_id, subject, content)

		else
			to_id = "qiushui_007@foxmail.com"
			subject = "sharp2 reboot"
			content = str_time .. ", local_ip: " .. str_ip
			xucommon.mail_tx(to_id, subject, content)

			xucommon.reboot()	--重启, 你懂的
		end

	else
		invalid_times = 0
		-- 参考2.3, 1V -- 200ug(0.2mg)
		if f_vo <= 3 then
			k =  200;
		end
		f_data = f_vo * k
		str_data = "PM2.5 = " .. tostring(f_data) .. ", str = " .. pc_ret .. ", vo = " .. tostring(f_vo)
		print(str_data)

		break
	end

end

-- 获取正确数据, 上传到云
if f_data > 0.0 and f_data < 560 then
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


  -- yeelink
  local reqbody = "{\"value\":" .. tostring(f_data) .. "}"
  local respbody = {}
  local body, code, headers, status = http.request {
      method = "POST",
      url = "http://api.yeelink.net/v1.0/device/4420/sensor/21920/datapoints",
      source = ltn12.source.string(reqbody),
      headers = {
        ["U-ApiKey"] = "729d1ba15b26b6a48f4807ef3f2f4df4",
        --["Content-Type"] = "application/x-www-form-urlencoded",
        ["content-length"] = string.len(reqbody),
        ["Connection"] = "Close"
      },
      sink = ltn12.sink.table(respbody)
  }

  print("yeelink: " .. "reqbody = " .. reqbody)
  print('body:' .. tostring(body))
  print('code:' .. tostring(code))
  print('status:' .. tostring(status) .. "\n")

end
