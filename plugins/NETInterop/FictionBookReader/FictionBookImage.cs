//
// FictionBookImage.cs
//
// Copyright (c) Shareaza Development Team, 2008.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

using System.Xml.Serialization;

namespace Schemas
{
	[XmlType(Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0")]
	[XmlRoot(ElementName = "FictionBook", Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0", IsNullable = false)]
	public class FictionBookImage
	{
		[XmlElement("description")]
		public FbTitleInfo description;	
		/// <remarks>
		/// Any binary data that is required for the presentation of this book in base64 format.
		/// Currently only images are used.
		///</remarks>
		[XmlElement("binary")]
		public FictionBookBinary[] binary;	
	}
	
	[XmlType()]
	public class FbTitleInfo
	{
		/// <remarks>
		/// Generic information about the book
		///</remarks>
		[XmlElement("title-info")]
		public FictionBookDescriptionTitleinfo titleinfo;
	}	
}
