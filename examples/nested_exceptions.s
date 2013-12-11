#
# def add(x, y):
#   try:
#     return x + y
#   except e:
#     e.what = 'From add function: ' + e.what
#     raise e
# 
# try:
#   print add(2, 'test')
# except e:
#   print e
#
.entry 0 0 3

  .func 2 0 1
    add 0 0 1
    ret 0 2
    .resq
      .const 'From add function: '
      .const 'what'
      catch 0
      loadi 1 0 257
      add 1 256 1
      stori 1 0 257
      raise 0
    .end
  .end

  .const 2
  .const 'test'

  clos 0 0
  loadk 1 0
  loadk 2 1

  call 0 2 2
  puts 0

  .resq
    catch 0
    puts 0
  .end
.end
