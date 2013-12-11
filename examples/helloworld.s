.entry 0 0 3

  .func 2 0 1
    add 0 0 1
    ret 0 2
    .resq
      catch 0
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
