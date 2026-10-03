#include <zephyr/init.h>
#include <zephyr/sys/printk.h>

static int my_board_init(void)
{
    /* This prints to your primary UART (USART1) early in the boot sequence */
    printk("Board Initialized\n");
    return 0;
}

/*
 * Hook the function into the APPLICATION initialization phase.
 * This runs after the hardware clock and UART driver are ready, 
 * but right before the kernel shifts execution over to main().
 */
// 50 is a safe default application priority value in Zephyr
SYS_INIT(my_board_init, APPLICATION, 50);

