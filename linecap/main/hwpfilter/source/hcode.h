/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



/* $Id: hcode.h,v 1.3 2008-04-10 12:03:17 rt Exp $ */

#ifndef _HCODE_H_
#define _HCODE_H_

#include "hwplib.h"

/**
 * Codetype of Korean
 * KSSM - Johap, KS - Wansung
 */
enum { KSSM, KS, UNICODE };
/**
 *   Transfer combination-code for internal using of hwp to ascii
 */
DLLEXPORT int hcharconv(hchar ch, hchar *dest, int codeType) ;

DLLEXPORT int   kssm_hangul_to_ucs2(hchar ch, hchar *dest) ;
DLLEXPORT hchar ksc5601_han_to_ucs2 (hchar);
DLLEXPORT hchar ksc5601_sym_to_ucs2 (hchar);
DLLEXPORT hchar* hstr2ucsstr(hchar* hstr, hchar* ubuf);
/**
 * ���Ľ�Ʈ���� �ϼ�����Ʈ������ ��ȯ�Ѵ�.
 */
DLLEXPORT int hstr2ksstr(hchar* hstr, char* buf);

/**
 * �ѱ��� ������ �� �ִ� char����Ʈ���� ���Ľ�Ʈ������ ��ȯ�Ѵ�.
 */
DLLEXPORT hchar *kstr2hstr( uchar *src, hchar *dest );

/**
 * hwp�� ��θ� unix���·� �ٲ۴�.
 */
DLLEXPORT char *urltounix(const char *src, char *buf );

/**
 * hwp�� ��θ� windows���·� �ٲ۴�.
 */
#ifdef _WIN32
DLLEXPORT char *urltowin(const char *src, char *buf );
#endif
/**
 *  Transfer interger to string following format
 */
DLLEXPORT char* Int2Str(int value, const char *format, char *buf);

/**
 * color�ε��� ���� �������� �����Ͽ� ��Ÿ���ǽ��� color�� ��ȯ
 */
DLLEXPORT char *hcolor2str(uchar color, uchar shade, char *buf, bool bIsChar = false);

DLLEXPORT char *base64_encode_string( const uchar *buf, unsigned int len );
DLLEXPORT double calcAngle(int x1, int y1, int x2, int y2);


#endif                                            /* _HCODE_H_ */
