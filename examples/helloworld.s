# x = new Integer
#
# forever:
#   x = x + 1
#   print x
.entry 0 0 2
  .const 'print'
  .const 'Integer'
  .const 1
  loadg 0 0
  puts 0
  loadg 1 1
  new 1 1
  add 1 1 258
  call 0 1 1
  jmp -2
  .resq
    catch 0
    puts 0
  .end
.end
