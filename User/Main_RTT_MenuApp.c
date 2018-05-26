/*********************************************************************
*               SEGGER MICROCONTROLLER GmbH & Co. KG                 *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 2014 - 2015  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER RTT * Real Time Transfer for embedded targets         *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* * This software may in its unmodified form be freely redistributed *
*   in source form.                                                  *
* * The source code may be modified, provided the source code        *
*   retains the above copyright notice, this list of conditions and  *
*   the following disclaimer.                                        *
* * Modified versions of this software in source or linkable form    *
*   may not be distributed without prior consent of SEGGER.          *
* * This software may only be used for communication with SEGGER     *
*   J-Link debug probes.                                             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       RTT version: 5.10d                                           *
*                                                                    *
**********************************************************************
--------- END-OF-HEADER --------------------------------------------
File    : Main_RTT_MenuApp.c
Purpose : Sample application to demonstrate RTT bi-directional functionality
*/

#define MAIN_C

#include <stdio.h>

#include "SEGGER_RTT.h"

volatile int _Cnt1;
volatile int _Delay;

/*********************************************************************
*
*       main
*/
void main1(void) {
  int r;
  int CancelOp;

  do {
    _Cnt1 = 0;

    SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n");
    SEGGER_RTT_WriteString(0, "Press <1> to continue in blocking mode (Application waits if necessary, no data lost)\r\n");
    SEGGER_RTT_WriteString(0, "Press <2> to continue in non-blocking mode (Application does not wait, data lost if fifo full)\r\n");
    do {
      r = SEGGER_RTT_WaitKey();
    } while ((r != '1') && (r != '2'));
    if (r == '1') {
      SEGGER_RTT_WriteString(0, "\r\nSelected <1>. Configuring RTT and starting...\r\n");
      SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    } else {
      SEGGER_RTT_WriteString(0, "\r\nSelected <2>. Configuring RTT and starting...\r\n");
      SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    }
    CancelOp = 0;
    do {
      //for (_Delay = 0; _Delay < 10000; _Delay++);
      SEGGER_RTT_printf(0, "Count: %d. Press <Space> to get back to menu.\r\n", _Cnt1++);
      r = SEGGER_RTT_HasKey();
      if (r) {
        CancelOp = (SEGGER_RTT_GetKey() == ' ') ? 1 : 0;
      }
      //
      // Check if user selected to cancel the current operation
      //
      if (CancelOp) {
        SEGGER_RTT_WriteString(0, "Operation cancelled, going back to menu...\r\n");
        break;
      }
    } while (1);
    SEGGER_RTT_GetKey();
    SEGGER_RTT_WriteString(0, "\r\n");
  } while (1);
}

/*************************** End of file ****************************/
