using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Xml;
using System.Xml.Serialization;

namespace Updater.Common
{
	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(Namespace = "http://www.shareaza.com/schemas/Skin.xsd", IsNullable = false)]
	public class skin
	{
		[XmlElement("manifest")]
		public skinManifest manifest;

		[XmlArrayItem("dialog", IsNullable = false)]
		public skinDialog[] dialogs;

		[XmlArrayItem("toolbar", IsNullable = false)]
		public skinToolbar[] toolbars;

		[XmlArrayItem("tip", IsNullable = false)]
		public skinCommandTip[] commandTips;

		[XmlArrayItem("tip", IsNullable = false)]
		public skinControlTip[] controlTips;

		[XmlArrayItem("string", IsNullable = false)]
		public skinString[] strings;

		[XmlAttribute]
		public string version;

		[XmlIgnore]
		public XmlElement RootElement { get; set; }

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
		
		public static string GetElementChildName(Type elementType) {
			object[] attributes = elementType.GetCustomAttributes(typeof(XmlRootAttribute), false);
			if (attributes != null && attributes.Length > 0)
				return (attributes[0] as XmlRootAttribute).ElementName;
			return String.Empty;		
		}
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "manifest", Namespace = "", IsNullable = false)]
	public class skinManifest
	{
		[XmlAttribute("name")]
		public string name;

		[XmlAttribute("language")]
		public string lang;
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "dialog", Namespace = "", IsNullable = false)]
	public class skinDialog : INamedElement
	{
		[XmlElement("control")]
		public skinDialogControl[] controls;

		[XmlText]
		public string[] Text;

		[XmlAttribute]
		public string name;

		[XmlAttribute]
		public string cookie;

		[XmlAttribute]
		public string caption;

		[XmlAnyAttribute]
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

		[XmlIgnore]
		public string Id {
			get { return name; }
		}

		[XmlIgnore]
		public XmlNodeList NodeList { get; set; }

		#endregion
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	public class skinDialogControl
	{
		[XmlAttribute]
		public string caption;

		[XmlAnyAttribute]
		public XmlAttribute[] junk;
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "toolbar", Namespace = "", IsNullable = false)]
	public class skinToolbar : INamedElement
	{
		[XmlText]
		public string[] Text;
			
		[XmlElement]
		public string rightAlign;

		[XmlElement("button")]
		public skinToolbarButton[] buttons;

		[XmlElement("separator")]
		public skinToolbarSeparator[] separators;

		[XmlElement("label")]
		public skinToolbarLabel[] labels;

		[XmlElement("control")]
		public skinToolbarControl[] controls;

		[XmlAttribute]
		public string name;

		[XmlAnyAttribute]
		public XmlAttribute[] junk;

		#region INamedElement Members

		[XmlIgnore]
		public string Id {
			get { return name; }
		}

		[XmlIgnore]
		public XmlNodeList NodeList { get; set; }

		#endregion

		public skinToolbar Clone() {
			return new skinToolbar()
			{
				rightAlign = this.rightAlign,
				buttons = this.buttons == null ? null : (skinToolbarButton[])this.buttons.Clone(),
				labels = this.labels == null ? null : (skinToolbarLabel[])this.labels.Clone(),
				separators = this.separators == null ? null : (skinToolbarSeparator[])this.separators.Clone(),
				controls = this.controls == null ? null : (skinToolbarControl[])this.controls.Clone(),
				name = this.name
			};
		}
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	public class skinToolbarButton
	{
		[XmlAttribute]
		public string id;

		[XmlAttribute]
		public string text;

		[XmlAttribute]
		public bool visible;

		[XmlAttribute]
		public string tip;

		[XmlAnyAttribute]
		public XmlAttribute[] junk;		
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	public class skinToolbarSeparator
	{
		[XmlAnyAttribute]
		public XmlAttribute[] junk;	
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	public class skinToolbarLabel
	{
		[XmlAttribute]
		public string text;

		[XmlAttribute]
		public string tip;

		[XmlAnyAttribute]
		public XmlAttribute[] junk;		
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	public class skinToolbarControl
	{
		[XmlAttribute]
		public string id;

		[XmlAttribute]
		public int width;

		[XmlAttribute]
		public int height;

		[XmlAttribute]
		public string text;

		[XmlAttribute]
		public bool @checked;

		[XmlAnyAttribute]
		public XmlAttribute[] junk;		
	}
	
	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "tip", Namespace = "", IsNullable = false)]
	public class skinCommandTip : skinTip
	{
		public skinCommandTip Clone() {
			return new skinCommandTip()
			{
				id = base.id,
				message = base.message
			};
		}	
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "tip", Namespace = "", IsNullable = false)]
	public class skinControlTip : skinTip
	{
		public skinControlTip Clone() {
			return new skinControlTip()
			{
				id = base.id,
				message = base.message
			};
		}	
	}	
	
	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "tip", Namespace = "", IsNullable = false)]
	public class skinTip : INamedElement
	{
		[XmlText]
		public string[] Text;
			
		[XmlAttribute]
		public string id;

		[XmlAttribute]
		public string message;

		[XmlAnyAttribute]
		public XmlAttribute[] junk;

		#region INamedElement Members

		[XmlIgnore]
		public string Id {
			get { return id; }
		}
		
		[XmlIgnore]
		public XmlNodeList NodeList { get; set; }

		#endregion
	}

	[Serializable]
	[XmlType(AnonymousType = true, Namespace = "http://www.shareaza.com/schemas/Skin.xsd")]
	[XmlRoot(ElementName = "string", Namespace = "", IsNullable = false)]
	public class skinString : INamedElement
	{
		[XmlText]
		public string[] Text;
			
		[XmlAttribute]
		public string id;

		[XmlAttribute]
		public string value;

		[XmlAnyAttribute]
		public XmlAttribute[] junk;

		#region INamedElement Members

		[XmlIgnore]
		public string Id {
			get { return id; }
		}

		[XmlIgnore]
		public XmlNodeList NodeList { get; set; }

		#endregion

		public skinString Clone() {
			return new skinString() {
				id = this.id,
				value = this.value
			};
		}
	}	
}
