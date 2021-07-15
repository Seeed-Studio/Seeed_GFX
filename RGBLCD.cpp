#include <Arduino.h>
#include <User_Setup.h>
#include "RGBLCD.h"

#ifdef HAL_LTDC_MODULE_ENABLED

void MX_LTDC_Init(const uint32_t addr);

LCDClass::LCDClass(uint32_t addr)
{
    DataBuffer = (volatile uint16_t*)addr;
    /*border*/
    x_start = 0;
    x_end   = TFT_WIDTH;
    y_start = 0;
    y_end   = TFT_HEIGHT;
    x_border_left = x_border_right = 0;
    y_border_left = y_border_right = 0;
    /*direction*/
    rotation = ROTATION_UP;
    /*start point*/
    x = 0;
    y = 0;
}

LCDClass::~LCDClass()
{

}

void LCDClass::begin(void)
{
    MX_LTDC_Init((uint32_t)DataBuffer);
}

void LCDClass::set_state(enum LCD_STATE _state)
{
    state = _state;
}

enum LCD_STATE LCDClass::get_state(void)
{
    return state;
}

void LCDClass::set_x(uint16_t _x)
{
    x = _x;
}

void LCDClass::set_x_border(uint16_t _x_s , uint16_t _x_e)
{
    x_start = _x_s;
    x_end   = _x_e;
}

void LCDClass::get_x_border(uint16_t* _x_s , uint16_t* _x_e)
{
    *_x_s = x_start;
    *_x_e = x_end;
}

uint16_t LCDClass::get_x(void)
{
    return x;
}
void LCDClass::set_y(uint16_t _y)
{
    y = _y;
}

void LCDClass::set_y_border(uint16_t _y_s , uint16_t _y_e)
{
    y_start = _y_s;
    y_end   = _y_e;
}

void LCDClass::get_y_border(uint16_t* _y_s , uint16_t* _y_e)
{
    *_y_s = y_start;
    *_y_e = y_end;
}

uint16_t LCDClass::get_y(void)
{
    return y;
}

void LCDClass::set_color(uint32_t _color)
{
    color = _color;
}

void LCDClass::set_rotation(enum LCD_ROTATION  _rotation)
{
  rotation = _rotation;
}

enum LCD_ROTATION LCDClass::get_rotation(void)
{
  return rotation;
}

void LCDClass::draw_pixel(void)
{
    switch(rotation)
    {
      case ROTATION_UP:
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( (TFT_WIDTH * y) + x ) * 2 ) = (uint16_t)(color & 0xffff);   
      break;
      case ROTATION_LEFT:   
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( ( TFT_WIDTH * x ) + TFT_WIDTH - y - 1) * 2 ) = (uint16_t)(color & 0xffff);
      
      break;
      case ROTATION_RIGHT:   
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( ( TFT_WIDTH * (TFT_HEIGHT - x - 1) ) + y ) * 2 ) = (uint16_t)(color & 0xffff);
      break;
      case ROTATION_DOWN:   
      *(volatile uint16_t *)((uint32_t)DataBuffer + ( (TFT_WIDTH * (TFT_HEIGHT - y - 1) ) + TFT_WIDTH - x - 1) * 2 ) = (uint16_t)(color & 0xffff);
      break;
      default:
      break;
    }
}

static inline void wait(uint32_t us)
{
		for(int j = 0; j < 55; j++)
		{
			 asm volatile("nop");
		}

}

void SPI_WriteComm(uint8_t val)
{
	digitalWrite(PA1,  LOW); // CS
    
	for (uint8_t bit = 0u; bit < 9u; bit++)
	{
	   if(bit == 0)
	   {
		   digitalWrite(PH2,  LOW);
	   }else{
		  digitalWrite(PH2, (val & (1 << (8 - bit)))?HIGH:LOW);
	   }
	   wait(1);
	   digitalWrite(PH3, LOW);
	   wait(1);
	   digitalWrite(PH3, HIGH);
	}
	digitalWrite(PH2,  LOW); // SDO
	digitalWrite(PH3, LOW); // CLK

	digitalWrite(PA1,  HIGH); // CS
}

void SPI_WriteData(uint8_t val)
{

	digitalWrite(PA1,  LOW); // CS

	for (uint8_t bit = 0u; bit < 9u; bit++)
	{
	   if(bit == 0)
	   {
		    digitalWrite(PH2,  HIGH);
	   }else{
	  		digitalWrite(PH2, (val & (1 << (8 - bit)))?HIGH:LOW);
	   }
		wait(1);
	    digitalWrite(PH3, LOW);
	    wait(1);
		digitalWrite(PH3, HIGH);
	}
	digitalWrite(PH2,  LOW); // SDO
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET); // CLK

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); // CS
}

void ST7789V_Init()
{
    pinMode(PH2, OUTPUT);
    pinMode(PH3, OUTPUT);
    pinMode(PA1, OUTPUT);

    digitalWrite(PA1, HIGH); // CS

    delay(1);
	//----------------Star Initial Sequence-------//
	SPI_WriteComm(0x11);
	delay(120); //Delay 120ms

	SPI_WriteComm(0xB0);
	SPI_WriteData(0x11);
	SPI_WriteData(0xf0);//RGB interface		 //00

	SPI_WriteComm(0xB1);
	SPI_WriteData(0xC0);//40=RGB-DE-mode/60=RGB HV MODE
	SPI_WriteData(0x02);//VBP	  //02
	SPI_WriteData(0x14);//HBP	  //14

	SPI_WriteComm(0xB2);
	SPI_WriteData(0x0C);
	SPI_WriteData(0x0C);
	SPI_WriteData(0x00);
	SPI_WriteData(0x33);
	SPI_WriteData(0x33);

	SPI_WriteComm(0xB7);
	SPI_WriteData(0x35);

	SPI_WriteComm(0xBB);
	SPI_WriteData(0x28);//VCOM	   //28

	SPI_WriteComm(0xC0);
	SPI_WriteData(0x2C);

	SPI_WriteComm(0xC2);
	SPI_WriteData(0x01);

	SPI_WriteComm(0xc3);
	SPI_WriteData(0x0B);//GVDD/GVCL

	//SPI_Write_index(0xc5);
	//SPI_Write_data(0x28);//VCOM 

	SPI_WriteComm(0xC4);
	SPI_WriteData(0x20);

	SPI_WriteComm(0xC6);
	SPI_WriteData(0x0F);

	SPI_WriteComm(0xD0);
	SPI_WriteData(0xA4);
	SPI_WriteData(0xA1);
	////////////////////////////////////GAMMA
	SPI_WriteComm(0xE0);
	SPI_WriteData(0xD0);
	SPI_WriteData(0x01);
	SPI_WriteData(0x08);
	SPI_WriteData(0x0F);
	SPI_WriteData(0x11);
	SPI_WriteData(0x2A);
	SPI_WriteData(0x36);
	SPI_WriteData(0x55);
	SPI_WriteData(0x44);
	SPI_WriteData(0x3A);
	SPI_WriteData(0x0B);
	SPI_WriteData(0x06);
	SPI_WriteData(0x11);
	SPI_WriteData(0x20);

	SPI_WriteComm(0xE1);
	SPI_WriteData(0xD0);
	SPI_WriteData(0x02);
	SPI_WriteData(0x07);
	SPI_WriteData(0x0A);
	SPI_WriteData(0x0B);
	SPI_WriteData(0x18);
	SPI_WriteData(0x34);
	SPI_WriteData(0x43);
	SPI_WriteData(0x4A);
	SPI_WriteData(0x2B);
	SPI_WriteData(0x1B);
	SPI_WriteData(0x1C);
	SPI_WriteData(0x22);
	SPI_WriteData(0x1F);
	/////////////////////////////////////

	SPI_WriteComm(0x3A);
	SPI_WriteData(0x55);//16-Bit RGB/65k

	SPI_WriteComm(0x36);
	SPI_WriteData(0x00);


	SPI_WriteComm(0x29);//display on
	delay(50);

}

/* USER CODE END 0 */
LTDC_HandleTypeDef hltdc;

/* LTDC init function */
void MX_LTDC_Init(uint32_t addr)
{

  /* USER CODE BEGIN LTDC_Init 0 */
   /* USER CODE BEGIN LTDC_Init 0 */
    pinMode(PG5,OUTPUT);
    pinMode(PF5,OUTPUT);
    delay(10);
    digitalWrite(PG5, LOW);         //reset operate
    delay(10);
    digitalWrite(PG5, HIGH);
    delay(10);
    digitalWrite(PF5, HIGH);         //backlight enable

  Serial.println("I am here\n\r");
  ST7789V_Init();

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 9;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 19;
  hltdc.Init.AccumulatedVBP = 7;
  hltdc.Init.AccumulatedActiveW = 259;
  hltdc.Init.AccumulatedActiveH = 327;
  hltdc.Init.TotalWidth = 297;
  hltdc.Init.TotalHeigh = 335;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 240;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 320;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = addr;
  pLayerCfg.ImageWidth = 240;
  pLayerCfg.ImageHeight = 320;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef* ltdcHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(ltdcHandle->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspInit 0 */

  /* USER CODE END LTDC_MspInit 0 */
  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLL3.PLL3M = 5;
    PeriphClkInitStruct.PLL3.PLL3N = 48;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 5;
    PeriphClkInitStruct.PLL3.PLL3R = 20;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* LTDC clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**LTDC GPIO Configuration
    PB8     ------> LTDC_B6
    PH13     ------> LTDC_G2
    PB9     ------> LTDC_B7
    PA15(JTDI)     ------> LTDC_R3
    PA8     ------> LTDC_R6
    PC6     ------> LTDC_HSYNC
    PC7     ------> LTDC_G6
    PG7     ------> LTDC_CLK
    PA7     ------> LTDC_VSYNC
    PD10     ------> LTDC_B3
    PA5     ------> LTDC_R4
    PE12     ------> LTDC_B4
    PB10     ------> LTDC_G4
    PH11     ------> LTDC_R5
    PB11     ------> LTDC_G5
    PB15     ------> LTDC_G7
    PA3     ------> LTDC_B5
    PC5     ------> LTDC_DE
    PE11     ------> LTDC_G3
    PE15     ------> LTDC_R7
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_5|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* LTDC interrupt Init */
    HAL_NVIC_SetPriority(LTDC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
  /* USER CODE BEGIN LTDC_MspInit 1 */

  /* USER CODE END LTDC_MspInit 1 */
  }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* ltdcHandle)
{

  if(ltdcHandle->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspDeInit 0 */

  /* USER CODE END LTDC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    /**LTDC GPIO Configuration
    PB8     ------> LTDC_B6
    PH13     ------> LTDC_G2
    PB9     ------> LTDC_B7
    PA15(JTDI)     ------> LTDC_R3
    PA8     ------> LTDC_R6
    PC6     ------> LTDC_HSYNC
    PC7     ------> LTDC_G6
    PG7     ------> LTDC_CLK
    PA7     ------> LTDC_VSYNC
    PD10     ------> LTDC_B3
    PA5     ------> LTDC_R4
    PE12     ------> LTDC_B4
    PB10     ------> LTDC_G4
    PH11     ------> LTDC_R5
    PB11     ------> LTDC_G5
    PB15     ------> LTDC_G7
    PA3     ------> LTDC_B5
    PC5     ------> LTDC_DE
    PE11     ------> LTDC_G3
    PE15     ------> LTDC_R7
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_13|GPIO_PIN_11);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_5
                          |GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_15);

    /* LTDC interrupt Deinit */
    HAL_NVIC_DisableIRQ(LTDC_IRQn);
  /* USER CODE BEGIN LTDC_MspDeInit 1 */

  /* USER CODE END LTDC_MspDeInit 1 */
  }
}

#endif