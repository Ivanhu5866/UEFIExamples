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

#define min(a, b) ({(a) < (b) ? (a) : (b);})

static int
count_lines(CHAR16 *str_arr[])
{
	int i = 0;

	while (str_arr[i])
		i++;
	return i;
}

static void
SetMem16(CHAR16 *dst, UINT32 n, CHAR16 c)
{
	unsigned int i;

	for (i = 0; i < n/2; i++) {
		dst[i] = c;
	}
}


EFI_STATUS
enable_cursor(BOOLEAN enable)
{
	SIMPLE_TEXT_OUTPUT_INTERFACE *co = ST->ConOut;
        EFI_STATUS status;

	status = uefi_call_wrapper(co->EnableCursor,
                           	2,
				co,
				enable);

	if (status != EFI_SUCCESS) {
		Print(L"Cannot EnableCursor\n");
	}
	return status;
}

EFI_STATUS
set_attribute(UINTN attr)
{
	SIMPLE_TEXT_OUTPUT_INTERFACE *co = ST->ConOut;
        EFI_STATUS status;

	status = uefi_call_wrapper(co->SetAttribute,
                           	2,
				co,
				attr);

	if (status != EFI_SUCCESS) {
		Print(L"Cannot Set Attribute\n");
	}
	return status;
}

EFI_STATUS
set_cursor_position(UINTN col, UINTN row)
{
	SIMPLE_TEXT_OUTPUT_INTERFACE *co = ST->ConOut;
        EFI_STATUS status;

	status = uefi_call_wrapper(co->SetCursorPosition,
                           	3,
				co,
				col,
				row);

	if (status != EFI_SUCCESS) {
		Print(L"Cannot SetCursorPosition\n");
	}
	return status;
}

EFI_STATUS
output_string(CHAR16 *string)
{
	SIMPLE_TEXT_OUTPUT_INTERFACE *co = ST->ConOut;
        EFI_STATUS status;

	status = uefi_call_wrapper(co->OutputString,
                           	2,
				co,
				string);

	if (status != EFI_SUCCESS) {
		Print(L"Cannot output_string\n");
	}
	return status;
}

void
print_box(CHAR16 *str_arr[], int lines)
{
        EFI_STATUS status;

	SIMPLE_TEXT_OUTPUT_MODE SavedConsoleMode;
	SIMPLE_TEXT_OUTPUT_INTERFACE *co = ST->ConOut;

	UINTN rows, cols, size_rows, size_cols;
	CHAR16 *Line;
	UINTN i;

	//if (!console_text_mode)
	//	setup_console(1);
	CopyMem(&SavedConsoleMode, co->Mode, sizeof(SavedConsoleMode));
	enable_cursor(FALSE);
	set_attribute(EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE);

	status = uefi_call_wrapper(co->QueryMode,
                           	4,
				co,
                          	co->Mode->Mode,
                           	&cols,
				&rows);

	if (status != EFI_SUCCESS) {
		Print(L"Cannot QueryMode, status 0x%llx\n", status);
		return;
	}

	rows--;
	size_rows = rows;
	size_cols = cols;
	Line = AllocatePool((size_cols+1)*sizeof(CHAR16));
	if (!Line) {
		Print(L"Cannot QueryMode, status 0x%llx\n", status);
		return;
	}

	SetMem16 (Line, size_cols * 2, BOXDRAW_HORIZONTAL);

	Line[0] = BOXDRAW_DOWN_RIGHT;
	Line[size_cols - 1] = BOXDRAW_DOWN_LEFT;
	Line[size_cols] = L'\0';
	set_cursor_position(0, 0);
	output_string(Line);
	UINTN start;
	start = (size_rows - lines)/2;

	for (i = 1; i < size_rows - 1; i++) {
		UINTN line = i - start;

		SetMem16 (Line, size_cols*2, L' ');
		Line[0] = BOXDRAW_VERTICAL;
		Line[size_cols - 1] = BOXDRAW_VERTICAL;
		Line[size_cols] = L'\0';
		if (line >= 0 && line < lines) {
			CHAR16 *s = str_arr[line];
			int len = StrLen(s);
			int col = (size_cols - 2 - len)/2;

			if (col < 0)
				col = 0;

			CopyMem(Line + col + 1, s, min(len, size_cols - 2)*2);
		}

		set_cursor_position(0, i);
		output_string(Line);

	}

	SetMem16 (Line, size_cols * 2, BOXDRAW_HORIZONTAL);
	Line[0] = BOXDRAW_UP_RIGHT;
	Line[size_cols - 1] = BOXDRAW_UP_LEFT;
	Line[size_cols] = L'\0';
	set_cursor_position (0, i);
	output_string(Line);

	FreePool (Line);
}



EFI_STATUS
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{ 
	EFI_INPUT_KEY eik;

	SIMPLE_TEXT_OUTPUT_MODE SavedConsoleMode;
	SIMPLE_TEXT_OUTPUT_INTERFACE *co = ST->ConOut;

	CHAR16 *text[9];

        InitializeLib(image_handle, systab);

	CopyMem(&SavedConsoleMode, co->Mode, sizeof(SavedConsoleMode));
	enable_cursor(FALSE);
	set_attribute(EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE);

	text[0] = L"TESTING!";
	text[1] = L"";
	text[2] = L"1111111111111111111111111111111111111111111111111111111";
	text[3] = L"222222 3333333333 44444444 555555555555 6666666 7777777";
	text[4] = L"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	text[5] = L"bbbbbbbbbbbbbbbbbb  ccccccccccccccccccccccccccccccccccc";
	text[6] = L"";
	text[7] = L"Please enter any key to continue.";
	text[8] = NULL;

	//console_reset();
	//console_print_box(text, -1);

	print_box(text, count_lines(text));

	while (1) {
		WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
		uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &eik);
		break;
	}	

	enable_cursor(SavedConsoleMode.CursorVisible);
	set_cursor_position(SavedConsoleMode.CursorColumn, SavedConsoleMode.CursorRow);
	set_attribute(SavedConsoleMode.Attribute);

        return EFI_SUCCESS;
}
