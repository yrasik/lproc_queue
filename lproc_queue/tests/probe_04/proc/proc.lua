
local MSG_ERROR   = 1
local MSG_WARNING = 2
local MSG_INFO    = 3

local DEBUG       = MSG_INFO


local my_name = 'proc_01'


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
    func = my_name
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






local channel_in = <channel_in>
local channel_out = <channel_out>
local semaphore_in = <semaphore_in>
local semaphore_out = <semaphore_out>

REPORT(MSG_INFO, '--------- '..my_name..' -----------')
local ret, message, queue_count, queue_space
local i = 10000

while true do
  ret, message = lproc_queue.sem_timedwait_ms(semaphore_in, 987)
  if (ret == false) then
    REPORT(MSG_INFO, 'in '..my_name..': sem_timedwait_ms = false  message = \''..tostring(message)..'\'')
  else
    ret, message, queue_count = lproc_queue.queue_nb_pop(channel_in)
    REPORT(MSG_INFO, 'in '..my_name..': ret = '..tostring(ret)..'  message = \''..tostring(message)..'\'  queue_count = '..tostring(queue_count))
  end

  lproc_queue.delay_ms(498)

  ret, queue_space = lproc_queue.queue_push(channel_out, string.format("{send_from = '%s', send_to = 'main', message = '%d'}", my_name, i))
  if (ret == false) then
    REPORT(MSG_INFO, 'in '..my_name..': error = proc_queue.queue_push()')
  else
    ret = lproc_queue.sem_post(semaphore_out)
    if (ret == false) then
      REPORT(MSG_INFO, 'in '..my_name..': error = lproc_queue.sem_post()')
    else

    end
  end

  i = i + 1
end

