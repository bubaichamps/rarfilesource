/*
 * Copyright (C) 2008, OctaneSnail <os@v12pwr.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OUTPUT_PIN_H
#define OUTPUT_PIN_H

#include "List.h"

class CRARFileSource;
class File;
class FilePart;

class SubRequest : public Node<SubRequest>
{
public:
	SubRequest (void) : file (INVALID_HANDLE_VALUE), expected (0)
		{ memset (&o, 0, sizeof (OVERLAPPED)); o.hEvent = INVALID_HANDLE_VALUE; }
	~SubRequest (void) { if (o.hEvent != INVALID_HANDLE_VALUE) CloseHandle (o.hEvent); }

	HANDLE file;
	DWORD expected;
	OVERLAPPED o;
};

class ReadRequest : public Node<ReadRequest>
{
public:
	~ReadRequest (void) { subreqs.Clear (); }

	DWORD_PTR dwUser;
	IMediaSample *pSample;

	DWORD count;
	List<SubRequest> subreqs;
};

class CRFSOutputPin :
	public CBasePin,
	public IAsyncReader
{
public:
	CRFSOutputPin (CRARFileSource *pFilter, CCritSec *pLock, HRESULT *phr);
	~CRFSOutputPin (void);

	DECLARE_IUNKNOWN;

	// Reveals IAsyncReader
	STDMETHODIMP NonDelegatingQueryInterface (REFIID riid, void **ppv);

	// IPin interface
	STDMETHODIMP Connect (IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);

	// CBasePin
	HRESULT GetMediaType (int iPosition, CMediaType *pMediaType);
	HRESULT CheckMediaType (const CMediaType* pType);
	HRESULT CheckConnect (IPin *pPin);
	HRESULT CompleteConnect (IPin *pReceivePin);
	HRESULT BreakConnect ();

	// IAsyncReader interface
	STDMETHODIMP RequestAllocator (IMemAllocator *pPreferred, ALLOCATOR_PROPERTIES *pProps, IMemAllocator **ppActual);

	STDMETHODIMP Request (IMediaSample* pSample, DWORD_PTR dwUser);
	STDMETHODIMP WaitForNext (DWORD dwTimeout, IMediaSample **ppSample, DWORD_PTR *pdwUser);

	STDMETHODIMP SyncReadAligned (IMediaSample *pSample);
	STDMETHODIMP SyncRead (LONGLONG llPosition, LONG lLength, BYTE *pBuffer);

	STDMETHODIMP Length (LONGLONG *pTotal, LONGLONG *pAvailable);

	STDMETHODIMP BeginFlush (void);
	STDMETHODIMP EndFlush (void);

	void SetFile (File *file) { m_file = file; }

private:
	DWORD m_align;
	BOOL m_asked_for_reader;
	File *m_file;
	BOOL m_flush;
	HANDLE m_event;

	List<ReadRequest> m_requests;
	CCritSec m_lock;

	HRESULT ConvertSample (IMediaSample *sample, LONGLONG *pos, DWORD *length, BYTE **buffer);
	HRESULT DoFlush (IMediaSample **ppSample, DWORD_PTR *pdwUser);

	BOOL IsAligned (DWORD l) { return !(l & (m_align - 1)); }
	BOOL IsAligned (LONGLONG l) { return IsAligned ((DWORD) l); }
	BOOL IsAligned (INT_PTR l) { return IsAligned ((DWORD) l); }
};

#endif // OUTPUT_PIN_H
