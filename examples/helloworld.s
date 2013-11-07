# print(print)
# x = new Real
# while x < 20000.0:
#   x = x + 0.1
#   print(x)
.entry 0 0 3
  .const 'print'
  .const 'Real'
  .const 0.1
  .const 20000.0
  loadg 0 0
  puts 0
  loadg 1 1
  new 1 1
  lt 2 1 259
  if 2 4
  add 1 1 258
  call 0 1 1
  jmp -4
  .resq
    catch 0
    puts 0
  .end
.end
