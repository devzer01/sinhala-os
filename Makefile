.DEFAULT_GOAL := bootloader

bootloader: src/mode0.asm
	nasm -f bin -o builds/bootloader src/mode0.asm

scripts: gdb/.gdbinit gdb/singlestep.gdb gdb/config
	cp gdb/.gdbinit ~/
	cp gdb/*.gdb ~/.gdb/
	cp gdb/config ~/.config/terminator/

debug: scripts run
	terminator --layout=bootloader --geometry=1920x1080+0+0

run: builds/bootloader
	qemu-system-i386 -m size=4M -nodefaults -cpu base -no-acpi -no-hpet -monitor tcp::4444,server,nowait -singlestep -s -D logs/debug.log \
		-d in_asm,int,pcall,trace:vga_vbe_write,trace:vga_vbe_read,trace:vga_std_write_io,trace:vga_std_read_io -vga std \
		-S  -hda builds/bootloader -pidfile /tmp/qemu.pid -daemonize -full-screen


## terminator --working-directory=~/ -x gdb &
##terminator --working-directory=~/ -x telnet localhost 1234 &