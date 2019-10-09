#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PWM_MAGIC 'P'
#define PWM_ON _IOW(PWM_MAGIC, 0, struct pwm_capture_t)
#define PWM_OFF _IOW(PWM_MAGIC, 1, struct pwm_capture_t)

struct pwm_capture_t {
	unsigned int period;
	unsigned int duty_cycle;
};

int main(int argc, char **argv)
{
	int fd;
	int a;
	int period, duty_cycle;
	if (argc < 4) {
		printf("usage: %s on/off duty_cycle period\n", argv[0]);
		return -1;
	}
	duty_cycle = strtol(argv[2], NULL, 10);
	period = strtol(argv[3], NULL, 10);
	if (duty_cycle > period) {
		printf("duty_cycle shouldn't larger than period\n");
		return -1;
	}
	struct pwm_capture_t pwmm = {period, duty_cycle};

	fd = open("/dev/mypwm", O_RDWR);
	if (fd < 0)
		return -1;
	while(1) {
		for(a = 0; a < period; a += 5) {
			pwmm.duty_cycle = a;
			ioctl(fd, PWM_ON, &pwmm);
			usleep(1000);
		}
	}
	
/*
	if (strcmp(argv[1], "on") == 0) {
		ioctl(fd, PWM_ON, &pwmm);
	} else if (strcmp(argv[1], "off") == 0) {
		ioctl(fd, PWM_OFF, &pwmm);
	} else {
		printf("invalid parameter\n");
	}
*/
	close(fd);
	return 0;
}
