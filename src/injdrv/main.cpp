
#include "../injlib/injlib.h"
#include <strsafe.h>
#pragma region unuse
//typedef union
//{
//	WCHAR Name[sizeof(ULARGE_INTEGER) / sizeof(WCHAR)];
//	ULARGE_INTEGER Alignment;
//} ALIGNEDNAME;

//
// DOS Device Prefix \??\
//

//ALIGNEDNAME ObpDosDevicesShortNamePrefix = { { L'\\', L'?', L'?', L'\\' } };
//UNICODE_STRING ObpDosDevicesShortName = {
//  sizeof(ObpDosDevicesShortNamePrefix), // Length
//  sizeof(ObpDosDevicesShortNamePrefix), // MaximumLength
//  (PWSTR)&ObpDosDevicesShortNamePrefix  // Buffer
//};

//NTSTATUS
//NTAPI
//InjpJoinPath(
//	_In_ PUNICODE_STRING Directory,
//	_In_ PUNICODE_STRING Filename,
//	_Inout_ PUNICODE_STRING FullPath
//)
//{
//	UNICODE_STRING UnicodeBackslash = RTL_CONSTANT_STRING(L"\\");
//
//	BOOLEAN DirectoryEndsWithBackslash = Directory->Length > 0 &&
//		Directory->Buffer[Directory->Length - 1] == L'\\';
//
//	if (FullPath->MaximumLength < Directory->Length ||
//		FullPath->MaximumLength - Directory->Length -
//		(!DirectoryEndsWithBackslash ? 1 : 0) < Filename->Length)
//	{
//		return STATUS_DATA_ERROR;
//	}
//
//	RtlCopyUnicodeString(FullPath, Directory);
//
//	if (!DirectoryEndsWithBackslash)
//	{
//		RtlAppendUnicodeStringToString(FullPath, &UnicodeBackslash);
//	}
//
//	RtlAppendUnicodeStringToString(FullPath, Filename);
//
//	return STATUS_SUCCESS;
//}

//NTSTATUS
//NTAPI
//copeeee(
//	_In_ PUNICODE_STRING Directory,
//	_Inout_ PUNICODE_STRING FullPath
//)
//{
//	RtlCopyUnicodeString(FullPath, Directory);
//
//	return STATUS_SUCCESS;
//}


//NTSTATUS GetRegistrySettings(
//	/*_Inout_ DWORD* DXVK_IMG,*/
//	_Inout_ UNICODE_STRING* DXVK_DIR,
//	_Inout_ UNICODE_STRING* DXVK_DDLS
//)
//{
//	RTL_QUERY_REGISTRY_TABLE paramTable[] = {
//		/*{NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_TYPECHECK, L"DXVK_IMG", DXVK_IMG, (REG_DWORD << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_DWORD, DXVK_IMG, sizeof(ULONG)},*/
//		{NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_TYPECHECK, L"DXVK_DIR", DXVK_DIR, (REG_SZ << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_SZ, DXVK_DIR, sizeof(WCHAR) * (DXVK_DIR->Length + 1)},
//		{NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_TYPECHECK, L"DXVK_DLLS", DXVK_DDLS, (REG_SZ << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_SZ, DXVK_DDLS, sizeof(WCHAR) * (DXVK_DDLS->Length + 1)},
//		{NULL, 0, NULL, NULL, 0, NULL, 0}
//	};
//
//	PAGED_CODE();
//
//	NTSTATUS ntStatus = RtlQueryRegistryValues(
//		RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
//		L"\\Registry\\Machine\\Software\\Rominky Soft\\DXVK addition",
//		&paramTable[0],
//		NULL,
//		NULL
//	);
//
//	if (!NT_SUCCESS(ntStatus))
//	{
//		InjDbgPrint("RtlQueryRegistryValues failed, using default values, 0x%x\n", ntStatus);
//	}
//	return ntStatus;
//}



//BOOLEAN WriteRegistryValue()
//{
//	NTSTATUS status;
//	ULONG data = 0xFF;
//
//	status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
//		L"\\Registry\\Machine\\Software\\Rominky Soft\\DXVK addition",
//		L"ValueName",
//		REG_DWORD,
//		&data,
//		sizeof(ULONG));
//
//	if (!NT_SUCCESS(status))
//	{
//		InjDbgPrint("Failed to write registry value: 0x%X\n", status);
//		return FALSE;
//	}
//
//	InjDbgPrint("Registry value written successfully\n");
//	return TRUE;
//}

//ULONG CountDelimitersInMultiSz(PUNICODE_STRING MultiSzString)
//{
//	ULONG count = 0;
//	for (ULONG i = 0; i < MultiSzString->Length / sizeof(WCHAR); i++)
//	{
//		if (MultiSzString->Buffer[i] == L';')
//		{
//			count++;
//		}
//	}
//	return count;
//}

//NTSTATUS
//NTAPI
//InjCreateSettings(
//	_In_ PUNICODE_STRING RegistryPath,
//	_Inout_ PINJ_SETTINGS Settings
//)
//{
//
//	UNICODE_STRING DXVK_DIR;
//	UNICODE_STRING DXVK_DIR_X64;
//	UNICODE_STRING DXVK_DIR_X86;
//	UNICODE_STRING DXVK_DLLS;
//
//	/* DWORD DXVK_IMG = 0;*/
//	RtlInitUnicodeString(&DXVK_DIR, NULL);
//	RtlInitUnicodeString(&DXVK_DIR_X64, NULL);
//	RtlInitUnicodeString(&DXVK_DIR_X86, NULL);
//	RtlInitUnicodeString(&DXVK_DLLS, NULL);
//
//	GetRegistrySettings(/*&DXVK_IMG,*/ &DXVK_DIR, &DXVK_DLLS);
//
//	DXVK_DIR_X64.MaximumLength = DXVK_DIR.Length + sizeof(L"\\x64") + sizeof(WCHAR);
//	DXVK_DIR_X86.MaximumLength = DXVK_DIR_X64.MaximumLength;
//
//	DXVK_DIR_X64.Buffer = (PWCH)ExAllocatePool2(POOL_FLAG_PAGED, DXVK_DIR_X64.MaximumLength, 'DX64');
//	DXVK_DIR_X86.Buffer = (PWCH)ExAllocatePool2(POOL_FLAG_PAGED, DXVK_DIR_X86.MaximumLength, 'DX86');
//	if (DXVK_DIR_X64.Buffer == NULL || DXVK_DIR_X86.Buffer == NULL)
//	{
//		return STATUS_INSUFFICIENT_RESOURCES;
//	}
//
//	RtlAppendUnicodeToString(&DXVK_DIR_X64, DXVK_DIR.Buffer);
//	RtlAppendUnicodeToString(&DXVK_DIR_X86, DXVK_DIR.Buffer);
//
//	RtlFreeUnicodeString(&DXVK_DIR);
//
//	RtlAppendUnicodeToString(&DXVK_DIR_X64, L"\\x64");
//	RtlAppendUnicodeToString(&DXVK_DIR_X86, L"\\x32");
//
//	/*InjDbgPrint("DXVK_DIR_86: %wZ\n", &DXVK_DIR_X86);
//	InjDbgPrint("DXVK_DIR_64: %wZ\n", &DXVK_DIR_X64);*/
//
//
//	//InjDbgPrint("DXVK_IMG: %u\n", DXVK_IMG);
//
//
//
//	//ULONG delimiterCount = CountDelimitersInMultiSz(&DXVK_DLLS);
//
//							//NTSTATUS Status;
//
//							//UNICODE_STRING ValueName = RTL_CONSTANT_STRING(L"ImagePath");
//
//
//							//OBJECT_ATTRIBUTES ObjectAttributes;
//							//InitializeObjectAttributes(&ObjectAttributes,
//							//	RegistryPath,
//							//	OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
//							//	NULL,
//							//	NULL);
//
//							//HANDLE KeyHandle;
//							//Status = ZwOpenKey(&KeyHandle,
//							//	KEY_READ,
//							//	&ObjectAttributes);
//
//							//if (!NT_SUCCESS(Status))
//							//{
//							//	return Status;
//							//}
//
//							////
//							//// Save all information on stack - simply fail if path
//							//// is too long.
//							////
//
//							//UCHAR KeyValueInformationBuffer[sizeof(KEY_VALUE_FULL_INFORMATION) + sizeof(WCHAR) * 128];
//							//PKEY_VALUE_FULL_INFORMATION KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)KeyValueInformationBuffer;
//
//							//ULONG ResultLength;
//							//Status = ZwQueryValueKey(KeyHandle,
//							//	&ValueName,
//							//	KeyValueFullInformation,
//							//	KeyValueInformation,
//							//	sizeof(KeyValueInformationBuffer),
//							//	&ResultLength);
//
//							//ZwClose(KeyHandle);
//
//							////
//							//// Check for succes.  Also check if the value is of expected
//							//// type and whether the path has a meaninful length.
//							////
//
//							//if (!NT_SUCCESS(Status) ||
//							//	KeyValueInformation->Type != REG_EXPAND_SZ ||
//							//	KeyValueInformation->DataLength < sizeof(ObpDosDevicesShortNamePrefix))
//							//{
//							//	return Status;
//							//}
//
//							////
//							//// Save pointer to the fetched ImagePath value and test if
//							//// the path starts with "\??\" prefix - if so, skip it.
//							////
//
//							//PWCHAR ImagePathValue = (PWCHAR)((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset);
//							//ULONG  ImagePathValueLength = KeyValueInformation->DataLength;
//
//							//if (*(PULONGLONG)(ImagePathValue) == ObpDosDevicesShortNamePrefix.Alignment.QuadPart)
//							//{
//							//	ImagePathValue += ObpDosDevicesShortName.Length / sizeof(WCHAR);
//							//	ImagePathValueLength -= ObpDosDevicesShortName.Length;
//							//}
//
//							////
//							//// Cut the string by the last '\' character, leaving there
//							//// only the directory path.
//							////
//
//							//PWCHAR LastBackslash = wcsrchr(ImagePathValue, L'\\');
//
//							//if (!LastBackslash)
//							//{
//							//	return STATUS_DATA_ERROR;
//							//}
//
//							//*LastBackslash = UNICODE_NULL;
//
//							//UNICODE_STRING Directory;
//							//RtlInitUnicodeString(&Directory, ImagePathValue);
//
//	//
//	// Finally, fill all the buffers...
//	//
//
//	RtlCopyUnicodeString(&Settings->Path[InjArchitectureX86], &DXVK_DIR_X86);
//	ExFreePool(DXVK_DIR_X86.Buffer);
//
//	RtlCopyUnicodeString(&Settings->Path[InjArchitectureX64], &DXVK_DIR_X64);
//	ExFreePool(DXVK_DIR_X64.Buffer);
//
//	RtlCopyUnicodeString(&Settings->Dlls, &DXVK_DLLS);
//
//	//RtlFreeUnicodeString(&DXVK_DIR_X86);
//	//RtlFreeUnicodeString(&DXVK_DIR_X64);
//	RtlFreeUnicodeString(&DXVK_DLLS);
//
//
//	InjDbgPrint("[injdrv]: &Settings->Path[InjArchitectureX64]:   '%wZ'\n", &Settings->Path[InjArchitectureX64]);
//	InjDbgPrint("[injdrv]: &Settings->Path[InjArchitectureX86]:   '%wZ'\n", &Settings->Path[InjArchitectureX86]);
//	InjDbgPrint("[injdrv]: &Settings->Dlls:   '%wZ'\n", &Settings->Dlls);
//
//
//
//
//	return STATUS_SUCCESS;
//}
#pragma endregion

//////////////////////////////////////////////////////////////////////////
// DriverEntry and DriverDestroy.
//////////////////////////////////////////////////////////////////////////

VOID
NTAPI
DriverDestroy(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	PsRemoveLoadImageNotifyRoutine(&InjLoadImageNotifyRoutine);
	PsSetCreateProcessNotifyRoutineEx(&InjCreateProcessNotifyRoutineEx, TRUE);
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\BlueStreetDriver");

	IoDeleteSymbolicLink(&symLink);

	PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;

	if (deviceObject != NULL)
	{
		IoDeleteDevice(deviceObject);
	}

	InjDestroy();
}

NTSTATUS BlueDriverCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS BlueDriverDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

PDRIVER_OBJECT g_DriverObject;
PUNICODE_STRING g_RegistryPath;
INJ_SETTINGS g_Settings;

NTSTATUS DriverCreateClose(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

#pragma region unuse
//typedef struct _KERNEL_NOTIFICATION {
				//	KSPIN_LOCK Lock;
				//	LIST_ENTRY ListHead;
				//} KERNEL_NOTIFICATION, * PKERNEL_NOTIFICATION;

				//KERNEL_NOTIFICATION KernelNotification;
				//void InitializeKernelNotification() {
				//	KeInitializeSpinLock(&KernelNotification.Lock);
				//	InitializeListHead(&KernelNotification.ListHead);
				//}

				//void SendKernelDataToUser(PIRP Irp) {
				//	CHAR* messageFromKernel = "ohai from them kernelz";

				//	// Проверка на NULL
				//	if (Irp == NULL) {
				//		DbgPrint("Invalid IRP: NULL pointer\n");
				//		return;
				//	}

				//	// Получение размера буфера
				//	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
				//	ULONG bufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

				//	// Проверка размера буфера
				//	if (bufferLength < strlen(messageFromKernel) + 1) {
				//		Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				//		Irp->IoStatus.Information = 0;
				//		IoCompleteRequest(Irp, IO_NO_INCREMENT);
				//		return;
				//	}

				//	// Установим статус и информацию об объеме данных перед копированием данных в буфер
				//	Irp->IoStatus.Status = STATUS_SUCCESS;
				//	Irp->IoStatus.Information = strlen(messageFromKernel) + 1;

				//	// Копируем данные в буфер
				//	RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, messageFromKernel, strlen(messageFromKernel) + 1); // +1 для включения null-терминатора

				//	// Завершаем запрос
				//	IoCompleteRequest(Irp, IO_NO_INCREMENT);
				//}

#pragma endregion

				
NTSTATUS DriverDeviceControl(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);
	
	auto stack = IoGetCurrentIrpStackLocation(Irp); // IOC_STACK_LOCATION*
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {

	case IOCTL_BLUESTREET_SEND_DATA:
	{
		auto pData = (sMSG*)stack->Parameters.DeviceIoControl.Type3InputBuffer;

		NTSTATUS Status = InjUpdateSettings(pData, stack);

		if (!NT_SUCCESS(Status)) {
			Irp->IoStatus.Status = Status;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			return Status;
		}
		break;
	}
#pragma region unuse
	//case IOCTL_BLUESTREET_DEBUG_PRINT:
//{
//	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

//	// Проверка размера буфера
//	if (stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof("ohai from them kernelz")) {
//		Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
//		Irp->IoStatus.Information = 0;
//		IoCompleteRequest(Irp, IO_NO_INCREMENT);
//		return STATUS_BUFFER_TOO_SMALL;
//	}

//	// Убедимся, что буфер IRP корректно инициализирован
//	if (Irp->AssociatedIrp.SystemBuffer == NULL) {
//		Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
//		Irp->IoStatus.Information = 0;
//		IoCompleteRequest(Irp, IO_NO_INCREMENT);
//		return STATUS_INSUFFICIENT_RESOURCES;
//	}

//	SendKernelDataToUser(Irp);
//	break;
//}
#pragma endregion

	default:
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

extern "C" 
NTSTATUS
NTAPI
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{

	NTSTATUS Status;

	DriverObject->DriverUnload = &DriverDestroy;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\BlueStreetDriver");
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		InjDbgPrint("Failed to create device object (0x%08X)\n", status);
		return status;
	}

	// Create Symbolic link for the Device which is accessible from user-mode
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\BlueStreetDriver");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		InjDbgPrint("Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}
	
#pragma region unuse
	//InitializeKernelNotification();

						//PIRP Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

						//if (Irp == NULL) {
						//	// Обработка ошибки
						//	return STATUS_SUCCESS;
						//}

						//// Настройка Irp для использования в SendKernelDataToUser
						//SendKernelDataToUser(Irp);
#pragma endregion

	InjInitialize(DriverObject, RegistryPath, InjMethodThunk);

	//
	// Install CreateProcess and LoadImage notification routines.
	//

	Status = PsSetCreateProcessNotifyRoutineEx(&InjCreateProcessNotifyRoutineEx, FALSE);

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	Status = PsSetLoadImageNotifyRoutine(&InjLoadImageNotifyRoutine);

	if (!NT_SUCCESS(Status))
	{
		PsSetCreateProcessNotifyRoutineEx(&InjCreateProcessNotifyRoutineEx, TRUE);
		return Status;
	}

	return STATUS_SUCCESS;
}

