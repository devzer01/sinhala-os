.DEFAULT_GOAL := bootloader

bootloader: mode0.asm
	nasm -f bin -o bootloader mode0.asm

scripts: gdb/.gdbinit gdb/singlestep.gdb gdb/config
	cp gdb/.gdbinit ~/
	cp gdb/*.gdb ~/.gdb/
	cp gdb/config ~/.config/terminator/

debug: scripts run
	terminator --layout=bootloader --geometry=1920x1080+0+0

run: bootloader
	qemu-system-i386 -m size=4M -nodefaults -cpu base -no-acpi -no-hpet -monitor tcp::4444,server,nowait -singlestep -s -D debug.log \
		-d in_asm,int,pcall,trace:vga_vbe_write,trace:vga_vbe_read,trace:vga_std_write_io,trace:vga_std_read_io -vga std \
		-S  -hda bootloader -pidfile /tmp/qemu.pid -daemonize -full-screen


## terminator --working-directory=~/ -x gdb &
##terminator --working-directory=~/ -x telnet localhost 1234 &