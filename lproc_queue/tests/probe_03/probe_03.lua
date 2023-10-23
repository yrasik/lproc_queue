#!/usr/bin/lua5.3


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




local proc_XX_pattern = [[
local my_name = '<my_name>'
local dest_name = 'dest_name'

local channel_in = <channel_in>
local channel_out = <channel_out>
local semaphore_in = <semaphore_in>
local semaphore_out = <semaphore_out>

print('--------- '..my_name..' -----------')
local ret, message, queue_count, queue_space
local i = 0

while true do
  ret, message = lproc_queue.sem_timedwait_ms(semaphore_in, 987)
  if (ret == false) then
    print('in '..my_name..': sem_timedwait_ms = false  message = \''..tostring(message)..'\'')
  else
    ret, message, queue_count = lproc_queue.queue_nb_pop(channel_in)
    print('in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))
  end

  lproc_queue.delay_ms(876)

  ret, queue_space = lproc_queue.queue_push(channel_out, string.format("{send_from = '%s', send_to = '%s', message = '%d'}", my_name, dest_name, i))
  if (ret == false) then
    print('in '..my_name..': error = proc_queue.queue_push()')
  else
    ret = lproc_queue.sem_post(semaphore_out)
    if (ret == false) then
      print('in '..my_name..': error = lproc_queue.sem_post()')
    else

    end
  end


  i = i + 1
end
]]




------------------- main ---------------------
REPORT(MSG_INFO, lproc_queue.version())



local ENABLE_proc_01 = true


local debug_info = lproc_queue.debug_mode('0b1000_0000_0000_0000_0000_0000_0001_1110');
REPORT(MSG_INFO, debug_info)


local my_name = 'proc_main'


local channel__to_proc_01 = lproc_queue.queue_create()
local channel__to_proc_02 = lproc_queue.queue_create()
local semaphore__to_proc_01 = lproc_queue.sem_create()
local semaphore__to_proc_02 = lproc_queue.sem_create()

local proc_01 = proc_XX_pattern
proc_01 = string.gsub(proc_01, "<my_name>", "proc_01")
proc_01 = string.gsub(proc_01, "<dest_name>", "proc_02")
proc_01 = string.gsub(proc_01, "<channel_in>", string.format("0x%016X", channel__to_proc_01))
proc_01 = string.gsub(proc_01, "<channel_out>", string.format("0x%016X", channel__to_proc_02))
proc_01 = string.gsub(proc_01, "<semaphore_in>", string.format("0x%016X", semaphore__to_proc_01))
proc_01 = string.gsub(proc_01, "<semaphore_out>", string.format("0x%016X", semaphore__to_proc_02))


local proc_02 = proc_XX_pattern
proc_02 = string.gsub(proc_02, "<my_name>", "proc_02")
proc_02 = string.gsub(proc_02, "<dest_name>", "proc_01")
proc_02 = string.gsub(proc_02, "<channel_in>", string.format("0x%016X", channel__to_proc_02))
proc_02 = string.gsub(proc_02, "<channel_out>", string.format("0x%016X", channel__to_proc_01))
proc_02 = string.gsub(proc_02, "<semaphore_in>", string.format("0x%016X", semaphore__to_proc_02))
proc_02 = string.gsub(proc_02, "<semaphore_out>", string.format("0x%016X", semaphore__to_proc_01))


local ret = lproc_queue.proc_start(proc_01)

local ret = lproc_queue.proc_start(proc_02)







while true do
  lproc_queue.delay_ms(498)
end


