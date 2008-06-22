using System;
using System.Collections;
using System.Diagnostics;
using System.Globalization;
using System.Text;
using System.Threading;
using System.Xml;
using System.Xml.Serialization;

namespace Schemas
{
	[XmlTypeAttribute(Namespace = "http://www.limewire.com/schemas/book.xsd")]
	[XmlRootAttribute(Namespace = "http://www.limewire.com/schemas/book.xsd", IsNullable = false)]
	public class Books
	{
		[XmlElementAttribute("book")]
		public ShareazaBook[] books;
		public static readonly string URI = @"http://www.limewire.com/schemas/book.xsd";

		public Books() {
			books = (ShareazaBook[])Array.CreateInstance(typeof(ShareazaBook), 1);
		}
	}

	[XmlTypeAttribute(Namespace = "http://www.limewire.com/schemas/book.xsd")]
	public class ShareazaBook
	{
		[XmlAttributeAttribute(Namespace = "")]
		public string title;

		[XmlAttributeAttribute(Namespace = "")]
		public string author;

		[XmlAttributeAttribute(Namespace = "")]
		public string publisher;

		[XmlAttributeAttribute(Namespace = "")]
		public string edition;

		[XmlAttributeAttribute(Namespace = "")]
		public string description;

		[XmlAttributeAttribute(Namespace = "")]
		public RazaGenreType genre;

		[XmlAttributeAttribute(Namespace = "")]
		public BookType type;

		[XmlAttributeAttribute(Namespace = "")]
		public FormatType format;

		[XmlAttributeAttribute(Namespace = "")]
		public string subject;

		[XmlAttributeAttribute(Namespace = "")]
		public string keywords;

		private uint pages;
		private bool hasPages;

		[XmlAttributeAttribute("pages", Namespace = "")]
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

		[XmlAttributeAttribute(Namespace = "", DataType = "gYear")]
		public string year;

		[XmlAttributeAttribute(Namespace = "", DataType = "language")]
		public string language;

		private Int64 nISBN;
		private bool hasISBN;

		[XmlAttributeAttribute("ISBN", Namespace = "")]
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

		[XmlAttributeAttribute("length", Namespace = "")]
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

		[XmlAttributeAttribute("width", Namespace = "")]
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

		[XmlAttributeAttribute("height", Namespace = "")]
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

		[XmlAttributeAttribute(Namespace = "")]
		public BackType back;

		[XmlAttributeAttribute(Namespace = "")]
		public string qualitynotes;

		[XmlAttributeAttribute(Namespace = "", DataType = "date")]
		public DateTime releaseDate;

		[XmlAttributeAttribute(Namespace = "")]
		public string distributer;

		[XmlAttributeAttribute(Namespace = "", DataType = "anyURI")]
		public string distributerLink;

		[XmlAttributeAttribute(Namespace = "")]
		public string releasegroup;

		[XmlAttributeAttribute(Namespace = "", DataType = "anyURI")]
		public string releasegroupLink;

		[XmlAttributeAttribute(Namespace = "", DataType = "anyURI")]
		public string link;

		public ShareazaBook() {
		}

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

				// this.year = fb.description.titleinfo.date.value.Year;
				if (fb.description.titleinfo.keywords != null)
					this.keywords = fb.description.titleinfo.keywords.Value;
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

		[XmlTypeAttribute(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum RazaGenreType
		{
			[XmlEnumAttribute("Arts & Photograhy")]
			ArtsPhotograhy,
			[XmlEnumAttribute("Biographies & Memoirs")]
			BiographiesMemoirs,
			[XmlEnumAttribute("Business & Investing")]
			BusinessInvesting,
			Calendars,
			[XmlEnumAttribute("Children's Books")]
			ChildrensBooks,
			Comics,
			[XmlEnumAttribute("Computers & Internet")]
			ComputersInternet,
			[XmlEnumAttribute("Cooking, Food & Wine")]
			CookingFoodWine,
			Entertainment,
			[XmlEnumAttribute("Gay & Lesbian")]
			GayLesbian,
			[XmlEnumAttribute("Health & Fitness")]
			HealthFitness,
			History,
			[XmlEnumAttribute("Home & Garden")]
			HomeGarden,
			Horror,
			[XmlEnumAttribute("Literature & Fiction")]
			LiteratureFiction,
			[XmlEnumAttribute("Mind & Body")]
			MindBody,
			[XmlEnumAttribute("Mystery & Thrillers")]
			MysteryThrillers,
			Nonfiction,
			[XmlEnumAttribute("Outdoors & Nature")]
			OutdoorsNature,
			[XmlEnumAttribute("Parenting & Families")]
			ParentingFamilies,
			[XmlEnumAttribute("Professional & Technical")]
			ProfessionalTechnical,
			Reference,
			[XmlEnumAttribute("Religion & Spirituality")]
			ReligionSpirituality,
			Romance,
			Science,
			[XmlEnumAttribute("Science Fiction & Fantasy")]
			ScienceFictionFantasy,
			[XmlEnumAttribute("Sheet Music & Scores")]
			SheetMusicScores,
			Sports,
			Teens,
			Travel
		}

		[XmlTypeAttribute(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum BookType
		{
			Book,
			Magazine,
			Article
		}

		[XmlTypeAttribute(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum FormatType
		{
			[XmlEnumAttribute("Adobe Reader")]
			AdobeReader,
			[XmlEnumAttribute("Compiled HTML Help")]
			CompiledHTMLHelp,
			[XmlEnumAttribute("Fiction Book")]
			FictionBook,
			Image,
			[XmlEnumAttribute("Microsoft Reader")]
			MicrosoftReader,
			PDF,
			[XmlEnumAttribute("Pocket PDF")]
			PocketPDF,
			Text
		}

		[XmlTypeAttribute(Namespace = "http://www.limewire.com/schemas/book.xsd")]
		public enum BackType
		{
			Digital,
			Hardback,
			Paperback
		}
	}
}
