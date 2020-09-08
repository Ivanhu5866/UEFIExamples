/*
 *  Copyright(C) 2020 Canonical Ltd.
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

	void *Addr = NULL;
	EFI_STATUS Status;
	UINTN size = 0;
	CHAR16 **argv;
	INTN argc;

	InitializeLib(image, systab);

	Print(L"Get shell ArdcArgv\n");
	argc = GetShellArgcArgv(image, &argv);
	Print(L"argc = %d\n", argc);
	size = xtoi(argv[1]);
	Print(L"Allocate Pool with size: %d\n", size);

	Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, size, &Addr);

	if ( EFI_ERROR(Status) ) {
		Print(L"Allocate Pool Failed: %r\n", Status);
		return Status;
	}

	Status = uefi_call_wrapper(BS->FreePool, 1, Addr);

	if ( EFI_ERROR(Status) ) {
		Print(L"Free Pool Failed: %r\n", Status);
		return Status;
	}

	return EFI_SUCCESS;
}
