"# Linux-Windows_ChatApplication" 
To use this service, please download a C compiler. I suggest using winlib

Step 1:
compile your clien2.c and your server.c in your computer
For example:
  Using “gcc client2.c -o WhatsChat.exe -lws2_32” in Windows environment
  Using “gcc client2.c -o WhatsChat –pthread” in Linux environment

Step 2:
get your IP address on your computer that run the server

Step 3:
run the compiled file. 
In Windows, run in Windows cmd by using “WhatsChat.exe” command.  OR run the WhatsChat.exe in the folder with your client.c file
In Linux terminal, run “./WhatsChat”.

Step 4:
Set your username and fill in the IP address of the server

Step 5:
Start chatting and transfering file
