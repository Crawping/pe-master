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

PIMAGE_NT_HEADERS img_nt_hdr;
PIMAGE_SECTION_HEADER img_section_hdrs;

DWORD img_load_base;
DWORD img_load_size;

DWORD img_file_align;
DWORD img_section_align;

INT32 img_sect_num;

DWORD headerSize; 

PBYTE img_base;

PBYTE import_codes;
DWORD import_codes_size;

DWORD no_pack_res_size;
PBYTE no_pack_res = NULL;

PVOID org_sect_hdrs;
DWORD org_sect_size;

PVOID img_res_dir = NULL;   //�������ԴĿ¼
DWORD img_res_dir_size;     //��ԴĿ¼��С

PCHAR pack_work_space = NULL;		// ѹ���������ʱ�ڴ�
PCHAR packed_data = NULL;		// ѹ����ĵ�ַ
UINT32 packed_size;			// ѹ�������ݴ�С

HANDLE packed_file = INVALID_HANDLE_VALUE;				// ����ѹ���ļ��ľ��

INLINE PBYTE rva2va( DWORD rva )
{
	if ( ( ULONG )rva < img_load_size )
	{
		return ( PBYTE )( img_base + rva );
	}  
	else  
	{
		return NULL;
	}
}

ULONG align(UINT size, UINT aglign)
{
	return ( ( size + aglign - 1 ) / aglign * aglign);
}
