#include <stdio.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

static struct itimerval timer;
//Signal Handler
void timer_handler(int signum){
    setitimer(ITIMER_VIRTUAL,0,NULL);
    static int count = 0;
    printf("Timer has expired %d times\n", ++ count);
    sleep(1);
    setitimer(ITIMER_VIRTUAL,&timer,NULL);
}



int main(){
    //Creating the signal structure
    struct sigaction sa;
    //struct itimerval timer;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    //I believe we can make the below SIGNAL any signal we want, so prefer
    //SIGINT?
    sigaction(SIGVTALRM, &sa, NULL);
    
    //Set up the timer to occur every 25msecs
   	//second and milliseconds both included so you can have x.y seconds (we think)
	//timeval struct containing time until timer next expires and signal sent. if it's 0, timer is disabled
	timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 25000;
	//timeval struct with value to which timer will be reset after expiration. if 0, timer will be disabled after the interval, else will expire repeatedly after this interval
	//sets the signal for when timers expire to the specified handler
    //if ITIMER_REAL, process sent to SIGALRM after WALL CLOCK time elapsed. if ITIMER_VIRTUAL, sent to SIGVWTALRM after specified PROCESS RUNTIME (i.e. execution time in user mode)
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
    //setitimer(ITIMER_VIRTUAL,0,NULL);
    //setitimer(ITIMER_VIRTUAL,0,NULL);
    //setitimer(ITIMER_VIRTUAL,&timer,NULL);
    //At this point you should be done
    int i = 0;
    while(i != 2000000000){
        printf("IN MAIN!\n");
        i+=1;
    }
    return 0;
}
