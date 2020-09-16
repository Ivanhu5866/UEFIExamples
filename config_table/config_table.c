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

#define EFI_ACPI_20_TABLE_GUID \
{0x8868e871,0xe4f1,0x11d3,\
 {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}
/*
#define ACPI_TABLE_GUID \
 {0xeb9d2d30,0x2d88,0x11d3,\
 {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SAL_SYSTEM_TABLE_GUID \
 {0xeb9d2d32,0x2d88,0x11d3,\
 {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SMBIOS_TABLE_GUID \
 {0xeb9d2d31,0x2d88,0x11d3,\
 {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
#define SMBIOS3_TABLE_GUID \
 {0xf2fd1544, 0x9794, 0x4a2c,\
 {0x99,0x2e,0xe5,0xbb,0xcf,0x20,0xe3,0x94})
#define MPS_TABLE_GUID \
 {0xeb9d2d2f,0x2d88,0x11d3,\
 {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}
*/
#define EFI_JSON_CONFIG_DATA_TABLE_GUID \
 {0x87367f87, 0x1119, 0x41ce, \
 {0xaa, 0xec, 0x8b, 0xe0, 0x11, 0x1f, 0x55, 0x8a }}
#define EFI_JSON_CAPSULE_DATA_TABLE_GUID \
{0x35e7a725, 0x8dd2, 0x4cac, \
{ 0x80, 0x11, 0x33, 0xcd, 0xa8, 0x10, 0x90, 0x56 }}
#define EFI_JSON_CAPSULE_RESULT_TABLE_GUID \
{0xdbc461c3, 0xb3de, 0x422a,\
{0xb9, 0xb4, 0x98, 0x86, 0xfd, 0x49, 0xa1, 0xe5 }}

#define EFI_TCG2_FINAL_EVENTS_TABLE_GUID \
{ 0x1e2ed096, 0x30e2, 0x4254, \
{ 0xbd, 0x89, 0x86, 0x3b, 0xbe, 0xf8, 0x23, 0x25 } }

static EFI_GUID Guid1  = EFI_ACPI_20_TABLE_GUID;
static EFI_GUID Guid2  = ACPI_TABLE_GUID;
static EFI_GUID Guid3  = SAL_SYSTEM_TABLE_GUID;
static EFI_GUID Guid4  = SMBIOS_TABLE_GUID;
static EFI_GUID Guid5  = SMBIOS3_TABLE_GUID;
static EFI_GUID Guid6  = MPS_TABLE_GUID;
static EFI_GUID Guid7  = EFI_JSON_CONFIG_DATA_TABLE_GUID;
static EFI_GUID Guid8  = EFI_JSON_CAPSULE_DATA_TABLE_GUID;
static EFI_GUID Guid9  = EFI_JSON_CAPSULE_RESULT_TABLE_GUID;
static EFI_GUID Guid10 = EFI_TCG2_FINAL_EVENTS_TABLE_GUID;
 
static struct {
    EFI_GUID  *Guid;
    CHAR16    *GuidStr;
} Guids[] = {
    { &Guid1,  L"ACPI 2.0 table" },
    { &Guid2,  L"ACPI 1.0 table" },
    { &Guid3,  L"SAL system table" },
    { &Guid4,  L"SMBIOS table" },
    { &Guid5,  L"SMBIOS 3 table" },
    { &Guid6,  L"MPS table" },
    { &Guid7,  L"Json Config table" },
    { &Guid8,  L"Jason capsule date table" },
    { &Guid9,  L"Jason capsule result table" },
    { &Guid10, L"TCG final events table" },
    { NULL }
};

static void dump_guid(EFI_GUID *guid) {
	Print(L"%08x-", guid->Data1);
	Print(L"%04x-", guid->Data2);
	Print(L"%04x-", guid->Data3);
	for (int i = 0; i < 8; i++)
		Print(L"%02x", guid->Data4[i]);
	return;
}

EFI_STATUS
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
 
	InitializeLib(image_handle, systab);

	EFI_CONFIGURATION_TABLE *ct = systab->ConfigurationTable;
	Print(L"table number: %d\n", systab->NumberOfTableEntries);

	for (int i = 0; i < systab->NumberOfTableEntries; i++) {
		dump_guid(&(ct->VendorGuid));
		for (int j=0; Guids[j].Guid; j++) {
			if (!CompareMem(&ct->VendorGuid, Guids[j].Guid, sizeof(Guid1))) {
			Print(L" %s", Guids[j].GuidStr);
			break;
		}
        }
        Print(L"\n");
        ct++;
    }
 
    return EFI_SUCCESS;
}
