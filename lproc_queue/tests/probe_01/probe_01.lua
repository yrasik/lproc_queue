#!/usr/bin/lua5.3
--cp1251


local lproc_queue = require ("lproc_queue")


local MSG_ERROR   = 1
local MSG_WARNING = 2
local MSG_INFO    = 3

local DEBUG       = MSG_INFO


local function REPORT(level, fmt, ...)
  if(DEBUG < MSG_ERROR) then
    return
  end

  local status, str = pcall(function (fmt, ...) return string.format(fmt, ...) end, fmt, ...)

  if(status == false) then
    str ='<ERROR in args>'
  end

  local func = debug.getinfo(2).name
  if( func == null) then
    func = 'main'
  end

  local line = debug.getinfo(2).currentline
  str = string.format('%s(): @%s - %s', func, line, str)

  if (level == MSG_INFO) and (DEBUG >= MSG_INFO) then
    print('INFO: '..str)
  elseif (level == MSG_WARNING) and (DEBUG >= MSG_WARNING) then
    print('WARNING: '..str)
  elseif (level == MSG_ERROR) and (DEBUG >= MSG_ERROR) then
    print('ERROR: '..str)
  else

  end

  return
end



local proc_01_pattern = [[
local my_name = 'proc_01'
local channel_in = <channel_in>

print('--------- '..my_name..' -----------')
local ret, message, queue_count

while true do
  lproc_queue.delay_ms(987)

  ret, message, queue_count = lproc_queue.queue_nb_pop(channel_in)
  print('in proc_01: ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))

  while( ret == true and queue_count > 5) do
    ret, message, queue_count = lproc_queue.queue_nb_pop(channel_in)
    print('in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))
  end

end
]]


local proc_02_pattern = [[
local my_name = 'proc_02'
local channel_in = <channel_in>

print('--------- '..my_name..' -----------')
local ret, message, queue_count

while true do
  lproc_queue.delay_ms(987)
  ret, message, queue_count = lproc_queue.queue_nb_pop(channel_in)
  print('in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))
end
]]


local proc_03_pattern = [[
local my_name = 'proc_03'
local destination = '<destination>'
local channel_in = <channel_in>
local channel_out = <channel_out>

print('--------- '..my_name..' -----------')
local ret, message, queue_count
local i = 0

while true do
  lproc_queue.delay_ms(987)
  ret, message, queue_count = lproc_queue.queue_nb_pop(channel_in)
  print('in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))

  lproc_queue.queue_push(channel_out, string.format("{send_from = '%s', send_to = '%s', message = '%d'}", my_name, destination, i))
  i = i + 1
end
]]



------------------- main ---------------------
--REPORT(MSG_INFO, "channel_02 = 0x%016X", channel_02)

REPORT(MSG_INFO, lproc_queue.version())


local debug_info = lproc_queue.debug_mode('0b1000_0000_0000_0000_0000_0000_0001_1110');
REPORT(MSG_INFO, debug_info)


local my_name = 'proc_main'


local channel__to_proc_01 = lproc_queue.queue_create()
local proc_01 = string.gsub(proc_01_pattern, "<channel_in>", string.format("0x%016X", channel__to_proc_01))

local channel__to_proc_02 = lproc_queue.queue_create()
local proc_02 = string.gsub(proc_02_pattern, "<channel_in>", string.format("0x%016X", channel__to_proc_02))

local channel__to_proc_03 = lproc_queue.queue_create()
local channel__from_proc_03 = lproc_queue.queue_create()
local proc_03 = string.gsub(proc_03_pattern, "<channel_in>", string.format("0x%016X", channel__to_proc_03))
proc_03 = string.gsub(proc_03, "<channel_out>", string.format("0x%016X", channel__from_proc_03))
proc_03 = string.gsub(proc_03, "<destination>", my_name)


local ret = lproc_queue.proc_start(proc_01)
local ret = lproc_queue.proc_start(proc_02)
local ret = lproc_queue.proc_start(proc_03)


local ret, message, size
local i = 0

while true do
  lproc_queue.delay_ms(498)
  lproc_queue.queue_push(channel__to_proc_01, string.format("{send_from = '%s', send_to = 'proc_01', message = '%d'}", my_name, i))
  lproc_queue.queue_push(channel__to_proc_02, string.format("{send_from = '%s', send_to = 'proc_02', message = '%d'}", my_name, i))
  lproc_queue.queue_push(channel__to_proc_03, string.format("{send_from = '%s', send_to = 'proc_03', message = '%d'}", my_name, i))

  ret, message, size = lproc_queue.queue_nb_pop(channel__from_proc_03)
  print('in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  size = '..tostring(size))

  i = i + 1
end





