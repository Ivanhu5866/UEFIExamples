#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	INTN Argc, i;
	CHAR16 **Argv;

	InitializeLib(image, systab);
	Argc = GetShellArgcArgv(image, &Argv);

	Print(L"Argument Argc=%d\n", Argc);
	for (i = 0 ; i < Argc ; ++i)
		Print(L"  Argv[%d] = '%s'\n", i, Argv[i]);

	return EFI_SUCCESS;
}
