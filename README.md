<h2 class="code-line" data-line-start=0 data-line-end=1 ><a id="Client_for_a_chat_server_using_IPK24CHAT_protocol_0"></a>Client for a chat server using IPK24-CHAT protocol</h2>
<h3 class="code-line" data-line-start=1 data-line-end=2 ><a id="Description_1"></a>Description</h3>
<p class="has-line-data" data-line-start="2" data-line-end="3">Project task was to implement a client application in two variants (TCP [RFC9293] and UDP [RFC768]). Client is able to communicate with a remote server using the <code>IPK24-CHAT</code>. Protokol supports only IPv4. Remaining specification and features of the protocol are described in the 1 reference.</p>
<h3 class="code-line" data-line-start=3 data-line-end=4 ><a id="Usage_3"></a>Usage</h3>
<p class="has-line-data" data-line-start="4" data-line-end="5">Inplementation contains MAKEFILE. The client can be compiled using <code>make</code> command. The executable file is called <code>ipk24chat-client</code> and can be run by <code>./ipk24chat-client</code>. Below are the arguments with which the executable file can be run.</p>
<table class="table table-striped table-bordered">
<thead>
<tr>
<th>Argument</th>
<th>Value</th>
<th>Possible values</th>
<th>Meaning or expected program behaviour</th>
</tr>
</thead>
<tbody>
<tr>
<td><code>-t</code></td>
<td><code>User provided</code></td>
<td><code>tcp</code> or <code>udp</code></td>
<td>Transport protocol used for connection</td>
</tr>
<tr>
<td><code>-s</code></td>
<td><code>User provided</code></td>
<td>IP address or hostname</td>
<td>Server IP or hostname</td>
</tr>
<tr>
<td><code>-p</code></td>
<td><code>4567</code></td>
<td><code>uint16</code></td>
<td>Server port</td>
</tr>
<tr>
<td><code>-d</code></td>
<td><code>250</code></td>
<td><code>uint16</code></td>
<td>UDP confirmation timeout</td>
</tr>
<tr>
<td><code>-r</code></td>
<td><code>3</code></td>
<td><code>uint8</code></td>
<td>Maximum number of UDP retransmissions</td>
</tr>
<tr>
<td><code>-h</code></td>
<td></td>
<td></td>
<td>Prints program help output and exits</td>
</tr>
</tbody>
</table>
<h3 class="code-line" data-line-start=13 data-line-end=14 ><a id="Implementation_13"></a>Implementation</h3>
<p class="has-line-data" data-line-start="14" data-line-end="15">Client application starts running in the <code>client.c</code> file. Here in the function <code>main</code> program explores its arguments, after that formats recieved adress, port, internet protokol by the <code>struct addrinfo</code>. Than it creates and allocates memory for <code>userInfo</code> structure which will be in usage whole client execution. Next step is choosing right protocol to communicate with server.</p>
<h4 class="code-line" data-line-start=15 data-line-end=16 ><a id="UDP_15"></a>UDP</h4>
<p class="has-line-data" data-line-start="16" data-line-end="19">Main UDP function is <code>udp_connection</code>. Here program creates buffer which will contain protocol commands before sending or after receiving. Afterward, creating instance of the <code>msg_id_storage</code> that will contain the numbers of messages sent and all numbers of received messages by the client to verify the correct operation of the UDP protocol .Then there is an allocation of polled_fds for <code>poll</code> function .There are many flags such as <code>timer_running</code> or <code>user-&gt;authorized</code> , <code>user-&gt;reply_request</code> atd. This flags helps to define state in which we are and decide to send msg to the server or not. Program in infinity loop (<code>udp_connection</code>) checks events that may occur (client_socket POLLIN or stdin POLLIN). Than if user type a message into stdin, the program checks to see if it expects any replies or confirms(UDP feature). Later there is a condition that checks for server incomes. And finally, every loop program checks time between last user msg and confirm from server. If the time has expired, the client sends this message again several times before the connection closes. If msg from client confirmed, flag <code>timer_running</code> and count of message sends <code>retry_num</code> are set to zero and the client can send messages to the server again. Solutions of the transport protocol issues was implemented with the help of timer because i didn’t find another way with <code>poll</code> function without <code>fork</code> and multiple threads. This is done so that the client can receive other messages from the server before the confirmation.<br>
Client formats messages before sending by <code>udp_format_msg</code> function which tokenize buffer. From there, the program enters separate functions to handle each user command: every time while calling <code>regular_exp_check_creating</code> checks if the data entered by the user using the regular expression library <code>&lt;regex.h&gt;</code> is suitable and puts tokens int the right format to <code>msg_packet</code> or or if it’s not necessary do something on client side(change display name).<br>
To receive msg from server program use <code>udp_recieve_msg</code> function where program parse recieved msg and checks the number to see if the message is repeated by <code>id_check</code> and <code>was_id_previously</code>. If it is repeated, the program stops processing the message and returns to the main function to wait for new activities. Server messages also are checked for correctness of the individual data <code>regular_exp_check_parsing</code>. Connection can be interrupted by server with <code>BYE</code> message or by client by <code>Ctrl+c</code>. In this case client also send <code>BYE</code> to the server and close the socket.</p>
<h4 class="code-line" data-line-start=19 data-line-end=20 ><a id="TCP_19"></a>TCP</h4>
<p class="has-line-data" data-line-start="20" data-line-end="24">This communication protocol also begins with buffer and poll declaration, then here there is the difference between TCP and UDP. TCP requires a constant connection with the server. That’s why program use function <code>connect</code>. Then there are two flags to control communication state <code>user-&gt;reply_request</code> and <code>user-&gt;authorized</code>.  Next, program uses the function poll and expects some events on both sides. Here the program doesn’t need to keep track of the elapsed time in the loop race because there is no <em>CONFIRM</em> message in the protocol.<br>
Сontent to send is processed in a function <code>tcp_format_input</code> as in UDP and then sent by the send function over an already connected connection to the server. This command from the user goes through the same checks as in UDP.<br>
Program receives an incoming message from the server in the command <code>tcp_receive_msg</code>. All in all, it’s pretty much the same as in UDP.<br>
Server interrupt connection with <code>BYE</code>. Client interrupt connection with <code>Ctrl+C</code> and <em>BYE</em> msg to the server.</p>
<h3 class="code-line" data-line-start=24 data-line-end=25 ><a id="Limitations_24"></a>Limitations</h3>
<p class="has-line-data" data-line-start="25" data-line-end="26">These header files are specific to Unix-like systems such as macOS and Linux, offering essential functionalities for Unix-like environments:</p>
<ul>
<li class="has-line-data" data-line-start="27" data-line-end="29">
<p class="has-line-data" data-line-start="27" data-line-end="28"><strong>&lt;netdb.h&gt;</strong>: For network-related operations.</p>
</li>
<li class="has-line-data" data-line-start="29" data-line-end="31">
<p class="has-line-data" data-line-start="29" data-line-end="30"><strong>&lt;signal.h&gt;</strong>: For handling signals.</p>
</li>
<li class="has-line-data" data-line-start="31" data-line-end="33">
<p class="has-line-data" data-line-start="31" data-line-end="32"><strong>&lt;sys/socket.h&gt;</strong>: For socket programming.</p>
</li>
<li class="has-line-data" data-line-start="33" data-line-end="35">
<p class="has-line-data" data-line-start="33" data-line-end="34"><strong>&lt;poll.h&gt;</strong>: For monitoring file descriptors.</p>
</li>
<li class="has-line-data" data-line-start="35" data-line-end="37">
<p class="has-line-data" data-line-start="35" data-line-end="36"><strong>&lt;unistd.h&gt;</strong>: For POSIX system calls and constants.</p>
</li>
</ul>
<p class="has-line-data" data-line-start="37" data-line-end="38">Note that these libraries are not available on Windows systems natively. Windows provides alternative libraries and APIs for similar functionalities.</p>
<h3 class="code-line" data-line-start=39 data-line-end=40 ><a id="Testing_39"></a>Testing</h3>
<p class="has-line-data" data-line-start="40" data-line-end="41">The project was tested partly manually (netcat) partly using a remote server.</p>
<h4 class="code-line" data-line-start=41 data-line-end=42 ><a id="TCP_scenarios_using_netcat_41"></a>TCP scenarios (using netcat)</h4>
<p class="has-line-data" data-line-start="42" data-line-end="43">These scenarios were made through netcat because tcp is easy to implement there.</p>
<ol>
<li class="has-line-data" data-line-start="43" data-line-end="46">Start with bad parameters.<br>
User: ./client -t tcp -p 4567<br>
Client: Missing required arguments -t -s</li>
<li class="has-line-data" data-line-start="46" data-line-end="54">Logging in to the server and trying to send a message.<br>
User: ./client -t tcp -p 4567 -s 127.0.0.1<br>
Client: Connected to server<br>
User: /auth xkurak03 key anon<br>
Client: Success: Authentication successful.<br>
User: hello world<br>
User: Ctrl+C<br>
Client: Disconnecting…</li>
<li class="has-line-data" data-line-start="54" data-line-end="64">Logging in to the server and trying to join another channel.<br>
User: ./client -t tcp -p 4567 -s 127.0.0.1<br>
Client: Connected to server<br>
User: /auth xkurak03 key anon<br>
Client: Success: Authentication successful.<br>
User: /join anotherChanel<br>
/join discord.verified-1<br>
Client: Success: Channel discord.verified-1 successfully joined.<br>
User: Ctrl+C<br>
Client: Disconnecting…</li>
</ol>
<h4 class="code-line" data-line-start=64 data-line-end=65 ><a id="UDP_scenarios_using_remote_server_64"></a>UDP scenarios (using remote server)</h4>
<p class="has-line-data" data-line-start="65" data-line-end="66">These scripts were made through a remote server because it is quite difficult to write and send local binary commands yourself.</p>
<ol>
<li class="has-line-data" data-line-start="66" data-line-end="83">Loggin, message, join discord-verified-1.<br>
./client -t udp -p 4567 -s <a href="http://anton5.fit.vutbr.cz">anton5.fit.vutbr.cz</a><br>
/auth xkurak03 63a8dfc6-ba91-4418-bd2b-407a8c3d8d4b anon<br>
Success: Authentication successful.<br>
Server: ano joined discord.general.<br>
Server: Next left discord.general.<br>
Server: sonya joined discord.general.<br>
hello<br>
Server: hihi joined discord.general.<br>
Server: sonya left discord.general.<br>
hihi: hello<br>
Server: Arcitronex left discord.general.<br>
hihi: u guys are cool!!<br>
/join discord.verified-1<br>
Success: Channel discord.verified-1 successfully joined.<br>
Server: ano joined discord.verified-1.<br>
^CDisconnecting…</li>
<li class="has-line-data" data-line-start="83" data-line-end="90">Trying to loggin in with bad parameters and the with right parameters.<br>
/auth xkurak03 63a8dfc6-ba91-4418-bd2b-407a8c3d8d anon<br>
Failure: Authentication failed - Provided user secret is not valid.<br>
/auth xkurak03 63a8dfc6-ba91-4418-bd2b-407a8c3d8d4b anon<br>
Success: Authentication successful.<br>
Server: ano joined discord.general.<br>
^CDisconnecting…</li>
</ol>
<h3 class="code-line" data-line-start=90 data-line-end=91 ><a id="References_90"></a>References</h3> </ol> <p class="has-line-data" data-line-start="92" data-line-end="93">[1]: DOLEJŠKA, Daniel . NESFIT/IPK-Projekty - IPK Project 1: Client for a chat server using IPK24-CHAT protocol - FIT - VUT Brno - git [online]. [cit. 2024-03-01]. Avaiable at: <a href="https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024">https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024</a></p> <p class="has-line-data" data-line-start="94" data-line-end="95">[2]: MEEHEAN, Eric. C - Creating a Web Client in C - Youtube [online]. March 10, 2021 [cit. 2024-03-01]. Avaiable at: <a href="https://www.youtube.com/watch?v=cSXsuPtqlGU">https://www.youtube.com/watch?v=cSXsuPtqlGU</a></p>