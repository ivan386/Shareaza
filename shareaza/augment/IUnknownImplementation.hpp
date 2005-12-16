////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// augment/IUnknownImplementation.hpp                                         //
//                                                                            //
// Copyright (C) 2002-2005 Shareaza Development Team.                         //
// This file is part of SHAREAZA (www.shareaza.com).                          //
//                                                                            //
// Shareaza is free software; you can redistribute it                         //
// and/or modify it under the terms of the GNU General Public License         //
// as published by the Free Software Foundation; either version 2 of          //
// the License, or (at your option) any later version.                        //
//                                                                            //
// Shareaza is distributed in the hope that it will be useful,                //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       //
// See the GNU General Public License for more details.                       //
//                                                                            //
// You should have received a copy of the GNU General Public License          //
// along with Shareaza; if not, write to the                                  //
// Free Software Foundation, Inc,                                             //
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef AUGMENT_IUNKNOWNIMPLEMENTATION_HPP_INCLUDED
#define AUGMENT_IUNKNOWNIMPLEMENTATION_HPP_INCLUDED

namespace augment
{

	// this is a simple implementation for IUnknown - it is restricted to
	// single interface COM objects
	template<typename Interface, typename MostDerived>
	class IUnknownImplementation : public Interface
	{
		BOOST_STATIC_ASSERT(( boost::is_convertible< Interface*, IUnknown* >::value ));
	protected:
		IUnknownImplementation()
			: Interface(), ref_count_( 0 )
		{
			ATLTRACE2( atlTraceCOM, 1, L"%s(%p)::ctor()\n", \
				AfxGetIIDString( __uuidof( Interface ) ), this );
		}
	private:
		HRESULT __stdcall QueryInterface(const IID& iid, void** ppvObject)
		{
			ATLTRACE2( atlTraceQI, 1, L"%s(%p)->QueryInterface(%s)\n", \
				AfxGetIIDString( __uuidof( Interface ) ), this, AfxGetIIDString( iid ) );
			if ( !ppvObject )
				return E_POINTER;

			if ( iid == __uuidof( IUnknown ) )
			{
				*ppvObject = static_cast< IUnknown* >( this );
				AddRef();
				return S_OK;
			}
			if ( iid == __uuidof( Interface ) )
			{
				*ppvObject = static_cast< Interface* >( this );
				AddRef();
				return S_OK;
			}

			ATLTRACE2( atlTraceQI, 1, "--> failed\n" );
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}
		ULONG __stdcall AddRef()
		{
			ULONG ref_count = ULONG( InterlockedIncrement( &ref_count_ ) );
			ATLTRACE2( atlTraceRefcount, 1, L"%s(%p)->AddRef - ref_count = %lu\n", \
				AfxGetIIDString( __uuidof( Interface ) ), this, ref_count );
			return ref_count;
		}
		ULONG __stdcall Release()
		{
			ULONG ref_count = ULONG( InterlockedDecrement( &ref_count_ ) );
			ATLTRACE2( atlTraceRefcount, 1, L"%s(%p)->Release - ref_count = %lu\n", \
				AfxGetIIDString( __uuidof( Interface ) ), this, ref_count );
			if ( ref_count != 0 )
				return ref_count;
			delete static_cast< MostDerived* >( this );
			return 0;
		};
	private:
		volatile LONG ref_count_;
	};

} // namespace augment

#endif // #ifndef AUGMENT_IUNKNOWNIMPLEMENTATION_HPP_INCLUDED
