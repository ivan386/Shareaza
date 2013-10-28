//
// ShareazaBook.cs
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
using System;
using System.Collections;
using System.Diagnostics;
using System.Globalization;
using System.Text;
using System.Threading;
using System.Xml;
using System.Xml.Serialization;

#endregion

namespace Schemas
{
	[XmlType(Namespace = "http://www.limewire.com/schemas/book.xsd")]
	[XmlRoot(Namespace = "http://www.limewire.com/schemas/book.xsd", IsNullable = false)]
	public class Books
	{
		[XmlElement("book")]
		public ShareazaBook[] books;
		public static readonly string URI = @"http://www.limewire.com/schemas/book.xsd";

		public Books() {
			books = (ShareazaBook[])Array.CreateInstance(typeof(ShareazaBook), 1);
		}
	}

	[XmlType(Namespace = "http://www.limewire.com/schemas/book.xsd")]
	public class ShareazaBook
	{
		public ShareazaBook() {}
		
		[XmlAttribute(Namespace = "")]
		public string title;

		[XmlAttribute(Namespace = "")]
		public string author;

		[XmlAttribute(Namespace = "")]
		public string publisher;

		[XmlAttribute(Namespace = "")]
		public string edition;

		[XmlAttribute(Namespace = "")]
		public string description;

		private RazaGenreType _genre;
		
		[XmlAttribute(AttributeName = "genre", Namespace = "")]
		public RazaGenreType Genre {
			get { return _genre; }
			set { _genre = value; }
		}

		[XmlAttribute(Namespace = "")]
		public BookType type;

		[XmlAttribute(Namespace = "")]
		public FormatType format;

		[XmlAttribute(Namespace = "")]
		public string subject;

		[XmlAttribute(Namespace = "")]
		public string keywords;

		private uint pages;
		private bool hasPages;

		[XmlAttribute("pages", Namespace = "")]
		public string PagesString {
			get {
				if (!hasPages) return null;
				return Pages.ToString(CultureInfo.InvariantCulture);
			}
			set {
				uint newValue;
				if (UInt32.TryParse(value, out newValue))
					this.Pages = newValue;
			}
		}

		[XmlIgnore()]
		public uint Pages {
			get { return this.pages; }
			set {
				this.pages = value;
				hasPages = true;
			}
		}

		[XmlAttribute(Namespace = "", DataType = "gYear")]
		public string year;

		[XmlAttribute(Namespace = "", DataType = "language")]
		public string language;

		private Int64 nISBN;
		private bool hasISBN;

		[XmlAttribute("ISBN", Namespace = "")]
		public string ISBNString {
			get {
				if (!hasISBN) return null;
				return nISBN.ToString();
			}
			set {
				Int64 newValue;
				if (Int64.TryParse(value, out newValue))
					this.ISBN = newValue;
			}
		}

		[XmlIgnore()]
		public Int64 ISBN {
			get { return this.nISBN; }
			set {
				this.nISBN = value;
				hasISBN = true;
			}
		}

		private decimal length;
		private bool hasLength;

		[XmlAttribute("length", Namespace = "")]
		public string LengthString {
			get {
				if (!hasLength) return null;
				return Length.ToString(CultureInfo.InvariantCulture);
			}
			set {
				decimal newValue;
				if (Decimal.TryParse(value, out newValue))
					this.Length = newValue;
			}
		}

		[XmlIgnore()]
		public decimal Length {
			get { return this.length; }
			set {
				this.length = value;
				hasLength = true;
			}
		}

		private decimal width;
		private bool hasWidth;

		[XmlAttribute("width", Namespace = "")]
		public string WidthString {
			get {
				if (!hasWidth) return null;
				return Width.ToString(CultureInfo.InvariantCulture);
			}
			set {
				decimal newValue;
				if (Decimal.TryParse(value, out newValue))
					this.Length = newValue;
			}
		}

		[XmlIgnore()]
		public decimal Width {
			get { return this.width; }
			set {
				this.width = value;
				hasWidth = true;
			}
		}

		private decimal height;
		private bool hasHeight;

		[XmlAttribute("height", Namespace = "")]
		public string HeightString {
			get {
				if (!hasHeight) return null;
				return Height.ToString(CultureInfo.InvariantCulture);
			}
			set {
				decimal newValue;
				if (Decimal.TryParse(value, out newValue))
					this.Height = newValue;
			}
		}

		[XmlIgnore()]
		public decimal Height {
			get { return this.height; }
			set {
				this.height = value;
				hasHeight = true;
			}
		}

		[XmlAttribute(Namespace = "")]
		public BackType back;

		[XmlAttribute(Namespace = "")]
		public string qualitynotes;

		[XmlAttribute(Namespace = "", DataType = "date")]
		public DateTime releaseDate;

		[XmlAttribute(Namespace = "")]
		public string distributer;

		[XmlAttribute(Namespace = "", DataType = "anyURI")]
		public string distributerLink;

		[XmlAttribute(Namespace = "")]
		public string releasegroup;

		[XmlAttribute(Namespace = "", DataType = "anyURI")]
		public string releasegroupLink;

		[XmlAttribute(Namespace = "", DataType = "anyURI")]
		public string link;

		public ShareazaBook(FictionBook fb) {
			Trace.Assert(fb != null);

			if (fb.description == null) return;

			CultureInfo ciDefault = Thread.CurrentThread.CurrentCulture;
			CultureInfo ciDoc = ciDefault;

			if (fb.description.titleinfo == null) {
				if (fb.description.custominfo != null) {
					// Title in english ???
					// custominfo and titleinfo should have a common interface then...
					// this.title = fb.description.custominfo...
				}
			} else {
				this.title = fb.description.titleinfo.booktitle.Value;
				this.author = GetAuthors(fb.description.titleinfo.author);
				try {
					string sLang = fb.description.titleinfo.lang;
					if (!String.IsNullOrEmpty(sLang)) {
						// returns neutral culture!!! Beware when formatting
						ciDoc = CultureInfo.GetCultureInfoByIetfLanguageTag(sLang);
						this.language = ciDoc.EnglishName;
						// guess it
						ciDoc = CultureInfo.CreateSpecificCulture(sLang + "-" + sLang);
					}
				} catch { }

				if (fb.description.titleinfo.genre != null && fb.description.titleinfo.genre.Length > 0)
				{
					this._genre = GenreMap.GetRazaGenre(fb.description.titleinfo.genre[0].Value);
				}
				// this.year = fb.description.titleinfo.date.value.Year;
				if (fb.description.titleinfo.keywords != null)
					this.keywords = fb.description.titleinfo.keywords.Value;
				
				this.description = GetDescription(fb.description.titleinfo.annotation);
			}

			if (fb.description.publishinfo != null) {
				this.publisher = fb.description.publishinfo.publisher.Value;
				this.year = fb.description.publishinfo.year;

				if (fb.description.publishinfo.isbn != null) {
					string sISBN = fb.description.publishinfo.isbn.Value;
					if (!String.IsNullOrEmpty(sISBN)) {
						Int64 isbn;
						if (Int64.TryParse(sISBN, out isbn)) {
							this.ISBN = isbn;
						} else {
							sISBN = sISBN.Replace(@"-", String.Empty);
							if (Int64.TryParse(sISBN, out isbn)) {
								this.ISBN = isbn;
							}
						}
					}
				}
			}

			if (fb.description.documentinfo != null) {
				this.distributer = GetAuthors(fb.description.documentinfo.author);
				this.releaseDate = GetDate(ciDoc, fb.description.documentinfo.date);
				this.edition = fb.description.documentinfo.version.ToString();
			}

			this.back = BackType.Digital;
			this.format = FormatType.FictionBook;
		}

		private string GetGenres(IEnumerable enu) {
			StringBuilder sb = new StringBuilder();
			foreach (FictionBookDescriptionTitleinfoGenre genre in enu) {
				string sGenre = GenreMap.GetRazaGenre(genre.Value).ToString();
				if (!String.IsNullOrEmpty(sGenre))
					sb.Append(sGenre + "; ");
			}
			return sb.ToString().TrimEnd(new char[] { ';', ' ' });
		}

		private string GetAuthors(IEnumerable enu) {
			StringBuilder sb = new StringBuilder();
			foreach (AuthorType author in enu) {
				string sAuthor = GetAuthor(author);
				if (!String.IsNullOrEmpty(sAuthor))
					sb.Append(sAuthor + "; ");
			}
			return sb.ToString().TrimEnd(new char[] { ';', ' ' });
		}

		private string GetAuthor(AuthorType author) {
			string[] names = new string[3];

			for (int i = 0; i < author.ItemsElementName.Length; i++) {
				// Should we check the lang field ?
				if (author.ItemsElementName[i] == AuthorDataChoice.firstname) {
					names[0] = (string)((TextFieldType)author.Items[i]).Value;
				} else if (author.ItemsElementName[i] == AuthorDataChoice.middlename) {
					names[1] = (string)((TextFieldType)author.Items[i]).Value;
				} else if (author.ItemsElementName[i] == AuthorDataChoice.lastname) {
					names[2] = (string)((TextFieldType)author.Items[i]).Value;
				}
			}

			return String.Join(" ", names).Replace("  ", " ");
		}

		private DateTime GetDate(CultureInfo doc, DateType date) {
			DateTime result = default(DateTime);
			if (date == null) return result;

			bool bFailed = false;
			
			// try to fallback to the document format if the language was missing
			if (String.IsNullOrEmpty(date.lang)) {
				if (doc != null && !doc.IsNeutralCulture) {
					try {
						result = XmlConvert.ToDateTime(date.Value,
							new string[] { doc.DateTimeFormat.ShortDatePattern, "yyyy-MM-dd" });
					} catch { bFailed = true; }
				}
			} else {
				doc = CultureInfo.CreateSpecificCulture(date.lang + "-" + date.lang);
				if (doc.IsNeutralCulture)
					bFailed = true;
				else {
					try {
						result = XmlConvert.ToDateTime(date.Value,
							new string[] { doc.DateTimeFormat.ShortDatePattern, "yyyy-MM-dd" });
					} catch { bFailed = true; }
				}
			}
			if (bFailed)
				return default(DateTime);

			return result;
		}
		
		private string GetDescription(AnnotationType annotations) {
			if (annotations == null || annotations.Items == null ||
				annotations.Items.Length == 0) return String.Empty;
			
			StringBuilder sb = new StringBuilder();
			
			for (int i = 0; i < annotations.Items.Length; i++) {
				ParaType paragraph = annotations.Items[i] as ParaType;
				if (paragraph != null) {
					sb.Append(String.Join(@" ", paragraph.Text));
					sb.Append(@" ");
				}
			}
			return sb.ToString();
		}

		[XmlType(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum RazaGenreType
		{
			[XmlEnum("Arts & Photograhy")]
			ArtsPhotograhy,
			[XmlEnum("Biographies & Memoirs")]
			BiographiesMemoirs,
			[XmlEnum("Business & Investing")]
			BusinessInvesting,
			Calendars,
			[XmlEnum("Children's Books")]
			ChildrensBooks,
			Comics,
			[XmlEnum("Computers & Internet")]
			ComputersInternet,
			[XmlEnum("Cooking, Food & Wine")]
			CookingFoodWine,
			Entertainment,
			[XmlEnum("Gay & Lesbian")]
			GayLesbian,
			[XmlEnum("Health & Fitness")]
			HealthFitness,
			History,
			[XmlEnum("Home & Garden")]
			HomeGarden,
			Horror,
			[XmlEnum("Literature & Fiction")]
			LiteratureFiction,
			[XmlEnum("Mind & Body")]
			MindBody,
			[XmlEnum("Mystery & Thrillers")]
			MysteryThrillers,
			Nonfiction,
			[XmlEnum("Outdoors & Nature")]
			OutdoorsNature,
			[XmlEnum("Parenting & Families")]
			ParentingFamilies,
			[XmlEnum("Professional & Technical")]
			ProfessionalTechnical,
			Reference,
			[XmlEnum("Religion & Spirituality")]
			ReligionSpirituality,
			Romance,
			Science,
			[XmlEnum("Science Fiction & Fantasy")]
			ScienceFictionFantasy,
			[XmlEnum("Sheet Music & Scores")]
			SheetMusicScores,
			Sports,
			Teens,
			Travel
		}

		[XmlType(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum BookType
		{
			Book,
			Magazine,
			Article
		}

		[XmlType(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum FormatType
		{
			[XmlEnum("Adobe Reader")]
			AdobeReader,
			[XmlEnum("Compiled HTML Help")]
			CompiledHTMLHelp,
			[XmlEnum("Fiction Book")]
			FictionBook,
			Image,
			[XmlEnum("Microsoft Reader")]
			MicrosoftReader,
			PDF,
			[XmlEnum("Pocket PDF")]
			PocketPDF,
			Text
		}

		[XmlType(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum BackType
		{
			Digital,
			Hardback,
			Paperback
		}
	}
}
