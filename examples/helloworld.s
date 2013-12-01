# x = new Array
# print(x.push_back(42))
# print(x.push_back(101))
# print(x.push_back(1337))
# puts x.remove(-2)
# puts x
.entry 0 0 4
  .const 'Array'
  .const 'push_back'
  .const 42
  .const 101
  .const 1337
  .const 'remove'
  .const -2
  loadg 0 0
  new 0 0

  loadi 1 0 257
  move 2 0  
  move 3 258
  call 1 2 2
  puts 1
  
  loadi 1 0 257
  move 2 0
  move 3 259
  call 1 2 2
  puts 1
  
  loadi 1 0 257
  move 2 0
  move 3 260
  call 1 2 2
  puts 1

  loadi 1 0 261
  move 2 0
  move 3 262
  call 1 2 2
  puts 1
  puts 0
  .resq
    catch 0
    puts 0
  .end
.end
