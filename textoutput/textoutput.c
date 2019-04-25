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
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
        EFI_STATUS status;
	UINT32 i = 0;
	UINT32 maxmode = 0;
	EFI_INPUT_KEY eik;
	UINT32 mode_set = 0;

	EFI_TEXT_QUERY_MODE text_querymode;
	UINTN rows, columns;
 
        InitializeLib(image_handle, systab);

	/* text output info */
	maxmode = ST->ConOut->Mode->MaxMode;
	Print(L"Output MaxMode %d\n", maxmode);
	Print(L"Current Mode %d :\n", ST->ConOut->Mode->Mode);
	Print(L"    Attribute 0x%x\n", ST->ConOut->Mode->Attribute);
	Print(L"    CursorColumn %d\n", ST->ConOut->Mode->CursorColumn);
	Print(L"    CursorRow %d\n", ST->ConOut->Mode->CursorRow);

	text_querymode = (void *)(unsigned long)ST->ConOut->QueryMode;
	for (i = 0; i < maxmode; i++) {
        	status = uefi_call_wrapper(text_querymode,
                                   	4,
					ST->ConOut,
                                  	i,
                                   	&columns,
					&rows);

		if (status == EFI_SUCCESS) {
			Print(L"Mode %d :\n", i);
			Print(L"    columns %d\n", columns);
			Print(L"    rows %d\n", rows);
		}
		else {
			Print(L"Cannot QueryMode, status 0x%llx\n", status);
			return status;
		}
	}

	Print(L"Pleas enter the output text mode you would like to set:\n");

	while (1) {
		WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
		status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &eik);
		Print(L"ScanCode: %lxh  UnicodeChar: %lxh CallRtStatus: %lx\n",
		eik.ScanCode, eik.UnicodeChar, status);
		
		if (eik.UnicodeChar < 0x30 || eik.UnicodeChar > (0x30 + maxmode - 1)) {
			Print(L"Invalid mode number, please enter again:\n");
			break;
		}
		mode_set = eik.UnicodeChar - 0x30;
		status = uefi_call_wrapper(ST->ConOut->SetMode,
	                           	2,
	                           	ST->ConOut,
 	                         	mode_set);

		if (status != EFI_SUCCESS) {
			Print(L"Cannot SetMode, status 0x%llx\n", status);
		}
		break;
	}


        return status;
}
