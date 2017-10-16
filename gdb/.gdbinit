define cr
    monitor x /8i 0x7e00
    delete
    x /8i ($cs*16)+$eip
    monitor x /8i ($cs*16)+$eip
    info registers
end

define s2
    x /8xb 0x7e00
end

set architecture i8086
set pagination off
target remote localhost:1234
hbreak *0x7c00
continue
cr
hbreak *0x00007e08
s2
continue
cr
s2

