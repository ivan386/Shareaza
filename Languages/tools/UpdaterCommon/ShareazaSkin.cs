using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Xml;
using System.Xml.Serialization;

namespace Updater.Common
{
	[Serializable()]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(Namespace = "http://www.shareaza.com/schemas/Skin.xsd", IsNullable = false)]
	public class skin
	{
		[XmlElement("manifest")]
		public skinManifest manifest;

		[XmlArrayItem("dialog", IsNullable = false)]
		public skinDialog[] dialogs;

		[XmlAttribute()]
		public string version;

		public skin() {
			this.version = "1.0";
		}

		public object[] GetElements(Type elementType) {
			var match = (from FieldInfo fieldInfo in typeof(skin).GetFields(BindingFlags.Public |
																			BindingFlags.Instance)
						 where fieldInfo.FieldType.IsArray &&
							   fieldInfo.FieldType.Name == elementType.Name + @"[]"
						 select fieldInfo.GetValue(this)).FirstOrDefault();
			if (match == null)
				return null;
			object[] array = (object[])match;

			// Order by Identification field to put them to drop down list
			if (array.Length > 0) {
				if (!(array[0] is INamedElement))
					return null;
				array = array.OrderBy(item => ((INamedElement)item).Id).ToArray();
			}
			return array;
		}

		public static string GetElementName(Type elementType) {
			string match = (from FieldInfo fieldInfo in typeof(skin).GetFields(BindingFlags.Public |
																			   BindingFlags.Instance)
							where fieldInfo.FieldType.IsArray &&
								  fieldInfo.FieldType.Name == elementType.Name + @"[]"
							select fieldInfo.Name).FirstOrDefault();
			return match;
		}
	}

	[Serializable()]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "manifest", Namespace = "", IsNullable = false)]
	public class skinManifest
	{
		[XmlAttribute("name")]
		public string name;

		[XmlAttribute("language")]
		public string lang;
	}

	[Serializable()]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "dialog", Namespace = "", IsNullable = false)]
	public class skinDialog : INamedElement
	{
		[XmlElement("control")]
		public skinDialogControl[] controls;

		[XmlText()]
		public string[] Text;

		[XmlAttribute()]
		public string name;

		[XmlAttribute()]
		public string cookie;

		[XmlAttribute()]
		public string caption;

		[XmlAnyAttribute()]
		public XmlAttribute[] junk;

		public skinDialog Clone() {
			return new skinDialog()
			{
				caption = this.caption,
				controls = this.controls == null ? null : (skinDialogControl[])this.controls.Clone(),
				cookie = this.cookie,
				name = this.name
			};
		}

		#region INamedElement Members

		public string Id {
			get { return name; }
		}

		#endregion
	}

	[Serializable()]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	public class skinDialogControl
	{
		[XmlAttribute()]
		public string caption;

		[XmlAnyAttribute()]
		public XmlAttribute[] junk;
	}
}
