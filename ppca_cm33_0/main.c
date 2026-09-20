/******************************************************************************
* File Name:   main.c
*
* Description: PPCA Core 0 application for the Hibernate GPIO wakeup example.
*              Toggles LED1 every 1000 ms while the main CM33 core manages
*              Hibernate power mode entry and GPIO-triggered wakeup.
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
#include "cycfg.h"
#include <stdio.h>

/*******************************************************************************
* Macros
********************************************************************************/
#define LOOP_DELAY_MS 1000

/* These are the addresses that the other application running on main core should be using to print. */
#define PPCA_M1_VAR_ADDRESS 0x20000400
#define PPCA_M3_VAR_ADDRESS 0x20000800

/*******************************************************************************
* Global Variables
********************************************************************************/


/*******************************************************************************
* Function Prototypes
********************************************************************************/


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for PPCA Core 0. Toggles LED1 every 1000 ms
* while the main CM33 core manages Hibernate power mode entry and GPIO
* wakeup. A conditional compilation guard prevents the toggle on kit
* variants that do not have LED1 connected to this core.
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
     for(;;)
     {
         /* Toggle LED1 every 1000 ms. The conditional compilation guard
            prevents the LED toggle on kit variants that do not have LED1
            connected to this PPCA core. */
         #if !defined(APP_KIT_PSC3M8_CC1)
          Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN);
         #endif

          Cy_SysLib_Delay(LOOP_DELAY_MS);
     }
}
