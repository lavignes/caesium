# PUS params upvals stacks
.entry 0 0 1
  .const 'test'
  .const 3.14
  add 0 256 257
  puts 0
  ret 0 1

  .resq
    .const 'Rescue from error:'
    loadk 0 2
    puts 0
    catch 0
    puts 0
  .end
.end
