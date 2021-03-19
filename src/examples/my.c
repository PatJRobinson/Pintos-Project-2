#include <stdio.h>
#include <syscall.h>
#include "tests/lib.h"

int handle;
int bad_pointer;
int bad_pid;
char * test_buf = "writing to the console";

//System calls are tested to check that they work as required.
int main (void) {

	//halt();
	//exit(57);
	//create_test();
	//remove_test();
	//write_test();
	//filesize_test();
	filesize_test();
	
    
    return EXIT_SUCCESS;
 }


/*The different syscall tests need to be run one at a time.
So before running, comment out one of the specific syscall test
and run it on it's own.*/

void create_test() {

	//create((char *) 0x20101234, 0);  //Passing a bad pointer
	//create("", 0); //Passing an empty string
	//create("quux.dat", 0); //Passing valid file pointer
	//create(NULL, 0); //Passing in a null file pointer
}

void remove_test() {

	//remove((char *) 0x20101234);  //Passing a bad pointer
	//remove("");  //Passing an empty string
}

void write_test() {

	char buffer = 123;
	handle = 1;

	//write(bad_pointer, "", 0); //Passing a bad function directive, return exit code -1.
	//write(handle, buffer, 0);  //Passing a 1 for the function directive 
	//write(handle, "", 0);  //Passing emtpy string to buffer
	//write(1, test_buf, strlen(test_buf));  //When fd == 1, we should see the buffer written to the console
}

void filesize_test() {

	handle = 5;

	//filesize(handle);  //Passing valid integer
	//filesize(NULL);  //Passing null value
}
