   
/* Calcul de distance en C avec HC-SR04 */
/* Philippe PARMENTIER 2014-11-17 */

/* ECHO on GPIO 24, pin 18  */
/* TRIG on GPIO 23, pin 16 */
/* All GPIO are in the same Bank address 0x20200008L */

    #include <stdio.h>
    #include <stdlib.h>
    #include <fcntl.h>
    #include <string.h>
    #include <errno.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <stdint.h>
    #include <time.h>

    #define GPIO_ADD 0x20200000L /* GPIO controller */
    // GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
    #define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
    #define OUT_GPIO(g) *(gpio+((g)/10)) |= (1<<(((g)%10)*3))
    #define GPIO_SET(g) *(gpio+7) = 1<<(g) // sets bits which are 1 ignores bits which are 0
    #define GPIO_CLR(g) *(gpio+10) = 1<<(g) // clears bits which are 1 ignores bits which are 0
    #define GPIO_GET  *(gpio+13)

    #define PAGE_SIZE (4*1024)
    #define BLOCK_SIZE (4*1024)

    // Easy to change GPIOS
    #define GPIO_TRIG 23
    #define GPIO_ECHO 24

    int  mem_fd     = 0;
    unsigned char *gpio_mmap = NULL;
    char *gpio_ram  = NULL;
    volatile uint32_t *gpio = NULL;
    int g = 0;

    
    void setup_io(); // Set up memory regions to access GPIO
    void close_io(); // Free memory regions
    void nsleep(long us); //delay in usec using nanosleep rather than sleep
    

    int main(int argc, char **argv)
    {
    double distance;

    struct timespec start_echo , stop_echo, diff_echo;
    double echo_time;
    

    // Set up gpi pointer for direct register access
    setup_io();

    INP_GPIO(GPIO_TRIG); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(GPIO_TRIG);
    
    INP_GPIO(GPIO_ECHO); //input for echo pin
    printf("Waiting For Sensor To Settle\n");
    nsleep(500000);

//Sens 10us to trigger
    GPIO_SET(GPIO_TRIG);
    nsleep(10); // send pulse 10us to trigger
    GPIO_CLR(GPIO_TRIG);
    
    clock_gettime(CLOCK_REALTIME, &start_echo);
    while ((GPIO_GET >> GPIO_ECHO) & 0 )
    {} // Boucler tant que le bit 23 est à 0, puis enregistrer le temps.
    clock_gettime(CLOCK_REALTIME, &start_echo);    

    //clock_gettime(CLOCK_REALTIME, &stop_echo);
    while ((GPIO_GET >> GPIO_ECHO) & 1 )
    {} // Boucler tant que le bit 23 est à 1, puis enregistrer le temps.
    clock_gettime(CLOCK_REALTIME, &stop_echo);
    

//calcul de la duree de l'echo
if ((stop_echo.tv_nsec - start_echo.tv_nsec) < 0 /* ns */) {
        diff_echo.tv_sec = stop_echo.tv_sec - start_echo.tv_sec - 1;
        diff_echo.tv_nsec = 1000000000 /* ns */ + stop_echo.tv_nsec - start_echo.tv_nsec;
    } else {
        diff_echo.tv_sec = stop_echo.tv_sec - start_echo.tv_sec;
        diff_echo.tv_nsec = stop_echo.tv_nsec - start_echo.tv_nsec;
    }
echo_time = ((diff_echo.tv_sec*1000000000) + (diff_echo.tv_nsec)); //nsec
echo_time /= 1000000000;//sec
//calcul de la distance en cm
distance = (echo_time*17150); //cm

printf("Distance: %.2f cm\n", distance);

close_io();

return 0;
} // main


//
// Set up a memory regions to access GPIO
//
void setup_io()
{

   /* open /dev/mem to get acess to physical ram */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem. Did you run the program with administrator rights?\n");
      exit (-1);
   }

   /* Allocate a block of virtual RAM in our application's address space */
   if ((gpio_ram = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("allocation error \n");
      exit (-1);
   }

   /* Make sure the pointer is on 4K boundary */
   if ((unsigned long)gpio_ram % PAGE_SIZE)
     gpio_ram += PAGE_SIZE - ((unsigned long)gpio_ram % PAGE_SIZE);

   /* Now map the physical addresses of the peripheral control registers 
      into our address space */
   gpio_mmap = (unsigned char *)mmap(
      (caddr_t)gpio_ram,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED|MAP_FIXED,
      mem_fd,
      GPIO_ADD
   );

   if ((long)gpio_mmap < 0) {
      printf("unable to map the memory. Did you run the program with administrator rights?\n");
      exit (-1);
   }

   /* Always use a volatile pointer to hardware registers */
   gpio = (volatile uint32_t *)gpio_mmap;
} // setup_io

//
/* Release GPIO memory region */
//
void close_io()
{
	int ret;

	/* munmap GPIO */
	ret = munmap(gpio_mmap, BLOCK_SIZE);
	if (ret == -1) {
		perror("munmap() failed");
		exit(1);
	}

	/* close /dev/mem */
	ret = close(mem_fd);
	if (ret == -1) {
		perror("Cannot close /dev/mem");
		exit(1);
	}
}



void nsleep(long us)
{
struct timespec wait;
wait.tv_sec = us / (1000 * 1000);
wait.tv_nsec = (us % (1000 * 1000)) * 1000;
nanosleep(&wait, NULL);
}



