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

#define SHA1_DIGEST_SIZE 	20
#define SHA256_DIGEST_SIZE	32
#define SHA384_DIGEST_SIZE	48
#define SHA512_DIGEST_SIZE	64

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

typedef struct {
	UINT16 algorithmId;
	UINT16 digestSize;
} TCG_EfiSpecIdEventAlgorithmSize;

typedef struct {
	UINT8 signature[16];
	UINT32 platformClass;
	UINT8 specVersionMinor;
	UINT8 specVersionMajor;
	UINT8 specErrata;
	UINT8 uintnSize;
	UINT32 numberOfAlgorithms;
	TCG_EfiSpecIdEventAlgorithmSize digestSizes[];
//	UINT8 vendorInfoSize;
//	UINT8 vendorInfo;
} TCG_EfiSpecIDEventStruct;

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

	int i, j;

	EFI_GUID Guid = EFI_TCG2_PROTOCOL_GUID;

	EFI_STATUS Status;

	InitializeLib(image, systab);

	EFI_TCG2_PROTOCOL *Tcg2_protocal;

	Status = LibLocateProtocol(&Guid, (VOID **)&Tcg2_protocal);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to locate EFI_TCG2_Protocol: %r\n", Status);
		return Status;
	}

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

		/* dump first event (EfiSpecIdEvent) */
		UINT8 *Pos = (UINT8 *)StartAddr;
		CHAR16 tmpchar[16];
		TCG_PCREventStruc *Event = (TCG_PCREventStruc *)Pos;
		Print(L"PCR Index: %d\n", Event->pcrIndex);
		Print(L"Event Type: 0x%x\n", Event->eventType);
		Print(L"Digest:\n");
		for (i = 0; i < 20; i++) {				
			Print(L"%2.2x ", Event->digest[i]);
		}
		Print(L"\n");
		Print(L"Event Size: 0x%x\n", Event->eventDataSize);
		TCG_EfiSpecIDEventStruct *EventSpecID;
		EventSpecID = (TCG_EfiSpecIDEventStruct *)Event->event;
		Print(L"EfiSpecIdEvent:\n");
		for (i = 0; i < 16; i++) {
			tmpchar[i] = (CHAR16)(EventSpecID->signature[i]);
		}
		Print(L" Signature: %s\n", tmpchar);
		Print(L" platformClass: 0x%x\n", EventSpecID->platformClass);
		Print(L" specVersionMinor: 0x%x\n", EventSpecID->specVersionMinor);
		Print(L" specVersionMajor: 0x%x\n", EventSpecID->specVersionMajor);
		Print(L" specErrata: 0x%x\n", EventSpecID->specErrata);
		Print(L" uintnSize: 0x%x\n", EventSpecID->uintnSize);
		Print(L" numberOfAlgorithms: %d\n", EventSpecID->numberOfAlgorithms);
		Pos += (sizeof(TCG_PCREventStruc) + sizeof(TCG_EfiSpecIDEventStruct));
		TCG_EfiSpecIdEventAlgorithmSize *SpecIDigestSz = NULL;
		for (i = 0; i < EventSpecID->numberOfAlgorithms; i++) {
			SpecIDigestSz = (TCG_EfiSpecIdEventAlgorithmSize *)Pos;
			Print(L" algorithmId: 0x%x \n", SpecIDigestSz->algorithmId);
			Print(L" digestSize: 0x%x \n", SpecIDigestSz->digestSize);
			Pos += sizeof(TCG_EfiSpecIdEventAlgorithmSize);
		}
		UINT8 vendorInfoSize = *(UINT8 *)Pos;
		Print(L" vendorInfoSize: %d\n", vendorInfoSize);
		Pos += sizeof(UINT8);
		UINT8 vdata;
		for (i = 0; i < vendorInfoSize; i++) {
			if (Pos + vendorInfoSize > (UINT8 *)LastAddr) {
				Print(L"Event data too large, skip dump\n");
				return EFI_SUCCESS;
			}
			vdata = *(UINT8 *)Pos;
			Print(L"%2.2x ", vdata);
			Pos += sizeof(UINT8);
		}
		Print(L"\n");

		/* dump others events */
		TCG_PCR_EVENT2 *Event2 = NULL;
		UINTN len_digest = 0;
		UINT8 data;
		UINT32 EventSize;
		while (Pos < (UINT8 *)LastAddr) {
			Event2 = (TCG_PCR_EVENT2 *)Pos;
			Print(L"PCR Index: %d\n", Event2->PCRIndex);
			Print(L"Event Type: 0x%x\n", Event2->EventType);
			Print(L"Digests:\n");
			Print(L"Digests.Count: %d\n", Event2->Digests.Count);
			Pos += (sizeof(Event2->PCRIndex) + sizeof(Event2->EventType) + sizeof(Event2->Digests.Count));
			for (i = 0; i < Event2->Digests.Count; i++) {
			
				len_digest = GetDigestAlgLength(((TPMT_HA *)Pos)->Alg_Id);
				Print(L"  Digests.Alg_ID: 0x%x: \n", ((TPMT_HA *)Pos)->Alg_Id);
				Print(L"  Digests.Digest:\n");
				Pos += sizeof(TPMT_HA);
				for (j = 0; j < len_digest; j++) {
					if (Pos + len_digest > (UINT8 *)LastAddr) {
						Print(L"Event data too large, skip dump\n");
						return EFI_SUCCESS;
					}
					data = *(UINT8 *)Pos;
					Print(L"%02.2x ", data);
					Pos += sizeof(UINT8);					
				}
				Print(L"\n");
			}
			EventSize = *(UINT32 *)Pos;
			Print(L"EventSize: %d\n", EventSize);
			Pos += sizeof(UINT32);
			Print(L"Event:\n");
			if (Pos + EventSize > (UINT8 *)LastAddr) {
				Print(L"Event data too large, skip dump\n");
				return EFI_SUCCESS;
			}
			for (i = 0; i < EventSize; i++) {
				data = *(UINT8 *)Pos;
				Print(L"%02.2x ", data);
				Pos += sizeof(UINT8);
			}
			Print(L"\n");
		}

		Print (L"Dump last entry event\n");
		Event2 = (TCG_PCR_EVENT2 *)LastAddr;

		Print(L"PCR Index: %d\n", Event2->PCRIndex);
		Print(L"Event Type: 0x%x\n", Event2->EventType);
		Print(L"Digests: %d\n", Event2->Digests.Count);

		Pos = (UINT8 *)Event2->Digests.Digests;
		for (UINT8 i = 0; i < Event2->Digests.Count; i++) {
			
			len_digest = GetDigestAlgLength(((TPMT_HA *)Pos)->Alg_Id);
			Print(L"  Digests.Alg_ID: 0x%x: \n", ((TPMT_HA *)Pos)->Alg_Id);
			Print(L"  Digests.Digest:\n");
			Pos += sizeof(TPMT_HA);
			for (j = 0; j < len_digest; j++) {
				data = *(UINT8 *)Pos;
				Print(L"%02.2x ", data);
				Pos += sizeof(UINT8);					
			}
			Print(L"\n");
		}

		EventSize = *(UINT32 *)Pos;
		Print(L"EventSize: %d\n", EventSize);
		Pos += sizeof(UINT32);
		Print(L"Event:\n");
		for (i = 0; i < EventSize; i++) {
			data = *(UINT8 *)Pos;
			Print(L"%02.2x ", data);
			Pos += sizeof(UINT8);
		}
		Print(L"\n");

	} else {
		Print(L"GetEventLog with format Tcg1.2\n");
		Print (L"Dump event log\n");

		Print (L"first entry event addr 0x%lx\n", StartAddr);
		Print (L"Last entry event addr 0x%lx\n", LastAddr);
		UINT8 *Pos = (UINT8 *)StartAddr;
		TCG_PCREventStruc *Event = NULL;

		while (Pos < (UINT8 *)LastAddr) {
			Event = (TCG_PCREventStruc *)Pos;
			Print(L"  PCR Index: %d\n", Event->pcrIndex);
			Print(L"  Event Type: 0x%x\n", Event->eventType);
			Print(L"  Digest:\n");
			for (i = 0; i < 20; i++) {				
				Print(L"%02.2x ", Event->digest[i]);
			}
			Print (L"\n");
			Print(L"  Event Data Size: %d\n", Event->eventDataSize);
			Print(L"  Event:\n");
			if (Pos + Event->eventDataSize > (UINT8 *)LastAddr) {
				Print(L"Event data too large, skip dump\n");
				return EFI_SUCCESS;
			}
			for (i = 0; i < Event->eventDataSize; i++) {				
				Print(L"%02.2x ", Event->event[i]);
			}
			Pos += (sizeof(TCG_PCREventStruc) + Event->eventDataSize);
			Print (L"\n");
		}
		Print (L"Dump last entry event\n");
		Event = (TCG_PCREventStruc *)LastAddr;
		Print(L"  PCR Index: %d\n", Event->pcrIndex);
		Print(L"  Event Type: 0x%x\n", Event->eventType);
		Print(L"  Digest:\n");
		for (i = 0; i < 20; i++) {				
			Print(L"%2.2x ", Event->digest[i]);
		}
		Print (L"\n");
		Print(L"  Event Data Size: %d\n", Event->eventDataSize);
		Print(L"  Event:\n");
		for (i = 0; i < Event->eventDataSize; i++) {				
			Print(L"%02.2x ", Event->event[i]);
		}			
	}

	return EFI_SUCCESS;
}
