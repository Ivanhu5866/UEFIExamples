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

#define EFI_TCG2_PROTOCOL_GUID \
{ \
0x607f766c, 0x7455, 0x42be, { 0x93, 0x0b, 0xe4, 0xd7, 0x6d, 0xb2, 0x72, 0x0f } \
}

#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2 0x00000001
#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_2 0x00000002
typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT32 EFI_TCG2_EVENT_LOG_FORMAT;
typedef UINT16 TPM_ALG_ID;
typedef UINT32 EFI_TCG2_EVENT_LOG_BITMAP;
typedef UINT32 EFI_TCG2_EVENT_ALGORITHM_BITMAP;

#define SHA1_DIGEST_SIZE 	20
#define SHA256_DIGEST_SIZE	32
#define SHA384_DIGEST_SIZE	48
#define SHA512_DIGEST_SIZE	64

#define EFI_TCG2_BOOT_HASH_ALG_SHA1	0x00000001
#define EFI_TCG2_BOOT_HASH_ALG_SHA256	0x00000002
#define EFI_TCG2_BOOT_HASH_ALG_SHA384	0x00000004
#define EFI_TCG2_BOOT_HASH_ALG_SHA512	0x00000008
#define EFI_TCG2_BOOT_HASH_ALG_SM3_256	0x00000010

#define TPM_ALG_SHA1 		(TPM_ALG_ID)(0x0004)
#define TPM_ALG_AES 		(TPM_ALG_ID)(0x0006)
#define TPM_ALG_KEYEDHASH	(TPM_ALG_ID)(0x0008)
#define TPM_ALG_SHA256		(TPM_ALG_ID)(0x000B)
#define TPM_ALG_SHA384		(TPM_ALG_ID)(0x000C)
#define TPM_ALG_SHA512		(TPM_ALG_ID)(0x000D)
#define TPM_ALG_NULL		(TPM_ALG_ID)(0x0010)
#define TPM_ALG_SM3_256		(TPM_ALG_ID)(0x0012)
#define TPM_ALG_SM4		(TPM_ALG_ID)(0x0013)
#define TPM_ALG_RSASSA		(TPM_ALG_ID)(0x0014)
#define TPM_ALG_RSAES		(TPM_ALG_ID)(0x0015)
#define TPM_ALG_RSAPSS		(TPM_ALG_ID)(0x0016)
#define TPM_ALG_OAEP		(TPM_ALG_ID)(0x0017)
#define TPM_ALG_ECDSA		(TPM_ALG_ID)(0x0018)
#define TPM_ALG_ECDH		(TPM_ALG_ID)(0x0019)
#define TPM_ALG_ECDAA		(TPM_ALG_ID)(0x001A)
#define TPM_ALG_SM2		(TPM_ALG_ID)(0x001B)
#define TPM_ALG_ECSCHNORR	(TPM_ALG_ID)(0x001C)
#define TPM_ALG_ECMQV		(TPM_ALG_ID)(0x001D)
#define TPM_ALG_KDF1_SP800_56a	(TPM_ALG_ID)(0x0020)
#define TPM_ALG_KDF2		(TPM_ALG_ID)(0x0021)
#define TPM_ALG_KDF1_SP800_108	(TPM_ALG_ID)(0x0022)
#define TPM_ALG_ECC 		(TPM_ALG_ID)(0x0023)
#define TPM_ALG_SYMCIPHER	(TPM_ALG_ID)(0x0025)
#define TPM_ALG_CTR		(TPM_ALG_ID)(0x0040)
#define TPM_ALG_OFB		(TPM_ALG_ID)(0x0041)
#define TPM_ALG_CBC		(TPM_ALG_ID)(0x0042)
#define TPM_ALG_CFB		(TPM_ALG_ID)(0x0043)
#define TPM_ALG_ECB		(TPM_ALG_ID)(0x0044)

typedef struct {
	UINT8 Major;
	UINT8 Minor;
} EFI_TCG2_VERSION;

typedef struct {
	UINT8 Size;
	EFI_TCG2_VERSION StructureVersion;
	EFI_TCG2_VERSION ProtocolVersion;
	EFI_TCG2_EVENT_ALGORITHM_BITMAP HashAlgorithmBitmap;
	EFI_TCG2_EVENT_LOG_BITMAP SupportedEventLogs;
	BOOLEAN TPMPresentFlag;
	UINT16 MaxCommandSize;
	UINT16 MaxResponseSize;
	UINT32 ManufacturerID;
	UINT32 NumberOfPcrBanks;
	EFI_TCG2_EVENT_ALGORITHM_BITMAP ActivePcrBanks;
} EFI_TCG2_BOOT_SERVICE_CAPABILITY;

#pragma pack(1)

typedef struct {
	VOID* GetCapability;
	VOID* GetEventLog;
	VOID* HashLogExtendEvent;
	VOID* SubmitCommand;
	VOID* GetActivePcrBanks;
	VOID* SetActivePcrBanks;
	VOID* GetResultOfSetActivePcrBanks;
} EFI_TCG2_PROTOCOL;

typedef struct {
	UINT16 Alg_Id;
	UINT8 Digest[];
} TPMT_HA;

typedef struct {
	UINT32 Count;
	TPMT_HA Digests[];
} TPML_DIGEST_VALUES;

typedef struct {
	UINT32 PCRIndex;
	UINT32 EventType;
	TPML_DIGEST_VALUES Digests;
	UINT32 EventSize;
	UINT8 Event[];
} TCG_PCR_EVENT2;

typedef struct {
	UINT32	pcrIndex;
	UINT32	eventType;
	UINT8	digest[20];
	UINT32	eventDataSize;
	UINT8	event[];
} TCG_PCREventStruc;

STATIC UINTN
GetDigestAlgLength(TPM_ALG_ID AlgId)
{
	UINTN Length;

	switch (AlgId) {
	case TPM_ALG_SHA1:
		Length = SHA1_DIGEST_SIZE;
		break;
	case TPM_ALG_SHA256:
		Length = SHA256_DIGEST_SIZE;
		break;
	case TPM_ALG_SHA384:
		Length = SHA384_DIGEST_SIZE;
		break;
	case TPM_ALG_SHA512:
		Length = SHA512_DIGEST_SIZE;
		break;
	default:
		Length = 0;
	}

	return Length;
}

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{

	EFI_GUID Guid = EFI_TCG2_PROTOCOL_GUID;

	EFI_STATUS Status;

	InitializeLib(image, systab);

	EFI_TCG2_PROTOCOL *Tcg2_protocal;

	Status = LibLocateProtocol(&Guid, (VOID **)&Tcg2_protocal);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to locate EFI_TCG2_Protocol: %r\n", Status);
		return Status;
	}

	EFI_TCG2_BOOT_SERVICE_CAPABILITY cap;
	cap.Size = sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY);

	Status = uefi_call_wrapper(Tcg2_protocal->GetCapability, 2, Tcg2_protocal, &cap);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to get capability %r\n", Status);
		return Status;
	}

	Print(L"StructureVersion: %d.%d\n", (UINT8)cap.StructureVersion.Major, (UINT8)cap.StructureVersion.Minor);
	Print(L"ProtocolVersion: %d.%d\n", (UINT8)cap.ProtocolVersion.Major, (UINT8)cap.ProtocolVersion.Minor);
	Print(L"HashAlgorithmBitmap: %d\n", cap.HashAlgorithmBitmap);
	Print(L"TPMPresentFlag: %d\n", cap.TPMPresentFlag);
	Print(L"SupportedEventLogs: %d\n", cap.SupportedEventLogs);
	/* check event log format firmware supported */
	if (cap.SupportedEventLogs & EFI_TCG2_EVENT_LOG_FORMAT_TCG_2)
		Print(L" Crypto agile log format supported\n");
	if(cap.SupportedEventLogs & EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2)
		Print(L" SHA-1 log format supported\n");
	/* check Active PCR Bank */
	Print(L"Number of PCR banks: %d\n", cap.NumberOfPcrBanks);	
	if (cap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA1)
		Print(L" Sha-1 supported\n");
	if(cap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA256)
		Print(L" SHA-256 supported\n");
	if(cap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA384)
		Print(L" SHA-384 supported\n");
	if(cap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SHA512)
		Print(L" SHA-512 supported\n");
	if(cap.ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_SM3_256)
		Print(L" SM3-256 supported\n");	
	
	UINT8 version = EFI_TCG2_EVENT_LOG_FORMAT_TCG_2;
	EFI_PHYSICAL_ADDRESS StartAddr;
	EFI_PHYSICAL_ADDRESS LastAddr;
	BOOLEAN Truncated;
	Status = uefi_call_wrapper(Tcg2_protocal->GetEventLog, 5, Tcg2_protocal,
				   version, &StartAddr,
				   &LastAddr, &Truncated);

	if (EFI_ERROR(Status)) {
		Print(L"Unable to GetEventLog with format Tcg2: %r\n", Status);
		Print(L"Try GetEventLog with format Tcg1.2\n");
		version = EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2;
		Status = uefi_call_wrapper(Tcg2_protocal->GetEventLog, 5, Tcg2_protocal,
			   version, &StartAddr,
			   &LastAddr, &Truncated);
		if (EFI_ERROR(Status)) {
			Print(L"Unable to GetEventLog with format Tcg1.2: %r\n", Status);
			return Status;
		}
		Print(L"Able to GetEventLog with format Tcg1.2\n");
	}

	if (version == EFI_TCG2_EVENT_LOG_FORMAT_TCG_2) {
		Print(L"GetEventLog with format Tcg2\n");

		Print (L"first entry event addr 0x%lx\n", StartAddr);
		Print (L"Last entry event addr 0x%lx\n", LastAddr);

		Print (L"Dump last entry event\n");
		UINT64 size = 0;
		TCG_PCR_EVENT2 *Event2 = (TCG_PCR_EVENT2 *)LastAddr;

		Print(L"  PCR Index: %d\n", Event2->PCRIndex);
		Print(L"  Event Type: 0x%x\n", Event2->EventType);
		Print(L"  Digests: %d\n", Event2->Digests.Count);
		
		size += sizeof(Event2->PCRIndex) + sizeof(Event2->EventType) + Event2->Digests.Count;
		
		UINT8 *Pos = (UINT8 *)Event2->Digests.Digests;
		UINTN len_digest = 0;
		for (UINT8 i = 0; i < Event2->Digests.Count; i++) {
			
			len_digest = GetDigestAlgLength(((TPMT_HA *)Pos)->Alg_Id);
			Print(L"  Alg_ID 0x%x: \n", ((TPMT_HA *)Pos)->Alg_Id);
			/* skip dump digest */

			size += sizeof(TPMT_HA) + len_digest;
			Pos += sizeof(TPMT_HA) + len_digest;
		}

		UINT32 DataSize = *(UINT32 *)Pos;
		size += sizeof(UINT32);
		Pos += sizeof(UINT32);
		Print(L"  Event Data Size: %d\n", DataSize);
		size += DataSize;
		Pos += DataSize;
		UINT64 total_event_sz = LastAddr - StartAddr + size;
		Print(L"  Total event size %d\n", total_event_sz);
	} else {
		Print(L"GetEventLog with format Tcg1.2\n");

		Print (L"first entry event addr 0x%lx\n", StartAddr);
		Print (L"Last entry event addr 0x%lx\n", LastAddr);

		Print (L"Dump last entry event\n");
		TCG_PCREventStruc *Event = (TCG_PCREventStruc *)LastAddr;

		Print(L"  PCR Index: %d\n", Event->pcrIndex);
		Print(L"  Event Type: 0x%x\n", Event->eventType);

		/* skip dump digest */

		UINT32 evDataSize = Event->eventDataSize;
		Print(L"  Event Data Size: %d\n", evDataSize);
		UINT64 total_event_sz = LastAddr - StartAddr + sizeof(TCG_PCREventStruc) + evDataSize;
		Print(L"  Total event size %d\n", total_event_sz);
	}

	return EFI_SUCCESS;
}
