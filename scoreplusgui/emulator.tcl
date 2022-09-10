#
# Copyright 2012 Sensorweb Laboratory, Georgia State University, USA.
# Author:          Song Tan <stan6@student.gsu.edu>
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# This work was supported by NSF CPS Program.
# 
#




#****f* SCORE.tcl/shutDownPowerSimulator
# NAME
#   shutDownPowerSimulator 
# SYNOPSIS
#   shutDownPowerSimulator
# FUNCTION
#   -- signal the power simulator to shut down current session
#   
# INPUTS
#   NONE
#****
proc shutDownPowerSimulator { } {
	set plugin [lindex [getEmulPlugin "*"] 0]
	set scoreplus_server_ip [getplugin_ip $plugin connect false]
        set power_server $scoreplus_server_ip
#	set power_server "localhost"
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


#****f* SCORE.tcl/sendToPowerSimulator
# NAME
#   sendToPowerSimulator 
# SYNOPSIS
#   sendToPowerSimulator
# FUNCTION
#   -- send the initial configurations to power simulator.
#      in both batch mode or interactive mode.
#   
# INPUTS
#   NONE
#****
proc sendToPowerSimulator { scoreplus_server_ip } {	
	global exec_servers
	global node_list 
	global power_line_list
#	global dynamic_connection_list
	global power_socket
        set nodecount [llength $node_list]
	global isTestbed
        if { $nodecount == 0 } {
	# This allows switching to exec mode without extra API messages,
	# such as when connecting to a running session.
	    return
        }
	
#	foreach dynamic_connection $dynamic_connection_list {
#		 
#	     set t1 [getDynamic_connectionLocalPi $dynamic_connection] 
#	     set t2 [getDynamic_connectionRemotePi $dynamic_connection]
#	     set t3 [getDynamic_connectionRemoteIp $dynamic_connection]
#	     puts "$t3:$t2:$t1"
#		
#	}
	
	
        puts "Deploying to power simulator..."
	global execMode argv argv1
	set servernames [getAssignedRemoteServers]	
#	set power_server "localhost"
        set power_server $scoreplus_server_ip
	set power_port 9009
	set power_socket [socket $power_server $power_port]
	#puts "The default buffer size is: [fconfigure $power_socket -buffersize ]"
	fconfigure $power_socket -buffersize 40000
    #	fconfigure $power_socket -buffering line
    #    fileevent $power_socket readable [list ProcessData $power_socket]
	#puts "The modified buffer size is: [fconfigure $power_socket -buffersize ]"
	
	#This batch mode part could be reimplemented since it is really the same as deploycnf
	# all the needed info is already loaded in tcl by loadcfg, so it can send as normal.
	# no need to let the c++ power simulator to parse the imn file. This could also enable remote batch.
	if { $execMode == "batch" } {
		puts $power_socket "$argv $argv1"
		set end_sending_packet "$$$$"
		puts $power_socket $end_sending_packet
		flush $power_socket 
		puts "Deploying Completed!"
		close $power_socket
		puts "Disconnected with power simulator."
		return 
	} else {
	set values "Create new session:$isTestbed"
	puts $power_socket $values
	set node_num [llength $node_list]
	set power_line_num [llength $power_line_list]
	set server_num [expr [llength $servernames]]
	set local_used 0
	foreach node $node_list {
		    set location [getNodeLocation $node]
		    if { $location == "" } {
			 set location "localhost" 
			
			 incr local_used
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
	if { $local_used !=0 } {
	    incr server_num
	}	
	set values "$node_num:$power_line_num:$server_num"
	puts $power_socket $values	
	foreach node $node_list {
	    set location [getNodeLocation $node]
	    if { $location == "" } {
		 set location "localhost" 
	    }
	    set type [nodeType $node]
	    if { $type == "tunnel" } {
		set model "Tunnel"  
		set power_interfaces 0
	    }  else {
	        set model [getNodeModel $node]
		set power_interfaces [getNodePowerInterfaces $node]
	    }
		
	    
	    set power_interfaces_flag [getNodePowerInterfacesFlag $node]
	    set desired_current [getNodeDesiredCurrent $node]
	    set max_current [getNodeMaxCurrent $node]
	    if { $model == "Storage" } {
	    
		set e_stored [getNodeEStored $node]
		set capacity [getNodeCapacity $node]
		set operation_mode [getNodeOperationMode $node]
		set values "n:$node:$model:$desired_current:$max_current:$power_interfaces:$power_interfaces_flag:$location:$e_stored:$capacity:$operation_mode"
		puts $power_socket $values	    
	    } else {
	      set values "n:$node:$model:$desired_current:$max_current:$power_interfaces:$power_interfaces_flag:$location"
	      puts $power_socket $values
	    }
	}
	    
	foreach power_line $power_line_list {
	    set peers [power_linePeers $power_line]
	    set power_line_interfaces [getPower_linePowerInterfaces $power_line]
	    set start [lindex $peers 0]
	    set end  [lindex $peers 1]
	    set resistor [getPower_lineResistor $power_line]
	    set power_linePacket "p:$power_line:$resistor:$start:$end:$power_line_interfaces"
	    puts $power_socket $power_linePacket	
					      
	    }	
	foreach servername $servernames {
           set host [lindex $exec_servers($servername) 0]
		#Power Slave
		    set port 10000
	            set server_packet "s:$servername:$host:$port"
		    puts $power_socket $server_packet
	}
	if { $local_used !=0 } {
	  set server_packet "s:localhost:localhost:10000"
	  puts $power_socket $server_packet  
	}
     }
	set end_sending_packet "$$$$"
	puts $power_socket $end_sending_packet
	flush $power_socket 
	puts "Deploying Completed!"
	close $power_socket
	
}
#****f* SCORE.tcl/GuiServerStart
# NAME
#   GuiServerStart 
# SYNOPSIS
#   GuiServerStart $port
# FUNCTION
#   -- start the Gui server socket.
# INPUTS
#  * port  --the port number of Gui server socket. Default is 9010.
#  
#****
proc GuiServerStart { port } {
	#the global listenSocket needs to be closed down somewhere later.
	global listenSocket
	set listenSocket [socket -server SocketAccepted $port]
}
#****f* SCORE.tcl/SocketAccepted
# NAME
#   SocketAccepted  
# SYNOPSIS
#   SocketAccepted $newSock $addr $port
# FUNCTION
#   -- event handler when a new socket is connected.
#****
proc SocketAccepted {newSock addr port} {
	#this means the data can be received until one line termination symbol is received.
	fconfigure $newSock -buffering line
	fileevent $newSock readable [list ProcessData $newSock]
}
#****f* SCORE.tcl/ProcessData
# NAME
#   ProcessData 
# SYNOPSIS
#   ProcessData $sock
# FUNCTION
#   -- parse the data when received from socket.
# INPUTS
#  * sock --socket.
#  
#****
proc ProcessData {sock} {
	##this socket consider newline as the receiving end.
	global node_list 
	global changed
	if { [eof $sock] || [catch {gets $sock line}]} {
		#puts "connection closed!"
		close $sock
	} else {
		puts $line
             if { $line != "" } {
		#puts "real-time message received!"
		#puts $line     
		set records [split $line "|"]
		foreach rec $records {
		   set contents [split $rec ":"]
		   set id [lindex $contents 0]
		   set config [lindex $contents 1]
		   if { $id != "" && $config != "" } {
		       set letter [string index $id 0]
		       if { $letter == "p" } {
		         setPower_lineCurrent $id $config
		       }
		       if { $letter == "n" } {
			  set model [getNodeModel $id]
			  if { $model == "Switch" } {
			     set changed 1
			     setNodePowerInterfacesFlag $id $config
			  } elseif { $model == "Storage" } {
			     set operation_mode [lindex $contents 2]	  
			     set e_stored [lindex $contents 3]
			     setNodeOperationMode $id $operation_mode
			     setNodeCurrent $id $config
			     setNodeEStored $id $e_stored
			  } else {
		             setNodeCurrent $id $config
			  }
			
			 
		       }
		   }
		  }
	      }
	}
#		close $sock	
}
#****f* SCORE.tcl/GuiServerStop
# NAME
#   GuiServerStop 
# SYNOPSIS
#   GuiServerStop
# FUNCTION
#   -- stop the GUI server socket
# INPUTS
#  * NONE.
#  
#****
proc GuiServerStop { } {
	global listenSocket
	if { $listenSocket != "" } {
	  close $listenSocket
	}
}
#****f* SCORE.tcl/updateGUISwitchInterface
# NAME
#   updateGUISwitchInterface 
# SYNOPSIS
#   updateGUISwitchInterface $node
# FUNCTION
#   -- update the switch interface drawing.
# INPUTS
#  * node  --the switch node id.
#  
#****
proc updateGUISwitchInterface { node } {
	set c .c
	$c itemconfigure "switchlabel && $node" -state hidden
	set value [getNodePowerInterfacesFlag $node]
	for { set i 1 } { $i <= 15 } { incr i } {
	      if { $value > 0 } {
		set re [expr $value%2]
		if { $re == 1 } {
			drawSwitchInterfaces $node $i
		}
		set value [expr $value>>1]
	      }
	}
	
}
proc updateGUIpSwitchInterface { node } {
	set c .c
	$c itemconfigure "pswitchlabel && $node" -state hidden
	set value [getNodePowerInterfacesFlag $node]
	for { set i 1 } { $i <= 15 } { incr i } {
	      if { $value > 0 } {
		set re [expr $value%2]
		if { $re == 1 } {
			drawSwitchInterfaces $node $i
		}
		set value [expr $value>>1]
	      }
	}
	
}
#****f* SCORE.tcl/drawSwitchInterfaces
# NAME
#   drawSwitchInterfaces
# SYNOPSIS
#   drawSwitchInterfaces $node $i
# FUNCTION
#   -- draw the Switch interface $i for $node .
# INPUTS
#  * node  --the switch node id.
#  * i     --the switch interface connection flag.
#****
proc drawSwitchInterfaces { node i } {
	global zoom
	set c .c
	set coords [getNodeCoords $node]
	set x [expr {[lindex $coords 0] * $zoom}]
	set y [expr {[lindex $coords 1] * $zoom}]
	if { $i == 1 } {
         $c create line $x [expr $y-37] $x $y [expr $x-29] [expr $y-18] -fill #9400d3 \
		-width 3 -tags "switchlabel $node" -smooth true -splinesteps 15	
#	  $c create line $x [expr $y-37] [expr $x-29] [expr $y-18] -fill #9400d3 \
	  -width 3 -tags "switchlabel $node"	
	}
	if { $i == 2 } {
	  $c create line $x [expr $y-37] $x $y [expr $x-29] [expr $y+18] -fill #9400d3 \
		-width 3 -tags "switchlabel $node" -smooth true -splinesteps 15	
		
	}
	if { $i == 3 } {
		  $c create line $x [expr $y-37] $x $y [expr $x] [expr $y+37] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 4 } {
		  $c create line $x [expr $y-37] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 5 } {
		  $c create line $x [expr $y-37] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 6 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x-29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 13
			
		}
	if { $i == 7 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x] [expr $y+37] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 15	
			
		}
	if { $i == 8 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 15	
			
		}
	if { $i == 9 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 13	
			
		}
	if { $i == 10 } {
		  $c create line [expr $x-29] [expr $y+18] $x $y [expr $x] [expr $y+37] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 11 } {
		  $c create line [expr $x-29] [expr $y+18] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 12 } {
		  $c create line [expr $x-29] [expr $y+18] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 13 } {
		  $c create line [expr $x] [expr $y+37] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 14 } {
		  $c create line [expr $x] [expr $y+37] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 15 } {
		  $c create line [expr $x+29] [expr $y+18] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "switchlabel $node" -smooth true -splinesteps 11	
			
		}
	
}

proc drawpSwitchInterfaces { node i } {
	global zoom
	set c .c
	set coords [getNodeCoords $node]
	set x [expr {[lindex $coords 0] * $zoom}]
	set y [expr {[lindex $coords 1] * $zoom}]
	if { $i == 1 } {
	 $c create line $x [expr $y-37] $x $y [expr $x-29] [expr $y-18] -fill #9400d3 \
		-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 15	
#	  $c create line $x [expr $y-37] [expr $x-29] [expr $y-18] -fill #9400d3 \
	  -width 3 -tags "switchlabel $node"	
	}
	if { $i == 2 } {
	  $c create line $x [expr $y-37] $x $y [expr $x-29] [expr $y+18] -fill #9400d3 \
		-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 15	
		
	}
	if { $i == 3 } {
		  $c create line $x [expr $y-37] $x $y [expr $x] [expr $y+37] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 4 } {
		  $c create line $x [expr $y-37] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 5 } {
		  $c create line $x [expr $y-37] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 6 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x-29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 13
			
		}
	if { $i == 7 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x] [expr $y+37] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 15	
			
		}
	if { $i == 8 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 15	
			
		}
	if { $i == 9 } {
		  $c create line [expr $x-29] [expr $y-18] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 13	
			
		}
	if { $i == 10 } {
		  $c create line [expr $x-29] [expr $y+18] $x $y [expr $x] [expr $y+37] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 11 } {
		  $c create line [expr $x-29] [expr $y+18] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 12 } {
		  $c create line [expr $x-29] [expr $y+18] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 13 } {
		  $c create line [expr $x] [expr $y+37] $x $y [expr $x+29] [expr $y+18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 14 } {
		  $c create line [expr $x] [expr $y+37] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	if { $i == 15 } {
		  $c create line [expr $x+29] [expr $y+18] $x $y [expr $x+29] [expr $y-18] -fill #9400d3 \
			-width 3 -tags "pswitchlabel $node" -smooth true -splinesteps 11	
			
		}
	
}


#****f* SCORE.tcl/drawPower_line
# NAME
#   drawPower_line 
# SYNOPSIS
#   drawPower_line $power_line
# FUNCTION
#   -- draw power line.
# INPUTS
#  * power_line  --the power_line id.
#  
#****


proc drawPower_line { power_line } {
    set nodes [power_linePeers $power_line]
    set lnode1 [lindex $nodes 0]
    set lnode2 [lindex $nodes 1]
    set lwidth [getPower_lineWidth $power_line]
  #  if { [getLinkMirror $link] != "" } {
#	set newlink [.c create line 0 0 0 0 \
#	    -fill [getLinkColor $link] -width $lwidth \
#	    -tags "link $link $lnode1 $lnode2" -arrow both]
#    } else {
	set newPower_line [.c create line 0 0 0 0 \
	    -fill [getPower_lineColor $power_line] -width $lwidth \
	    -tags "power_line $power_line $lnode1 $lnode2"]
 #   }
    # Boeing: links between two nodes on different servers
    if { [getNodeLocation $lnode1] != [getNodeLocation $lnode2]} {
	    .c itemconfigure $newPower_line -dash ",";
    }
    # end Boeing
    # XXX Invisible pseudo-liks
#    global invisible
#    if { $invisible == 1 && [getLinkMirror $link] != "" } {
#	.c itemconfigure $link -state hidden
#    }
#     Boeing: wlan links are hidden
#    if { [nodeType $lnode1] == "wlan" || [nodeType $lnode2] == "wlan" } {
#	global zoom
#	set imgzoom $zoom
#	if { $zoom == 0.75 || $zoom == 1.5 } { set imgzoom 1.0 }
#	global antenna$imgzoom
#	.c itemconfigure $link -state hidden
#	.c create image 0 0 -image [set antenna$imgzoom] \
#			   -tags "antenna $lnode2 $link"
#	.c create text 0 0 -tags "interface $lnode1 $link" -justify center
#	.c create text 0 0 -tags "interface $lnode2 $link" -justify center
#	.c raise interface "link || linklabel || background"
#    } else {
#    .c raise $newlink background
# Boeing: disabled this thick white line, it messes up the mobility display
#    set newlink [.c create line 0 0 0 0 \
#	-fill white -width [expr {$lwidth + 4}] \
#	-tags "link $link $lnode1 $lnode2"]
#    .c raise $newlink background
   .c create text 0 0 -tags "power_linelabel $power_line" -justify center
#    .c create text 0 0 -tags "interface $lnode1 $link" -justify center
#    .c create text 0 0 -tags "interface $lnode2 $link" -justify center
#    .c raise linklabel "link || background"
#    .c raise interface "link || linklabel || background"
#    }
}

proc redrawPower_line { power_line } {
    global $power_line
    set limages [.c find withtag "power_line && $power_line"]
    set limage1 [lindex $limages 0]
    set limage2 [lindex $limages 1]
    set tags [.c gettags $limage1]
    set power_line [lindex $tags 1]
    set lnode1 [lindex $tags 2]
    set lnode2 [lindex $tags 3]
    set entry [lsearch -inline [set $power_line] "power_interfaces {*}"]
    set power_interfaces [lindex $entry 1]
    set lnode1_power_interface [lindex $power_interfaces 0]
    set lnode2_power_interface [lindex $power_interfaces 1]
#adjust the line position
    set coords1 [.c coords "node && $lnode1"]
    set coords2 [.c coords "node && $lnode2"]
    set x1 [expr [lindex $coords1 0]+0]
    set y1 [expr [lindex $coords1 1]+0]
    set x2 [expr [lindex $coords2 0]-0]
    set y2 [expr [lindex $coords2 1]-0]
    if { $lnode1_power_interface == 1} {
       set y1 [expr [lindex $coords1 1]-42]
    }
    if { $lnode1_power_interface == 2} {
       set x1 [expr [lindex $coords1 0]-35]
       set y1 [expr [lindex $coords1 1]-22]     
    }
    if { $lnode1_power_interface == 6} {
       set x1 [expr [lindex $coords1 0]+35]
       set y1 [expr [lindex $coords1 1]-22]		    
    }
    if { $lnode1_power_interface == 3} {
       set x1 [expr [lindex $coords1 0]-35]
       set y1 [expr [lindex $coords1 1]+22]			    
    }
    if { $lnode1_power_interface == 5} {
       set x1 [expr [lindex $coords1 0]+35]
       set y1 [expr [lindex $coords1 1]+22]    
    }    	
    if { $lnode1_power_interface == 4} {
       set y1 [expr [lindex $coords1 1]+44]
    }   
	
	
    if { $lnode2_power_interface == 1} {
	    set y2 [expr [lindex $coords2 1]-42]
    }
    if { $lnode2_power_interface == 2} {
	    set x2 [expr [lindex $coords2 0]-35]
	    set y2 [expr [lindex $coords2 1]-22]
    }
    if { $lnode2_power_interface == 6} {
	    set x2 [expr [lindex $coords2 0]+35]
	    set y2 [expr [lindex $coords2 1]-22]		    
    }
    if { $lnode2_power_interface == 3} {
	    set x2 [expr [lindex $coords2 0]-35]
	    set y2 [expr [lindex $coords2 1]+22]
			  
    }
    if { $lnode2_power_interface == 5} {
	    set x2 [expr [lindex $coords2 0]+35]
            set y2 [expr [lindex $coords2 1]+22]
    }    	
    if { $lnode2_power_interface == 4} {
	    set y2 [expr [lindex $coords2 1]+44]
    }
		
    .c coords $limage1 $x1 $y1 $x2 $y2
    .c coords $limage2 $x1 $y1 $x2 $y2

    set lx [expr {0.5 * ($x1 + $x2)}]
    set ly [expr {0.5 * ($y1 + $y2)}]
    .c coords "power_linelabel && $power_line" $lx $ly

    set n [expr {sqrt (($x1 - $x2) * ($x1 - $x2) + \
	($y1 - $y2) * ($y1 - $y2)) * 0.015}]
    if { $n < 1 } {
	set n 1
    }

    calcDxDy $lnode1
#    set lx [expr {($x1 * ($n * $dx - 1) + $x2) / $n / $dx}]
#    set ly [expr {($y1 * ($n * $dy - 1) + $y2) / $n / $dy}]
#    .c coords "interface && $lnode1 && $link" $lx $ly
#    updateIfcLabel $lnode1 $lnode2
    
	
    calcDxDy $lnode2
   set lx [expr {($x1 + $x2 * ($n * $dx - 1)) / $n / $dx}]
    set ly [expr {($y1 + $y2 * ($n * $dy - 1)) / $n / $dy}]
#    .c coords "interface && $lnode2 && $link" $lx $ly
#    updateIfcLabel $lnode2 $lnode1
#    # Boeing - wlan antennas
#    if { [nodeType $lnode1] == "wlan" } {
#	global zoom
#	set an [lsearch -exact [findWlanNodes $lnode2] $lnode1]
#	if { $an < 0 || $an >= 5 } { set an 0 }
#	set dx [expr {20 - (10*$an)}]
#	.c coords "antenna && $lnode2 && $link" [expr {$x2-($dx*$zoom)}] \
#						[expr {$y2-(20*$zoom)}]
#    }
}

#****f* SCORE.tcl/popupSGModelConfig
# NAME
#   popupSGModelConfig 
# SYNOPSIS
#   popupSGModelConfig $node
# FUNCTION
#   -- pop up config dialog for each node.
# INPUTS
#  * node  --node id.
#  
#****
proc popupSGModelConfig { node } {
     set model [getNodeModel $node]
     if { $model == "Switch" } {
	   popupSwitchConfig $node
	   return 
     }
     if { $model == "Demander" } {
	   popupDemanderConfig $node
	   return 
     }
     if { $model == "PowerPlant" } {
	   popupPowerPlantConfig $node
	   return     
     }
     if { $model == "WindTurbine" } {
	   popupWindTurbineConfig $node  
	   return 
     }
     if { $model == "SolarPanel" } {
	   popupSolarPanelConfig $node
	   return 
     }
     if { $model == "Storage" } {
	   popupStorageConfig $node
	   return
     }
	    if { $model == "pSwitch" } {
		       popuppSwitchConfig $node
		       return 
		 }
		 if { $model == "pDemander" } {
		       popuppDemanderConfig $node
		       return 
		 }
		 if { $model == "pPowerPlant" } {
		       popuppPowerPlantConfig $node
		       return     
		 }
		 if { $model == "pWindTurbine" } {
		       popuppWindTurbineConfig $node  
		       return 
		 }
		 if { $model == "pSolarPanel" } {
		       popuppSolarPanelConfig $node
		       return 
		 }
		 if { $model == "pStorage" } {
		       popuppStorageConfig $node
		       return
		 }    	
	
	
}
proc popupStorageConfig { node } {
     global desired_current max_current e_stored capacity operation_mode oper_mode
     set desired_current [getNodeDesiredCurrent $node]
	    set max_current [getNodeMaxCurrent $node]
     set e_stored [getNodeEStored $node]
     set capacity [getNodeCapacity $node]
     set operation_mode [getNodeOperationMode $node]
	    set wi .storageDialog
	    catch {destroy $wi}
	    toplevel $wi -takefocus 1
	    wm transient $wi .popup 
	    wm resizable $wi 0 0
	    wm title $wi "[getNodeName $node] Storage configure"
	    grab $wi
	    frame $wi.ftop -borderwidth 4
	    label $wi.ftop.operation_mode -text "Operation_mode:"
	    label $wi.ftop.desired_current -text "Desired charging/discharging rate (w):"
	    label $wi.ftop.max_current -text "Maximum charging/discharging rate (w):"
	    label $wi.ftop.e_stored -text "Energy stored (J): "
	    label $wi.ftop.capacity -text "Capacity (J):"
	    entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	    entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	    entry $wi.ftop.e_stored_text -width 10 -relief sunken -bd 2 -textvariable e_stored
	    entry $wi.ftop.capacity_text -width 10 -relief sunken -bd 2 -textvariable capacity
	    set operation_mode_list {charging discharging standing}
	    eval tk_optionMenu $wi.ftop.operation_mode_list operation_mode $operation_mode_list
	    
	    grid $wi.ftop.operation_mode -row 0 -column 0
	    grid $wi.ftop.operation_mode_list -row 0 -column 1
	    grid $wi.ftop.desired_current -row 1 -column 0
	    grid $wi.ftop.desired_current_text -row 1 -column 1
	    grid $wi.ftop.max_current -row 1 -column 2
	    grid $wi.ftop.max_current_text -row 1 -column 3
	    grid $wi.ftop.e_stored -row 2 -column 0
	    grid $wi.ftop.e_stored_text -row 2 -column 1
	    grid $wi.ftop.capacity -row 2 -column 2
	    grid $wi.ftop.capacity_text -row 2 -column 3
	    pack $wi.ftop -side top
		    
	    frame $wi.fbot -borderwidth 4
	    button $wi.fbot.apply -text "Apply" -command "StorageConfigApply $wi $node"
	    if  { $oper_mode == "exec"} {
			    $wi.fbot.apply configure -state disabled
		    }
	    set msg "Select nodes in same energy model to apply configs to:"
	    set cmd "energyModelConfigApplyMultiple $wi"
	    button $wi.fbot.applym -text "Apply to multiple..." \
		 -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	    if  { $oper_mode == "exec"} {
			    $wi.fbot.applym configure -state disabled
		    }
	    button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	    pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
		 -side right -padx 4 -pady 4
	    pack  $wi.fbot -side bottom    		
	
	
}
proc popuppStorageConfig { node } {
     global desired_current max_current e_stored capacity operation_mode oper_mode
     set desired_current [getNodeDesiredCurrent $node]
	    set max_current [getNodeMaxCurrent $node]
     set e_stored [getNodeEStored $node]
     set capacity [getNodeCapacity $node]
     set operation_mode [getNodeOperationMode $node]
	    set wi .pstorageDialog
	    catch {destroy $wi}
	    toplevel $wi -takefocus 1
	    wm transient $wi .popup 
	    wm resizable $wi 0 0
	    wm title $wi "[getNodeName $node] pStorage configure"
	    grab $wi
	    frame $wi.ftop -borderwidth 4
	    label $wi.ftop.operation_mode -text "Operation_mode:"
	    label $wi.ftop.desired_current -text "Desired charging/discharging rate (w):"
	    label $wi.ftop.max_current -text "Maximum charging/discharging rate (w):"
	    label $wi.ftop.e_stored -text "Energy stored (J): "
	    label $wi.ftop.capacity -text "Capacity (J):"
	    entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	    entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	    entry $wi.ftop.e_stored_text -width 10 -relief sunken -bd 2 -textvariable e_stored
	    entry $wi.ftop.capacity_text -width 10 -relief sunken -bd 2 -textvariable capacity
	    set operation_mode_list {charging discharging standing}
	    eval tk_optionMenu $wi.ftop.operation_mode_list operation_mode $operation_mode_list
	    
	    grid $wi.ftop.operation_mode -row 0 -column 0
	    grid $wi.ftop.operation_mode_list -row 0 -column 1
	    grid $wi.ftop.desired_current -row 1 -column 0
	    grid $wi.ftop.desired_current_text -row 1 -column 1
	    grid $wi.ftop.max_current -row 1 -column 2
	    grid $wi.ftop.max_current_text -row 1 -column 3
	    grid $wi.ftop.e_stored -row 2 -column 0
	    grid $wi.ftop.e_stored_text -row 2 -column 1
	    grid $wi.ftop.capacity -row 2 -column 2
	    grid $wi.ftop.capacity_text -row 2 -column 3
	    pack $wi.ftop -side top
		    
	    frame $wi.fbot -borderwidth 4
	    button $wi.fbot.apply -text "Apply" -command "pStorageConfigApply $wi $node"
	    if  { $oper_mode == "exec"} {
			    $wi.fbot.apply configure -state disabled
		    }
	    set msg "Select nodes in same energy model to apply configs to:"
	    set cmd "energyModelConfigApplyMultiple $wi"
	    button $wi.fbot.applym -text "Apply to multiple..." \
		 -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	    if  { $oper_mode == "exec"} {
			    $wi.fbot.applym configure -state disabled
		    }
	    button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	    pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
		 -side right -padx 4 -pady 4
	    pack  $wi.fbot -side bottom    		
	
	
}


proc StorageConfigApply { wi node } {
	global desired_current max_current e_stored capacity operation_mode
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	setNodeEStored $node $e_stored
	setNodeCapacity $node $capacity
	setNodeOperationMode $node $operation_mode
	destroy $wi
	
}
proc pStorageConfigApply { wi node } {
	global desired_current max_current e_stored capacity operation_mode
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	setNodeEStored $node $e_stored
	setNodeCapacity $node $capacity
	setNodeOperationMode $node $operation_mode
	destroy $wi
	
}

proc energyModelConfigApplyMultiple { wi nodes } {
	global operation_mode
	set node [lindex $nodes 0]
	set model [getNodeModel $node]
	if { $model == "Storage" || $model == "pStorage"} {
	 set desired_current [$wi.ftop.desired_current_text get]
	 set max_current [$wi.ftop.max_current_text get]
         set e_stored [$wi.ftop.e_stored_text get]
	 set capacity [$wi.ftop.capacity_text get]
	 foreach node $nodes {
	      setNodeDesiredCurrent $node $desired_current
	      setNodeMaxCurrent $node $max_current
	      setNodeEStored $node $e_stored
	      setNodeCapacity $node $capacity
	      setNodeOperationMode $node $operation_mode
	 }
	} else {
	    set desired_current [$wi.ftop.desired_current_text get]
            set max_current [$wi.ftop.max_current_text get]
	 
	    foreach node $nodes {
		  setNodeDesiredCurrent $node $desired_current
		  setNodeMaxCurrent $node $max_current
		     
	    }	
		
	}
	  destroy $wi
}
proc popupSameSGModeNodes { msg initsel callback } {
	global node_list	
	set model [getNodeModel $initsel]
	global same_model_node_list
	set same_model_node_list {}
	foreach node $node_list {
	   set temp_model [getNodeModel $node]
	   if { $model == $temp_model } {
             lappend same_model_node_list $node		   
	   }
		
	}
	set wi .nodeselect
	catch {destroy $wi}
	toplevel $wi -takefocus 1
	wm resizable $wi 0 0
        wm title $wi "Select Nodes"
	grab $wi
	
        frame $wi.nodes -borderwidth 4
	
	if { $msg == "" } { set msg "Select one or more nodes of the same model:" }
	label $wi.nodes.label -text $msg
	frame $wi.nodes.fr
	listbox $wi.nodes.fr.nodelist -selectmode single -width 20 \
	-listvariable same_model_node_list -yscrollcommand "$wi.nodes.fr.scroll set" \
	-activestyle dotbox -selectmode extended
	    scrollbar $wi.nodes.fr.scroll -command "$wi.nodes.fr.nodelist yview" 
	    pack $wi.nodes.fr.nodelist $wi.nodes.fr.scroll -fill both -side left
	    pack $wi.nodes.label $wi.nodes.fr -side top -padx 4 -pady 4 \
		-anchor w
	    pack $wi.nodes -side top
	
	    frame $wi.fbot -borderwidth 4
	    button $wi.fbot.apply -text "OK" \
		-command "selectNodesHelper $wi {$callback}"
	    button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	    pack $wi.fbot.cancel $wi.fbot.apply -side right -padx 4 -pady 4
	    pack  $wi.fbot -side bottom
	
	    #set n [$wi.nodes.fr.from get $i]
	    #itemconfigure $i -foreground blue
	    set idx 0
	    foreach node $node_list {
		foreach sel $initsel {
		    if { $node == $sel } {
			$wi.nodes.fr.nodelist selection set $idx
			break
		    }
		}
		incr idx
	    }
}
proc popupSolarPanelConfig { node } {
	global desired_current max_current oper_mode
	set desired_current [getNodeDesiredCurrent $node]
	set max_current [getNodeMaxCurrent $node]
	set wi .solarPanelDialog
	catch {destroy $wi}
	toplevel $wi -takefocus 1
	wm transient $wi .popup 
	wm resizable $wi 0 0
	wm title $wi "[getNodeName $node] SolarPanel configure"
	grab $wi
	frame $wi.ftop -borderwidth 4
	label $wi.ftop.desired_current -text "Desired Power Supply (w):"
	label $wi.ftop.max_current -text "Maximum Power Supply(w):"
	entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	grid $wi.ftop.desired_current -row 0 -column 0
	grid $wi.ftop.desired_current_text -row 0 -column 1
	grid $wi.ftop.max_current -row 0 -column 2
	grid $wi.ftop.max_current_text -row 0 -column 3
	pack $wi.ftop -side top
		
	frame $wi.fbot -borderwidth 4
	button $wi.fbot.apply -text "Apply" -command "SolarPanelConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
		$wi.fbot.apply configure -state disabled
	}
	set msg "Select nodes in same energy model to apply supply configs to:"
	set cmd "energyModelConfigApplyMultiple $wi"
	button $wi.fbot.applym -text "Apply to multiple..." \
	     -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
	$wi.fbot.applym configure -state disabled
	}
	button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
	     -side right -padx 4 -pady 4
	pack  $wi.fbot -side bottom    	
	
}
proc popuppSolarPanelConfig { node } {
	global desired_current max_current oper_mode
	set desired_current [getNodeDesiredCurrent $node]
	set max_current [getNodeMaxCurrent $node]
	set wi .psolarPanelDialog
	catch {destroy $wi}
	toplevel $wi -takefocus 1
	wm transient $wi .popup 
	wm resizable $wi 0 0
	wm title $wi "[getNodeName $node] pSolarPanel configure"
	grab $wi
	frame $wi.ftop -borderwidth 4
	label $wi.ftop.desired_current -text "Desired Power Supply (w):"
	label $wi.ftop.max_current -text "Maximum Power Supply(w):"
	entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	grid $wi.ftop.desired_current -row 0 -column 0
	grid $wi.ftop.desired_current_text -row 0 -column 1
	grid $wi.ftop.max_current -row 0 -column 2
	grid $wi.ftop.max_current_text -row 0 -column 3
	pack $wi.ftop -side top
		
	frame $wi.fbot -borderwidth 4
	button $wi.fbot.apply -text "Apply" -command "pSolarPanelConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
		$wi.fbot.apply configure -state disabled
	}
	set msg "Select nodes in same energy model to apply supply configs to:"
	set cmd "energyModelConfigApplyMultiple $wi"
	button $wi.fbot.applym -text "Apply to multiple..." \
	     -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
	$wi.fbot.applym configure -state disabled
	}
	button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
	     -side right -padx 4 -pady 4
	pack  $wi.fbot -side bottom    	
	
}

proc SolarPanelConfigApply { wi node } {
	global desired_current max_current
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi
}
proc pSolarPanelConfigApply { wi node } {
	global desired_current max_current
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi
}


proc popupWindTurbineConfig { node } {
	global desired_current max_current oper_mode
	set desired_current [getNodeDesiredCurrent $node]
	set max_current [getNodeMaxCurrent $node]
	set wi .windTurbineDialog
	catch {destroy $wi}
	toplevel $wi -takefocus 1
	wm transient $wi .popup 
	wm resizable $wi 0 0
	wm title $wi "[getNodeName $node] WindTurbine configure"
	grab $wi
	frame $wi.ftop -borderwidth 4
	label $wi.ftop.desired_current -text "Desired Power Supply (w):"
	label $wi.ftop.max_current -text "Maximum Power Supply(w):"
	entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	grid $wi.ftop.desired_current -row 0 -column 0
	grid $wi.ftop.desired_current_text -row 0 -column 1
	grid $wi.ftop.max_current -row 0 -column 2
	grid $wi.ftop.max_current_text -row 0 -column 3
	pack $wi.ftop -side top
		
	frame $wi.fbot -borderwidth 4
	button $wi.fbot.apply -text "Apply" -command "WindTurbineConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
	set msg "Select nodes to apply supply configs to:"
	set cmd "energyModelConfigApplyMultiple $wi"
	button $wi.fbot.applym -text "Apply to multiple..." \
	     -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
	button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
	     -side right -padx 4 -pady 4
	pack  $wi.fbot -side bottom    	
	
}
proc popuppWindTurbineConfig { node } {
	global desired_current max_current oper_mode
	set desired_current [getNodeDesiredCurrent $node]
	set max_current [getNodeMaxCurrent $node]
	set wi .pwindTurbineDialog
	catch {destroy $wi}
	toplevel $wi -takefocus 1
	wm transient $wi .popup 
	wm resizable $wi 0 0
	wm title $wi "[getNodeName $node] pWindTurbine configure"
	grab $wi
	frame $wi.ftop -borderwidth 4
	label $wi.ftop.desired_current -text "Desired Power Supply (w):"
	label $wi.ftop.max_current -text "Maximum Power Supply(w):"
	entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	grid $wi.ftop.desired_current -row 0 -column 0
	grid $wi.ftop.desired_current_text -row 0 -column 1
	grid $wi.ftop.max_current -row 0 -column 2
	grid $wi.ftop.max_current_text -row 0 -column 3
	pack $wi.ftop -side top
		
	frame $wi.fbot -borderwidth 4
	button $wi.fbot.apply -text "Apply" -command "pWindTurbineConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
	set msg "Select nodes to apply supply configs to:"
	set cmd "energyModelConfigApplyMultiple $wi"
	button $wi.fbot.applym -text "Apply to multiple..." \
	     -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
	button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
	     -side right -padx 4 -pady 4
	pack  $wi.fbot -side bottom    	
	
}

proc WindTurbineConfigApply { wi node } {
	global desired_current max_current
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi
}

proc pWindTurbineConfigApply { wi node } {
	global desired_current max_current
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi
}


proc popupPowerPlantConfig { node } {
    global desired_current max_current oper_mode
    set desired_current [getNodeDesiredCurrent $node]
    set max_current [getNodeMaxCurrent $node]
    set wi .powerPlantDialog
    catch {destroy $wi}
    toplevel $wi -takefocus 1
    wm transient $wi .popup 
    wm resizable $wi 0 0
    wm title $wi "[getNodeName $node] PowerPlant configure"
    grab $wi
    frame $wi.ftop -borderwidth 4
    label $wi.ftop.desired_current -text "Desired Power Supply (w):"
    label $wi.ftop.max_current -text "Maximum Power Supply(w):"
    entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
    entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
    grid $wi.ftop.desired_current -row 0 -column 0
    grid $wi.ftop.desired_current_text -row 0 -column 1
    grid $wi.ftop.max_current -row 0 -column 2
    grid $wi.ftop.max_current_text -row 0 -column 3
    pack $wi.ftop -side top
	    
    frame $wi.fbot -borderwidth 4
    button $wi.fbot.apply -text "Apply" -command "PowerPlantConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
    set msg "Select nodes to apply supply configs to:"
    set cmd "energyModelConfigApplyMultiple $wi"
    button $wi.fbot.applym -text "Apply to multiple..." \
	 -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
    button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
    pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
	 -side right -padx 4 -pady 4
    pack  $wi.fbot -side bottom    	
}
proc popuppPowerPlantConfig { node } {
    global desired_current max_current oper_mode
    set desired_current [getNodeDesiredCurrent $node]
    set max_current [getNodeMaxCurrent $node]
    set wi .ppowerPlantDialog
    catch {destroy $wi}
    toplevel $wi -takefocus 1
    wm transient $wi .popup 
    wm resizable $wi 0 0
    wm title $wi "[getNodeName $node] pPowerPlant configure"
    grab $wi
    frame $wi.ftop -borderwidth 4
    label $wi.ftop.desired_current -text "Desired Power Supply (w):"
    label $wi.ftop.max_current -text "Maximum Power Supply(w):"
    entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
    entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
    grid $wi.ftop.desired_current -row 0 -column 0
    grid $wi.ftop.desired_current_text -row 0 -column 1
    grid $wi.ftop.max_current -row 0 -column 2
    grid $wi.ftop.max_current_text -row 0 -column 3
    pack $wi.ftop -side top
	    
    frame $wi.fbot -borderwidth 4
    button $wi.fbot.apply -text "Apply" -command "pPowerPlantConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
    set msg "Select nodes to apply supply configs to:"
    set cmd "energyModelConfigApplyMultiple $wi"
    button $wi.fbot.applym -text "Apply to multiple..." \
	 -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
    button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
    pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
	 -side right -padx 4 -pady 4
    pack  $wi.fbot -side bottom    	
}


proc PowerPlantConfigApply { wi node } {
	global desired_current max_current oper_mode
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi
}
proc pPowerPlantConfigApply { wi node } {
	global desired_current max_current oper_mode
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi
}

proc popupDemanderConfig { node } {
	global desired_current max_current oper_mode
	set desired_current [getNodeDesiredCurrent $node]
	set max_current [getNodeMaxCurrent $node]
	set wi .demanderDialog
	catch {destroy $wi}
	toplevel $wi -takefocus 1
	wm transient $wi .popup 
	wm resizable $wi 0 0
	wm title $wi "[getNodeName $node] Demander configure"
	grab $wi
	frame $wi.ftop -borderwidth 4
	label $wi.ftop.desired_current -text "Desired Power Demand(w):"
	label $wi.ftop.max_current -text "Maximum Power Demand(w):"
	entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	grid $wi.ftop.desired_current -row 0 -column 0
	grid $wi.ftop.desired_current_text -row 0 -column 1
	grid $wi.ftop.max_current -row 0 -column 2
	grid $wi.ftop.max_current_text -row 0 -column 3
	pack $wi.ftop -side top
	
	frame $wi.fbot -borderwidth 4
	button $wi.fbot.apply -text "Apply" -command "DemanderConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
	set msg "Select nodes to apply demand configs to:"
	set cmd "energyModelConfigApplyMultiple $wi"
	button $wi.fbot.applym -text "Apply to multiple..." \
	     -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
	button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
		-side right -padx 4 -pady 4
	pack  $wi.fbot -side bottom
}
proc DemanderConfigApply { wi node } {
	global desired_current max_current
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi	
	
}
proc popuppDemanderConfig { node } {
	global desired_current max_current oper_mode
	set desired_current [getNodeDesiredCurrent $node]
	set max_current [getNodeMaxCurrent $node]
	set wi .pdemanderDialog
	catch {destroy $wi}
	toplevel $wi -takefocus 1
	wm transient $wi .popup 
	wm resizable $wi 0 0
	wm title $wi "[getNodeName $node] pDemander configure"
	grab $wi
	frame $wi.ftop -borderwidth 4
	label $wi.ftop.desired_current -text "Desired Power Demand(w):"
	label $wi.ftop.max_current -text "Maximum Power Demand(w):"
	entry $wi.ftop.desired_current_text -width 10 -relief sunken -bd 2 -textvariable desired_current
	entry $wi.ftop.max_current_text -width 10 -relief sunken -bd 2 -textvariable max_current
	grid $wi.ftop.desired_current -row 0 -column 0
	grid $wi.ftop.desired_current_text -row 0 -column 1
	grid $wi.ftop.max_current -row 0 -column 2
	grid $wi.ftop.max_current_text -row 0 -column 3
	pack $wi.ftop -side top
	
	frame $wi.fbot -borderwidth 4
	button $wi.fbot.apply -text "Apply" -command "pDemanderConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
	set msg "Select nodes to apply demand configs to:"
	set cmd "energyModelConfigApplyMultiple $wi"
	button $wi.fbot.applym -text "Apply to multiple..." \
	     -command "popupSameSGModeNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
	button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
		-side right -padx 4 -pady 4
	pack  $wi.fbot -side bottom
}
proc pDemanderConfigApply { wi node } {
	global desired_current max_current
	setNodeDesiredCurrent $node $desired_current
	setNodeMaxCurrent $node $max_current
	destroy $wi	
	
}

proc popupSwitchConfig { node } {
    global flag12 flag13 flag14 flag15 flag16 flag23 flag24 flag25 flag26
    global flag34 flag35 flag36 flag45 flag46 flag56 oper_mode
	    
		set wi .switchDialog
		catch {destroy $wi}
		toplevel $wi -takefocus 1
		wm transient $wi .popup 
		wm resizable $wi 0 0
		wm title $wi "[getNodeName $node] Switch configure"
		grab $wi
		frame $wi.ftop -borderwidth 4
	       
		
		label $wi.ftop.filelabel -text "Power interface connections"
		pack  $wi.ftop.filelabel \
			-side right -padx 4 -pady 4
		pack  $wi.ftop -side top
		    
		frame $wi.fmid -borderwidth 4
		
		set value [getNodePowerInterfacesFlag $node]
	       
		checkbutton $wi.fmid.12 -variable flag12 
		$wi.fmid.12 deselect
		checkbutton $wi.fmid.13 -variable flag13
		$wi.fmid.13 deselect
		checkbutton $wi.fmid.14 -variable flag14
		$wi.fmid.14 deselect
		checkbutton $wi.fmid.15 -variable flag15
		$wi.fmid.15 deselect
		checkbutton $wi.fmid.16 -variable flag16
		$wi.fmid.16 deselect
		checkbutton $wi.fmid.23 -variable flag23
		$wi.fmid.23 deselect
		checkbutton $wi.fmid.24 -variable flag24
		$wi.fmid.24 deselect
		checkbutton $wi.fmid.25 -variable flag25
		$wi.fmid.25 deselect
		checkbutton $wi.fmid.26 -variable flag26
		$wi.fmid.26 deselect
		checkbutton $wi.fmid.34 -variable flag34
		$wi.fmid.34 deselect
		checkbutton $wi.fmid.35 -variable flag35
		$wi.fmid.35 deselect
		checkbutton $wi.fmid.36 -variable flag36
		$wi.fmid.36 deselect
		checkbutton $wi.fmid.45 -variable flag45
		$wi.fmid.45 deselect
		checkbutton $wi.fmid.46 -variable flag46
		$wi.fmid.46 deselect
		checkbutton $wi.fmid.56 -variable flag56
		$wi.fmid.56 deselect
		for { set i 1 } { $i <= 15 } { incr i } {	     
		      if { $value > 0 } {
			 set re [expr $value%2]
			 if { $re == 1 } {
				 if { $i == 1 } {
					 $wi.fmid.12 select  
				 }
				 if { $i == 2 } {
								   $wi.fmid.13 select  
							   }
				 if { $i == 3 } {
								   $wi.fmid.14 select  
							   }
				 if { $i == 4 } {
								   $wi.fmid.15 select  
							   }
				 if { $i == 5 } {
								   $wi.fmid.16 select  
							   }
				 if { $i == 6 } {
								   $wi.fmid.23 select  
							   }
				 if { $i == 7 } {
								   $wi.fmid.24 select  
							   }
				 if { $i == 8 } {
								   $wi.fmid.25 select  
							   }
				 if { $i == 9 } {
								   $wi.fmid.26 select  
							   }
				 if { $i == 10 } {
								   $wi.fmid.34 select  
							   }
				 if { $i == 11 } {
								   $wi.fmid.35 select  
							   }
				 if { $i == 12 } {
								   $wi.fmid.36 select  
							   }
				 if { $i == 13 } {
								   $wi.fmid.45 select  
							   }
				 if { $i == 14 } {
								   $wi.fmid.46 select  
							   }
				 if { $i == 15 } {
								   $wi.fmid.56 select  
							   }
			 }
			  set value [expr $value>>1]
		      }
		}    
		 
		label $wi.fmid.1 -text "n2"
		label $wi.fmid.2 -text "n3"
		label $wi.fmid.3 -text "n4"
		label $wi.fmid.4 -text "n5"
		label $wi.fmid.5 -text "n6"
		label $wi.fmid.6 -text "n1"
		label $wi.fmid.7 -text "n2"
		label $wi.fmid.8 -text "n3"
		label $wi.fmid.9 -text "n4"
		label $wi.fmid.10 -text "n5"
		  
		grid  $wi.fmid.12 -row 1 -column 1
		grid  $wi.fmid.13 -row 1 -column 2
		grid  $wi.fmid.14 -row 1 -column 3
		grid  $wi.fmid.15 -row 1 -column 4
		grid  $wi.fmid.16 -row 1 -column 5
		grid  $wi.fmid.23 -row 2 -column 2
		grid  $wi.fmid.24 -row 2 -column 3
		grid  $wi.fmid.25 -row 2 -column 4
		grid  $wi.fmid.26 -row 2 -column 5
		grid  $wi.fmid.34 -row 3 -column 3
		grid  $wi.fmid.35 -row 3 -column 4
		grid  $wi.fmid.36 -row 3 -column 5
		grid  $wi.fmid.45 -row 4 -column 4
		grid  $wi.fmid.46 -row 4 -column 5
		grid  $wi.fmid.56 -row 5 -column 5
		grid  $wi.fmid.1 -row 0 -column 1
		grid  $wi.fmid.2 -row 0 -column 2
		grid  $wi.fmid.3 -row 0 -column 3
		grid  $wi.fmid.4 -row 0 -column 4
		grid  $wi.fmid.5 -row 0 -column 5
		grid  $wi.fmid.6 -row 1 -column 0
		grid  $wi.fmid.7 -row 2 -column 0
		grid  $wi.fmid.8 -row 3 -column 0
		grid  $wi.fmid.9 -row 4 -column 0
		grid  $wi.fmid.10 -row 5 -column 0
		pack $wi.fmid -side top
		    
		    
		frame $wi.fbot -borderwidth 4
		button $wi.fbot.apply -text "Apply" -command "switchConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
		set msg "Select nodes to apply custom image to:"
		set cmd "customImageApplyMultiple $wi"
		button $wi.fbot.applym -text "Apply to multiple..." \
		    -command "popupSelectNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
		button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
		pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
			-side right -padx 4 -pady 4
		pack  $wi.fbot -side bottom
		
}
proc switchConfigApply { wi node } {
     global changed
     global flag12 flag13 flag14 flag15 flag16 flag23 flag24 flag25 flag26
     global flag34 flag35 flag36 flag45 flag46 flag56
     set changed 1
     set switchFlag 0
     if { $flag12 == 1 } {
	set switchFlag [expr $switchFlag+1]
     }
     if { $flag13 == 1 } {
	set switchFlag [expr $switchFlag+2]
     }
     if { $flag14 == 1 } {
	set switchFlag [expr $switchFlag+4]
     }
     if { $flag15 == 1 } {
	set switchFlag [expr $switchFlag+8]
     }
     if { $flag16 == 1 } {
	set switchFlag [expr $switchFlag+16]	     
     }
	if { $flag23 == 1 } {
		set switchFlag [expr $switchFlag+32]	     
	     }
	if { $flag24 == 1 } {
		set switchFlag [expr $switchFlag+64]	     
	     }	
	if { $flag25 == 1 } {
		set switchFlag [expr $switchFlag+128]	     
	     }
	if { $flag26 == 1 } {
		set switchFlag [expr $switchFlag+256]	     
	     }
	if { $flag34 == 1 } {
		set switchFlag [expr $switchFlag+512]	     
	     }
	if { $flag35 == 1 } {
		set switchFlag [expr $switchFlag+1024]	     
	     }
	if { $flag36 == 1 } {
		set switchFlag [expr $switchFlag+2048]	     
	     }
	if { $flag45 == 1 } {
		set switchFlag [expr $switchFlag+4096]	     
	     }
	if { $flag46 == 1 } {
		set switchFlag [expr $switchFlag+8192]	     
	     }
	if { $flag56 == 1 } {
		set switchFlag [expr $switchFlag+16384]	     
	     }
     setNodePowerInterfacesFlag $node $switchFlag
     destroy $wi	
}
proc popuppSwitchConfig { node } {
    global flag12 flag13 flag14 flag15 flag16 flag23 flag24 flag25 flag26
    global flag34 flag35 flag36 flag45 flag46 flag56 oper_mode
	    
		set wi .switchDialog
		catch {destroy $wi}
		toplevel $wi -takefocus 1
		wm transient $wi .popup 
		wm resizable $wi 0 0
		wm title $wi "[getNodeName $node] pSwitch configure"
		grab $wi
		frame $wi.ftop -borderwidth 4
	       
		
		label $wi.ftop.filelabel -text "Power interface connections"
		pack  $wi.ftop.filelabel \
			-side right -padx 4 -pady 4
		pack  $wi.ftop -side top
		    
		frame $wi.fmid -borderwidth 4
		
		set value [getNodePowerInterfacesFlag $node]
	       
		checkbutton $wi.fmid.12 -variable flag12 
		$wi.fmid.12 deselect
		checkbutton $wi.fmid.13 -variable flag13
		$wi.fmid.13 deselect
		checkbutton $wi.fmid.14 -variable flag14
		$wi.fmid.14 deselect
		checkbutton $wi.fmid.15 -variable flag15
		$wi.fmid.15 deselect
		checkbutton $wi.fmid.16 -variable flag16
		$wi.fmid.16 deselect
		checkbutton $wi.fmid.23 -variable flag23
		$wi.fmid.23 deselect
		checkbutton $wi.fmid.24 -variable flag24
		$wi.fmid.24 deselect
		checkbutton $wi.fmid.25 -variable flag25
		$wi.fmid.25 deselect
		checkbutton $wi.fmid.26 -variable flag26
		$wi.fmid.26 deselect
		checkbutton $wi.fmid.34 -variable flag34
		$wi.fmid.34 deselect
		checkbutton $wi.fmid.35 -variable flag35
		$wi.fmid.35 deselect
		checkbutton $wi.fmid.36 -variable flag36
		$wi.fmid.36 deselect
		checkbutton $wi.fmid.45 -variable flag45
		$wi.fmid.45 deselect
		checkbutton $wi.fmid.46 -variable flag46
		$wi.fmid.46 deselect
		checkbutton $wi.fmid.56 -variable flag56
		$wi.fmid.56 deselect
		for { set i 1 } { $i <= 15 } { incr i } {	     
		      if { $value > 0 } {
			 set re [expr $value%2]
			 if { $re == 1 } {
				 if { $i == 1 } {
					 $wi.fmid.12 select  
				 }
				 if { $i == 2 } {
								   $wi.fmid.13 select  
							   }
				 if { $i == 3 } {
								   $wi.fmid.14 select  
							   }
				 if { $i == 4 } {
								   $wi.fmid.15 select  
							   }
				 if { $i == 5 } {
								   $wi.fmid.16 select  
							   }
				 if { $i == 6 } {
								   $wi.fmid.23 select  
							   }
				 if { $i == 7 } {
								   $wi.fmid.24 select  
							   }
				 if { $i == 8 } {
								   $wi.fmid.25 select  
							   }
				 if { $i == 9 } {
								   $wi.fmid.26 select  
							   }
				 if { $i == 10 } {
								   $wi.fmid.34 select  
							   }
				 if { $i == 11 } {
								   $wi.fmid.35 select  
							   }
				 if { $i == 12 } {
								   $wi.fmid.36 select  
							   }
				 if { $i == 13 } {
								   $wi.fmid.45 select  
							   }
				 if { $i == 14 } {
								   $wi.fmid.46 select  
							   }
				 if { $i == 15 } {
								   $wi.fmid.56 select  
							   }
			 }
			  set value [expr $value>>1]
		      }
		}    
		 
		label $wi.fmid.1 -text "n2"
		label $wi.fmid.2 -text "n3"
		label $wi.fmid.3 -text "n4"
		label $wi.fmid.4 -text "n5"
		label $wi.fmid.5 -text "n6"
		label $wi.fmid.6 -text "n1"
		label $wi.fmid.7 -text "n2"
		label $wi.fmid.8 -text "n3"
		label $wi.fmid.9 -text "n4"
		label $wi.fmid.10 -text "n5"
		  
		grid  $wi.fmid.12 -row 1 -column 1
		grid  $wi.fmid.13 -row 1 -column 2
		grid  $wi.fmid.14 -row 1 -column 3
		grid  $wi.fmid.15 -row 1 -column 4
		grid  $wi.fmid.16 -row 1 -column 5
		grid  $wi.fmid.23 -row 2 -column 2
		grid  $wi.fmid.24 -row 2 -column 3
		grid  $wi.fmid.25 -row 2 -column 4
		grid  $wi.fmid.26 -row 2 -column 5
		grid  $wi.fmid.34 -row 3 -column 3
		grid  $wi.fmid.35 -row 3 -column 4
		grid  $wi.fmid.36 -row 3 -column 5
		grid  $wi.fmid.45 -row 4 -column 4
		grid  $wi.fmid.46 -row 4 -column 5
		grid  $wi.fmid.56 -row 5 -column 5
		grid  $wi.fmid.1 -row 0 -column 1
		grid  $wi.fmid.2 -row 0 -column 2
		grid  $wi.fmid.3 -row 0 -column 3
		grid  $wi.fmid.4 -row 0 -column 4
		grid  $wi.fmid.5 -row 0 -column 5
		grid  $wi.fmid.6 -row 1 -column 0
		grid  $wi.fmid.7 -row 2 -column 0
		grid  $wi.fmid.8 -row 3 -column 0
		grid  $wi.fmid.9 -row 4 -column 0
		grid  $wi.fmid.10 -row 5 -column 0
		pack $wi.fmid -side top
		    
		    
		frame $wi.fbot -borderwidth 4
		button $wi.fbot.apply -text "Apply" -command "pswitchConfigApply $wi $node"
	if  { $oper_mode == "exec"} {
			$wi.fbot.apply configure -state disabled
		}
		set msg "Select nodes to apply custom image to:"
		set cmd "customImageApplyMultiple $wi"
		button $wi.fbot.applym -text "Apply to multiple..." \
		    -command "popupSelectNodes \"$msg\" $node {$cmd}"
	if  { $oper_mode == "exec"} {
			$wi.fbot.applym configure -state disabled
		}
		button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
		pack $wi.fbot.cancel $wi.fbot.applym $wi.fbot.apply \
			-side right -padx 4 -pady 4
		pack  $wi.fbot -side bottom
		
}
proc pswitchConfigApply { wi node } {
     global changed
     global flag12 flag13 flag14 flag15 flag16 flag23 flag24 flag25 flag26
     global flag34 flag35 flag36 flag45 flag46 flag56
     set changed 1
     set switchFlag 0
     if { $flag12 == 1 } {
	set switchFlag [expr $switchFlag+1]
     }
     if { $flag13 == 1 } {
	set switchFlag [expr $switchFlag+2]
     }
     if { $flag14 == 1 } {
	set switchFlag [expr $switchFlag+4]
     }
     if { $flag15 == 1 } {
	set switchFlag [expr $switchFlag+8]
     }
     if { $flag16 == 1 } {
	set switchFlag [expr $switchFlag+16]	     
     }
	if { $flag23 == 1 } {
		set switchFlag [expr $switchFlag+32]	     
	     }
	if { $flag24 == 1 } {
		set switchFlag [expr $switchFlag+64]	     
	     }	
	if { $flag25 == 1 } {
		set switchFlag [expr $switchFlag+128]	     
	     }
	if { $flag26 == 1 } {
		set switchFlag [expr $switchFlag+256]	     
	     }
	if { $flag34 == 1 } {
		set switchFlag [expr $switchFlag+512]	     
	     }
	if { $flag35 == 1 } {
		set switchFlag [expr $switchFlag+1024]	     
	     }
	if { $flag36 == 1 } {
		set switchFlag [expr $switchFlag+2048]	     
	     }
	if { $flag45 == 1 } {
		set switchFlag [expr $switchFlag+4096]	     
	     }
	if { $flag46 == 1 } {
		set switchFlag [expr $switchFlag+8192]	     
	     }
	if { $flag56 == 1 } {
		set switchFlag [expr $switchFlag+16384]	     
	     }
     setNodePowerInterfacesFlag $node $switchFlag
     destroy $wi	
}





#****f* SCORE.tcl/get*
# NAME
#   get* 
# SYNOPSIS
#   get* $node
# FUNCTION
#   -- return the corresponding field of each node.
# INPUTS
#  * node  --node id.
#  
#****
proc getNodePowerInterfaces { node } {
	global $node
	return [lindex [lsearch -inline [set $node] "power_interfaces {*}"] 1]
}

proc setNodePowerInterfaces { node value } {
	global $node
	set i [lsearch [set $node] "power_interfaces {*}"]
	set $node [lreplace [set $node] $i $i "power_interfaces {$value}"]
	
}
proc getNodeEStored { node } {
	global $node
	set model [getNodeModel $node]
	if { $model != "Storage" && $model != "pStorage" } {
		return
	}
	set index [lsearch [set $node] "e_stored *"]
	if { $index == -1 } {
	    lappend $node "e_stored 100"
	}
	return [lindex [lsearch -inline [set $node] "e_stored *"] 1]
}
proc setNodeEStored { node value } {
	global $node
	set model [getNodeModel $node]
	if { $model != "Storage" && $model != "pStorage" } {
		return	
	}
	set i [lsearch [set $node] "e_stored *"]
	if { $i == -1 } {
	    lappend $node "e_stored 100"
	}
	set $node [lreplace [set $node] $i $i "e_stored $value"]
}
proc getNodeCapacity { node } {
	global $node
	set model [getNodeModel $node]
	if { $model != "Storage" && $model != "pStorage" } {
		return
	}
	set index [lsearch [set $node] "capacity *"]
	if { $index == -1 } {
	    lappend $node "capacity 500"
	}
	return [lindex [lsearch -inline [set $node] "capacity *"] 1]
}
proc setNodeCapacity { node value } {
	global $node
	set model [getNodeModel $node]
	if { $model != "Storage" && $model != "pStorage" } {
		return	
	}
	set i [lsearch [set $node] "capacity *"]
	if { $i == -1 } {
		   lappend $node "capacity 500"	
	}
	set $node [lreplace [set $node] $i $i "capacity $value"]
}
proc getNodeOperationMode { node } {
	global $node
	set model [getNodeModel $node]
	if { $model != "Storage" && $model != "pStorage" } {
		return
	}
	set index [lsearch [set $node] "operation_mode *"]
	if { $index == -1 } {
	    lappend $node "operation_mode charging"
	}
	return [lindex [lsearch -inline [set $node] "operation_mode *"] 1]
}
proc setNodeOperationMode { node value } {
	global $node
	set model [getNodeModel $node]
	if { $model != "Storage" && $model != "pStorage" } {
		return	
	}
	set i [lsearch [set $node] "operation_mode *"]
	if { $i == -1 } {
	    lappend $node "operation_mode charging"
	}
	set $node [lreplace [set $node] $i $i "operation_mode $value"]
}

proc getNodeCurrent { node } {
	global $node
	return [lindex [lsearch -inline [set $node] "current *"] 1]
}

proc setNodeCurrent { node value } {
	global $node
	set i [lsearch [set $node] "current *"]
	set $node [lreplace [set $node] $i $i "current $value"]
	
}
proc getNodeDesiredCurrent { node } {
	global $node
	return [lindex [lsearch -inline [set $node] "desired_current *"] 1]
}

proc setNodeDesiredCurrent { node value } {
	global $node
	set i [lsearch [set $node] "desired_current *"]
	set $node [lreplace [set $node] $i $i "desired_current $value"]
	
}
proc getNodeMaxCurrent { node } {
	global $node
	return [lindex [lsearch -inline [set $node] "max_current *"] 1]
}

proc setNodeMaxCurrent { node value } {
	global $node
	set i [lsearch [set $node] "max_current *"]
	set $node [lreplace [set $node] $i $i "max_current $value"]
	
}
proc getNodePowerInterfacesFlag { node } {
	global $node
	return [lindex [lsearch -inline [set $node] "power_interfaces_flag *"] 1]
}

proc setNodePowerInterfacesFlag { node value } {
	global $node
	set i [lsearch [set $node] "power_interfaces_flag *"]
	set $node [lreplace [set $node] $i $i "power_interfaces_flag $value"]
	
}

#****f* SCORE.tcl/widget_current_config
# NAME
#   widget_current_config 
# SYNOPSIS
#   widget_current_config
# FUNCTION
#   -- periodically display the current value of each power line.
# INPUTS
#  * NONE.
#  
#****
array set currentConfig { show 1 avg 1 thresh 250.0 width 10 color #FF0000 }

proc widget_current_config {} {
	
} 

proc widget_current_init { command } {
	global current_last_time
	set current_last_time [clock clicks -milliseconds]
	#reference throughput implementation later
} 
proc widget_current_periodic { now } {
	global power_line_list
	global current_last_time currentConfig 
	# get the number of seconds elapsed since we were last here
	set dt [expr { ($now - $current_last_time)/1000.0 }]
	set current_last_time $now
	if { $dt <= 0.0 } { return }
	foreach power_line $power_line_list {
	
	  #set kbps_str [format "%.3f" "5"] 
	   set kbps_str [getPower_lineCurrent $power_line]
		if { $kbps_str < 0 } {
			
			set   kbps_str [expr {-1*$kbps_str}]
		} 
	  
	  
	      if { $currentConfig(thresh) > 0.0 && \
		   $kbps_str > $currentConfig(thresh) } {
		  set width $currentConfig(width)
		  set color $currentConfig(color)
	      } else {
		      set width [getPower_lineWidth $power_line]
		  set color [getPower_lineColor $power_line]
	      }
	      if { $currentConfig(show) } {
		  .c itemconfigure "power_linelabel && $power_line" -text "$kbps_str w"
		    
	      }
	      .c itemconfigure "power_line && $power_line" -width $width -fill $color
	  
	 };
	
} 

proc widget_current_move { c node done } {
	$c delete -withtag "$node && rangecircles"
}

#****f* SCORE.tcl/power_line*
# NAME
#   power_line* 
# SYNOPSIS
#   power_line*
# FUNCTION
#   -- return corresponding info for power_line
# INPUTS
#  * power_line  --the power_line id.
#  
#****

proc power_linePeers { power_line } {
    global $power_line

    set entry [lsearch -inline [set $power_line] "nodes {*}"]
    return [lindex $entry 1]
}


proc power_lineByPeers { node1 node2 } {
    global power_line_list

    foreach power_line $power_line_list {
	set peers [power_linePeers $power_line]
	if { $peers == "$node1 $node2" || $peers == "$node2 $node1" } {
	    return $power_line
	}
    }
}
proc power_lineByPeer { node } {
    global power_line_list
    set value -1
    foreach power_line $power_line_list {
        set peers [power_linePeers $power_line]
	set value [lsearch $peers $node]
	if { $value != -1 } {
	    return $value
	}
    }
    return $value	
}
proc power_linesByPeer { node } {
   global power_line_list
   set power_lines {}
   foreach power_line $power_line_list {
      set peers [power_linePeers $power_line]
      set value [lsearch $peers $node]
      if { $value != -1 } {
	  lappend power_lines $power_line
      }
   }
	return $power_lines
}

proc removePower_line { power_line } {
    global power_line_list $power_line
    set i [lsearch -exact $power_line_list $power_line]
    set power_line_list [lreplace $power_line_list $i $i]
}


proc getPower_lineResistor { power_line } {
    global $power_line

    set entry [lsearch -inline [set $power_line] "resistor *"]
    return [lindex $entry 1]
}


proc getPower_lineResistorString { power_line } {
    global $power_line
    set powerstr ""
    set resistor [getPower_lineResistor $power_line]
    if { $resistor > 0 } {
	
	    set powerstr "$resistor ohm"
	
    }
    return $powerstr
}


proc setPower_lineResistor { power_line value } {
    global $power_line

    set i [lsearch [set $power_line] "resistor *"]
    if { $value <= 0 } {
	set $power_line [lreplace [set $power_line] $i $i]
    } else {
	set $power_line [lreplace [set $power_line] $i $i "resistor $value"]
    }
}


proc getPower_lineLength { power_line } {
    global $power_line defPower_lineLength

    set entry [lsearch -inline [set $power_line] "length *"]
    if { $entry == "" } {
	return $defPower_lineLength
    } else {
	return [lindex $entry 1]
    }
}

proc setPower_lineLength { power_line value } {
    global $power_line

    set i [lsearch [set $power_line] "length *"]
    set $power_line [lreplace [set $power_line] $i $i "length $value"]
}



proc getPower_lineColor { power_line } {
    global $power_line defPower_lineColor

    set entry [lsearch -inline [set $power_line] "color *"]
    if { $entry == "" } {
	return $defPower_lineColor
    } else {
	return [lindex $entry 1]
    }
}

proc setPower_lineColor { power_line value } {
    global $power_line

    set i [lsearch [set $power_line] "color *"]
    set $power_line [lreplace [set $power_line] $i $i "color $value"]
}

proc getPower_lineWidth { power_line } {
    global $power_line defPower_lineWidth

    set entry [lsearch -inline [set $power_line] "width *"]
    if { $entry == "" } {
	return $defPower_lineWidth
    } else {
	return [lindex $entry 1]
    }
}

proc setPower_lineWidth { power_line value } {
    global $power_line

    set i [lsearch [set $power_line] "width *"]
    set $power_line [lreplace [set $power_line] $i $i "width $value"]
}

proc getPower_lineCurrent { power_line } {
	global $power_line
	
	set entry [lsearch -inline [set $power_line] "current *"]
	return [lindex $entry 1]
}
proc getPower_linePowerInterfaces { power_line } {
	global $power_line
	
	set entry [lsearch -inline [set $power_line] "power_interfaces {*}"]
	return [lindex $entry 1]
}
proc setPower_lineCurrent { power_line value } {
	global $power_line
	set i [lsearch [set $power_line] "current *"]
	set $power_line [lreplace [set $power_line] $i $i "current $value"]
}
## this is used to create GUI power line when the existing one is in the power simulator.
proc newGUIPower_line { power_line current lnode1 lnode2 lnode1_power_interface_index lnode2_power_interface_index } {
	global power_line_list
	global $lnode1 $lnode2
	global defLength defResistor
	global defPower_lineColor defPower_lineWidth	   
	
	    
	#DEFAULT LENGTH AND RESISTOR ARE ALL 1
	global $power_line
	set $power_line {}
	lappend $power_line "resistor 1"
	lappend $power_line "nodes {$lnode1 $lnode2}"
	lappend $power_line "power_interfaces {$lnode1_power_interface_index $lnode2_power_interface_index}"
	lappend $power_line "length 1"
	lappend $power_line "width 2"
	lappend $power_line "color #ff0000"
	lappend $power_line "current $current"
	lappend power_line_list $power_line
	return $power_line
}
## create brand new power line
proc newPower_line { lnode1 lnode2 lnode1_power_interface_index lnode2_power_interface_index } {
    global power_line_list
    global $lnode1 $lnode2
    global defLength defResistor
    global defPower_lineColor defPower_lineWidth
    global curcanvas		   
    # Boeing: wlan node is always first of the two nodes
    if { [nodeType $lnode2] == "wlan" } {
	set tmp $lnode1
	set lnode1 $lnode2
	set lnode2 $tmp
    }
    # end Boeing
	
    #DEFAULT LENGTH AND RESISTOR ARE ALL 1
    set power_line [newObjectId power_line]
    global $power_line
    set $power_line {}
    lappend $power_line "resistor 1"
    lappend $power_line "nodes {$lnode1 $lnode2}"
    lappend $power_line "power_interfaces {$lnode1_power_interface_index $lnode2_power_interface_index}"
    lappend $power_line "length 1"
    lappend $power_line "width 2"
    lappend $power_line "color #ff0000"
    lappend $power_line "current 0"
    lappend power_line_list $power_line
    return $power_line
}

proc getPower_lineOpaque { power_line key } {
    global $power_line

    set entry [lsearch -inline [set $power_line] "$key *"]
    return [lindex $entry 1]
}

proc setPower_lineOpaque { power_line key value } {
    global $power_line

    set i [lsearch [set $power_line] "$key *"]
    if { $value <= 0 } {
	set $power_line [lreplace [set $power_line] $i $i]
    } else {
	set $power_line [lreplace [set $power_line] $i $i "$key $value"]
    }
}


proc updatePower_lineLabel { power_line } {
  
   
}
#****f* SCORE.tcl/requestPowerNetworkStatus
# NAME
#   requestPowerNetworkStatus 
# SYNOPSIS
#   requestPowerNetworkStatus $flags
# FUNCTION
#   -- retrieve or shutdown session info from power simulator.
#   -- then the info is held temporarily in pnode, used by apiNodeCreate function later.
# INPUTS
#  * flags --flags for retrieve or shutdown.same as core.
#  
#****
proc requestPowerNetworkStatus { flags } {
    
    if { $flags == 0x11 } {
           # this function is basically invoked in batchmode, so keep it as localhost.
	      set power_socket [socket "localhost" 9009]   
      if { $power_socket > 0 } {
       puts "Power simulator connected!"
      }
      set value "Request session power info:"
      puts $power_socket $value
      set end_sending_packet "$$$$"
      puts $power_socket $end_sending_packet    
      flush $power_socket
      while {[gets $power_socket line] >= 0} { 
	#  puts $line
	  set records [split $line ":"]
	  set length [llength $records]
	  set id [lindex $records 0]    
	  set letter [string index $id 0]
	  set flag 0
	  if { $letter == "p" } {
	      set flag 1
	  }
	  if { $letter == "n" } {
	      set flag 2
	  }
	  if { $flag == 1 } {   
	     set current [lindex $records 1]
             set lpeer [lindex $records 2]
	     set rpeer [lindex $records 3]
	     set lpeerPI [lindex $records 4]
	     set rpeerPI [lindex $records 5]
	     newGUIPower_line $id $current $lpeer $rpeer $lpeerPI $rpeerPI
	  }
	  if { $flag ==2 } {
	     set pnode "p$id"
	     set current [lindex $records 1]
             set desired_current [lindex $records 2]
	     set max_current [lindex $records 3]
	     set power_interface_flag [lindex $records 4]
	     set power_interfaces [lindex $records 5]
             set e_stored [lindex $records 6]
             set capacity [lindex $records 7]
	     set op_mode [lindex $records 8]
	     newGUIPowerNode $pnode $current $desired_current $max_current $power_interfaces $power_interface_flag $e_stored $capacity $op_mode
	  }
      } 
      close $power_socket	
     }
     if { $flags == 0x2 } {
	     ## since only one session now. The power simulator don't need
	     #message to delete. May need to send api message to delete session in the future.
         shutDownPowerSimulator
#	 puts "Power simulator session shutdown."    
	 
	     
     }
}
#****f* SCORE.tcl/newGUIPowerNode
# NAME
#   newGUIPowerNode 
# SYNOPSIS
#   newGUIPowerNode $all node fields
# FUNCTION
#   -- instantiate a power node to hold all the power info.
# INPUTS
#  * all the power node related fields.
#  
#****
proc newGUIPowerNode { pnode current desired_current max_current power_interfaces power_interface_flag e_stored capacity op_mode} {
    global power_node_list
    global viewid
    catch {unset viewid}
    global $pnode
    set $pnode {}
    #stan
	#will set to 0 when the gui configuration part is ready.
	lappend $pnode "current $current"
        lappend $pnode "desired_current $desired_current"
	lappend $pnode "max_current $max_current"
	lappend $pnode "power_interfaces {$power_interfaces}"
	lappend $pnode "power_interfaces_flag $power_interface_flag"
	if { $e_stored != "" } {
		lappend $pnode "model Storage"
		lappend $pnode "e_stored $e_stored"
		lappend $pnode "capacity $capacity"
		lappend $pnode "operation_mode $op_mode"
	}
	
    #-stan	    
	
    lappend power_node_list $pnode
    return $pnode
}
#****f* SCORE.tcl/draw_*loop
# NAME
#   draw_*loop 
# SYNOPSIS
#   drawAllPower_lines_loop
# FUNCTION
#   -- drawing power lines one by one. A periodic process.
# INPUTS
#  * NONE
#  
#****
set draw_loop_ID -1
proc drawAllPower_lines_loop { } {
	global power_line_list node_list
	foreach power_line $power_line_list {
		set nodes [power_linePeers $power_line]
		set lnode1 [lindex $nodes 0]
		set lnode2 [lindex $nodes 1]
		set index1 [lsearch $node_list $lnode1]
		set index2 [lsearch $node_list $lnode2]
		if { $index1 != -1 && $index2 != -1 } {
			drawPower_line $power_line
			redrawPower_line $power_line
			updatePower_lineLabel $power_line
		}
	}
	global oper_mode draw_loop_ID
	set now [clock clicks -milliseconds]
	set refresh_ms 1000
    
	# terminates this event loop
	if { $oper_mode != "exec" } {
	    # cleanup here
	    drawAllPower_lines_loop_stop
	    return
	}
	update ;# let the GUI process things
    
	# account for time elapsed doing periodic functions
	set now2 [clock clicks -milliseconds]
	set refresh_ms [expr {$refresh_ms - ($now2-$now)}]
	if { $refresh_ms <= 0 } {
    #	puts "warning: widget periodic functions are unable to keep up ($refresh_ms ms lost)"
		set refresh_ms 100
	}
	set draw_loop_ID [after $refresh_ms { drawAllPower_lines_loop }]
}
proc drawAllPower_lines_loop_stop { } {
    # call periodic function for each widget
    global draw_loop_ID

    after cancel $draw_loop_ID ;# prevent the widget loop from executing
    set draw_loop_ID -1
}

proc button3power_line { c x y } {
	set power_line [lindex [$c gettags {power_line && current}] 1]
	if { $power_line == "" } {
	    set power_line [lindex [$c gettags {power_linelabel && current}] 1]
	    if { $power_line == "" } {
		return
	    }
	}
	.button3menu delete 0 end
    
	#
	# Configure power_line
	#
	.button3menu add command -label "Configure" \
	    -command "popupConfigDialog $c"
	
	set x [winfo pointerx .]
	set y [winfo pointery .]
	tk_popup .button3menu $x $y
	
}

proc connectToAnotherEmulation { } {
	global isDynamicConnected dynamic_connection_list
	if { $isDynamicConnected == 0 } {
        global target_ip target_pi oper_mode local_pi
        set wi .connectToAnotherEmulation
        catch {destroy $wi}
        toplevel $wi
        wm transient $wi .
        wm resizable $wi 1 1
        wm title $wi "Connect to another emulation"
        frame $wi.ftop -borderwidth 4
        label $wi.ftop.from -text "Connecting from:"
        label $wi.ftop.to -text "Connecting to:"
        label $wi.ftop.target_ip -text "Remote emulation server IP address:"
	label $wi.ftop.target_pi -text "Remote power interface to be connected:"
        entry $wi.ftop.local_pi_text -width 20 -relief sunken -bd 2 -textvariable local_pi
	entry $wi.ftop.target_ip_text -width 20 -relief sunken -bd 2 -textvariable target_ip
	entry $wi.ftop.target_pi_text -width 20 -relief sunken -bd 2 -textvariable target_pi
	
	grid $wi.ftop.from -row 0 -column 0
	grid $wi.ftop.local_pi_text -row 0 -column 1
	grid $wi.ftop.to -row 1 -column 0
	grid $wi.ftop.target_ip -row 2 -column 0
	grid $wi.ftop.target_ip_text -row 2 -column 1
	grid $wi.ftop.target_pi -row 3 -column 0
	grid $wi.ftop.target_pi_text -row 3 -column 1
	pack $wi.ftop -side top
		
	frame $wi.fbot -borderwidth 4	
       
	button $wi.fbot.request -text "Request" -command "destroy $wi;requestDynamicConnection"
	if  { $oper_mode == "edit"} {
		$wi.fbot.request configure -state disabled
	}
	set dccount [llength $dynamic_connection_list]
	if  { $dccount == 0 } {
	    $wi.fbot.request configure -state disabled
	}
		
	
	button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	pack $wi.fbot.cancel $wi.fbot.request \
	     -side right -padx 4 -pady 4
	pack  $wi.fbot -side bottom    	
      } else {
	  puts "Current session has already been connected!"    
      }
   
}

proc requestDynamicConnection { } {
  global target_ip target_pi local_pi
  set target_server $target_ip
  set target_port 9011
  if { [catch {set dynamic_socket [socket $target_server $target_port]} errmsg] } {
    tk_messageBox -message $errmsg -type ok
   } else {   
    set values "DynamicConnectionRequest|$local_pi|$target_pi"
    puts $dynamic_socket $values
    flush $dynamic_socket
    close $dynamic_socket
    tk_messageBox -message "The request is sent successfully!" -type ok
  }  
}
proc DynamicConnectionServerStart { port } {
        global dynamicListenSocket
	set dynamicListenSocket [socket -server DynamicSocketAccepted $port]
}
proc DynamicSocketAccepted {newSock addr port} {
	#this means the data can be received until one line termination symbol is received.
	#the addr should be the client address.
	fconfigure $newSock -buffering line
	fileevent $newSock readable [list DynamicProcessData $newSock $addr]
	
}
proc DynamicProcessData { sock addr } {
        global target_ip target_pi local_pi xcoordinatorIp
        if { [eof $sock] || [catch {gets $sock line}]} {
		#puts "connection closed!"
		close $sock
	} else {
		
             if { $line != "" } {
		set records [split $line "|"]
		set messageType [lindex $records 0]
		if { $messageType == "DynamicConnectionRequest" } {
		  set remote_pi [lindex $records 1]
		  set local2_pi [lindex $records 2]
		  set answer [tk_messageBox -message "Emulation instance from $addr is trying to connected its $remote_pi with your $local2_pi" -type yesno -icon question]
                  switch -- $answer {
                   yes { DynamicConnectionRequestAccept $addr $remote_pi $local2_pi }
                   no  { DynamicConnectionRequestReject $addr }
                  }
		
		}
		if { $messageType =="DynamicConnectionReply" } {
		  set reply [lindex $records 1]
		  set coordinatorIp [lindex $records 2]
		  switch -- $reply {
		    Reject { DynamicConnectionRequestRejected $addr }
		    Accept { DynamicConnectionRequestAccepted $addr $coordinatorIp $target_pi $local_pi}
		  }
		}
		
		
             }
	      
	}

}
proc DynamicConnectionRequestReject { addr } {
  set target_server $addr
  set target_port 9011
  if { [catch {set dynamic_socket [socket $target_server $target_port]} errmsg] } {
    tk_messageBox -message $errmsg -type ok
   } else {   
    set values "DynamicConnectionReply|Reject"
    puts $dynamic_socket $values
    flush $dynamic_socket
    close $dynamic_socket
    tk_messageBox -message "You have rejected the dynamic connection from $addr!" -type ok
  }  

}
proc DynamicConnectionRequestAccept { addr remote_pi local2_pi } {
  # need to send back the coordinator ip address.	
   set coordinatorIp [ GetCoordinatorIp $local2_pi ]
   global isDynamicConnected
   
   set target_server $addr
   set target_port 9011
   if { [catch {set dynamic_socket [socket $target_server $target_port]} errmsg] } {
       tk_messageBox -message $errmsg -type ok
   } else {
      set values "DynamicConnectionReply|Accept|$coordinatorIp"
      puts $dynamic_socket $values
      flush $dynamic_socket
      close $dynamic_socket
      #send info to power simulator: coordinator ip, remote ip, remote pi,local pi.
      puts "accepter $coordinatorIp $addr $remote_pi $local2_pi to power simulator"
      #the extra power line, tunnel icon, etc should be added to the GUI. then the power line info should be sent to the power
      #simulator too.
    
       # if itself is not connected with the coordinator at the moment, then sent
      if { $isDynamicConnected == 0 } {
         after 3000
         SendCoordinatingRequest $coordinatorIp $addr $remote_pi $local2_pi	
      }
      tk_messageBox -message "You have accepted the dynamic connection from $addr. The joint emulation starts!" -type ok
   }
   
}
proc GetCoordinatorIp { local2_pi } {
  set power_server "localhost"
  set power_port 9009
  set power_socket [socket $power_server $power_port]
  set values "Get coordinator ip:"
  puts $power_socket $values
  puts $power_socket $local2_pi
  set end_sending_packet "$$$$"
  puts $power_socket $end_sending_packet
  flush $power_socket 
  if { [eof $power_socket] || [catch {gets $power_socket line}]} {
		#puts "connection closed!"
		close $power_socket
   } else {
          if { $line != "" } {
             set coordinatorIp $line
             return $coordinatorIp
          }
   }
 
}

proc DynamicConnectionRequestRejected { addr } {

tk_messageBox -message "Your dynamic connection request from $addr is rejected!" -type ok

}
proc DynamicConnectionRequestAccepted { addr coordinatorIp target_pi local_pi } {
 
 if { $coordinatorIp == "default" } {
    set coordinatorIp $addr
    
 }
 #send info to power simulator: coordinator ip, remote ip, remote pi,local pi.
 puts "request $coordinatorIp $addr $target_pi $local_pi to power simulator."
 SendCoordinatingRequest $coordinatorIp $addr $target_pi $local_pi	
 #SendDynamicConnectionInfo $coordinatorIp $addr $target_pi $local_pi 
 tk_messageBox -message "Your request for dynamic connection with $addr is accepted. The joint emulation starts! " -type ok
}


proc DynamicConnectionServerStop { } {
 	global dynamicListenSocket
 	if { $dynamicListenSocket != "" } {
 	  close $dynamicListenSocket
 	}
}

#proc SendDynamicConnectionInfo { coordinatorIp remoteIp remotePi localPi } {
#  set power_server "localhost"
#  set power_port 9009
#  set power_socket [socket $power_server $power_port]
#  set values "DynamicConnectionInfo:"
#  puts $power_socket $values
#  set values "$coordinatorIp:$remoteIp:$remotePi:$localPi"
#  puts $power_socket $values
#  set end_sending_packet "$$$$"
#  puts $power_socket $end_sending_packet
#  flush $power_socket 
#  close $power_socket
#}
# send coordinate request to coordinator such that the coordinator can control the request sequence. 
proc SendCoordinatingRequest { coordinatorIp remoteIp remotePi localPi } {
  global xcoordinatorIp isDynamicConnected dynamic_connection_list
	##avoid sending multiple request once connected since all the connect information is sent once all.
  if { $isDynamicConnected == 0 } {
  set port 9017
  set isDynamicConnected 1
  if { $coordinatorIp == "default" } {
          set coordinatorIp "localhost"
  }	
  set xcoordinatorIp $coordinatorIp	
  set coordinate_socket [socket $coordinatorIp $port]
  set values "CoordinatingRequest:"
  puts $coordinate_socket $values
  set local_ip [ipaddr] 
## a new dynamic_connectin is created	
#  newDynamic_connection $remoteIp $remotePi $localPi
	# here send all the boundary nodes info to the coordinator, no matter it is connected or not.
  foreach dynamic_connection $dynamic_connection_list {
	  
      set t1 [getDynamic_connectionLocalPi $dynamic_connection] 
      set t2 [getDynamic_connectionRemotePi $dynamic_connection]
      set t3 [getDynamic_connectionRemoteIp $dynamic_connection]
      set values "$local_ip:$coordinatorIp:$t3:$t2:$t1"
	  puts $coordinate_socket $values
  }
  set end_sending_packet "$$$$"
  puts $coordinate_socket $end_sending_packet
  flush $coordinate_socket 
  close $coordinate_socket	
  }
}

#Send cancel coordinate request to the coordiator, may add a parameter to specify which 
#dynamic_connection is disconnected. Now consider only one dynamic_connection. 
proc SendCancelRequest {} {
  global xcoordinatorIp isDynamicConnected dynamic_connection_list
  if { $isDynamicConnected == 1 } {
   	  
    #only one connection at this time, actually it is the code for disconnecting all the connections.
	  set port 9017
	  set coordinate_socket [socket $xcoordinatorIp $port]
	  set values "CancelRequest:"
	  puts $coordinate_socket $values  	  
	  set local_ip [ipaddr]
    foreach dynamic_connection $dynamic_connection_list {    	    
#	    global $dynamic_connection
	    set local_pi [getDynamic_connectionLocalPi $dynamic_connection]
	    set values "$local_ip:$local_pi"
	    puts $coordinate_socket $values
	  
	    removeDynamic_connection $dynamic_connection
	   	  
    }
	 set end_sending_packet "$$$$"
	 puts $coordinate_socket $end_sending_packet	  
	 flush $coordinate_socket 
	 close $coordinate_socket 
   set isDynamicConnected 0
  }
}
#### dynamic_connection related functions
proc newDynamic_connection { remoteIp remotePi localPi } {
    global dynamic_connection_list
    # end Boeing
	
    #DEFAULT LENGTH AND RESISTOR ARE ALL 1
    set dynamic_connection [newObjectId dynamic_connection]
    global $dynamic_connection
    set $dynamic_connection {}
    lappend $dynamic_connection "remote_ip $remoteIp"
    lappend $dynamic_connection "remote_pi $remotePi"
    lappend $dynamic_connection "local_pi $localPi"
    lappend dynamic_connection_list $dynamic_connection
    return $dynamic_connection
}

proc getDynamic_connectionLocalPi { dynamic_connection } {
    global $dynamic_connection

    set entry [lsearch -inline [set $dynamic_connection] "local_pi *"]
    return [lindex $entry 1]
}
proc getDynamic_connectionRemotePi { dynamic_connection } {
    global $dynamic_connection

    set entry [lsearch -inline [set $dynamic_connection] "remote_pi *"]
    return [lindex $entry 1]
}
proc getDynamic_connectionRemoteIp { dynamic_connection } {
    global $dynamic_connection

    set entry [lsearch -inline [set $dynamic_connection] "remote_ip *"]
    return [lindex $entry 1]
}



proc removeDynamic_connection { dynamic_connection } {
    global dynamic_connection_list $dynamic_connection
    set i [lsearch -exact $dynamic_connection_list $dynamic_connection]
    set dynamic_connection_list [lreplace $dynamic_connection_list $i $i]
}



#find public ipaddress to send to coordinator
proc ipaddr {} {
      #this uses the TCP DNS port of the DNS root-servers.
      #If these aren't reachable, you probably don't
      #have a working external internet connection anyway.
      set MyIP ""
      foreach a {a b c d e f g h i j k} {
	  catch {
	      set external [socket $a.root-servers.net 53]
	      set MyIP [lindex [fconfigure $external -sockname] 0]
	      close $external
	  }
	  if { ![string equal $MyIP ""] } { break }
      }
      return $MyIP
  }


#### to support multiple connections

proc popAddDynamicConnection {} {
	   global target_ip_t target_pi_t oper_mode local_pi_t
	   set wi .addDynamicConnection
	   catch {destroy $wi}
	   toplevel $wi
	   wm transient $wi .
	   wm resizable $wi 1 1
	   wm title $wi "Adding another connection"
	   frame $wi.ftop -borderwidth 4
	   label $wi.ftop.from -text "Connecting from:"
	   label $wi.ftop.to -text "Connecting to:"
	   label $wi.ftop.target_ip -text "Remote emulation server IP address:"
	   label $wi.ftop.target_pi -text "Remote power interface to be connected:"
	   entry $wi.ftop.local_pi_text -width 20 -relief sunken -bd 2 -textvariable local_pi_t
	   entry $wi.ftop.target_ip_text -width 20 -relief sunken -bd 2 -textvariable target_ip_t
	   entry $wi.ftop.target_pi_text -width 20 -relief sunken -bd 2 -textvariable target_pi_t
	  
	   grid $wi.ftop.from -row 0 -column 0
	   grid $wi.ftop.local_pi_text -row 0 -column 1
	#   grid $wi.ftop.to -row 1 -column 0
	   grid $wi.ftop.target_ip -row 1 -column 0
	   grid $wi.ftop.target_ip_text -row 1 -column 1
	   grid $wi.ftop.target_pi -row 2 -column 0
	   grid $wi.ftop.target_pi_text -row 2 -column 1
	   pack $wi.ftop -side top
		   
	   frame $wi.fbot -borderwidth 4	
	  
	   button $wi.fbot.add -text "Add" -command "destroy $wi;AddDynamicConnection"
	   if  { $oper_mode == "exec"} {
		   $wi.fbot.add configure -state disabled
	   }
	   
	   button $wi.fbot.cancel -text "Cancel" -command "destroy $wi"
	  # grid $wi.fbot.add -row 0 -column 0
	  # grid $wi.fbot.cancel -row 0 -column 1
	   pack $wi.fbot.cancel $wi.fbot.add \
		-side right -padx 4 -pady 4
	   pack  $wi.fbot -side bottom   	
	
}

proc AddDynamicConnection {} {
	global target_ip_t target_pi_t local_pi_t
	newDynamic_connection $target_ip_t $target_pi_t $local_pi_t
}


proc getplugin_ip { name cmd retry } {
	
	global g_plugins
	if { $name == "" } { set name \"cored\" }
	if { ![info exists g_plugins($name)] } { 
	    puts "pluginConnect error: $name does not exist!"
	    return -1
	}
    
	set plugin_data $g_plugins($name)
	set ip     [lindex $plugin_data 0]
	return $ip

}
