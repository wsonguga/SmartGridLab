#!/bin/sh
# testbed.tcl \
exec tclsh "$0" ${1+"$@"}


proc sendToTestbed { } {	
	global node_list 
	global power_line_list
	global power_socket
	global isTestbed
	set nodecount [llength $node_list]
	if { $nodecount == 0 } {
	# This allows switching to exec mode without extra API messages,
	# such as when connecting to a running session.
	    return
	}
	puts "Deploying to testbed..."
	global execMode argv argv1
	set servernames [getAssignedRemoteServers]	
	
	#server and port
	set forwarder_server "localhost"
	set forwarder_port 9009
	set forwarder_socket [socket $forwarder_server $forwarder_port]
	fconfigure $forwarder_socket -buffersize 40000
	if { $execMode == "batch" } {
		puts $forwarder_socket "$argv $argv1"
		set end_sending_packet "$$$$"
		puts $forwarder_socket $end_sending_packet
		flush $forwarder_socket 
		puts "Deploying Completed!"
		close $forwarder_socket
		return 
	} else {
	set values "Create new session:$isTestbed"
	#set values "Create new session:"	
	puts $forwarder_socket $values
	set node_num 0
	foreach	node $node_list {
	   set model [getNodeModel $node]
	    set type [nodeType $node]
	    if { $type == "tunnel" } {
		     set model "Tunnel"  
		     
	    } 	
		if { $model != "" } {
			incr node_num
		}
		set type [nodeType $node]
		
	     #use desired_current field of switch node to identify boundary node
	     set model [getNodeModel $node]
	     if { $model == "Switch" } {
		     setNodeDesiredCurrent $node 0
	     }
	     if { $type == "tunnel" } {
		     foreach node2 $node_list {
			set link  [linkByPeers  $node $node2] 
			if { $link != "" } {
			    setNodeDesiredCurrent $node2 1
			}
			
		     }
	     }
		
	}		
	set power_line_num [llength $power_line_list]
	set values "$node_num:$power_line_num"
	puts $forwarder_socket $values	
	foreach node $node_list {
		
	    set model [getNodeModel $node]
		
	    set power_interfaces [getNodePowerInterfaces $node]
	    set power_interfaces_flag [getNodePowerInterfacesFlag $node]
	    set desired_current [getNodeDesiredCurrent $node]
	    set max_current [getNodeMaxCurrent $node]
		set type [nodeType $node]
		if { $type == "tunnel" } {
			   set model "pTunnel"  
			   set power_interfaces 0
			   set power_interfaces_flag 0
		 } 	
	    if { $model == "pStorage" } {
		set e_stored [getNodeEStored $node]
		set capacity [getNodeCapacity $node]
		set operation_mode [getNodeOperationMode $node]
		set values "n:$node:$model:$desired_current:$max_current:$power_interfaces:$power_interfaces_flag:$e_stored:$capacity:$operation_mode"
		puts $forwarder_socket $values	    
	    } elseif { $model == "Switch" } {
		 #deal with the interface virtual node in testbed mode.   
		 set temp "pSwitch" 
		 set values "n:$node:$temp:$desired_current:$max_current:$power_interfaces:$power_interfaces_flag"
		 puts $forwarder_socket $values
	    } elseif { $model != "" } {
		    set values "n:$node:$model:$desired_current:$max_current:$power_interfaces:$power_interfaces_flag"
		    puts $forwarder_socket $values
	    }
	}
	    
	foreach power_line $power_line_list {
	    set peers [power_linePeers $power_line]
	    set power_line_interfaces [getPower_linePowerInterfaces $power_line]
	    set start [lindex $peers 0]
	    set end  [lindex $peers 1]
	    set resistor [getPower_lineResistor $power_line]
	    set power_linePacket "p:$power_line:$resistor:$start:$end:$power_line_interfaces"
	    puts $forwarder_socket $power_linePacket	
					      
	    }	
	
     }
	set end_sending_packet "$$$$"
	puts $forwarder_socket $end_sending_packet
	flush $forwarder_socket 
	puts "Deploying Completed!"
	close $forwarder_socket
	
}
proc shutDownTestbedSession { } {
	set power_server "localhost"
	set power_port 9009
	set power_socket [socket $power_server $power_port]
	set values "Shut down session:"
	puts $power_socket $values
	set end_sending_packet "$$$$"
	puts $power_socket $end_sending_packet
	flush $power_socket 
	puts "Power session shut down!"
	close $power_socket
}

proc runServices {node service} {
	global IPADDRESSES
	if { $service == "Switch" } {
	  exec ssh root@$IPADDRESSES($node) /home/root/smartgrid/services/service.sh &
	}
}