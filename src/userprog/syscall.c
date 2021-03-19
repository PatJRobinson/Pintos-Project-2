#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "user/syscall.h"
#include "devices/shutdown.h"  //Imports shutdown_power_off()
#include "threads/synch.h" //imports semaphore
#include "threads/vaddr.h"


struct lock fileLock; //defines lock to be used with file sys calls

bool check_user (void *esp, int argc); 

bool copyArgs(void *esp, uint32_t *argv, int argc);

static void syscall_handler (struct intr_frame *f UNUSED);

void syscall_init (void) {
  
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&fileLock);
}



static void syscall_handler (struct intr_frame *f) {


  int syscall_number = *((int*)f->esp);//gets syscall number

  //printf ("syscall number - %d\n", syscall_number);;
  //first check if f->esp is a valid pointer)

  if (syscall_number > PHYS_BASE)
  {
	exit(-1);
  }

   

/****************************************************************************
*                       Handlers
****************************************************************************/


  switch (syscall_number) { //this switch determines which syscall we are trying to call
  case SYS_HALT:

    {
      shutdown_power_off(); //we decided to just execute halt here as it's only calling shutdown_power_off
      break; 
    }
  case SYS_EXIT:
    {

    //PR -  interrupt frame contains SYS_CODE, followed by up to 3 args

    int argc = 1; //number of arguements that exit needs
    uint32_t argv[argc];	// 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;		// beginning of the frame

    if (copyArgs(esp, argv, argc))	//creates a copy of arguments
      exit(argv[0]); 	// store return value

    break;
    }
  case SYS_EXEC:
    {
	  const char * cmd_line = *((int*)f->esp + 1); //PR - the 1st and only
//argument here, contains whatever was typed on the command line: i.e. an
//executable, followed by its argumenthread_exts. 

	pid_t pid; //PR - process id, of "process id" type thread_ex

        lock_acquire(&fileLock);lock_release(&fileLock);
	pid = (pid_t)process_execute(cmd_line); //PR -(inside process.c) creates 
//a new thread, with the function of start_process(), and returns this new
//thread's id
        lock_release(&fileLock);
	//return pid;
        f->eax = pid;

	break;
    }
  case SYS_CREATE:
  {
	int argc = 2; //number of arguements that create needs
    uint32_t argv[argc]; // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp; // beginning of the frame

    if (copyArgs(esp, argv, argc)) //creates a copy of arguments
      f->eax = create((char*)&argv[0], argv[1]); //create requires a pointer

	break;
  }

  case SYS_REMOVE:
  {
    int argc = 1; //number of arguements that remove needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame

    if (copyArgs(esp, argv, argc)) //creates a copy of arguments
      f->eax = remove((char*)&argv[0]); //remove requires a pointer 

	break;
  }

  case SYS_OPEN:
  {
    int argc = 1;//number of arguements that open needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame

    if (copyArgs(esp, argv, argc))
      f->eax = open((char*)argv[0]); //open() needs a pointer:
				    // if char * ip = x;		
				    // (value of) ip is &x
				    // (value of) *ip is x

	break;
  }

  case SYS_FILESIZE:
  {
    int argc = 1; //number of arguements that filesize needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame

    if (copyArgs(esp, argv, argc))
      f->eax = filesize(argv[0]);


	break;
  }

  case SYS_READ:
  {
    int argc = 3; //number of arguements that read needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame
    if (copyArgs(esp, argv, argc))
      f->eax = read(argv[0], argv[1], argv[2]);

       break;
}



  case SYS_WRITE:
  {


    int argc = 3; //number of arguements that write needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame

    if (copyArgs(esp, argv, argc))
      f->eax = write(argv[0], argv[1], argv[2]);

	break;
    }

  case SYS_SEEK:
  {
	    int argc = 2; //number of arguements that seek needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame

    if (copyArgs(esp, argv, argc))
      seek(argv[0], argv[1]);

	break;
  }

  case SYS_TELL:
  {
    int argc = 1; //number of arguements that tell needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame

    if (copyArgs(esp, argv, argc))
      tell(argv[0]);

    else
      exit(-1);

	break;
  }

  case SYS_CLOSE:
  {
	    int argc = 1; //number of arguements that close needs
    uint32_t argv[argc];  // 4 bytes, as code/arg will be an int or int *
    void *esp = f->esp;  // beginning of the frame

    if (copyArgs(esp, argv, argc))
      close(argv[0]);
	break;
  }
  }
}




/****************************************************************************
*                       Syscalls
****************************************************************************/

void exit(int status)
{

	struct thread* cur =  thread_current(); //PR - by setting a pointer to
//the current thread, we can alter it's attributes...
  
	cur->exitcode = status; // ...and set its status

	thread_exit(); // exit code is now stored in current thread

}

int wait(pid_t pid)
{

  int ret = process_wait(pid);
  return ret;
}

bool create(const char* file, unsigned initialSize)
{


  //check to see if valid file pointerusing synchronization constructs:
  lock_acquire(&fileLock);
  bool success = filesys_create (file, initialSize);
  lock_release(&fileLock);

  return success;
}

bool remove(const char* file)
{

  lock_acquire(&fileLock);
  bool success = filesys_remove(file);
  lock_release(&fileLock);

  return success;
}

int open(const char* file)
{
  lock_acquire(&fileLock);
  int fd = (int)filesys_open(file);
  lock_release(&fileLock);

  thread_current()->fileTable[fd] = file; // so file can be retrieved using it's fd
  return fd;
}

int filesize(int fd) // we need to add an fd table to look for\ an open file, that way  we can pass a pointer to that file, given only its fd\ (file descriptor)	
{


  const char* file = thread_current()->fileTable[fd]; //lookup file
  lock_acquire(&fileLock);
  int length = (int)file_length(file);
  lock_release(&fileLock);

  return length;
}
 
int read(int fd, void* buffer, unsigned length)
{
  


  const char* file = thread_current()->fileTable[fd]; //lookup file
  lock_acquire(&fileLock);
  int bytesRead = (int)file_read(file, buffer, length);
  lock_release(&fileLock);

  return bytesRead;
}

int write(int fd, const void* buffer, unsigned size)
{

  const char* file = thread_current()->fileTable[fd]; //lookup file
  int bytesWritten= 0;

  
  if(buffer>=PHYS_BASE){exit(-1);}
  if((fd<0)||(fd>=128))
  {
    exit(-1);    // invalid file desc.
  } 

  if(fd==0)
  {
    exit(-1);  // writes to STDIN  
  }  

  if(fd==1)     //writes to STDOUT
  {
    int a=(int)size;
    while(a>=100)         // for long strings, we split into 100 character chunks
    {
      putbuf(buffer,100); // writes buffer to the console
      buffer=buffer+100;
      a-=100;
    }

    putbuf(buffer,a);
    bytesWritten=(int)size;

  } else {
      if(thread_current()->fileTable[fd]==NULL)   // file doesn't exist
        {
	  printf("file not found");    
	  bytesWritten =-1;
	} else {
          lock_acquire(&fileLock);
          bytesWritten =file_write(file, buffer, (uint32_t)size);  // write to file
          lock_release(&fileLock);
        }
  }


  return bytesWritten;
}

void seek(int fd, unsigned position) 
{

  const char* file = thread_current()->fileTable[fd]; //lookup file
lock_acquire(&fileLock);
  file_seek (file, position);
lock_release(&fileLock);

} 

unsigned tell(int fd) 
{

 const char* file = thread_current()->fileTable[fd]; //lookup file
lock_acquire(&fileLock);
  file_tell (file);
lock_release(&fileLock);

} 

void close(int fd) 	
{

  const char* file = thread_current()->fileTable[fd]; //lookup file
lock_acquire(&fileLock);
  file_close (file);
lock_release(&fileLock);

} 

/****************************************************************************
*                       Helpers
****************************************************************************/
bool copyArgs(void *esp, uint32_t *argv, int argc)
{
/*
    memcpy(argv, esp + 4, argc * 4);	//copy from 1st argument, 4 bytes per arg
  */
bool success = false;
  for (int i = 0; i < argc; i++)
  {
    if (check_user(esp+4,argc)) {
    memcpy(argv, esp + 4, argc * 4);
    success = true;
    }
  }

  return success;
}

/* [supplied by the Pintos documentation]

   Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}



//Checks that each argument points to a valid user memory address

bool
check_user (void *esp, int argc)
{
  int32_t ptr;
  
  for(int i=0; i<argc; i++) {
    ptr = get_user(esp + (i*4));
    if(ptr == -1) // segfault occured
    {

      return false;
    }

    
  return true;
}
}

