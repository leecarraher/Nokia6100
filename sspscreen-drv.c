#include  <linux/module.h>
#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/arch/hardware.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch/ssp.h>
#include <asm/arch/irqs.h>
#include <asm/errno.h>
#include "sspscreen-drv.h"


//**********************************************************************
// NSSP lines and GPIOs for pxa27xx, nssp1 is dedicated to the lcd
// so we have to use nssp2 lines (pxa25 were pins 81-84)
static int gNSSPClk = 19;   //NSSP2_CLOCK
static int gNSSPCS = 14;    //NSSP2_FRAME
static int gNSSPData = 13;  //NSSP2_TXD MOSI
static int gGPIOReset = 11; //NSSP2_RXD MISO

static struct ssp_dev gSpiDevice;

// Delay, this may need some tuning as i still seem to be getting noise
static void screen_delay(int n)
{
   int time_to_stop = get_jiffies_64() + n;
   int time_now;

   do
   {
      time_now = get_jiffies_64();
      cpu_relax();
      //printk("n: %d now: %d stop: %d\n", n, time_now, time_to_stop);
   }while(time_now < time_to_stop);
}

//**********************************************************************
// SET/CLEAR
static void set_gpio(int gpio)
{
   GPSR(gpio) = GPIO_bit(gpio);
}

static void clear_gpio(int gpio)
{
   GPCR(gpio) = GPIO_bit(gpio);
}

//**********************************************************************
// SEND BIT/DATA/COMMAND
static void send_cmd(unsigned char b)
{
   int value;
   u32 readOut;
   readOut=0;
   value = b;
   // Write data
   ssp_write_word(&gSpiDevice, value);
   // Read to keep SPI happy!
   ssp_read_word(&gSpiDevice, &readOut);
}

static void send_data(unsigned char b)
{
   int value;
   u32 readOut;
   readOut=0;
   value = b | 0x100;

   ssp_write_word(&gSpiDevice, value);
   ssp_read_word(&gSpiDevice,&readOut);
}


//**********************************************************
//create another send command that reads from the /dev/video
//for userland access to the configured lcd screen, similar to
//a character driver

static void send_to_ssp(unsigned char b)
{
   int value;
   u32 readOut;
   readOut=0;
   value = b;

   ssp_write_word(&gSpiDevice, value);
   ssp_read_word(&gSpiDevice,&readOut);
}


//add mutex and locking later, currently multiple things can write here
int memory_open(struct inode *inode, struct file *filp) {

  /* Success */
  return 0;
}

int memory_release(struct inode *inode, struct file *filp) {

  /* Success */
  return 0;
}

ssize_t memory_write( struct file *filp, char *buf, size_t count, loff_t *f_pos) 
{
	char *tmp;
	/* Buffer writing to the device */
	char data_buffer;

  tmp=buf+count-1;
  copy_from_user(&data_buffer,tmp,1);
	//use configured ssp
	send_to_ssp(&tmp);
  return 1;
}

/* Structure that declares the usual file */
/* access functions */
struct file_operations screen_fops = {
  write: memory_write,
  open: memory_open,
  release: memory_release
};

//**********************************************************************
// INIT

static int __init screen_init( void )
{
    int rc;
    int speed = SCREEN_BAUD;
    int mode = DSS_9 | FRF_SPI | SSE_ENABLED;
    int flags = SPO | SPH;
    int result=0;
   printk("screen: init called\n");

    // register our device with Linux

    if (( rc = register_chrdev( SCREEN_MAJOR, DEVICE_NAME, &screen_fops )) < 0 )
    {
        printk( KERN_WARNING "screen: register_chrdev failed for major %d\n", SCREEN_MAJOR );
        return rc;
    }

   printk("screen: init GPIO pins\n");

   // Pin functions and directions
   pxa_gpio_mode( gNSSPData  | GPIO_ALT_FN_1_OUT );
   pxa_gpio_mode( gNSSPClk   | GPIO_ALT_FN_1_OUT );
   pxa_gpio_mode( gNSSPCS    | GPIO_ALT_FN_1_OUT );
   pxa_gpio_mode( gGPIOReset | GPIO_OUT );

   // Setup GPIO levels
   set_gpio(gGPIOReset);
   
   printk("screen: init NSSP\n");
   result=ssp_init(&gSpiDevice, 2,flags);
    if (result)
    {
      printk("ssp_init() failed!\n");
    }
   printk("ssp_init() success!%d\n",result);

   /* configure NSSP port parameters */
   ssp_config(&gSpiDevice, mode, flags, 0, speed);

   ssp_enable(&gSpiDevice);

   printk("screen: display reset\n");

   // Reset the display
   clear_gpio(gGPIOReset);
   screen_delay(10);
   set_gpio(gGPIOReset);
   screen_delay(10);

   printk("screen: display reset : control\n");
 // Display control
         send_cmd(DISCTL);
         send_data(0x00); // P1: 0x00 = 2 divisions, switching period=8 (default)
         send_data(0x20); // P2: 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
         send_data(0x00); // P3: 0x00 = no inversely highlighted lines
         // COM scan
         send_cmd(COMSCN);
         send_data(1);           // P1: 0x01 = Scan 1->80, 160<-81
         // Internal oscilator ON
         send_cmd(OSCON);
         // Sleep out
         send_cmd(SLPOUT);
         // Power control
         send_cmd(PWRCTR);
         send_data(0x0f);    // reference voltage regulator on, circuit voltage follower on, BOOST ON
         // Inverse display
         send_cmd(DISINV);
         // Data control
         send_cmd(DATCTL);
         send_data(0x01); // P1: 0x01 = page address inverted, column address normal, address scan in column direction
         send_data(0x00); // P2: 0x00 = RGB sequence (default value)
         send_data(0x02); // P3: 0x02 = Grayscale -> 16 (selects 12-bit color, type A)
         // Voltage control (contrast setting)
         send_cmd(VOLCTR);
         send_data(32); // P1 = 32 volume value (experiment with this value to get the best contrast)
         send_data(3);    // P2 = 3    resistance ratio (only value that works)
         // allow power supply to stabilize
         screen_delay(20);
         // turn on the display
         send_cmd(DISON);

	//LCDClearScreen();

	//testLCD();

   return 0;
}

//**********************************************************************
// EXIT

static void __exit screen_exit( void )
{
   printk("screen: exit called\n");

   send_cmd(SLPIN);
   screen_delay(10);
   send_cmd(DISOFF);
   screen_delay(10);

   ssp_exit(&gSpiDevice);

    unregister_chrdev( SCREEN_MAJOR, DEVICE_NAME );
}

//**********************************************************************
// MODULE SETUP

module_init(screen_init);
module_exit(screen_exit);

MODULE_AUTHOR("Lee Carraher \nEpson lcd driver info from James Lynch\nssp info from Dan Taylor");
MODULE_DESCRIPTION("Screen Driver");
MODULE_LICENSE("GPL");