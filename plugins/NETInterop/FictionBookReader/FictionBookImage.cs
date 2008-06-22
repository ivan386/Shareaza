using System.Xml.Serialization;

namespace Schemas
{
	[XmlType(Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0")]
	[XmlRoot(ElementName = "FictionBook", Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0", IsNullable = false)]
	public class FictionBookImage
	{
		/// <remarks>
		/// Any binary data that is required for the presentation of this book in base64 format.
		/// Currently only images are used.
		///</remarks>
		[XmlElement("binary")]
		public FictionBookBinary[] binary;	
	}
}
