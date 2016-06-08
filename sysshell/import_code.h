/*
 * Copyright 2010 coderebasoft
 *
 * This file is part of PEMaster.
 *
 * PEMaster is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PEMaster is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PEMaster.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

WCHAR * anti2unicode(const char* ansi,DWORD *size)
{
	WCHAR *unicode = NULL;
	INT32 _size; 

	ASSERT( ansi != NULL && 
		size != NULL ); 

	_size = MultiByteToWideChar(0, 0, ansi, (int)strlen(ansi), NULL, 0);
	
	if ( _size == 0 )
		return NULL;
	
	*size = ( _size + 1 ) * sizeof( WCHAR );

	try
	{
		unicode = ( WCHAR* )malloc( sizeof( WCHAR ) * ( _size + 1 ) ); 
		if( NULL == unicode )
		{
			goto _err_return; 
		}

		if ( 0 == MultiByteToWideChar( 0, 0, ansi, ( INT32 )strlen( ansi ), unicode, _size ) )
		{
			goto _err_return; 
		}

		unicode[ _size ] = 0;
	}
	catch(...)
	{
		goto _err_return; 
	}

	return unicode; 

_err_return: 
	if( unicode != NULL )
	{
		free( unicode ); 
	}
	return NULL;
}


//ת�������
BOOL encode_import_table(PBYTE pImportBuff,DWORD *needSize)
{
	PIMAGE_DATA_DIRECTORY pImportDir;
	PIMAGE_IMPORT_DESCRIPTOR pImprot;
	PIMAGE_THUNK_DATA pFirstThunk;
	DWORD index;
	PDWORD pFunCount; 
	INT32 len = 0;
	PBYTE pData=NULL;
	PIMAGE_IMPORT_BY_NAME pImportByName;
	WCHAR *dll_name;
	DWORD dllNameSize;
	DWORD importSize; 

	pImportDir = ( PIMAGE_DATA_DIRECTORY )&img_nt_hdr->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ]; 

	if( pImportDir->VirtualAddress == 0 )
	{
		DBGPRINT( ( "this file have not import data\n" ) );
		return FALSE;
	}


	importSize = pImportDir->Size;
	pImprot = ( PIMAGE_IMPORT_DESCRIPTOR )rva2va( pImportDir->VirtualAddress );
	//������Ҫ���ڴ泤��
	if( pImportBuff == NULL )
	{
		while( pImprot->Name != 0 )
		{

			/*
			**********************************************************************************
			*  DWROD      |  BYTE    |  STRING  |  DWORD        |  BYTE     |  STRING  |������������
			*(FirstThunk) |Dll������ |   Dll��  | ��dll�������� |���������� |   ������ |������������
			*********************************************************************************
			*
			*
			*/
			//�õ�������unicode�ַ���
			len = strlen( ( CHAR* )rva2va( pImprot->Name ) ) + 1; 
			len *= sizeof( WCHAR );
			//FirstThunk��DWORD)+Dll�����ȣ�BYTE��+DLL����������DWORD��=9
			len += sizeof( ULONG ) + sizeof( BYTE ) + sizeof( ULONG );
			*needSize += len;
			
			if( pImprot->OriginalFirstThunk != 0 )
			{
				pFirstThunk = ( PIMAGE_THUNK_DATA )rva2va( pImprot->OriginalFirstThunk );
			}
			else
			{
				ASSERT( FALSE ); 
				pFirstThunk = ( PIMAGE_THUNK_DATA )rva2va( pImprot->FirstThunk );
			}

			while( pFirstThunk->u1.Ordinal != 0 )
			{
				//����ŷ�ʽ����
				//��ŵ�������������=0�����������溯�����
				if( IMAGE_SNAP_BY_ORDINAL32( pFirstThunk->u1.Ordinal ) )
				{
					len = 1;
					len += sizeof( ULONG );
					*needSize += len;

				}
				else
				{
					//���ַ���ʽ
					pImportByName = ( PIMAGE_IMPORT_BY_NAME )rva2va( ( DWORD )pFirstThunk->u1.AddressOfData );
					len = strlen( ( char* )pImportByName->Name ) + 1;
					len++;
					*needSize += len;
					DBGPRINT( ( "import function name %s \n", pImportByName ) ); 
				}
				DBGPRINT( ( "needSize:%D \n", *needSize ) );
				pFirstThunk++;
			}
			pImprot++;

		}
	}
	else
	{
		pData = pImportBuff;

		while(pImprot->Name!=0)
		{
			DBGPRINT( ( "Dll:%s\n",( CHAR* )rva2va( pImprot->Name ) ) );
			//����FirstThunk
			*( DWORD* )pData = pImprot->FirstThunk;
			pData+=4;
			//��dll��ת����unicode
			dll_name=anti2unicode((char*)rva2va(pImprot->Name),&dllNameSize);
			if(dll_name==NULL)
			{
				DBGPRINT( ("ansi dll name to unicode dll name failed\n" ) );
				return FALSE;
			}
			//����dll������
			*(BYTE*)pData=(BYTE)dllNameSize;
			pData++;
			//����dll������
			memcpy(pData,dll_name,dllNameSize);
			pData=pData+dllNameSize;
			//ָ��dll�еĺ�������
			pFunCount=(DWORD*)pData;
			pData+=4;
			if(pImprot->OriginalFirstThunk!=0)
			{
				pFirstThunk=(PIMAGE_THUNK_DATA)rva2va(pImprot->OriginalFirstThunk);
			}
			else
			{
				pFirstThunk=(PIMAGE_THUNK_DATA)rva2va(pImprot->FirstThunk);
			}

			while(pFirstThunk->u1.Ordinal!=0)
			{
				//����ŷ�ʽ����
				if(IMAGE_SNAP_BY_ORDINAL32(pFirstThunk->u1.Ordinal))
				{
					*(BYTE*)pData=0;
					pData++;
					index= (DWORD)(pFirstThunk->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32);
					*(DWORD*)pData=index;
					pData+=4;
					*pFunCount=*pFunCount+1;
					DBGPRINT( ( "index:%d\n",index ) );
   
				}
				else
				{
					pImportByName=(PIMAGE_IMPORT_BY_NAME)rva2va((DWORD)pFirstThunk->u1.AddressOfData);
					//�õ�����������
					*(BYTE*)pData=(BYTE)( ( strlen((char*)pImportByName->Name)+1 > 0xFF ) ? ASSERT( FALSE ), 0xFF : strlen((char*)pImportByName->Name)+1  );
					pData++;
					//�õ�������
					strcpy((char*)pData,(char*)pImportByName->Name);
					pData+=(strlen((char*)pImportByName->Name)+1);
					*pFunCount=*pFunCount+1;
					DBGPRINT( ( "function name:%s, count:%d\n", pImportByName->Name, *pFunCount ) );

				}
				pFirstThunk++;
			}
			pImprot++;

		}
	}
	return TRUE;
} 

//���ԭ�����
void clear_import_tbl()
{
	PIMAGE_DATA_DIRECTORY pImportDir;
	PIMAGE_IMPORT_DESCRIPTOR pImprot;
	PIMAGE_THUNK_DATA pFirstThunk;
	PIMAGE_IMPORT_BY_NAME pImportByName;
	pImportDir=(PIMAGE_DATA_DIRECTORY)&img_nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if(pImportDir->VirtualAddress==0)
	{
		DBGPRINT( ( "this file have not import data\n" ) );
		return ;
	}
	DWORD importSize=pImportDir->Size;
	pImprot=(PIMAGE_IMPORT_DESCRIPTOR)rva2va(pImportDir->VirtualAddress);
	while(pImprot->Name!=0)
	{
		//���dll��
		memset((char*)rva2va(pImprot->Name),0,strlen((char*)rva2va(pImprot->Name)));
		if(pImprot->OriginalFirstThunk!=0)
		{
			pFirstThunk=(PIMAGE_THUNK_DATA)rva2va(pImprot->OriginalFirstThunk);
			while(pFirstThunk->u1.Ordinal!=0)
			{
				if(IMAGE_SNAP_BY_ORDINAL32(pFirstThunk->u1.Ordinal))
				{
					//��ַ�ÿ�
					memset(pFirstThunk,0,4);
				}
				else
				{
					//���������
					pImportByName=(PIMAGE_IMPORT_BY_NAME)rva2va((DWORD)pFirstThunk->u1.AddressOfData);
					memset(pImportByName,0,strlen((char*)pImportByName->Name)+1); //+2
					//��ַ�ÿ�
					memset(pImprot,0,4);
				}
				pFirstThunk++;
				
			}
		}
		if(pImprot->FirstThunk!=0)
		{
			pFirstThunk=(PIMAGE_THUNK_DATA)rva2va(pImprot->FirstThunk);
			while(pFirstThunk->u1.AddressOfData!=NULL)
			{
				memset(pFirstThunk,0,4);
				pFirstThunk++;

			}
		}
		memset(pImprot,0,sizeof(IMAGE_IMPORT_DESCRIPTOR));
		pImprot++;	
	}
}

