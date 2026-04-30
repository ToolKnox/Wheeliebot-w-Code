#include "ssd1362.h"
#include "oled_font.h"

// uint8_t G_ram[5120];
/*Define the display coordinate offset.*/
uint8_t x_offset = 0;

void OLED_GPIOInit(void) {
  pinMode(SPI_SCK, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  // pinMode(SPI_MISO,INPUT);
  pinMode(SPI_CS, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  pinMode(OLED_RES, OUTPUT);
}
void OLED_WR_REG(uint8_t reg) {
  uint8_t i;
  OLED_DC_Clr();
  OLED_CS_Clr();
  for (i = 0; i < 8; i++) {
    OLED_SCK_Clr();
    if (reg & 0x80) {
      OLED_MOSI_Set();
    } else {
      OLED_MOSI_Clr();
    }
    OLED_SCK_Set();
    reg <<= 1;
  }
  OLED_CS_Set();
  OLED_DC_Set();
}

void OLED_WR_DATA8(uint8_t dat) {
  uint8_t i;
  OLED_DC_Set();
  OLED_CS_Clr();
  for (i = 0; i < 8; i++) {
    OLED_SCK_Clr();
    if (dat & 0x80) {
      OLED_MOSI_Set();
    } else {
      OLED_MOSI_Clr();
    }
    OLED_SCK_Set();
    dat <<= 1;
  }
  OLED_CS_Set();
  OLED_DC_Set();
}

void Column_Address(uint16_t a, uint16_t b)  //Set column address
{
  OLED_WR_REG(0x15);  // Set Column Address
  OLED_WR_REG(a + x_offset);
  OLED_WR_REG(b + x_offset);
}

void Row_Address(uint16_t a, uint16_t b) {
  OLED_WR_REG(0x75);  // Row Column Address
  OLED_WR_REG(a);
  OLED_WR_REG(b);
}

/**
 * @brief       OLED filling function
 * @param       x1:The starting address of the filled column
 * @param       x2:End address of the filled column
 * @param       y1:The starting address of the padding row
 * @param       y2:End address of the filled row
 * @retval      
 */
void OLED_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color) {
  uint8_t j, i;
  x1 /= 2;
  x2 /= 2;
  Column_Address(x1, x2 - 1);
  Row_Address(y1, y2 - 1);
  for (i = y1; i < y2; i++) {
    for (j = x1; j < x2; j++) {
      OLED_WR_DATA8(color);
    }
  }
}


/**
 * @brief       OLED displays individual characters
 * @param       x:The starting address of the character column
 * @param       y:The starting address of the character row
 * @param       chr:Characters to be displayed
 * @param       sizey:Font size
 * @param       mode:Whether to display in reverse color
 * @retval      
 */
void OLED_ShowChar(uint16_t x, uint16_t y, uint8_t chr, uint16_t sizey, uint8_t mode) {
  uint8_t i, j, c, t = 4, m, temp, DATA = 0;
  uint8_t num;
  x /= 2;
  c = chr - ' ';
  num = (sizey / 16 + ((sizey % 16) ? 1 : 0)) * sizey;
  Column_Address(x, x + sizey / 4 - 1);
  Row_Address(y, y + sizey - 1);
  for (i = 0; i < num; i++) {
    if (sizey == 16) temp = ascii_1608[c][i];       //Call 8x16 characters
    else if (sizey == 24) temp = ascii_2412[c][i];  //Call for 12x24 characters
    else if (sizey == 32) temp = ascii_3216[c][i];  //Call 16x32 characters
    //else if(sizey==48)temp=ascii_4824[c][i];//Call 24x18 characters
    else return;
    if (sizey % 16) {
      m = sizey / 16 + 1;
      if (i % m) {
        t = 2;
      } else {
        t = 4;
      }
    }
    for (j = 0; j < t; j++) {
      if (temp & (0x01 << (j * 2 + 0))) {
        DATA = 0xf0;
      }
      if (temp & (0x01 << (j * 2 + 1))) {
        DATA |= 0x0f;
      }
      if (mode) {
        OLED_WR_DATA8(~DATA);
      } else {
        OLED_WR_DATA8(DATA);
      }
      DATA = 0;
    }
  }
}

/**
 * @brief       OLED display string
 * @param       x:The starting address of the character column
 * @param       y:The starting address of the character row
 * @param       *dp:The string that needs to be displayed
 * @param       sizey:Font size
 * @param       mode:Whether to display in reverse color
 * @retval      
 */
void OLED_ShowString(uint16_t x, uint16_t y, char *dp, uint16_t sizey, uint8_t mode) {
  while (*dp != '\0') {
    OLED_ShowChar(x, y, *dp, sizey, mode);
    dp++;
    x += sizey / 2;
  }
}

/**
 * @brief       Exponentiation operation
 * @param       m:the truth of a matter
 * @param       n:index
 * @retval      result:The n-th power of m
 */

uint32_t oled_pow(uint8_t m, uint8_t n) {
  uint32_t result = 1;
  while (n--) {
    result *= m;
  }
  return result;
}

/**
 * @brief       OLED display variables
 * @param       x:The starting address of the variable column
 * @param       y:The starting address of the variable row
 * @param       num:The values of the variables that need to be displayed
 * @param       len:The number of bits for the variable values to be displayed
 * @param       sizey:font size
 * @param       mode:Whether to display in reverse color
 * @retval      
 */
void OLED_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint16_t sizey, uint8_t mode) {
  uint8_t t, temp;
  uint8_t enshow = 0;
  for (t = 0; t < len; t++) {
    temp = (num / oled_pow(10, len - t - 1)) % 10;
    if (enshow == 0 && t < (len - 1)) {
      if (temp == 0) {
        OLED_ShowChar(x + (sizey / 2) * t, y, ' ', sizey, mode);
        continue;
      } else {
        enshow = 1;
      }
    }
    OLED_ShowChar(x + (sizey / 2) * t, y, temp + '0', sizey, mode);
  }
}

/**
 * @brief       OLED displays 16-color images
 * @param       x:The starting address of the image list
 * @param       y:Starting address of the image row
 * @param       length:Image width
 * @param       width:Image height
 * @param       BMP[]:The content shown in the picture
 * @param       mode:Whether to display in reverse color
 * @retval      
 */
void OLED_DrawBMP(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t BMP[], uint8_t mode) {
  uint16_t i, num;
  length = (length / 2 + ((length % 2) ? 1 : 0)) * 2;
  num = length / 2 * width;
  x /= 2;
  length /= 2;
  Column_Address(x, x + length - 1);
  Row_Address(y, y + width - 1);
  for (i = 0; i < num; i++) {
    if (mode) {
      OLED_WR_DATA8(~BMP[i]);
    } else {
      OLED_WR_DATA8(BMP[i]);
    }
  }
}

/**
 * @brief       OLED displays monochrome images
 * @param       x:The starting address of the image list
 * @param       y:Starting address of the image row
 * @param       length:Image width
 * @param       width:Image height
 * @param       BMP[]:The content shown in the picture
 * @param       mode:Whether to display in reverse color
 * @retval      
 */
void OLED_DrawSingleBMP(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t BMP[], char luminance, uint8_t mode) {
  uint8_t k, DATA = 0;
  uint16_t i, num;
  uint8_t luminance_pixel_H;
  uint8_t luminance_pixel_L;
  length = (length / 8 + ((length % 8) ? 1 : 0)) * 8;
  num = length * width / 8;
  x /= 2;
  length /= 2;
  Column_Address(x, x + length - 1);
  Row_Address(y, y + width - 1);
  if(luminance>15){
    luminance = 15;
  }else if(luminance < 0){
    luminance = 0;
  }
  luminance_pixel_H = luminance << 4;
  luminance_pixel_L = luminance;

  for (i = 0; i < num; i++) {
    for (k = 0; k < 4; k++) {
      if (BMP[i] & (0x01 << (k * 2 + 0))) {
        DATA = luminance_pixel_H;
      }
      if (BMP[i] & (0x01 << (k * 2 + 1))) {
        DATA |= luminance_pixel_L;
      }
      if (mode) {
        OLED_WR_DATA8(~DATA);
      } else {
        OLED_WR_DATA8(DATA);
      }
      DATA = 0;
    }
  }
}

// void OLED_Refresh(void)
// {
//   int i;
//   Column_Address(0,159);
//   Row_Address(0,63);
//   for(i=0;i<5120;i++)
//   {
//     OLED_WR_DATA8(BMP[i]);
//   }
// }


void OLED_Init(void) {
  OLED_GPIOInit();

  // OLED_RES_Set();
  delay(20);
  // OLED_RES_Clr();
  delay(20);
  // OLED_RES_Set();
  delay(120);

  OLED_WR_REG(0xAE);  //Set Display Off
  OLED_WR_REG(0x15);  // Set Column Address
  OLED_WR_REG(0x00);
  OLED_WR_REG(0x4F);
  OLED_WR_REG(0x75);  // Set Row Address
  OLED_WR_REG(0x00);
  OLED_WR_REG(0x3F);
  OLED_WR_REG(0x81);  // Set Contrast Control
  OLED_WR_REG(0xB0);
  OLED_WR_REG(0xA0);  // Set Re-map
  if (USE_HORIZONTAL == 0) {
    OLED_WR_REG(0xC1);
    x_offset = 0;
  } else {
    OLED_WR_REG(0x50);
    x_offset = 0x30;
  }
  OLED_WR_REG(0xA1);  // Set Display Start Line
  OLED_WR_REG(0x00);
  OLED_WR_REG(0xA2);  // Set Display Offset
  OLED_WR_REG(0x00);
  OLED_WR_REG(0xA3);  // Set Vertical Scroll Area
  OLED_WR_REG(0x00);
  OLED_WR_REG(0x40);
  OLED_WR_REG(0xA4);  // Set Display Mode (Normal display)
  OLED_WR_REG(0xA8);  // Set MUX Ratio
  OLED_WR_REG(0x3F);
  OLED_WR_REG(0xAB);  // Function Selection A
  OLED_WR_REG(0x01);  // Select external VDD
  OLED_WR_REG(0xAD);  // External /Internal IREF Selection
  OLED_WR_REG(0x8E);  // Select external IREF
  OLED_WR_REG(0xB1);  // Set Phase Length
  OLED_WR_REG(0x82);
  OLED_WR_REG(0xB3);  // Set Front Clock Divider/Oscillator Frequency
  OLED_WR_REG(0xA0);
  OLED_WR_REG(0xB6);  // Set Second precharge Period
  OLED_WR_REG(0x04);
  OLED_WR_REG(0xB9);  // Linear LUT
  OLED_WR_REG(0xBC);  // Set Pre-charge voltage
  OLED_WR_REG(0x04);
  OLED_WR_REG(0xBD);  // Pre-charge voltage capacitor Selection
  OLED_WR_REG(0x01);
  OLED_WR_REG(0xBE);                      // Set VCOMH
  OLED_WR_REG(0x05);                      // 0.82*VCC
  OLED_Fill(0, 0, OLED_W, OLED_H, 0x00);  //clear screen
  OLED_WR_REG(0xAF);                      // Set Display ON
}
