# empManagement
A simple employee management system in C, with error logging (can be read over a TCP network) : localhost:8080

(Could still have some bugs. I haven't worked on it since my first build)

compilation

On windows.
gcc -o empManagement linkedList_v2.c -lws2_32 

No -lws2_32 flag needed on Linux Enviroment.

input parameters.

On Linux
./executable_name empInfo.bin

On windows
executable_name empInfo.bin

executable_name -> empManagement if compiled with above instructions.
Can be changed to your desire.

empInfo.bin contains employee details.
output.bin contains all the output
log.bin contains all the logs for a session (can be read over a network)


goto localhost:8080 to read all the logs over a TCP network

I have made some function calls in the main function.
Please change it to your liking, tweak it a lil bit, Improve on it if you'd like.
Just play around
