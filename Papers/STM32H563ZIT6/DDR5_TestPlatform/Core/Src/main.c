/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

I3C_HandleTypeDef hi3c1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I3C1_Init(void);
static void MX_ICACHE_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_MEMORYMAP_Init(void);
/* USER CODE BEGIN PFP */
static void I3C_DBG(uint32_t tag);
#define I3C_DBG_MAX 160

typedef struct
{
    uint32_t tag;
    uint32_t EVR;
    uint32_t SER;
    uint32_t SR;
    uint32_t CFGR;
    uint32_t CR;
    uint32_t RDR;
} I3C_DebugSnap;

volatile I3C_DebugSnap i3c_dbg[I3C_DBG_MAX];
volatile uint32_t i3c_dbg_i = 0;

void I3C_SETAASA(void)
{
	I3C_XferTypeDef I3C_Context;
	  uint32_t I3C_ControlBuffer[1];
	  uint8_t dummyTx[1];


	  I3C_CCCTypeDef SETAASA_Desc = {
	    0x00,
	    0x29,
	    {NULL, 0},
	    LL_I3C_DIRECTION_WRITE
	  };

	  I3C_Context.CtrlBuf.pBuffer = I3C_ControlBuffer;
	  I3C_Context.CtrlBuf.Size = 1;
	  I3C_Context.TxBuf.pBuffer = dummyTx;
	  I3C_Context.TxBuf.Size = 0;
	  I3C_Context.RxBuf.pBuffer = NULL;
	  I3C_Context.RxBuf.Size = 0;

	  if ( HAL_I3C_AddDescToFrame(&hi3c1,
	                         &SETAASA_Desc,
	                         NULL,
	                         &I3C_Context,
	                         1,
							 I3C_BROADCAST_WITHOUT_DEFBYTE_STOP)!= HAL_OK)
		  Error_Handler();

	  if ( HAL_I3C_Ctrl_TransmitCCC(&hi3c1, &I3C_Context, HAL_MAX_DELAY)!= HAL_OK )
	  {
		  volatile uint32_t err = hi3c1.ErrorCode;
		  volatile uint32_t ser = hi3c1.Instance->SER;
		  volatile uint32_t evr = hi3c1.Instance->EVR;
		  Error_Handler();
	  }

}
HAL_StatusTypeDef I3C_LL_SETAASA(void)
{
    uint32_t cr;
    uint32_t timeout;

    /* SETAASA:
       MTYPE = 0110 -> Broadcast CCC write
       CCC   = 0x29
       DCNT  = 0
       MEND  = 1
    */
    cr = (1UL << 31) |      /* MEND = 1 */
         (0x6UL << 27) |    /* MTYPE = 0110 */
         (0x29UL << 8) |    /* CCC = SETAASA */
         (0UL);             /* DCNT = 0 */

    /* Vaciar posible RX pendiente */
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U)
    {
        volatile uint32_t dummy = READ_REG(hi3c1.Instance->RDR);
        (void)dummy;
    }

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U)
    {
        volatile uint32_t d0 = READ_REG(hi3c1.Instance->RDR);
        volatile uint32_t d1 = READ_REG(hi3c1.Instance->RDWR);
        (void)d0;
        (void)d1;
    }

    /* Limpiar flags previos */
    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
    {
        LL_I3C_ClearFlag_ERR(hi3c1.Instance);
    }

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) != 0U)
    {
        LL_I3C_ClearFlag_FC(hi3c1.Instance);
    }

    /* Lanzar SETAASA */
    WRITE_REG(hi3c1.Instance->CR, cr);

    /* Esperar fin o error */
    timeout = 1000000U;

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        {
            volatile uint32_t ser = READ_REG(hi3c1.Instance->SER);
            volatile uint32_t evr = READ_REG(hi3c1.Instance->EVR);
            (void)ser;
            (void)evr;

            LL_I3C_ClearFlag_ERR(hi3c1.Instance);
            return HAL_ERROR;
        }

        if (--timeout == 0U)
        {
            return HAL_TIMEOUT;
        }
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}

void I3C_RSTDAA(void)
{
	I3C_XferTypeDef I3C_Context;
	  uint32_t I3C_ControlBuffer[1];
	  uint8_t dummyTx[1];


	  I3C_CCCTypeDef SETAASA_Desc = {
	    0x00,
	    0x06,
	    {NULL, 0},
	    LL_I3C_DIRECTION_WRITE
	  };

	  I3C_Context.CtrlBuf.pBuffer = I3C_ControlBuffer;
	  I3C_Context.CtrlBuf.Size = 1;
	  I3C_Context.TxBuf.pBuffer = dummyTx;
	  I3C_Context.TxBuf.Size = 0;
	  I3C_Context.RxBuf.pBuffer = NULL;
	  I3C_Context.RxBuf.Size = 0;

	  if ( HAL_I3C_AddDescToFrame(&hi3c1,
	                         &SETAASA_Desc,
	                         NULL,
	                         &I3C_Context,
	                         1,
							 I3C_BROADCAST_WITHOUT_DEFBYTE_STOP)!= HAL_OK)
		  Error_Handler();

	  if ( HAL_I3C_Ctrl_TransmitCCC(&hi3c1, &I3C_Context, HAL_MAX_DELAY)!= HAL_OK )
	  {
		  volatile uint32_t err = hi3c1.ErrorCode;
		  volatile uint32_t ser = hi3c1.Instance->SER;
		  volatile uint32_t evr = hi3c1.Instance->EVR;
		  Error_Handler();
	  }

}

HAL_StatusTypeDef I3C_LL_I2C_PrivateReadReg(uint8_t target,
                                            uint8_t reg,
                                            uint8_t *rx,
                                            uint16_t len)
{
    uint32_t cr_write;
    uint32_t cr_read;
    uint32_t timeout = 1000000;

    if ((rx == NULL) || (len == 0))
        return HAL_ERROR;

    /* MTYPE = 0100 -> legacy I2C message */
    cr_write =
        (0x4UL << 27) |              /* MTYPE legacy I2C */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (0UL << 16) |                /* RNW = 0 write */
        (1UL);                       /* DCNT = 1 */

    cr_read =
        (1UL << 31) |                /* MEND = 1 */
        (0x4UL << 27) |              /* MTYPE legacy I2C */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (1UL << 16) |                /* RNW = 1 read */
        ((uint32_t)len);             /* DCNT = len */

    /* Cargar byte de registro en TX-FIFO */
    WRITE_REG(hi3c1.Instance->TDR, reg);

    /* Primer mensaje: WRITE reg, MEND=0 */
    WRITE_REG(hi3c1.Instance->CR, cr_write);


    /* Esperar a que pida siguiente control word */
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_CFNFF) == 0U)
    {
        if (--timeout == 0) return HAL_TIMEOUT;
    }

    /* Segundo mensaje: READ len bytes, MEND=1 */
    WRITE_REG(hi3c1.Instance->CR, cr_read);

    for (uint16_t i = 0; i < len; i++)
    {
        timeout = 1000000;
        while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) == 0U)
        {
            if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF)
                return HAL_ERROR;

            if (--timeout == 0) return HAL_TIMEOUT;
        }

        rx[i] = (uint8_t)READ_REG(hi3c1.Instance->RDR);
    }

    timeout = 1000000;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF)
            return HAL_ERROR;

        if (--timeout == 0) return HAL_TIMEOUT;
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}

HAL_StatusTypeDef I3C_LL_I2C_PrivateWriteReg(uint8_t target,
                                             uint8_t reg,
                                             const uint8_t *tx,
                                             uint16_t len)
{
    uint32_t cr_write;
    uint32_t timeout;

    if ((tx == NULL) && (len > 0))
        return HAL_ERROR;

    /* MTYPE = 0100 -> legacy I2C message
       RNW   = 0    -> write
       DCNT  = 1 + len -> registro + datos
       MEND  = 1    -> último mensaje
    */
    cr_write =
        (1UL << 31) |                 /* MEND = 1 */
        (0x4UL << 27) |               /* MTYPE legacy I2C */
        ((uint32_t)target << 17) |    /* ADD[6:0] */
        (0UL << 16) |                 /* RNW = 0 */
        ((uint32_t)(1U + len));       /* DCNT */

    /* Cargar primero el registro */
    WRITE_REG(hi3c1.Instance->TDR, reg);

    /* Cargar datos */
    for (uint16_t i = 0; i < len; i++)
    {
        WRITE_REG(hi3c1.Instance->TDR, tx[i]);
    }

    /* Lanzar transferencia */
    WRITE_REG(hi3c1.Instance->CR, cr_write);

    /* Esperar fin o error */
    timeout = 1000000;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF)
        {
            LL_I3C_ClearFlag_ERR(hi3c1.Instance);
            return HAL_ERROR;
        }

        if (--timeout == 0U)
            return HAL_TIMEOUT;
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}


HAL_StatusTypeDef I3C_LL_PrivateWriteReg(uint8_t target,
                                         uint8_t reg,
                                         const uint8_t *tx,
                                         uint16_t len)
{
    uint32_t cr_write;
    uint32_t timeout;

    if ((tx == NULL) && (len > 0U))
        return HAL_ERROR;

    /* Asegurar private sin 0x7E inicial */
    SET_BIT(hi3c1.Instance->CFGR, I3C_CFGR_NOARBH);

    /* Limpiar flags previos */
    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U) //FLAG ERROR
        LL_I3C_ClearFlag_ERR(hi3c1.Instance);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) != 0U)	//FLAG TRAMA COMPLETADA
        LL_I3C_ClearFlag_FC(hi3c1.Instance);

    /* Vaciar RX pendiente */
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U) // Hay bytes por leer en la R-FIFO
    {
        volatile uint32_t dummy = READ_REG(hi3c1.Instance->RDR); //Leer para descartar todo.
        (void)dummy;
    }

    /*
      WRITE:
      MTYPE = 0010 I3C private
      RNW   = 0 escritura
      DCNT  = 1 + len  -> registro + datos
      MEND  = 1        -> STOP al final
    */
    cr_write =
        (1UL << 31) |                 /* MEND = 1 */
        (0x2UL << 27) |               /* MTYPE = private */
        ((uint32_t)target << 17) |    /* ADD[6:0] */
        (0UL << 16) |                 /* RNW = 0 */
        ((uint32_t)(1U + len));       /* DCNT */

    /* Cargar primero el registro TDR para cargar en la T-FIFO */
    WRITE_REG(hi3c1.Instance->TDR, reg);

    /* Cargar datos para cargar en la T-FIFO */
    for (uint16_t i = 0; i < len; i++)
    {
        WRITE_REG(hi3c1.Instance->TDR, tx[i]);
    }

    /* Lanzar transferencia */
    WRITE_REG(hi3c1.Instance->CR, cr_write);

    /* Esperar fin de frame */
    timeout = 1000000U;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U) //Si hay Error
        {
            volatile uint32_t evr_err = READ_REG(hi3c1.Instance->EVR);
            volatile uint32_t ser_err = READ_REG(hi3c1.Instance->SER);
            volatile uint32_t sr_err  = READ_REG(hi3c1.Instance->SR);
            (void)evr_err;
            (void)ser_err;
            (void)sr_err;

            return HAL_ERROR;
        }

        if (--timeout == 0U)
        {
            return HAL_TIMEOUT;
        }
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}

HAL_StatusTypeDef I3C_LL_SPD5_ReadReg(uint8_t target,
                                      uint16_t reg,
                                      uint8_t *rx,
                                      uint16_t len)
{
    uint32_t cr_write;
    uint32_t cr_read;
    uint32_t timeout;

    if ((rx == NULL) || (len == 0U))
        return HAL_ERROR;

    /* Registro interno: MemReg = 0 */
    uint8_t cmd      = 0xA0;  // 1010 HID W, HID=0, W=0
    uint8_t addr_lsb = (uint8_t)(reg & 0x3F);
    uint8_t addr_msb = (uint8_t)((reg >> 6) & 0x0F);

    /* WRITE: CMD + addr_lsb + addr_msb */
    cr_write =
        (0x2UL << 27) |              /* MTYPE = I3C private */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (0UL << 16) |                /* RNW = 0 */
        (3UL);                       /* DCNT = 3 */

    /* READ */
    cr_read =
        (1UL << 31) |                /* MEND = 1 */
        (0x2UL << 27) |              /* MTYPE = I3C private */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (1UL << 16) |                /* RNW = 1 */
        ((uint32_t)len);             /* DCNT = len */

    /* limpiar flags previos */
    if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF)
        LL_I3C_ClearFlag_ERR(hi3c1.Instance);

    if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF)
        LL_I3C_ClearFlag_FC(hi3c1.Instance);

    while (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF)
    {
        volatile uint32_t dummy = READ_REG(hi3c1.Instance->RDR);
        (void)dummy;
    }

    /* cargar payload de comando */
    WRITE_REG(hi3c1.Instance->TDR, cmd);
    I3C_DBG_Snapshot(); // A: TX cargado

    WRITE_REG(hi3c1.Instance->TDR, addr_lsb);
    WRITE_REG(hi3c1.Instance->TDR, addr_msb);

    /* WRITE + repeated START */
    WRITE_REG(hi3c1.Instance->CR, cr_write);

    timeout = 1000000U;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_CFNFF) == 0U)
    {
        if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF)
            return HAL_ERROR;

        if (--timeout == 0U)
            return HAL_TIMEOUT;
    }

    /* READ + STOP */
    WRITE_REG(hi3c1.Instance->CR, cr_read);

    for (uint16_t i = 0; i < len; i++)
    {
        timeout = 1000000U;
        while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) == 0U)
        {
            if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF)
                return HAL_ERROR;

            if (--timeout == 0U)
                return HAL_TIMEOUT;
        }

        rx[i] = (uint8_t)READ_REG(hi3c1.Instance->RDR);
    }

    timeout = 1000000U;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if (READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF)
            return HAL_ERROR;

        if (--timeout == 0U)
            return HAL_TIMEOUT;
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}

HAL_StatusTypeDef I3C_LL_PrivateReadReg_Debug(uint8_t target,
                                              uint8_t reg,
                                              uint8_t *rx,
                                              uint16_t len)
{
    uint32_t cr_write;
    uint32_t cr_read;
    uint32_t timeout;

    volatile uint32_t evr_err;
    volatile uint32_t ser_err;
    volatile uint32_t sr_err;

    I3C_DBG(100);   // entrada

    if ((rx == NULL) || (len == 0U))
    {
        I3C_DBG(900);
        return HAL_ERROR;
    }

    SET_BIT(hi3c1.Instance->CFGR, I3C_CFGR_NOARBH);
    I3C_DBG(101);   // NOARBH configurado

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        LL_I3C_ClearFlag_ERR(hi3c1.Instance);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) != 0U)
        LL_I3C_ClearFlag_FC(hi3c1.Instance);

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U)
    {
        volatile uint32_t dummy = READ_REG(hi3c1.Instance->RDR);
        (void)dummy;
    }

    I3C_DBG(102);

    cr_write =
        (0x2UL << 27) |
        ((uint32_t)target << 17) |
        (0UL << 16) |
        (1UL);

    cr_read =
        (1UL << 31) |
        (0x2UL << 27) |
        ((uint32_t)target << 17) |
        (1UL << 16) |
        ((uint32_t)len);

    I3C_DBG(103);

    WRITE_REG(hi3c1.Instance->TDR, reg);
    I3C_DBG(104);

    WRITE_REG(hi3c1.Instance->CR, cr_write);
    I3C_DBG(105);

    timeout = 1000000U;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_CFNFF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        {
            I3C_DBG(910);

            evr_err = READ_REG(hi3c1.Instance->EVR);
            ser_err = READ_REG(hi3c1.Instance->SER);
            sr_err  = READ_REG(hi3c1.Instance->SR);
            (void)evr_err; (void)ser_err; (void)sr_err;

            __NOP();
            return HAL_ERROR;
        }

        if (--timeout == 0U)
        {
            I3C_DBG(911);

            evr_err = READ_REG(hi3c1.Instance->EVR);
            ser_err = READ_REG(hi3c1.Instance->SER);
            sr_err  = READ_REG(hi3c1.Instance->SR);
            (void)evr_err; (void)ser_err; (void)sr_err;

            __NOP();
            return HAL_TIMEOUT;
        }
    }

    I3C_DBG(106);

    WRITE_REG(hi3c1.Instance->CR, cr_read);
    I3C_DBG(107);

    for (uint16_t i = 0; i < len; i++)
    {
        timeout = 1000000U;

        while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) == 0U)
        {
            if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
            {
                I3C_DBG(920);

                evr_err = READ_REG(hi3c1.Instance->EVR);
                ser_err = READ_REG(hi3c1.Instance->SER);
                sr_err  = READ_REG(hi3c1.Instance->SR);
                (void)evr_err; (void)ser_err; (void)sr_err;

                __NOP();
                return HAL_ERROR;
            }

            if (--timeout == 0U)
            {
                I3C_DBG(921);

                evr_err = READ_REG(hi3c1.Instance->EVR);
                ser_err = READ_REG(hi3c1.Instance->SER);
                sr_err  = READ_REG(hi3c1.Instance->SR);
                (void)evr_err; (void)ser_err; (void)sr_err;

                __NOP();
                return HAL_TIMEOUT;
            }
        }

        I3C_DBG(108);

        rx[i] = (uint8_t)READ_REG(hi3c1.Instance->RDR);

        I3C_DBG(109);
    }

    timeout = 1000000U;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        {
            I3C_DBG(930);

            evr_err = READ_REG(hi3c1.Instance->EVR);
            ser_err = READ_REG(hi3c1.Instance->SER);
            sr_err  = READ_REG(hi3c1.Instance->SR);
            (void)evr_err; (void)ser_err; (void)sr_err;

            __NOP();
            return HAL_ERROR;
        }

        if (--timeout == 0U)
        {
            I3C_DBG(931);

            evr_err = READ_REG(hi3c1.Instance->EVR);
            ser_err = READ_REG(hi3c1.Instance->SER);
            sr_err  = READ_REG(hi3c1.Instance->SR);
            (void)evr_err; (void)ser_err; (void)sr_err;

            __NOP();
            return HAL_TIMEOUT;
        }
    }

    I3C_DBG(110);

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    I3C_DBG(111);

    volatile uint32_t evr_final = hi3c1.Instance->EVR;
    volatile uint32_t ser_final = hi3c1.Instance->SER;
    volatile uint32_t sr_final  = hi3c1.Instance->SR;
    volatile uint8_t  rx_final  = rx[0];

    __NOP();   // breakpoint aquí

    return HAL_OK;
}

HAL_StatusTypeDef I3C_HAL_PrivateReadReg(uint8_t target,
                                         uint8_t reg,
                                         uint8_t *rx,
                                         uint16_t len)
{
    I3C_XferTypeDef xfer = {0};
    I3C_PrivateTypeDef desc[2] = {0};

    uint32_t ctrl[2];
    uint8_t txGlobal[1];

    if ((rx == NULL) || (len == 0U))
        return HAL_ERROR;

    /* Mensaje 1: WRITE registro */
    desc[0].TargetAddr    = target;
    desc[0].Direction     = HAL_I3C_DIRECTION_WRITE;
    desc[0].TxBuf.pBuffer = &reg;
    desc[0].TxBuf.Size    = 1;

    /* Mensaje 2: READ datos */
    desc[1].TargetAddr    = target;
    desc[1].Direction     = HAL_I3C_DIRECTION_READ;
    desc[1].RxBuf.pBuffer = rx;
    desc[1].RxBuf.Size    = len;

    xfer.CtrlBuf.pBuffer = ctrl;
    xfer.CtrlBuf.Size    = 2;

    xfer.TxBuf.pBuffer   = txGlobal;
    xfer.TxBuf.Size      = 1;

    xfer.RxBuf.pBuffer   = rx;
    xfer.RxBuf.Size      = len;

    SET_BIT(hi3c1.Instance->CFGR, I3C_CFGR_NOARBH);

    if (HAL_I3C_AddDescToFrame(&hi3c1,
                               NULL,
                               desc,
                               &xfer,
                               2,
                               I3C_PRIVATE_WITHOUT_ARB_RESTART) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_I3C_Ctrl_Receive(&hi3c1, &xfer, HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef I3C_LL_PrivateReadReg(uint8_t target,
                                        uint8_t reg,
                                        uint8_t *rx,
                                        uint16_t len)
{
    uint32_t cr_write;
    uint32_t cr_read;
    uint32_t timeout;
    uint16_t idx = 0;

    volatile uint32_t evr_err;
    volatile uint32_t ser_err;
    volatile uint32_t sr_err;

    if ((rx == NULL) || (len == 0U))
        return HAL_ERROR;

    SET_BIT(hi3c1.Instance->CFGR, I3C_CFGR_NOARBH);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        LL_I3C_ClearFlag_ERR(hi3c1.Instance);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) != 0U)
        LL_I3C_ClearFlag_FC(hi3c1.Instance);

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U)
    {
        volatile uint32_t dummy = READ_REG(hi3c1.Instance->RDR);
        (void)dummy;
    }

    /*
      WRITE PHASE:
      reg + dummy byte
    */
    cr_write =

        (0x2UL << 27) |              /* MTYPE = private */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (0UL << 16) |                /* RNW = 0 */
        (2UL);                       /* DCNT = 2 */

    /*
      READ PHASE
    */
    cr_read =
        (1UL << 31) |                /* MEND = 1 */
        (0x2UL << 27) |              /* MTYPE = private */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (1UL << 16) |                /* RNW = 1 */
        ((uint32_t)len);             /* DCNT = len */

    /*
      TX FIFO:
      [reg][dummy]
    */
    WRITE_REG(hi3c1.Instance->TDR, reg);
    WRITE_REG(hi3c1.Instance->TDR, 0x00);

    /*
      START WRITE
    */
    WRITE_REG(hi3c1.Instance->CR, cr_write);

    /*
      Esperar espacio en C-FIFO
    */
    timeout = 1000000U;

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_CFNFF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        {
            evr_err = READ_REG(hi3c1.Instance->EVR);
            ser_err = READ_REG(hi3c1.Instance->SER);
            sr_err  = READ_REG(hi3c1.Instance->SR);

            (void)evr_err;
            (void)ser_err;
            (void)sr_err;

            return HAL_ERROR;
        }

        if (--timeout == 0U)
            return HAL_TIMEOUT;
    }

    /*
      REPEATED START + READ
    */
    WRITE_REG(hi3c1.Instance->CR, cr_read);

    /*
      RX LOOP
    */
    while (idx < len)
    {
        timeout = 1000000U;

        while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) == 0U)
        {
            if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
            {
                evr_err = READ_REG(hi3c1.Instance->EVR);
                ser_err = READ_REG(hi3c1.Instance->SER);
                sr_err  = READ_REG(hi3c1.Instance->SR);

                (void)evr_err;
                (void)ser_err;
                (void)sr_err;

                return HAL_ERROR;
            }

            if (--timeout == 0U)
                return HAL_TIMEOUT;
        }

        /*
          Vaciar RX FIFO
        */
        while (((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U) &&
               (idx < len))
        {
            rx[idx++] = (uint8_t)READ_REG(hi3c1.Instance->RDR);
        }
    }

    /*
      Esperar fin de frame
    */
    timeout = 1000000U;

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        {
            evr_err = READ_REG(hi3c1.Instance->EVR);
            ser_err = READ_REG(hi3c1.Instance->SER);
            sr_err  = READ_REG(hi3c1.Instance->SR);

            (void)evr_err;
            (void)ser_err;
            (void)sr_err;

            return HAL_ERROR;
        }

        if (--timeout == 0U)
            return HAL_TIMEOUT;
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}

static void I3C_DBG(uint32_t tag)
{
    if (i3c_dbg_i < I3C_DBG_MAX)
    {
        i3c_dbg[i3c_dbg_i].tag  = tag;
        i3c_dbg[i3c_dbg_i].EVR  = hi3c1.Instance->EVR;
        i3c_dbg[i3c_dbg_i].SER  = hi3c1.Instance->SER;
        i3c_dbg[i3c_dbg_i].SR   = hi3c1.Instance->SR;
        i3c_dbg[i3c_dbg_i].CFGR = hi3c1.Instance->CFGR;
        i3c_dbg[i3c_dbg_i].CR   = hi3c1.Instance->CR;
        i3c_dbg[i3c_dbg_i].RDR  = hi3c1.Instance->RDR;
        i3c_dbg_i++;
    }
}

HAL_StatusTypeDef I3C_LL_PrivateWriteAddrOnly(uint8_t target, uint8_t reg)
{
    uint32_t cr_write;
    uint32_t timeout;

    SET_BIT(hi3c1.Instance->CFGR, I3C_CFGR_NOARBH);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        LL_I3C_ClearFlag_ERR(hi3c1.Instance);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) != 0U)
        LL_I3C_ClearFlag_FC(hi3c1.Instance);

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U)
    {
        volatile uint32_t dummy = READ_REG(hi3c1.Instance->RDR);
        (void)dummy;
    }

    /*
      WRITE:
      S target(W) reg 0x00 P
      MEND = 1 -> STOP
      DCNT = 2 -> reg + dummy
    */
    cr_write =
        (1UL << 31) |                /* MEND = 1 */
        (0x2UL << 27) |              /* MTYPE = private */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (0UL << 16) |                /* RNW = 0 */
        (2UL);                       /* DCNT = 2 */

    WRITE_REG(hi3c1.Instance->TDR, reg);
    WRITE_REG(hi3c1.Instance->TDR, 0x00);

    WRITE_REG(hi3c1.Instance->CR, cr_write);

    timeout = 1000000U;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        {
            volatile uint32_t evr_err = READ_REG(hi3c1.Instance->EVR);
            volatile uint32_t ser_err = READ_REG(hi3c1.Instance->SER);
            volatile uint32_t sr_err  = READ_REG(hi3c1.Instance->SR);
            (void)evr_err;
            (void)ser_err;
            (void)sr_err;
            return HAL_ERROR;
        }

        if (--timeout == 0U)
            return HAL_TIMEOUT;
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}

HAL_StatusTypeDef I3C_LL_PrivateReadOnly(uint8_t target,
                                         uint8_t *rx,
                                         uint16_t len)
{
    uint32_t cr_read;
    uint32_t timeout;
    uint16_t idx = 0;

    if ((rx == NULL) || (len == 0U))
        return HAL_ERROR;

    SET_BIT(hi3c1.Instance->CFGR, I3C_CFGR_NOARBH);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        LL_I3C_ClearFlag_ERR(hi3c1.Instance);

    if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) != 0U)
        LL_I3C_ClearFlag_FC(hi3c1.Instance);

    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U)
    {
        volatile uint32_t dummy = READ_REG(hi3c1.Instance->RDR);
        (void)dummy;
    }

    /*
      READ:
      S target(R) data... P
    */
    cr_read =
        (1UL << 31) |                /* MEND = 1 */
        (0x2UL << 27) |              /* MTYPE = private */
        ((uint32_t)target << 17) |   /* ADD[6:0] */
        (1UL << 16) |                /* RNW = 1 */
        ((uint32_t)len);             /* DCNT = len */

    WRITE_REG(hi3c1.Instance->CR, cr_read);

    while (idx < len)
    {
        timeout = 1000000U;

        while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) == 0U)
        {
            if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
            {
                volatile uint32_t evr_err = READ_REG(hi3c1.Instance->EVR);
                volatile uint32_t ser_err = READ_REG(hi3c1.Instance->SER);
                volatile uint32_t sr_err  = READ_REG(hi3c1.Instance->SR);
                (void)evr_err;
                (void)ser_err;
                (void)sr_err;
                return HAL_ERROR;
            }

            if (--timeout == 0U)
                return HAL_TIMEOUT;
        }

        while (((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_RXFNEF) != 0U) &&
               (idx < len))
        {
            rx[idx++] = (uint8_t)READ_REG(hi3c1.Instance->RDR);
        }
    }

    timeout = 1000000U;
    while ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_FCF) == 0U)
    {
        if ((READ_REG(hi3c1.Instance->EVR) & I3C_EVR_ERRF) != 0U)
        {
            volatile uint32_t evr_err = READ_REG(hi3c1.Instance->EVR);
            volatile uint32_t ser_err = READ_REG(hi3c1.Instance->SER);
            volatile uint32_t sr_err  = READ_REG(hi3c1.Instance->SR);
            (void)evr_err;
            (void)ser_err;
            (void)sr_err;
            return HAL_ERROR;
        }

        if (--timeout == 0U)
            return HAL_TIMEOUT;
    }

    LL_I3C_ClearFlag_FC(hi3c1.Instance);

    return HAL_OK;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I3C1_Init();
  MX_ICACHE_Init();
  MX_USART3_UART_Init();
  MX_MEMORYMAP_Init();
  /* USER CODE BEGIN 2 */

  uint8_t value[10] = {0};

  i3c_dbg_i = 0;


  /* ------------------------------------------------ */
  /* I2C legacy read antes de SETAASA                 */
  /* ------------------------------------------------ */



  if (I3C_LL_I2C_PrivateReadReg(0x50, 0x00, value, 1) != HAL_OK)
  {

      Error_Handler();
  }


  /* ------------------------------------------------ */
  /* SETAASA -> entrar en I3C Basic                   */
  /* ------------------------------------------------ */


  I3C_SETAASA();
  HAL_Delay(10);

  uint8_t rx[8] = {0};

  if (I3C_LL_PrivateReadReg(0x50, 0x12, value, 1) != HAL_OK)
    {
        Error_Handler();
    }




  __NOP();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10707DBC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I3C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I3C1_Init(void)
{

  /* USER CODE BEGIN I3C1_Init 0 */

  /* USER CODE END I3C1_Init 0 */

  I3C_FifoConfTypeDef sFifoConfig = {0};
  I3C_CtrlConfTypeDef sCtrlConfig = {0};

  /* USER CODE BEGIN I3C1_Init 1 */

  /* USER CODE END I3C1_Init 1 */
  hi3c1.Instance = I3C1;
  hi3c1.Mode = HAL_I3C_MODE_CONTROLLER;
  hi3c1.Init.CtrlBusCharacteristic.SDAHoldTime = HAL_I3C_SDA_HOLD_TIME_0_5;
  hi3c1.Init.CtrlBusCharacteristic.WaitTime = HAL_I3C_OWN_ACTIVITY_STATE_0;
  //hi3c1.Init.CtrlBusCharacteristic.SCLPPLowDuration = 0x4c;
  //hi3c1.Init.CtrlBusCharacteristic.SCLI3CHighDuration = 0x02;
  //hi3c1.Init.CtrlBusCharacteristic.SCLODLowDuration = 0x67;
  //hi3c1.Init.CtrlBusCharacteristic.SCLI2CHighDuration = 0x5f;
  hi3c1.Init.CtrlBusCharacteristic.BusFreeDuration = 0x42;
  hi3c1.Init.CtrlBusCharacteristic.BusIdleDuration = 0x4e;

  //I2c & I3C 500Khz
  //hi3c1.Init.CtrlBusCharacteristic.SCLI3CHighDuration = 0x27;  // ~500 ns
  //hi3c1.Init.CtrlBusCharacteristic.SCLODLowDuration   = 0x67;  // ~1300 ns
  //hi3c1.Init.CtrlBusCharacteristic.SCLI2CHighDuration = 0x27;  // ~500 ns o más
  //hi3c1.Init.CtrlBusCharacteristic.SCLPPLowDuration   = 0x67;  // conservador

  hi3c1.Init.CtrlBusCharacteristic.SCLI3CHighDuration = 0x27;  // I3C high ~500 ns
  hi3c1.Init.CtrlBusCharacteristic.SCLODLowDuration   = 0x63;  // I2C low  ~1250 ns
  hi3c1.Init.CtrlBusCharacteristic.SCLI2CHighDuration = 0x63;  // I2C high ~1250 ns
  hi3c1.Init.CtrlBusCharacteristic.SCLPPLowDuration   = 0x27;  // I3C low  ~500 ns

  if (HAL_I3C_Init(&hi3c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure FIFO
  */
  sFifoConfig.RxFifoThreshold = HAL_I3C_RXFIFO_THRESHOLD_1_4;
  sFifoConfig.TxFifoThreshold = HAL_I3C_TXFIFO_THRESHOLD_1_4;
  sFifoConfig.ControlFifo = HAL_I3C_CONTROLFIFO_DISABLE;
  sFifoConfig.StatusFifo = HAL_I3C_STATUSFIFO_DISABLE;
  if (HAL_I3C_SetConfigFifo(&hi3c1, &sFifoConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure controller
  */
  sCtrlConfig.DynamicAddr = 0;
  sCtrlConfig.StallTime = 0x00;
  sCtrlConfig.HotJoinAllowed = DISABLE;
  sCtrlConfig.ACKStallState = DISABLE;
  sCtrlConfig.CCCStallState = DISABLE;
  sCtrlConfig.TxStallState = DISABLE;
  sCtrlConfig.RxStallState = DISABLE;
  sCtrlConfig.HighKeeperSDA = ENABLE;
  if (HAL_I3C_Ctrl_Config(&hi3c1, &sCtrlConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I3C1_Init 2 */
  SET_BIT(hi3c1.Instance->CFGR, I3C_CFGR_NOARBH);
  /* USER CODE END I3C1_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief MEMORYMAP Initialization Function
  * @param None
  * @retval None
  */
static void MX_MEMORYMAP_Init(void)
{

  /* USER CODE BEGIN MEMORYMAP_Init 0 */

  /* USER CODE END MEMORYMAP_Init 0 */

  /* USER CODE BEGIN MEMORYMAP_Init 1 */

  /* USER CODE END MEMORYMAP_Init 1 */
  /* USER CODE BEGIN MEMORYMAP_Init 2 */

  /* USER CODE END MEMORYMAP_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_8;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXINVERT_INIT;
  huart3.AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, LED_CLK_Pin|LED_DATA_Pin|LED2_YELLOW_Pin|LED_RESET_Pin
                          |LED_LATCH_Pin|LED_STATUS_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED1_GREEN_Pin|TRIGGER_OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED3_RED_GPIO_Port, LED3_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, PWR_EN_RDIMM_6_Pin|PWR_EN_RDIMM_7_Pin|LED_STATUS_0_Pin|LED_STATUS_1_Pin
                          |LED_STATUS_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, PWR_EN_RDIMM_0_Pin|PWR_EN_RDIMM_1_Pin|PWR_EN_RDIMM_2_Pin|PWR_EN_RDIMM_3_Pin
                          |PWR_EN_RDIMM_4_Pin|PWR_EN_RDIMM_5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : USER_BUTTON_Pin ARROW_UP_Pin ARROW_DOWN_Pin ARROW_RIGTH_Pin
                           ARROW_LEFT_Pin BUTTON_OK_Pin BUTTON_CANCEL_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin|ARROW_UP_Pin|ARROW_DOWN_Pin|ARROW_RIGTH_Pin
                          |ARROW_LEFT_Pin|BUTTON_OK_Pin|BUTTON_CANCEL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_CLK_Pin LED_DATA_Pin LED2_YELLOW_Pin LED_RESET_Pin
                           LED_LATCH_Pin LED_STATUS_3_Pin */
  GPIO_InitStruct.Pin = LED_CLK_Pin|LED_DATA_Pin|LED2_YELLOW_Pin|LED_RESET_Pin
                          |LED_LATCH_Pin|LED_STATUS_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_SENSE_Pin */
  GPIO_InitStruct.Pin = VBUS_SENSE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_SENSE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_GREEN_Pin TRIGGER_OUT_Pin */
  GPIO_InitStruct.Pin = LED1_GREEN_Pin|TRIGGER_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TRIGGER_IN_Pin */
  GPIO_InitStruct.Pin = TRIGGER_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TRIGGER_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PWR_GOOD_RDIMM_0_Pin PWR_GOOD_RDIMM_1_Pin PWR_GOOD_RDIMM_2_Pin PWR_GOOD_RDIMM_3_Pin
                           PWR_GOOD_RDIMM_4_Pin PWR_GOOD_RDIMM_5_Pin PWR_GOOD_RDIMM_6_Pin PWR_GOOD_RDIMM_7_Pin */
  GPIO_InitStruct.Pin = PWR_GOOD_RDIMM_0_Pin|PWR_GOOD_RDIMM_1_Pin|PWR_GOOD_RDIMM_2_Pin|PWR_GOOD_RDIMM_3_Pin
                          |PWR_GOOD_RDIMM_4_Pin|PWR_GOOD_RDIMM_5_Pin|PWR_GOOD_RDIMM_6_Pin|PWR_GOOD_RDIMM_7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : UCPD_CC1_Pin UCPD_CC2_Pin */
  GPIO_InitStruct.Pin = UCPD_CC1_Pin|UCPD_CC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED3_RED_Pin */
  GPIO_InitStruct.Pin = LED3_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED3_RED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : UCPD_FLT_Pin */
  GPIO_InitStruct.Pin = UCPD_FLT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(UCPD_FLT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PWR_EN_RDIMM_6_Pin PWR_EN_RDIMM_7_Pin LED_STATUS_0_Pin LED_STATUS_1_Pin
                           LED_STATUS_2_Pin */
  GPIO_InitStruct.Pin = PWR_EN_RDIMM_6_Pin|PWR_EN_RDIMM_7_Pin|LED_STATUS_0_Pin|LED_STATUS_1_Pin
                          |LED_STATUS_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_FS_N_Pin USB_FS_P_Pin */
  GPIO_InitStruct.Pin = USB_FS_N_Pin|USB_FS_P_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_USB;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PWR_EN_RDIMM_0_Pin PWR_EN_RDIMM_1_Pin PWR_EN_RDIMM_2_Pin PWR_EN_RDIMM_3_Pin
                           PWR_EN_RDIMM_4_Pin PWR_EN_RDIMM_5_Pin */
  GPIO_InitStruct.Pin = PWR_EN_RDIMM_0_Pin|PWR_EN_RDIMM_1_Pin|PWR_EN_RDIMM_2_Pin|PWR_EN_RDIMM_3_Pin
                          |PWR_EN_RDIMM_4_Pin|PWR_EN_RDIMM_5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
