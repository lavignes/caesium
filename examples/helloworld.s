# PUS params upvals stacks
.entry 0 0 1
  .const 'Object'
  loadg 0 0
  add 0 0 0
  puts 0
  ret 0 1

  .resq
    catch 0
    puts 0
    ret 0 1
  .end
.end
