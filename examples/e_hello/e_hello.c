#include <stdio.h>
#include <e-hal.h>
#include <e-loader.h>

int main(int argc, char **argv)
{
    e_platform_t platform;
    e_epiphany_t dev;

    e_init(NULL);
    e_reset_system();
    e_get_platform_info(&platform);
    e_open(&dev, 0, 0, 1, 1);
    e_reset_group(&dev);

    e_load_group("bin/e_hello.srec", &dev, 0, 0, 1, 1, E_FALSE);

    int test = 12345;
    e_write(&dev, 0, 0, 0x7000, &test, sizeof(int));

    e_start_group(&dev);

    usleep(100000);
    int buf;
    e_read(&dev, 0, 0, 0x7000, &buf, sizeof(int));

    printf("Obtained %i put in %i\n", buf, test);

    e_finalize();

}
