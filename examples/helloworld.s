# PUS params upvals stacks
.entry 0 0 1
  .const nil
  add 0 256 256
  puts 0
  ret 0 1

  .resq
    catch 0
    puts 0
    ret 0 1
  .end
.end
