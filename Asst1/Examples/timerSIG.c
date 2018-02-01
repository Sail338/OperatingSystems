#include <stdio.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

//Signal Handler
void timer_handler(int signum){
    static int count = 0;
    printf("Timer has expired %d times\n", ++ count);
}



int main(){
    //Creating the signal structure
    struct sigaction sa;
    struct itimerval timer;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    //I believe we can make the below SIGNAL any signal we want, so prefer
    //SIGINT?
    sigaction(SIGVTALRM, &sa, NULL);
    
    //Set up the timer to occur every 25msecs
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 25000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 25000;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    //At this point you should be done
    while(1);
}
