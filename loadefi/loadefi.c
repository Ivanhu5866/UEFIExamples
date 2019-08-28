/*
 *  Copyright(C) 2019 Canonical Ltd.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

#include <efi.h>
#include <efilib.h>

#define EFI_SHELL_PARAMETERS_PROTOCOL_GUID \
  { \
  0x752f3136, 0x4e16, 0x4fdc, { 0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca } \
  }

typedef VOID *SHELL_FILE_HANDLE;

typedef struct _EFI_SHELL_PARAMETERS_PROTOCOL {
  ///
  /// Points to an Argc-element array of points to NULL-terminated strings containing
  /// the command-line parameters. The first entry in the array is always the full file
  /// path of the executable. Any quotation marks that were used to preserve
  /// whitespace have been removed.
  ///
  CHAR16 **Argv;

  ///
  /// The number of elements in the Argv array.
  ///
  UINTN Argc;

  ///
  /// The file handle for the standard input for this executable. This may be different
  /// from the ConInHandle in EFI_SYSTEM_TABLE.
  ///
  SHELL_FILE_HANDLE StdIn;

  ///
  /// The file handle for the standard output for this executable. This may be different
  /// from the ConOutHandle in EFI_SYSTEM_TABLE.
  ///
  SHELL_FILE_HANDLE StdOut;

  ///
  /// The file handle for the standard error output for this executable. This may be
  /// different from the StdErrHandle in EFI_SYSTEM_TABLE.
  ///
  SHELL_FILE_HANDLE StdErr;
} EFI_SHELL_PARAMETERS_PROTOCOL;


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	EFI_LOADED_IMAGE *loaded_image = NULL;

	EFI_DEVICE_PATH_PROTOCOL        *FilePath;

	EFI_STATUS status;
    	EFI_HANDLE image_handle = NULL;

  	static const EFI_GUID EfiShellParametersProtocolGuid
		= EFI_SHELL_PARAMETERS_PROTOCOL_GUID;

	EFI_SHELL_PARAMETERS_PROTOCOL *EfiShellParametersProtocol = NULL;
	INTN i;

	InitializeLib(image, systab);

	status = uefi_call_wrapper(systab->BootServices->OpenProtocol, 6,
		             image,
		             (EFI_GUID*)&EfiShellParametersProtocolGuid,
		             (VOID **)&EfiShellParametersProtocol,
		             image,
		             NULL,
		             EFI_OPEN_PROTOCOL_GET_PROTOCOL
		             );

	if (!EFI_ERROR(status))
	{
		// shell 2.0 interface
		// hard code argv0, argv1 and pass to the efi applicaiton
		// which be loaded.
		EfiShellParametersProtocol->Argc = 2;
		EfiShellParametersProtocol->Argv[0] = L"test.efi";
		EfiShellParametersProtocol->Argv[1] = L"-status";
		Print(L"loadefi:\n");
		Print(L"  Argc=%d\n", EfiShellParametersProtocol->Argc);
		for (i = 0 ; i < EfiShellParametersProtocol->Argc ; ++i)
			Print(L"  Argv[%d] = '%s'\n", i, EfiShellParametersProtocol->Argv[i]);
	} else {
		// shell 2.0 FI_SHELL_PARAMETERS_PROTOCOL_GUID open fail
		// this occurs when this application does not run under
		// ueif shell but run with bootentry instead
		Print(L"No EFI_SHELL_PARAMETERS_PROTOCOL installed:\n");
		EfiShellParametersProtocol = AllocateZeroPool(sizeof(EFI_SHELL_PARAMETERS_PROTOCOL));
		EfiShellParametersProtocol->Argc = 2;
		EfiShellParametersProtocol->Argv[0] = L"test.efi";
		EfiShellParametersProtocol->Argv[1] = L"-status";
	}

	status = uefi_call_wrapper(systab->BootServices->HandleProtocol,
				3,
				image, 
				&LoadedImageProtocol, 
				(void **) &loaded_image);
	if (EFI_ERROR(status)) {
		Print(L"HandleProtocol: %r\n", status);
	}

	Print(L"Image base        : %lx\n", loaded_image->ImageBase);
	Print(L"Image size        : %lx\n", loaded_image->ImageSize);
	Print(L"Load options size : %lx\n", loaded_image->LoadOptionsSize);
	Print(L"Load options      : %s\n", loaded_image->LoadOptions);

	Print(L"file path type    : %x\n", loaded_image->FilePath->Type);
	Print(L"file path subtype : %x\n", loaded_image->FilePath->SubType);
	Print(L"file path length0 : %x\n", loaded_image->FilePath->Length[0]);
	Print(L"file path length1 : %x\n", loaded_image->FilePath->Length[1]);
	Print(L"file name         : %s\n", DevicePathToStr(loaded_image->FilePath));

	FilePath = FileDevicePath(loaded_image->DeviceHandle, L"efi\\ubuntu\\test.efi");
	Print(L"file path         : %s\n", DevicePathToStr(FilePath));

	status = uefi_call_wrapper(systab->BootServices->LoadImage,
				6,
				FALSE,
				image,
				FilePath,
				NULL,
				0,
				&image_handle);
	if (EFI_ERROR(status)) {
		Print(L"Load Image Status = %x\n", status);
	}

	// pass Argv0, Argv1 with loadOption
	loaded_image->LoadOptionsSize = 36;
	loaded_image->LoadOptions = L"test.efi -status";

	// install EFI_SHELL_PARAMETERS_PROTOCOL for the application
	// which will be loaded
	status = uefi_call_wrapper(systab->BootServices->InstallProtocolInterface,
					4,
					&image_handle,
					&EfiShellParametersProtocolGuid,
					EFI_NATIVE_INTERFACE,
					(void *)EfiShellParametersProtocol);

	status = uefi_call_wrapper(systab->BootServices->StartImage,
				3,
				image_handle,
				NULL,
				NULL);
	if (!EFI_ERROR(status)) {
		Print(L"StartImage success\n");
	}
	else {
		Print(L"StartImage Status = %x\n", status);
	}

	uefi_call_wrapper(systab->BootServices->Stall, 1, 10000000);

	return EFI_SUCCESS;
}
