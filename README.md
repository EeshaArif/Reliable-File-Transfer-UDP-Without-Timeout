# Relaible_UDP_VideoFileTransfer

### Implementation
Implementing reliability in UDP: 
1. Re ordering on receiver side by using **sequence numbers** 
2. Selective repeat
3. Stop n Wait protocol **without timeout**  (5 UDP Segments)


### Compilation
Commands to compile the files
* `gcc UDPserver.c -lpthread -o serv.out`
* `gcc UDPclient.c -lpthread -o cli.out`

### Execution 
Commands to run the files
* `./serv.out 9898`
* `./cli.out 9898`

### Client
An abstract explanation of the client logic:

![client-logic-flowchart](https://github.com/EeshaArif/Relaible_UDP_VideoFileTransfer/blob/master/Client-FlowChart.png "Client-FlowChart")
### Server
An abstract explanation of the server logic:

![server-logic-flowchart](https://github.com/EeshaArif/Relaible_UDP_VideoFileTransfer/blob/master/Server-FlowChart.png "Server-FlowChart")
