### Fault tolerance:
#### driver
	addServer(hostname, ip, port);
	removeServer(hostname);
	moveReturn()
	actAddServer()
	addServerReturn()
	actRemoveServer()
	removeServerReturn()
#### msgHandler
	sendMove(id, local_ip, local_port, src_ip, src_port, dest_ip, dest_port, hash_begin, hash_end);
	handleMove():
		tell data.
	sendDataMove(id, local_ip, local_port, remote_ip, remote_id, data(list));
	handleDataMove():
		tell data.
	sendDataMoveReturn(id, remote_ip, remote_id, status):
		data call.
	handleDataMoveReturn():
		tell data.
	sendMoveReturn(id, remote_ip, remote_port, status):
		remote = primary.
		move finish.
	handelMoveReturn():
		tell driver.

	sendAddServer(id, local_ip, local_port, ip, port, new_hostname, new_ip, new_port)
	handleAddServer()
		tell Driver;
	sendAddServerReturn(id, ip, port, status)
	handleAddServerReturn()
		tell Driver;
#### data.
	
