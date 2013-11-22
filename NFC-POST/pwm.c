#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PWM_IOCTL_SET_FREQ		1
#define PWM_IOCTL_STOP			0

#define	ESC_KEY		0x1b

static void close_buzzer(void);
static int fd = -1;

static void set_buzzer_freq(int freq)
{
    // this IOCTL command is the key to set frequency
    int ret = ioctl(fd, PWM_IOCTL_SET_FREQ, freq);
    if(ret < 0) {
        perror("set the frequency of the buzzer");
        exit(1);
    }
}

static void close_buzzer(void)
{
    if (fd >= 0) {
        ioctl(fd, PWM_IOCTL_STOP);
        if (ioctl(fd, 2) < 0) {
                perror("ioctl 2:");
        }
        close(fd);
        fd = -1;
    }
}

static void open_buzzer(void)
{
    fd = open("/dev/pwm", 0);
    if (fd < 0) {
        perror("open pwm_buzzer device");
        exit(1);
    }

    // any function exit call will stop the buzzer
    atexit(close_buzzer);
}

static void stop_buzzer(void)
{
    int ret = ioctl(fd, PWM_IOCTL_STOP);
    if(ret < 0) {
        perror("stop the buzzer");
        exit(1);
    }
    if (ioctl(fd, 2) < 0) {
        perror("ioctl 2:");
    }
}

void pwmStart(){
    int freq = 1000 ;

    open_buzzer();
    set_buzzer_freq(freq);
}

void pwmStop(){
    close_buzzer();
}


void pwm()
{
    int freq = 1000 ;

    open_buzzer();
    set_buzzer_freq(freq);

    sleep(1);

    close_buzzer();
    return 0;
}
