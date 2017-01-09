#!/usr/bin/expect
#
set timeout 9

set st [lindex $argv 0]

if { ${st} == "" } {
	set st 1
}

# Send/Receive telnet commands to/from burne
spawn telnet localhost 10000
expect {
	timeout { send_user "\nNo welcoming message found."; exit 1 }
	eof { send_user "\nUnable to connect to server."; exit 1 }
	"*is '^]'."
}
send "s${st}\r"
expect {
	timeout { send_user "\nUnable to connect to server."; exit 1 }
	"{*}"
}
expect {
	timeout { send_user "\nAzimuth not reached."; exit 1 }
	"AZ330"
}
send "\035"
expect "telnet>"
send "q\n"
expect "closed."
exit
