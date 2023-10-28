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



------------------- main ---------------------
REPORT(MSG_INFO, lproc_queue.version())



local ENABLE_proc_01 = true


local debug_info = lproc_queue.debug_mode('0b1000_0000_0000_0000_0000_0000_0001_1110');
REPORT(MSG_INFO, debug_info)


local my_name = 'proc_main'


local channel__to_proc_01 = lproc_queue.queue_create()
local channel__from_proc_01 = lproc_queue.queue_create()
local semaphore__to_proc_01 = lproc_queue.sem_create()
local semaphore__from_proc_01 = lproc_queue.sem_create()


local f = io.open('./proc/proc.lua', 'r')
if (f == nil) then
  REPORT(MSG_ERROR, 'Файл с кодом процесса не открылся...')
  return -1
end
local proc_01_pattern = f:read('*all')

local proc_01 = string.gsub(proc_01_pattern, "<channel_in>", string.format("0x%016X", channel__to_proc_01))
proc_01 = string.gsub(proc_01, "<channel_out>", string.format("0x%016X", channel__from_proc_01))
proc_01 = string.gsub(proc_01, "<semaphore_in>", string.format("0x%016X", semaphore__to_proc_01))
proc_01 = string.gsub(proc_01, "<semaphore_out>", string.format("0x%016X", semaphore__from_proc_01))



if ENABLE_proc_01 then
  local ret = lproc_queue.proc_start(proc_01)
end



local ret, message, queue_count, queue_space
local i = 20000

while true do
  ret, queue_space = lproc_queue.queue_push(channel__to_proc_01, string.format("{send_from = '%s', send_to = 'proc_01', message = '%d'}", my_name, i))
  if (ret == false) then
    REPORT(MSG_ERROR, 'error = proc_queue.queue_push()')
  else
    ret = lproc_queue.sem_post(semaphore__to_proc_01)
    if (ret == false) then
      REPORT(MSG_ERROR, 'error = lproc_queue.sem_post()')
    else

    end
  end





  ret, message = lproc_queue.sem_timedwait_ms(semaphore__from_proc_01, 987)
  if (ret == false) then
    REPORT(MSG_ERROR, 'in '..my_name..': sem_timedwait_ms = false  message = \''..tostring(message)..'\'')
    if ( message == 'timeout' ) then

    end
  else
    ret, message, queue_count = lproc_queue.queue_nb_pop(channel__from_proc_01)
    if( ret == false ) then
      REPORT(MSG_ERROR, 'in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))
    else
      REPORT(MSG_INFO, 'in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))
    end

  end

  i = i + 1
end





