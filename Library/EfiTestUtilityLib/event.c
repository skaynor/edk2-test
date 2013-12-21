/*++
  The material contained herein is not a license, either        
  expressly or impliedly, to any intellectual property owned    
  or controlled by any of the authors or developers of this     
  material or to any contribution thereto. The material         
  contained herein is provided on an "AS IS" basis and, to the  
  maximum extent permitted by applicable law, this information  
  is provided AS IS AND WITH ALL FAULTS, and the authors and    
  developers of this material hereby disclaim all other         
  warranties and conditions, either express, implied or         
  statutory, including, but not limited to, any (if any)        
  implied warranties, duties or conditions of merchantability,  
  of fitness for a particular purpose, of accuracy or           
  completeness of responses, of results, of workmanlike         
  effort, of lack of viruses and of lack of negligence, all     
  with regard to this material and any contribution thereto.    
  Designers must not rely on the absence or characteristics of  
  any features or instructions marked "reserved" or             
  "undefined." The Unified EFI Forum, Inc. reserves any         
  features or instructions so marked for future definition and  
  shall have no responsibility whatsoever for conflicts or      
  incompatibilities arising from future changes to them. ALSO,  
  THERE IS NO WARRANTY OR CONDITION OF TITLE, QUIET ENJOYMENT,  
  QUIET POSSESSION, CORRESPONDENCE TO DESCRIPTION OR            
  NON-INFRINGEMENT WITH REGARD TO THE TEST SUITE AND ANY        
  CONTRIBUTION THERETO.                                         
                                                                
  IN NO EVENT WILL ANY AUTHOR OR DEVELOPER OF THIS MATERIAL OR  
  ANY CONTRIBUTION THERETO BE LIABLE TO ANY OTHER PARTY FOR     
  THE COST OF PROCURING SUBSTITUTE GOODS OR SERVICES, LOST      
  PROFITS, LOSS OF USE, LOSS OF DATA, OR ANY INCIDENTAL,        
  CONSEQUENTIAL, DIRECT, INDIRECT, OR SPECIAL DAMAGES WHETHER   
  UNDER CONTRACT, TORT, WARRANTY, OR OTHERWISE, ARISING IN ANY  
  WAY OUT OF THIS OR ANY OTHER AGREEMENT RELATING TO THIS       
  DOCUMENT, WHETHER OR NOT SUCH PARTY HAD ADVANCE NOTICE OF     
  THE POSSIBILITY OF SUCH DAMAGES.                              
                                                                
  Copyright 2006, 2007, 2008, 2009, 2010 Unified EFI, Inc. All  
  Rights Reserved, subject to all existing rights in all        
  matters included within this Test Suite, to which United      
  EFI, Inc. makes no claim of right.                            
                                                                
  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>   
   
--*/
/*++

Module Name:

  event.c

Abstract:

  Event operation functions

--*/

#include "lib.h"


EFI_EVENT
LibCreateProtocolNotifyEvent (
  IN EFI_GUID             *ProtocolGuid,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                *Registration
  )
/*++

Routine Description:
  Function creates a notification event and registers that event with all
  the protocol instances specified by ProtocolGuid.

Arguments:
  ProtocolGuid     - Supplies GUID of the protocol upon whose installation the event is fired.

  NotifyTpl        - Supplies the task priority level of the event notifications.

  NotifyFunction   - Supplies the function to notify when the event is signaled.

  NotifyContext    - The context parameter to pass to NotifyFunction.

  Registration     - A pointer to a memory location to receive the registration value.
                     This value is passed to LocateHandle() to obtain new handles that
                     have been added that support the ProtocolGuid-specified protocol.

Returns:

  This function returns the notification event that was created.

--*/
{
  EFI_STATUS              Status;
  EFI_EVENT               Event;

  //
  // Create the event
  //
  Status = tBS->CreateEvent (
                 EFI_EVENT_NOTIFY_SIGNAL,
                 NotifyTpl,
                 NotifyFunction,
                 NotifyContext,
                 &Event
                 );
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  //
  // Register for protocol notifactions on this event
  //
  Status = tBS->RegisterProtocolNotify (
                 ProtocolGuid,
                 Event,
                 Registration
                 );

  if (EFI_ERROR(Status)) {
    return NULL;
  }

  //
  // Kick the event so we will perform an initial pass of
  // current installed drivers
  //
  tBS->SignalEvent (Event);
  return Event;
}


EFI_STATUS
WaitForSingleEvent (
  IN EFI_EVENT        Event,
  IN UINT64           Timeout OPTIONAL
  )
/*++

Routine Description:
  Function waits for a given event to fire, or for an optional timeout to expire.

Arguments:
  Event            - The event to wait for

  Timeout          - An optional timeout value in 100 ns units.

Returns:

  EFI_SUCCESS       - Event fired before Timeout expired.
  EFI_TIME_OUT     - Timout expired before Event fired..

--*/
{
  EFI_STATUS          Status;
  UINTN               Index;
  EFI_EVENT           TimerEvent;
  EFI_EVENT           WaitList[2];

  if (Timeout) {
    //
    // Create a timer event
    //
    Status = tBS->CreateEvent (EFI_EVENT_TIMER, 0, NULL, NULL, &TimerEvent);
    if (!EFI_ERROR(Status)) {

      //
      // Set the timer event
      //
      tBS->SetTimer(
            TimerEvent,
            TimerRelative,
            Timeout
            );

      //
      // Wait for the original event or the timer
      //
      WaitList[0] = Event;
      WaitList[1] = TimerEvent;
      Status = tBS->WaitForEvent (2, WaitList, &Index);
      tBS->CloseEvent (TimerEvent);

      //
      // If the timer expired, change the return to timed out
      //
      if (!EFI_ERROR(Status)  &&  Index == 1) {
        Status = EFI_TIMEOUT;
      }
    }
  } else {

    //
    // No timeout... just wait on the event
    //

    Status = tBS->WaitForEvent (1, &Event, &Index);
    ASSERT (!EFI_ERROR(Status));
    ASSERT (Index == 0);
  }

  return Status;
}

VOID
WaitForEventWithTimeout (
  IN  EFI_EVENT       Event,
  IN  UINTN           Timeout,
  IN  UINTN           Row,
  IN  UINTN           Column,
  IN  CHAR16          *String,
  IN  EFI_INPUT_KEY   TimeoutKey,
  OUT EFI_INPUT_KEY   *Key
  )
/*++

Routine Description:
  function prints a string for the given number of seconds until either the
  timeout expires, or the user presses a key.

Arguments:
  Event            - The event to wait for

  Timeout          - A timeout value in 1 second units.

  Row              - A row to print String

  Column           - A column to print String

  String           - The string to display on the standard output device

  TimeoutKey       - The key to return in Key if a timeout occurs

  Key              - Either the key the user pressed or TimeoutKey if the Timeout expired.

Returns:

  none

--*/
{
  EFI_STATUS      Status;

  while (Timeout > 0) {
    PrintAt (Column, Row, String, Timeout);
    Status = WaitForSingleEvent (Event, 10000000);
    if (Status == EFI_SUCCESS) {
      if (!EFI_ERROR(tST->ConIn->ReadKeyStroke (tST->ConIn, Key))) {
        return;
      }
    }
    Timeout--;
  }
  *Key = TimeoutKey;
}