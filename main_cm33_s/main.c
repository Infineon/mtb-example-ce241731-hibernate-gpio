/******************************************************************************
* File Name:   main.c
*
* Description: Hibernate GPIO wakeup application for the PSOC™ Control C3M/P8
*              MCU. The main CM33 core blinks LED3 eight times, then configures
*              P7.0 (USER-BTN2 / SW4) as a low-level Hibernate wakeup source
*              and enters Hibernate mode. On the next SW4 press the device
*              resets and the wakeup reason is detected. PPCA cores run
*              concurrently, toggling LED1 and LED2.
*
* Related Document: See README.md
*
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
********************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
********************************************************************************/

/* These are the addresses where the core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START    //  0x12030000
#define CORE1_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START    //  0x12038000
#define PPCA0_IMAGE_SIZE       CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE       CYMEM_CM33_0_S_ppca1_code_SIZE
#define DELAY_LONG_MS           (500)   /* milliseconds */
#define LED_BLINK_COUNT         (8)     /* LED toggle count */
#define GPIO_INTERRUPT_PRIORITY (1u)    /* GPIO Interrupt priority */

/*******************************************************************************
* Global Variables
********************************************************************************/
cy_stc_sysint_t button_press_intr_config =
{
.intrSrc = CYBSP_USER_BTN2_IRQ,
.intrPriority = GPIO_INTERRUPT_PRIORITY,
};
/* Debug UART variables */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* DEBUG_UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug DEBUG_UART HAL object */

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void gpio_interrupt_handler(void);


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. It configures and initializes the GPIO
*  interrupt, blinks the LED and enter in hibernate mode. On user button
*  press it wakesup.
*  Hibernate wakeup source used in this example is P7.0 (SW4)
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/

int main(void)
{
    cy_rslt_t result;
    uint32_t count = 0;
    uint32_t delay_led_blink = DELAY_LONG_MS;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Check the IO freeze status: after Hibernate wakeup, IOs are frozen.
       Unfreeze them before driving any output pin. */
    if (Cy_SysPm_GetIoFreezeStatus())
    {
        /* Unfreeze the system */
        Cy_SysPm_IoUnfreeze();
    }
    
    
    /* Register the GPIO interrupt handler for USER-BTN2 (SW4) which is
       also used as the Hibernate wakeup pin (P7.0) */
    Cy_SysInt_Init(&button_press_intr_config, gpio_interrupt_handler);
    NVIC_EnableIRQ(button_press_intr_config.intrSrc);

    /* Note: global interrupts are enabled later, after UART is set up,
       so that printf is available before any interrupt can fire */

    /* Debug UART init */
    result = (cy_rslt_t)Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);

    /* UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL DEBUG_UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL DEBUG_UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize redirecting of low level IO */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* retarget IO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Transmit header to the terminal */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: Hibernate GPIO wakeup\r\n");
    printf("************************************************************\r\n\n");

    /* Enable global interrupts */
    __enable_irq();

    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    /* Check the reset reason: if we woke from Hibernate, print a message.
       A brief delay avoids spurious glitches from the button that triggered
       the wakeup. */
    if(CY_SYSLIB_RESET_HIB_WAKEUP == (Cy_SysLib_GetResetReason() &
    CY_SYSLIB_RESET_HIB_WAKEUP))
    {
        /* Wait a bit to avoid glitches from the button press */
        Cy_SysLib_Delay(DELAY_LONG_MS);
        /* The reset has occurred on a wakeup from Hibernate power mode */
        printf("Wake up from the Hibernate mode.\r\n");
    }


    for (;;)
    {
        /* Toggle LED3 eight times to indicate the device is active */
        for (count = 0; count < LED_BLINK_COUNT; count++)
        {
            Cy_GPIO_Inv(CYBSP_USER_LED3_PORT, CYBSP_USER_LED3_PIN);
            Cy_SysLib_Delay(delay_led_blink);
        }

        printf("Entering hibernate mode. Press the user button to wakeup.\r\n");
        /* Brief delay to allow UART to finish transmitting before power-down */
        Cy_SysLib_Delay(DELAY_LONG_MS);

        /* Configure PIN1 (P7.0 / SW4) low-level as the Hibernate wakeup source
           and enter Hibernate mode. The device will restart on the next button
           press; execution resumes from the top of main() after the reset. */
        Cy_SysPm_SetHibernateWakeupSource(CY_SYSPM_HIBERNATE_PIN1_LOW);

        /* Jump to Hibernate Mode */
        Cy_SysPm_SystemEnterHibernate();
    }
}


/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void gpio_interrupt_handler(void)
{
    /* Clear the GPIO interrupt for USER-BTN2 so it can fire again on the
       next button press after the device exits Hibernate mode */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_NUM);
}


/* [] END OF FILE */
