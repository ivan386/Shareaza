//
// FictionBook.cs
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

#region Using directives
using System.ComponentModel;
using System.Xml.Serialization;

#endregion

namespace Schemas
{
    [XmlType(Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0")]
    [XmlRoot(Namespace = "http://www.gribuser.ru/xml/fictionbook/2.0", IsNullable = false)]
	public class FictionBook
	{
        [XmlElement("description")]
		public FictionBookDescription description;
	}
	
	[XmlType()]
	public class FictionBookDescription : FbTitleInfo
	{
		/// <remarks>
		/// Information about this particular (xml) document
		///</remarks>
		[XmlElement("document-info")]
		public FictionBookDescriptionDocumentinfo documentinfo;
		
		/// <remarks>
		/// Information about some paper/other published document, that was used as a 
		/// source of this xml document
		///</remarks>
		[XmlElement("publish-info")]
		public FictionBookDescriptionPublishinfo publishinfo;
		
		/// <remarks>
		/// Any other information about the book/document that didn't fit in the above groups
		///</remarks>
		[XmlElement("custom-info")]
		public FictionBookDescriptionCustominfo[] custominfo;
	}
	
	[XmlType()]
	public class FictionBookDescriptionTitleinfo
	{
		/// <remarks>
		/// Genre of this book, with the optional match percentage
		///</remarks>
		[XmlElement()]
		public FictionBookDescriptionTitleinfoGenre[] genre;
		
		/// <remarks>
		/// Author(s) of this book
		///</remarks>
		[XmlElement()]
		public FictionBookDescriptionTitleinfoAuthor[] author;
		
		/// <remarks>
		/// Book title
		///</remarks>
		[XmlElement("book-title")]
		public TextFieldType booktitle;
		
		/// <remarks>
		/// Annotation for this book
		///</remarks>
        [XmlElement()]
		public AnnotationType annotation;
		
		/// <remarks>
		/// Any keywords for this book, intended for use in search engines
		///</remarks>
        [XmlElement()]
		public TextFieldType keywords;
		
		/// <remarks>
		/// Date this book was written, can be not exact, e.g. 1863-1867. 
		/// If an optional attribute is present, then it should contain some computer-readable 
		/// date from the interval for use by search and indexing engines
		///</remarks>
        [XmlElement()]
		public DateType date;
		
		[XmlArrayItem(ElementName="image", IsNullable=false)]
		public ImageType[] coverpage;
		
		[XmlElement(DataType="language")]
		public string lang;
		
		/// <remarks>
		/// Book's source language if this is a translation
		///</remarks>
		[XmlElement("src-lang", DataType="language")]
		public string srclang;
		
		/// <remarks>
		/// Translators if this is a translation
		///</remarks>
		[XmlElement()]
		public AuthorType[] translator;
		
		/// <remarks>
		/// Any sequences this book might be part of
		///</remarks>
		[XmlElement()]
		public SequenceType[] sequence;
	}
	
	[XmlType()]
	public class FictionBookDescriptionTitleinfoGenre
	{
		[DefaultValue("100")]
		[XmlAttribute(Namespace="", DataType="integer")]
		public string match = "100";

		[XmlText()]
		public genreType Value;
	}
	
	[XmlType()]
	public enum genreType
	{
		// SF, Fantasy
		sf_history,			// Alternative history
		sf_action,
		sf_epic,
		sf_heroic,
		sf_detective,
		sf_cyberpunk,
		sf_space,
		sf_social,
		sf_horror,			// Horror & Mystic
		sf_humor,
		sf_fantasy,
		sf,					// Science Fiction
		// Detectives, Thrillers
		det_classic,		// Classical Detective
		det_police,			// Police Stories
		det_action,
		det_irony,			// Ironical Detective
		det_history,		// Historical Detective
		det_espionage,
		det_crime,
		det_political,
		det_maniac,			// Maniacs
		det_hard,			// Hard-boiled Detective
		thriller,
		detective,
		// Prose
		prose_classic,
		prose_history,
		prose_contemporary,
		prose_counter,		// Counterculture
		prose_rus_classic,	// Russian Classics
		prose_su_classics,	// Soviet Classics
		// Romance
		love_contemporary,
		love_history,
		love_detective,
		love_short,
		love_erotica,
		// Adventure
		adv_western,
		adv_history,
		adv_indian,
		adv_maritime,
		adv_geo,			// Travel & Geography
		adv_animal,			// Nature & Animals
		adventure,
		// Children's
		child_tale,			// Fairy Tales
		child_verse,		// Verses
		child_prose,		// Prose for Kids
		child_sf,			// Science Fiction for Kids
		child_det,			// Detectives & Thrillers
		child_adv,			// Adventures for Kids
		child_education,	// Education for Kids
		children,			// For Kids: Miscellanious
		poetry,
		dramaturgy,
		antique_ant,
		antique_european,
		antique_russian,
		antique_east,
		antique_myths,
		antique,
		sci_history,
		sci_psychology,
		sci_culture,
		sci_religion,
		sci_philosophy,
		sci_politics,
		sci_business,
		sci_juris,
		sci_linguistic,
		sci_medicine,
		sci_phys,
		sci_math,
		sci_chem,
		sci_biology,
		sci_tech,
		science,
		comp_www,
		comp_programming,
		comp_hard,
		comp_soft,
		comp_db,
		comp_osnet,
		computers,
		ref_encyc,
		ref_dict,
		ref_ref,
		ref_guide,
		reference,
		nonf_biography,
		nonf_publicism,
		nonf_criticism,
		nonfiction,
		design,
		religion_rel,
		religion_esoterics,
		religion_self,
		religion,
		humor_anecdote,
		humor_prose,
		humor_verse,
		humor,
		home_cooking,
		home_pets,
		home_crafts,
		home_entertain,
		home_health,
		home_garden,
		home_diy,
		home_sport,
		home_sex,
		home
	}
	
	[XmlType()]
	public class FictionBookDescriptionTitleinfoAuthor : AuthorType
	{
	}
	
	/// <remarks>
	/// Information about a single author
	///</remarks>
	[XmlType()]
	[XmlInclude(typeof(FictionBookDescriptionTitleinfoAuthor))]
	public class AuthorType
	{
		[XmlIgnore()]
        public AuthorDataChoice[] ItemsElementName;

		[XmlElement("first-name", Type=typeof(TextFieldType))]
		[XmlElement("middle-name", Type=typeof(TextFieldType))]
		[XmlElement("last-name", Type=typeof(TextFieldType))]
		[XmlElement("nickname", Type=typeof(TextFieldType))]
		[XmlElement("home-page", Type=typeof(string))]
		[XmlElement("email", Type=typeof(string))]
		[XmlChoiceIdentifier("ItemsElementName")]
		public object[] Items;
	}

	[XmlType()]
	[XmlInclude(typeof(FictionBookDescriptionCustominfo))]
	public class TextFieldType
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlText()]
		public string Value;
	}
	
	[XmlType()]
	public class FictionBookDescriptionCustominfo : TextFieldType
	{
		[XmlAttribute("info-type", Namespace="")]
		public string infotype;
	}
	
	/// <remarks>
	/// A cut-down version of <section> used in annotations
	///</remarks>
	[XmlType()]
	public class AnnotationType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlElement("p", Type=typeof(ParaType))]
		[XmlElement("poem", Type=typeof(PoemType))]
		[XmlElement("cite", Type=typeof(CiteType))]
		[XmlElement("empty-line")]
		public object[] Items;
	}
	
	/// <remarks>
	/// A basic paragraph, may include simple formatting inside
	///</remarks>
	[XmlType()]
	public class ParaType : StyleType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="")]
		public string style;
	}
	
	/// <remarks>
	/// Markup
	///</remarks>
	[XmlType()]
	[XmlInclude(typeof(ParaType))]
	public class StyleType
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;
		
		[XmlIgnore()]
		public StyleChoice2[] ItemsElementName;
		
		[XmlElement("strong", Type=typeof(StyleType))]
		[XmlElement("emphasis", Type=typeof(StyleType))]
		[XmlElement("style", Type=typeof(NamedStyleType))]
		[XmlElement("a", Type=typeof(LinkType))]
		[XmlElement("image", Type=typeof(ImageType))]
		[XmlChoiceIdentifier("ItemsElementName")]
		public object[] Items;
		
		[XmlText()]
		public string[] Text;
	}
	
	/// <remarks>
	/// Markup
	///</remarks>
	[XmlType()]
    [XmlInclude(typeof(StyleType))]
    public class NamedStyleType : StyleType
	{
		[XmlAttribute(Namespace="", DataType="token")]
		public string name;
	}

	/// <remarks>
	/// Generic hyperlinks. Cannot be nested. Footnotes should be implemented by links referring to
	/// additional bodies in the same document
	///</remarks>
	[XmlType()]
	public class LinkType
	{
		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string type;

		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string href;

		[XmlAttribute("type", Namespace="", DataType="token")]
		public string type1;

		[XmlIgnore()]
        public StyleChoice1[] ItemsElementName;

		[XmlElement("strong")]
		[XmlElement("emphasis")]
		[XmlElement("style")]
		[XmlChoiceIdentifier("ItemsElementName")]
		public StyleLinkType[] Items;

		[XmlText()]
		public string[] Text;
	}

    [XmlType("StyleChoice1", IncludeInSchema = false)]
	public enum StyleChoice1
	{
		strong,
		emphasis,
		style
	}

    [XmlType("StyleChoice2", IncludeInSchema = false)]
    public enum StyleChoice2
    {
        strong,
        emphasis,
        style,
        a,
        image
    }

    [XmlType("AuthorDataChoice", IncludeInSchema = false)]
    public enum AuthorDataChoice
    {
        [XmlEnum("first-name")]
        firstname,
        [XmlEnum("middle-name")]
        middlename,
        [XmlEnum("last-name")]
        lastname,
        nickname,
        [XmlEnum("home-page")]
        homepage,
        email
    }		
	/// <remarks>
	/// Markup
	///</remarks>
	[XmlType()]
	public class StyleLinkType
	{
		[XmlIgnore()]
		public ItemsChoiceType[] ItemsElementName;

		[XmlElement("strong")]
		[XmlElement("emphasis")]
		[XmlElement("style")]
		[XmlChoiceIdentifier("ItemsElementName")]
		public StyleLinkType[] Items;

		[XmlText()]
		public string[] Text;
	}

	[XmlType(IncludeInSchema=false)]
	public enum ItemsChoiceType
	{
		strong,
		emphasis,
		style
	}
	
	/// <remarks>
	/// An empty element with an image name as an attribute
	///</remarks>
	[XmlType()]
	public class ImageType
	{
		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string type = string.Empty;

		[XmlAttribute(Namespace="http://www.w3.org/1999/xlink")]
		public string href = string.Empty;

		[XmlAttribute(Namespace="")]
		public string alt = string.Empty;
	}
	
	/// <remarks>
	/// A poem
	///</remarks>
	[XmlType()]
	public class PoemType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;
		
		/// <remarks>
		/// Poem title
		///</remarks>
		public TitleType title;
		
		/// <remarks>
		/// Poem epigraph(s), if any
		///</remarks>
		[XmlElement()]
		public EpigraphType[] epigraph;
		
		/// <remarks>
		/// Each poem should have at least one stanza. Stanzas are usually separated with 
		/// empty lines by user agents.
		///</remarks>
		[XmlElement()]
		public PoemTypeStanza[] stanza;
		
		[XmlElement("text-author")]
		public TextFieldType[] textauthor;
		
		/// <remarks>
		/// Date this poem was written.
		///</remarks>
		public DateType date;
	}
	
	/// <remarks>
	/// A title, used in sections, poems and body elements
	///</remarks>
	[XmlType()]
	public class TitleType
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

        [XmlElement("p", Type = typeof(ParaType))]
        [XmlElement("empty-line")]
        public object[] Items;
	}
	
	/// <remarks>
	/// An epigraph
	///</remarks>
	[XmlType()]
	public class EpigraphType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlElement("p", Type=typeof(ParaType))]
		[XmlElement("poem", Type=typeof(PoemType))]
		[XmlElement("cite", Type=typeof(CiteType))]
		[XmlElement("empty-line")]
		public object[] Items;

		[XmlElement("text-author")]
		public TextFieldType[] textauthor;
	}
	
	/// <remarks>
	/// A citation with an optional citation author at the end
	///</remarks>
	[XmlType()]
	public class CiteType
	{
		[XmlAttribute(Namespace="", DataType="ID")]
		public string id;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlElement("p", Type=typeof(ParaType))]
		[XmlElement("poem", Type=typeof(PoemType))]
		[XmlElement("empty-line")]
		public object[] Items;

		[XmlElement("text-author")]
		public TextFieldType[] textauthor;
	}

	[XmlType()]
	public class FictionBookBinary
	{
		[XmlAttribute("content-type", Namespace="")]
		public string ContentType;

		[XmlAttribute("id", Namespace="", DataType="ID")]
		public string Id;

		[XmlText(DataType = "base64Binary")]
		public byte[] Value;
	}

	[XmlType()]
	public class FictionBookDescriptionDocumentinfo
	{
		/// <remarks>
		/// Author(s) of this particular document
		///</remarks>
		[XmlElement()]
		public AuthorType[] author;
		
		/// <remarks>
		/// Any software used in preparation of this document, in free format
		///</remarks>
		[XmlElement("program-used")]
		public TextFieldType programused;
		
		/// <remarks>
		/// Date this document was created, same guidelines as in the <title-info> section apply
		///</remarks>
		public DateType date;
		
		/// <remarks>
		/// Source URL if this document is a conversion of some other (online) document
		///</remarks>
		[XmlElement("src-url")]
		public string[] srcurl;
		
		/// <remarks>
		/// Author of the original (online) document, if this is a conversion
		///</remarks>
		[XmlElement("src-ocr")]
		public TextFieldType srcocr;
		
		/// <remarks>
		/// This is a unique identifier for a document. this must not change unless you
		/// make substantial updates to the document
		///</remarks>
		[XmlElement(DataType="token")]
		public string id;
		
		/// <remarks>
		/// Document version, in free format, should be incremented if the document is changed
		/// and re-released to the public
		///</remarks>
		public System.Single version;

		public AnnotationType history;
	}
	
	/// <remarks>
	///A human readable date, maybe not exact, with an optional computer readable variant
	///</remarks>
	[XmlType()]
	public class DateType
	{
		[XmlAttribute(Namespace="", DataType="date")]
		public System.DateTime value;

		[XmlIgnore()]
		public bool valueSpecified;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlText()]
		public string Value;
	}

	[XmlType()]
	public class FictionBookDescriptionPublishinfo
	{
		/// <remarks>
		/// Original (paper) book name
		///</remarks>
		[XmlElement("book-name")]
		public TextFieldType bookname;
		
		/// <remarks>
		/// Original (paper) book publisher
		///</remarks>
		public TextFieldType publisher;
		
		/// <remarks>
		/// City where the original (paper) book was published
		///</remarks>
		public TextFieldType city;
		
		/// <remarks>
		/// Year of the original (paper) publication
		///</remarks>
		[XmlElement(DataType="gYear")]
		public string year;

		public TextFieldType isbn;

		[XmlElement()]
		public SequenceType[] sequence;
	}
	
	/// <remarks>
	/// Book sequences
	///</remarks>
    [XmlType()]
	public class SequenceType
	{
		[XmlAttribute()]
		public string name;

		[XmlAttribute(DataType="integer")]
		public string number;

		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;

		[XmlElement()]
		public SequenceType[] sequence;
	}

	[XmlType()]
	public class PoemTypeStanza
	{
		[XmlAttribute(Namespace="http://www.w3.org/XML/1998/namespace")]
		public string lang;
		public TitleType title;
		public ParaType subtitle;
		
		/// <remarks>
		/// An individual line in a stanza
		///</remarks>
		[XmlElement()]
		public ParaType[] v;
	}
}
