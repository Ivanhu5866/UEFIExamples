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

EFI_GUID EFI_SIMPLE_FILE_SYSTEM_GUID = SIMPLE_FILE_SYSTEM_PROTOCOL;
EFI_GUID EFI_FILE_INFO_GUID = EFI_FILE_INFO_ID;

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	EFI_LOADED_IMAGE *loaded_image = NULL;
	EFI_STATUS status;

	EFI_HANDLE device;
	EFI_FILE_INFO *fileinfo = NULL;
	EFI_FILE_IO_INTERFACE *drive;
	EFI_FILE *root, *load;
	UINTN buffersize = sizeof(EFI_FILE_INFO);
	void *data = NULL;

	InitializeLib(image, systab);

	status = uefi_call_wrapper(BS->HandleProtocol,
				3,
				image, 
				&LoadedImageProtocol, 
				(void **) &loaded_image);
	if (EFI_ERROR(status)) {
		Print(L"HandleProtocol: %r\n", status);
	}

	device = loaded_image->DeviceHandle;

	Print(L"Image base        : %lx\n", loaded_image->ImageBase);
	Print(L"Image size        : %lx\n", loaded_image->ImageSize);
	Print(L"Load options size : %lx\n", loaded_image->LoadOptionsSize);
	Print(L"Load options      : %s\n", loaded_image->LoadOptions);

	Print(L"file path type    : %x\n", loaded_image->FilePath->Type);
	Print(L"file path subtype : %x\n", loaded_image->FilePath->SubType);
	Print(L"file path length0 : %x\n", loaded_image->FilePath->Length[0]);
	Print(L"file path length1 : %x\n", loaded_image->FilePath->Length[1]);
	Print(L"file name         : %s\n", DevicePathToStr(loaded_image->FilePath));

	/*
	 * Open the device
	 */
	status = uefi_call_wrapper(BS->HandleProtocol,
				3,
				device,
				&EFI_SIMPLE_FILE_SYSTEM_GUID,
				(void **) &drive);
	if (EFI_ERROR(status)) {
		Print(L"Failed to find fs: %lx\n", status);
		goto error;
	}

	//Print(L"OpenVolume\n");
	status = uefi_call_wrapper(drive->OpenVolume,
				2,
				drive,
				&root);
	if (EFI_ERROR(status)) {
		Print(L"Failed to open fs: %lx\n", status);
		goto error;
	}

	/*
	 * And then open the file
	 */
	//Print(L"Open\n");
	status = uefi_call_wrapper(root->Open,
				5,
				root,
				&load,
				L"\\EFI\\ubuntu\\test.efi",
				1,
				1);
	if (EFI_ERROR(status)) {
		Print(L"Failed to open %lx\n", status);
		goto error;
	}

	//Print(L"AllocatePool\n");
	fileinfo = AllocatePool(buffersize);

	if (!fileinfo) {
		Print(L"Unable to allocate file info buffer\n");
		status = EFI_OUT_OF_RESOURCES;
		goto error;
	}

	/*
	 * Find out how big the file is in order to allocate the storage
	 * buffer
	 */
	//Print(L"GetInfo\n");
	status = uefi_call_wrapper(load->GetInfo,
				4,
				load,
				&EFI_FILE_INFO_GUID,
				&buffersize,
				fileinfo);
	if (status == EFI_BUFFER_TOO_SMALL) {
		FreePool(fileinfo);
		fileinfo = AllocatePool(buffersize);
		if (!fileinfo) {
			Print(L"Unable to allocate file info buffer\n");
			status = EFI_OUT_OF_RESOURCES;
			goto error;
		}
		status = uefi_call_wrapper(load->GetInfo,
					4,
					load,
					&EFI_FILE_INFO_GUID,
					&buffersize,
					fileinfo);
	}

	if (EFI_ERROR(status)) {
		Print(L"Unable to get file info: %lx\n", status);
		goto error;
	}
	//Print(L"AllocatePool2\n");
	buffersize = fileinfo->FileSize;
	data = AllocatePool(buffersize);
	if (!data) {
		Print(L"Unable to allocate file buffer\n");
		status = EFI_OUT_OF_RESOURCES;
		goto error;
	}

	/*
	 * Perform the actual read
	 */
	//Print(L"Read\n");
	status = uefi_call_wrapper(load->Read,
				4,
				load,
				&buffersize,
				data);
	if (status == EFI_BUFFER_TOO_SMALL) {
		FreePool(data);
		data = AllocatePool(buffersize);
		status = uefi_call_wrapper(load->Read,
					4,
					load,
					&buffersize,
					data);
	}
	if (EFI_ERROR(status)) {
		Print(L"Unexpected return from initial read: %lx, buffersize %x\n",
		       status, buffersize);
		goto error;
	}


	Print(L"buffersize: 0x%lx\n", buffersize);

	for (int i=0; i < buffersize; i++)
		Print(L"%x", ((char *)data)[i]);

	FreePool(fileinfo);
	FreePool(data);

	uefi_call_wrapper(systab->BootServices->Stall, 1, 10000000);

	return EFI_SUCCESS;

error:
	Print(L"error %lx\n", status);
	if (data) {
		FreePool(data);
		data = NULL;
	}

	if (fileinfo)
		FreePool(fileinfo);
	return status;
}
