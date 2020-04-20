Please read this file first before using the CPP file.

Introduction:
This assignment is a work of RAJAN Yaqzaan and JAIN Jai Jitendra both of whose SIDs are in the file name.

How to Compile the CPP File:
Save the file with a shorter file name such as OS.cpp [For Convenience]
Save it in the same folder as generate_frame_vector.cpp
In the terminal write the following command to compile the file:

g++ generate_frame_vector.cpp OS.cpp -lpthread -o OS

How to Run the File:
To run the compiled OS.cpp, type "./OS <interval>" in the terminal. In the place of interval, write any integer between 1 to 5 which will serve as the interval for the camera thread between generating frames.
Now all the test cases can be tried. We have added comments in the CPP file for better understanding of our code.

Use of Mutexes:
front is only updated by quantiser and rear is only updated by the camera
We are implementing circular queue and therefore there is an extra buffer space in the queue
There are 2 mutexes.
1. mtx: this mutex helps camera get updated value of front from quantiser to check if cache is full or not. If it is full then camera will sleep for interval seconds and again check. This will prevent camera from inserting any frame in the 6th buffered space. 
2. mtx2: This mutex helps quantiser to get updated value of rear from the camera and check if cache is empty or not. So this mutex is used to prevent the quantizer from removing any frame if the queue is empty.

Feel free to contact us via yrajan2-c@my.cityu.edu.hk or jjjjain2-c@my.cityu.edu.hk for any confusions or clarifications.

Happy Testing!!!!