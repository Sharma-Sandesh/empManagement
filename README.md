# empManagement
A simple employee management system in C, with error logging (can be read over a TCP network) : localhost:8080

(Could still have some bugs. I haven't worked on it since my first build)

compilation

On windows.
gcc -o empManagement linkedList_v2.c -lws2_32 

Now -lws2_32 flag needed on Linux Enviroment.

input parameters.
./executable_name empInfo.bin

empInfo.bin contains employee details.
output.bin contains all the output
log.bin contains all the logs for a session (can be read over a network)


goto localhost:8080 to read all the logs over a TCP network
