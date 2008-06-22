using System;

namespace Shareaza
{
	internal sealed class Hresults
	{
		public const int E_FAIL = unchecked((int)0x80004005);
        public const int E_INVALIDARG = unchecked((int)0x80070057);
		public const int E_UNEXPECTED = unchecked((int)0x8000FFFF);
		public const int E_NOINTERFACE = unchecked((int)0x80004002);
		public const int E_OUTOFMEMORY = unchecked((int)0x8007000E);
		public const int E_NOTIMPL = unchecked((int)0x80004001);
		public const int S_OK = 0;
		public const int S_FALSE = 1;
	}
}
