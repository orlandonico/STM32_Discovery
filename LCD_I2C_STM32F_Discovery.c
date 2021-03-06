/**========================================================================
 * ?                                ABOUT
 * @author         :  N.O.
 * @email          :
 * @repo           :  v.7
 * @createdOn      :  31/07/2021
 * @description    :  USE LCD I2C
 *========================================================================**/

/**========================================================================
 * *                      RISULTATI LCD_16X02_I2C
 *
 * Simboli: ✔ ✗
 *
 * ✔ Trasformare un float in carattere e stamparlo sul LCD 16X02 i2c
 * ✔ Manipolare gli elementi del vettore "static uint8_t *message[]"
 * ✔ Lettura temperatura Chip RP2040
 * ✔ Illuminare singoli elementi del LCD 16X02 i2c (createChar)
 * ✔ Stampare mediante bit map
 *========================================================================**/

/*================================ Include ==============================*/

#include "defines.h"

/*================================ Define & Const ==============================*/
const int LCD_CLEARDISPLAY = 0x01;
const int LCD_RETURNHOME = 0x02;
const int LCD_ENTRYMODESET = 0x04;
const int LCD_DISPLAYCONTROL = 0x08;
const int LCD_CURSORSHIFT = 0x10;
const int LCD_FUNCTIONSET = 0x20;
const int LCD_SETCGRAMADDR = 0x40;
const int LCD_SETDDRAMADDR = 0x80;

// flags for display entry mode
const int LCD_ENTRYSHIFTINCREMENT = 0x01;
const int LCD_ENTRYLEFT = 0x02;

// flags for display and cursor control
const int LCD_BLINKON = 0x01;
const int LCD_CURSORON = 0x02;
const int LCD_DISPLAYON = 0x04;

// flags for display and cursor shift
const int LCD_MOVERIGHT = 0x04;
const int LCD_DISPLAYMOVE = 0x08;

// flags for function set
const int LCD_5x10DOTS = 0x04;
const int LCD_2LINE = 0x08;
const int LCD_8BITMODE = 0x10;

// flag for backlight control
const int LCD_BACKLIGHT = 0x08;
const int LCD_ENABLE_BIT = 0x04;

// By default these LCD display drivers are on bus address 0x27
static int addr_lcd = 0x4E;

// Modes for lcd_send_byte
#define LCD_CHARACTER 1
#define LCD_COMMAND 0

#define MAX_LINES 2
#define MAX_CHARS 16

#define DELAY_MS 1000
#define TIME 1000
#define DELAY_US 60

/*================================ Function prototypes ==============================*/

void initialize(void);
void i2c_write_byte(uint8_t val);
void lcd_toggle_enable(uint8_t val);
void lcd_send_byte(uint8_t val, int mode);
void lcd_clear(void);
void lcd_set_cursor(int line, int position);
static void inline lcd_char(char val);
void lcd_string(const char *s);
void lcd_init();
void createChar(uint8_t location, uint8_t charmap[]);
void sleep_us(unsigned int delay);

/*================================ Main  ==============================*/

int main()
{
    static uint8_t *message[] = {"Hello by", "STM32F4"};

    //* Il massimo numero di elementi di questo vettore sono 8, perch+ la memoria è di 8 byte
    static uint8_t my_bitmap[][8] =
        {
            // chequer
            {0x15, 0x0A, 0x15, 0x0A, 0x15, 0x0A, 0x15, 0x00},
            // up arrow
            {0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x00, 0x00},
            // down arrow
            {0x00, 0x00, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04},
            // rectangle
            {0x00, 0x1F, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00},
            // up-left arrow
            {0x1F, 0x1E, 0x1C, 0x1A, 0x11, 0x00, 0x00, 0x00},
            // up-right arrow
            {0x1F, 0x0F, 0x07, 0x0B, 0x11, 0x00, 0x00, 0x00},
            // down-left arrow
            {0x00, 0x00, 0x00, 0x11, 0x1A, 0x1C, 0x1E, 0x1F},
            // down-right arrow
            {0x00, 0x00, 0x00, 0x11, 0x0B, 0x07, 0x0F, 0x1F},
        };

    //* Il massimo numero di elementi di questo vettore sono 8, perch+ la memoria è di 8 byte
    static uint8_t my_bitmap_2[][8] =
        {
            // heart
            {0b00000, 0b01010, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000},
            // bell
            {0b00100, 0b01110, 0b01110, 0b01110, 0b11111, 0b00000, 0b00100, 0b00000},
            // alien
            {0b11111, 0b10101, 0b11111, 0b11111, 0b01110, 0b01010, 0b11011, 0b00000},
            //check
            {0b00000, 0b00001, 0b00011, 0b10110, 0b11100, 0b01000, 0b00000, 0b00000},
            //speaker
            {0b00001, 0b00011, 0b01111, 0b01111, 0b01111, 0b00011, 0b00001, 0b00000},
            //sound
            {0b00001, 0b00011, 0b00101, 0b01001, 0b01001, 0b01011, 0b11011, 0b11000},
            //skull
            {0b00000, 0b01110, 0b10101, 0b11011, 0b01110, 0b01110, 0b00000, 0b00000},
            //lock
            {0b01110, 0b10001, 0b10001, 0b11111, 0b11011, 0b11011, 0b11111, 0b00000},
        };

    initialize();

    STM_EVAL_LEDOn(LED4);

    while (1)
    {
        lcd_clear();

        // Routine che stampa stringhe
        for (int m = 0; m < sizeof(message) / sizeof(message[0]); m += MAX_LINES)
        {
            for (int line = 0; line < MAX_LINES; line++)
            {
                lcd_set_cursor(line, (MAX_CHARS / 2) - strlen(message[m + line]) / 2);
                lcd_string(message[m + line]);
            }
            Delayms(1000);
            lcd_clear();
        }
        lcd_clear();

        // Routine che stampa my_bitmap
        int size_my_bipmap = sizeof(my_bitmap) / sizeof(my_bitmap[0]);
        for (int i = 0; i < size_my_bipmap; i++)
        {
            createChar(i, (uint8_t *)my_bitmap[i]);
        }
        for (int i = 0; i < size_my_bipmap; i++)
        {
            lcd_set_cursor(0, i);
            lcd_send_byte(i, 1);
        }
        Delayms(2000);
        lcd_clear();

        // Routine che stampa my_bitmap_2
        int size_my_bipmap_2 = sizeof(my_bitmap_2) / sizeof(my_bitmap_2[0]);
        for (int i = 0; i < size_my_bipmap_2; i++)
        {
            createChar(i, (uint8_t *)my_bitmap_2[i]);
        }
        for (int i = 0; i < size_my_bipmap_2; i++)
        {
            lcd_set_cursor(0, i);
            lcd_send_byte(i, 1);
        }
        Delayms(2000);
        lcd_clear();

    }
    return 0;
}

/*================================ INFO ==============================*/

/**========================================================================
 * *                        TM_I2C_WriteNoRegister
 * TM_I2C_WriteNoRegister(I2C_TypeDef* I2Cx, uint8_t address, uint8_t data);
 *
 * @brief  Writes byte to slave without specify register address
 *
 *         Useful if you have I2C device to read like that:
 *            - I2C START
 *            - SEND DEVICE ADDRESS
 *            - SEND DATA BYTE
 *            - I2C STOP
 *
 * Parameters:
 * @param  *I2Cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  data: data byte which will be send to device
 * @retval None
 *========================================================================**/

/*================================ FUNCTION  ==============================*/

/**========================================================================
 **                           FUNCTION NAME
 *?  What does it do?
 *@param name type
 *@param name type
 *@return type
 *========================================================================**/

/**========================================================================
 **                           i2c_initialize
 *?  Inizializza
 *@param void
 *@return void
 *========================================================================**/
void initialize(void)
{
    /* Initialize system */
    SystemInit();

    /* Initialize I2C, SCL: PB6 and SDA: PB7 with 100kHz serial clock */
    TM_I2C_Init(I2C1, TM_I2C_PinsPack_1, 100000);

    /* Initialize Delay */
    TM_DELAY_Init();

    /* Initialize Delay */
    lcd_init();

    /* Initialize Led */
    STM_EVAL_LEDInit(LED3); //Orange
    STM_EVAL_LEDInit(LED4); //Green
    STM_EVAL_LEDInit(LED5); //Red
    STM_EVAL_LEDInit(LED6); // Blue
}

/**========================================================================
 **                           i2c_write_byte
 *?  Quick helper function for single byte transfers
 *@param uint8_t val
 *@return void
 *========================================================================**/
void i2c_write_byte(uint8_t val)
{
    TM_I2C_WriteNoRegister(I2C1, addr_lcd, val);
}

/**========================================================================
 **                            lcd_toggle_enable
 *?  Toggle enable pin on LCD display
 *?  We cannot do this too quickly or things don't work
 *@param uint8_t val
 *@return void
 *========================================================================**/
void lcd_toggle_enable(uint8_t val)
{
    //Delayms(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    //Delayms(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    //Delayms(DELAY_US);
}

/**========================================================================
 **                           lcd_send_byte
 *?  The display is sent a byte as two separate nibble transfers
 *@param uint8_t val
 *@param int mode
 *@return void
 *========================================================================**/
void lcd_send_byte(uint8_t val, int mode)
{
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

/**========================================================================
 **                            lcd_clear
 *?  Recall lcd_send_byte
 *@param null
 *@return void
 *========================================================================**/
void lcd_clear(void)
{
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

/**========================================================================
 **                           lcd_set_cursor
 *?  go to location on LCD
 *@param int line
 *@param int position
 *@return void
 *========================================================================**/
void lcd_set_cursor(int line, int position)
{
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

/**========================================================================
 **                           lcd_char
 *?  Recall lcd_send_byte
 *@param char val
 *@return void
 *========================================================================**/
static void inline lcd_char(char val)
{
    lcd_send_byte(val, LCD_CHARACTER);
}

/**========================================================================
 **                        lcd_string
 *?  What does it do?
 *@param const char *s
 *@return void
 *========================================================================**/
void lcd_string(const char *s)
{
    while (*s)
    {
        lcd_char(*s++);
    }
}

/**========================================================================
 **                         lcd_init
 *?  initialize Lcd
 *@param null
 *@return void
 *========================================================================**/
void lcd_init()
{
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear();
}

/**========================================================================
 **                           createChar
 *?  Crea un nuovo carattere e lo inserisce in memoria, al massimo 8
 *@param uint8_t location
 *@param uint8_t charmap[]
 *@return void
 *========================================================================**/
void createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7; // we only have 8 locations 0-7
    lcd_send_byte((LCD_SETCGRAMADDR | (location << 3)), 0);
    for (int i = 0; i < 8; i++)
    {
        lcd_send_byte(charmap[i], 1);
    }
}
