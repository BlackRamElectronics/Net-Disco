# Net-Disco
System discovery over local network. Includes a daemon to run on each machine and a discover program

# Cross-compiling
Please change the Toolchain prifix in the Makefile for cross-compiling

# Install
To configure the daemon to run at start up:
1) Copy over the daemon over to your '/usr/bin' directory
2) Copy the Net-Disco-d-init script over to your 'init.d' directory (system dependent, could be '/etc/rc.d/init.d')
3) Add the init script to rc.local:
	i) Use update-rc.d command
		or
	ii) Add '/etc/rc.d/init.d/Net-Disco-d-init start' to your '/etc/rc.d/rc.local' file