using System;
using System.Linq;
using System.Xml.Serialization;
using System.Xml;
using System.Xml.Schema;

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
public class skinDialog
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
			cookie  = this.cookie,
			name    = this.name
		};
	}
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
