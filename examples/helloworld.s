.entry 0 0 2
  .func 0 0 2
    .const 'Object'
    .const 'TypeError'
    loadg 0 0
    loadg 1 1
    ret 0 3
  .end
  
  clos 0 0
  call 0 0 3
  puts 0
  puts 1

  .resq
    catch 0
    puts 0
  .end
.end
