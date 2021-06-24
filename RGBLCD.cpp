#include <Arduino.h>
#include <User_Setup.h>
#include "RGBLCD.h"


static void MX_LTDC_Init(const uint32_t addr);

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

LTDC_HandleTypeDef    hltdc;
static void MX_LTDC_Init(const uint32_t addr)
{

  /* USER CODE BEGIN LTDC_Init 0 */
    pinMode(LCD_RST,OUTPUT);
    pinMode(LCD_BL,OUTPUT);
    delay(10);
    digitalWrite(LCD_RST, LOW);         //reset operate
    delay(10);
    digitalWrite(LCD_RST, HIGH);
    delay(10);
    digitalWrite(LCD_BL, HIGH);         //backlight enable
  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 0;
  hltdc.Init.VerticalSync = 0;
  hltdc.Init.AccumulatedHBP = 46;
  hltdc.Init.AccumulatedVBP = 23;
  hltdc.Init.AccumulatedActiveW = 846;
  hltdc.Init.AccumulatedActiveH = 503;
  hltdc.Init.TotalWidth = 868;
  hltdc.Init.TotalHeigh = 525;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 255;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    digitalWrite(LED_RED, HIGH);
  }

  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 800;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 480;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = addr;
  pLayerCfg.ImageWidth = 800;
  pLayerCfg.ImageHeight = 480;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    digitalWrite(LED_RED, HIGH);
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
    PeriphClkInitStruct.PLL3.PLL3M = 25;
    PeriphClkInitStruct.PLL3.PLL3N = 55;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* LTDC clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**LTDC GPIO Configuration
    PB8     ------> LTDC_B6
    PD6     ------> LTDC_B2
    PH13     ------> LTDC_G2
    PB9     ------> LTDC_B7
    PG14     ------> LTDC_B0
    PA15(JTDI)     ------> LTDC_R3
    PD0     ------> LTDC_B1
    PA8     ------> LTDC_R6
    PC6     ------> LTDC_HSYNC
    PC7     ------> LTDC_G6
    PG7     ------> LTDC_CLK
    PA7     ------> LTDC_VSYNC
    PA1     ------> LTDC_R2
    PD10     ------> LTDC_B3
    PA5     ------> LTDC_R4
    PB1     ------> LTDC_G0
    PE12     ------> LTDC_B4
    PB10     ------> LTDC_G4
    PH11     ------> LTDC_R5
    PH3     ------> LTDC_R1
    PB0     ------> LTDC_G1
    PB11     ------> LTDC_G5
    PB15     ------> LTDC_G7
    PH2     ------> LTDC_R0
    PA3     ------> LTDC_B5
    PC5     ------> LTDC_DE
    PE11     ------> LTDC_G3
    PE15     ------> LTDC_R7
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_1|GPIO_PIN_10
                          |GPIO_PIN_0|GPIO_PIN_11|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_0|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_11|GPIO_PIN_3|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_1|GPIO_PIN_5
                          |GPIO_PIN_3;
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
    PD6     ------> LTDC_B2
    PH13     ------> LTDC_G2
    PB9     ------> LTDC_B7
    PG14     ------> LTDC_B0
    PA15(JTDI)     ------> LTDC_R3
    PD0     ------> LTDC_B1
    PA8     ------> LTDC_R6
    PC6     ------> LTDC_HSYNC
    PC7     ------> LTDC_G6
    PG7     ------> LTDC_CLK
    PA7     ------> LTDC_VSYNC
    PA1     ------> LTDC_R2
    PD10     ------> LTDC_B3
    PA5     ------> LTDC_R4
    PB1     ------> LTDC_G0
    PE12     ------> LTDC_B4
    PB10     ------> LTDC_G4
    PH11     ------> LTDC_R5
    PH3     ------> LTDC_R1
    PB0     ------> LTDC_G1
    PB11     ------> LTDC_G5
    PB15     ------> LTDC_G7
    PH2     ------> LTDC_R0
    PA3     ------> LTDC_B5
    PC5     ------> LTDC_DE
    PE11     ------> LTDC_G3
    PE15     ------> LTDC_R7
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_1|GPIO_PIN_10
                          |GPIO_PIN_0|GPIO_PIN_11|GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_6|GPIO_PIN_0|GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_13|GPIO_PIN_11|GPIO_PIN_3|GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_14|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_1
                          |GPIO_PIN_5|GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_15);

    /* LTDC interrupt Deinit */
    HAL_NVIC_DisableIRQ(LTDC_IRQn);
  /* USER CODE BEGIN LTDC_MspDeInit 1 */

  /* USER CODE END LTDC_MspDeInit 1 */
  }
}