TFTP Server Homework

Team: Steven Li (lis18), Numfor Tiapo (mbizin)

Comments:

	For the TFTP server-client connection, we created four structs to handle all of the packets:

		- request-packet struct (Read and Write Request)
			-- opcode
			-- filename

		- data-packet struct (Data Request)
			-- opcode
			-- block_number
			-- array of bytes

		- ack-packet struct (Acknowledgement Request)
			-- opcode
			-- block number

		- error-packet struct (Error Request)
			-- opcode
			-- error_code
			-- error_message

	All packets share the opcode field that indicates the operation that needs to be carried out.

	It would've been possible to directly read the bytes of data into a buffer instead of struct packets, but it would've gotten really complicated when debugging if there were any errors, so we decided to store it all in the respective structs.

	For the entirety of the requests, the mode was octet.

	The main() function is where the parent process creates a server and sits on the starting port while it listens for requests, and when a request comes in, a child process is implemented to carry out the process. The child process is created through a fork() call, and it carries out the process by executing the handleRequest() function. For each child process, the next highest port in the port ranges is used.

	There is also a handle_alarm() function, which is the alarm for the server to terminate when there is no response from the client after 10 seconds. The alarm signal handler is installed in the beginning of the main() function.
