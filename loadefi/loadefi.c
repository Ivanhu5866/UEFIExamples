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

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	EFI_LOADED_IMAGE *loaded_image = NULL;

	EFI_DEVICE_PATH_PROTOCOL        *FilePath;

	EFI_STATUS status;
    	EFI_HANDLE image_handle = NULL;

	InitializeLib(image, systab);
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
