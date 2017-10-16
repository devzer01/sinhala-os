define verifysec
set variable $vx = (x /1xb 0x0449)
while ($vx != 0x3)
   stepi
   set variable $vx = (x /1xb 0x0449)
end

define adder2
while ($eax > 0x02ff || $eax < 0x01ff)
   stepi 100
end
break *($cs*16)+$eip
print "break at" 
print *($cs*16)+$eip
break *($cs*16)+$eip+16
end

define adder
while ($eax > 0x02ff || $eax < 0x01ff)
   stepi
end
break *($cs*16)+$eip
print "break at" 
print *($cs*16)+$eip
break *($cs*16)+$eip+16
end
