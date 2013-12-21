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
                                                                
  Copyright 2006 - 2012 Unified EFI, Inc. All  
  Rights Reserved, subject to all existing rights in all        
  matters included within this Test Suite, to which United      
  EFI, Inc. makes no claim of right.                            
                                                                
  Copyright (c) 2010 - 2012, Intel Corporation. All rights reserved.<BR>   
   
--*/
/**
 *  This file is a test image for the Protocol Handler Boot Services Test
 */
/*++

Module Name:

    DBindingDriver2.c

Abstract:       

    for Protocol Handler Boot Services Black Box Test

--*/

#include "ProtocolDefinition.h"
#include "../Inc/TestDriver.h"

//
// DriverBindingDriver2:
// Open InterfaceTestProtocol1 BY_DRIVER;
// Open InterfaceTestProtocol2 BY_DRIVER;
// Open InterfaceTestProtocol3 BY_DRIVER
// Export EXTERNAL_DRIVER_PROTOCOL_1 protocol
//

//
// data definition here
//
#define DBINDING_DRIVER_2_PRIVATE_DATA_FROM_THIS(a) \
 _CR(a, DBINDING_DRIVER_PRIVATE_DATA, ExProt1)
 
#define DBINDING_DRIVER_2_PRIVATE_DATA_FROM_DRIVER_BINDING(a) \
 _CR(a, DBINDING_DRIVER_PRIVATE_DATA, DriverBinding)
 
DBINDING_DRIVER_PRIVATE_DATA          *mPrivateData;

EFI_STATUS
InitializeDBindingDriver2 (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
DBindingDriver2BindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
DBindingDriver2BindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
DBindingDriver2BindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

void
DBindingDriver2GetNextStatusReport (
  IN EXTERNAL_DRIVER_PROTOCOL_1   *This,
  IN EFI_STATUS                   *NextStatus
  );

VOID
InitializeDriverBinding (
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding
  );
  
EFI_STATUS
DBindingDriver2Unload (
  IN EFI_HANDLE       ImageHandle
  );

//
// global variable for this test driver's image handle
//
EFI_DRIVER_ENTRY_POINT(InitializeDBindingDriver2)

EFI_STATUS
InitializeDBindingDriver2 (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                            Status;  
  EFI_LOADED_IMAGE_PROTOCOL             *LoadedImageInfoPtr;
  
  EfiInitializeTestLib (ImageHandle, SystemTable);
  
  //
  // allocate memory for PrivateData
  //
  Status = gtBS->AllocatePool (
                        EfiBootServicesData,
                        sizeof (DBINDING_DRIVER_PRIVATE_DATA),
                        (VOID**)&mPrivateData
                        );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  gtBS->SetMem (mPrivateData,sizeof (DBINDING_DRIVER_PRIVATE_DATA),0);
  
  InitializeDriverBinding (&mPrivateData->DriverBinding);
  
  Status = gtBS->InstallProtocolInterface (
            &ImageHandle,
            &gEfiDriverBindingProtocolGuid,
            EFI_NATIVE_INTERFACE,
            &mPrivateData->DriverBinding
            );
  mPrivateData->DriverBinding.ImageHandle = ImageHandle;
  mPrivateData->DriverBinding.DriverBindingHandle = ImageHandle;
  //
  // UnLoad Function Handler
  //  
  gtBS->HandleProtocol (
        ImageHandle, 
        &gEfiLoadedImageProtocolGuid, 
        (VOID*)&LoadedImageInfoPtr
        ); 
        
  LoadedImageInfoPtr->Unload = DBindingDriver2Unload;
  
  mPrivateData->ArrayCount = 3;
  Status = gtBS->AllocatePool (
                        EfiBootServicesData,
                        mPrivateData->ArrayCount * sizeof (EFI_STATUS),
                        (VOID**)&(mPrivateData->StatusArray)
                        );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  //
  // init StatusArray with invalid data
  //
  gtBS->SetMem (
        mPrivateData->StatusArray,
        mPrivateData->ArrayCount * sizeof (EFI_STATUS),
        0xff
        );
  
  Status = gtBS->AllocatePool (
                        EfiBootServicesData,
                        mPrivateData->ArrayCount * sizeof (EFI_HANDLE),
                        (VOID**)&(mPrivateData->HandleArray)
                        );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  gtBS->SetMem (
        mPrivateData->HandleArray,
        mPrivateData->ArrayCount * sizeof (EFI_HANDLE),
        0
        );
  
  Status = gtBS->AllocatePool (
                        EfiBootServicesData,
                        mPrivateData->ArrayCount * sizeof (EFI_GUID),
                        (VOID**)&(mPrivateData->ProtGuidArray)
                        );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  mPrivateData->ProtGuidArray[0] = mInterfaceFunctionTestProtocol1Guid;
  mPrivateData->ProtGuidArray[1] = mInterfaceFunctionTestProtocol2Guid;
  mPrivateData->ProtGuidArray[2] = mInterfaceFunctionTestProtocol3Guid;
  
  Status = gtBS->InstallProtocolInterface (
                  &mPrivateData->ChildHandle,
                  &mTestNoInterfaceProtocol1Guid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  return EFI_SUCCESS;
  
}

EFI_STATUS
DBindingDriver2BindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )  
{
  EFI_STATUS        Status1,Status2,Status3;
  
  
  Status1 = gtBS->OpenProtocol (
                      Controller,
                      &mInterfaceFunctionTestProtocol1Guid,
                      NULL,
                      This->DriverBindingHandle,
                      NULL,
                      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                      );
  Status2 = gtBS->OpenProtocol (
                      Controller,
                      &mInterfaceFunctionTestProtocol2Guid,
                      NULL,
                      This->DriverBindingHandle,
                      NULL,
                      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                      );
  Status3 = gtBS->OpenProtocol (
                      Controller,
                      &mInterfaceFunctionTestProtocol3Guid,
                      NULL,
                      This->DriverBindingHandle,
                      NULL,
                      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                      );
  if (EFI_ERROR(Status1) || EFI_ERROR(Status2) || EFI_ERROR(Status3)) {
    return EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;  
}

EFI_STATUS
DBindingDriver2BindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  INTERFACE_FUNCTION_TEST_PROTOCOL_1    *IFTestProt1;
  INTERFACE_FUNCTION_TEST_PROTOCOL_2    *IFTestProt2;
  INTERFACE_FUNCTION_TEST_PROTOCOL_3    *IFTestProt3;
  DBINDING_DRIVER_PRIVATE_DATA          *PrivateData;
  
  PrivateData = DBINDING_DRIVER_2_PRIVATE_DATA_FROM_DRIVER_BINDING (This);
  
  PrivateData->HandleArray[0] = Controller;
  PrivateData->StatusArray[0] = gtBS->OpenProtocol (
                                  Controller,
                                  &mInterfaceFunctionTestProtocol1Guid,
                                  &IFTestProt1,
                                  This->DriverBindingHandle,
                                  PrivateData->ChildHandle,
                                  EFI_OPEN_PROTOCOL_BY_DRIVER
                                  );
  
  PrivateData->HandleArray[1] = Controller;
  PrivateData->StatusArray[1] = gtBS->OpenProtocol (
                                  Controller,
                                  &mInterfaceFunctionTestProtocol2Guid,
                                  &IFTestProt2,
                                  This->DriverBindingHandle,
                                  PrivateData->ChildHandle,
                                  EFI_OPEN_PROTOCOL_BY_DRIVER
                                  );
                                  
  PrivateData->HandleArray[2] = Controller;
  PrivateData->StatusArray[2] = gtBS->OpenProtocol (
                                  Controller,
                                  &mInterfaceFunctionTestProtocol3Guid,
                                  &IFTestProt3,
                                  This->DriverBindingHandle,
                                  PrivateData->ChildHandle,
                                  EFI_OPEN_PROTOCOL_BY_DRIVER
                                  );
  
  if (PrivateData->Handle == NULL) {
    PrivateData->ExProt1.GetNextStatusReport = DBindingDriver2GetNextStatusReport;
    gtBS->InstallProtocolInterface (
                    &PrivateData->Handle,
                    &mExternalDriverProtocol1Guid,
                    EFI_NATIVE_INTERFACE,
                    &PrivateData->ExProt1
                    );
  } 
  return EFI_SUCCESS;
}

EFI_STATUS
DBindingDriver2BindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  DBINDING_DRIVER_PRIVATE_DATA          *PrivateData;
  UINTN                                 Index;
  
  PrivateData = DBINDING_DRIVER_2_PRIVATE_DATA_FROM_DRIVER_BINDING (This);
  
  if (PrivateData->HandleArray != NULL && PrivateData->ProtGuidArray != NULL) {
    for (Index = 0; Index < PrivateData->ArrayCount; Index ++) {
      if (PrivateData->HandleArray[Index] != NULL) {
        gtBS->CloseProtocol (
              PrivateData->HandleArray[Index],
              &PrivateData->ProtGuidArray[Index],
              PrivateData->DriverBinding.DriverBindingHandle,
              PrivateData->ChildHandle
              );
      }
    }
  }
  
  return EFI_SUCCESS;
}

void
DBindingDriver2GetNextStatusReport (
  IN EXTERNAL_DRIVER_PROTOCOL_1   *This,
  IN EFI_STATUS                   *NextStatus
  )
{
  DBINDING_DRIVER_PRIVATE_DATA            *PrivateData;
  
  PrivateData = DBINDING_DRIVER_2_PRIVATE_DATA_FROM_THIS (This);
  
  if (*NextStatus == 0xffffffff) {
    PrivateData->Count = 0;
  } else {
    PrivateData->Count ++;
  }
  
  if (PrivateData->Count >= PrivateData->ArrayCount) {
    *NextStatus = 0xffffffff;
  } else {
    *NextStatus = PrivateData->StatusArray[PrivateData->Count];
  }
  
  return;
}

VOID
InitializeDriverBinding (
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding
  )
{
  DriverBinding->Supported            = DBindingDriver2BindingSupported;
  DriverBinding->Start                = DBindingDriver2BindingStart;
  DriverBinding->Stop                 = DBindingDriver2BindingStop;
  DriverBinding->Version              = 0x10;
  DriverBinding->ImageHandle          = NULL;
  DriverBinding->DriverBindingHandle  = NULL;
}

/**
 *  The driver's Unload function
 *  @param  ImageHandle The test driver image handle
 *  @return EFI_SUCCESS Indicates the interface was Uninstalled
*/
EFI_STATUS
DBindingDriver2Unload (
  IN EFI_HANDLE       ImageHandle
  )
{
  UINTN                                   Index;
  
  //
  // close protocols
  //
  if (mPrivateData->HandleArray != NULL && mPrivateData->ProtGuidArray != NULL) {
    for (Index = 0; Index < mPrivateData->ArrayCount; Index ++) {
      if (mPrivateData->HandleArray[Index] != NULL) {
        gtBS->CloseProtocol (
              mPrivateData->HandleArray[Index],
              &mPrivateData->ProtGuidArray[Index],
              mPrivateData->DriverBinding.DriverBindingHandle,
              mPrivateData->ChildHandle
              );
      }
    }
  }
  
  //
  // free resources
  //
  gtBS->UninstallProtocolInterface (
            ImageHandle,
            &gEfiDriverBindingProtocolGuid,
            &mPrivateData->DriverBinding
            );
  
  if (mPrivateData->Handle != NULL) {
    gtBS->UninstallProtocolInterface (
                    mPrivateData->Handle,
                    &mExternalDriverProtocol1Guid,
                    &mPrivateData->ExProt1
                    );
  }
  
  if (mPrivateData->HandleArray != NULL) {
    gtBS->FreePool (mPrivateData->HandleArray);
  }
  
  if (mPrivateData->ProtGuidArray != NULL) {
    gtBS->FreePool (mPrivateData->ProtGuidArray);
  }
  
  if (mPrivateData->StatusArray != NULL) {
    gtBS->FreePool (mPrivateData->StatusArray);
  }
  
  if (mPrivateData->ChildHandle != NULL) {
    gtBS->UninstallProtocolInterface (
              mPrivateData->ChildHandle,
              &mTestNoInterfaceProtocol1Guid,
              NULL
              );
  }
  gtBS->FreePool (mPrivateData);
  
  return EFI_SUCCESS;
}