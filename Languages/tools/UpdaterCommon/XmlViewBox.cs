using System;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using System.Xml.Linq;

namespace Updater.Common
{
	public partial class XmlViewBox : RichTextBox
	{
		public XmlViewBox() {
			this.MouseWheel += new MouseEventHandler(OnMouseWheel);
			this.LanguageOption = RichTextBoxLanguageOptions.UIFonts;
			this.DetectUrls = false; // Mono wants it in the constructor
		}

		bool clearing = false;
		protected override void OnTextChanged(EventArgs args) {
			if (clearing) return;
			if (String.IsNullOrEmpty(this.Text)) {
				clearing = true;
				// Clear formatting (fonts, colors etc.) which is not cleared by default
				this.Clear();
				clearing = false;
			}
			base.OnTextChanged(args);
			
			this.ZoomFactor = zoomFactor;
			if (zoomFactor != this.ZoomFactor) {
				if (growing && zoomFactor < this.ZoomFactor ||
					!growing && zoomFactor > this.ZoomFactor)
					zoomFactor = this.ZoomFactor; // save not immediate ZoomFactor change
				else {
					this.ZoomFactor = zoomFactor; // restore ZoomFactor when box is emptied and reset to 1.0
					zoomFactor = this.ZoomFactor;
				}
			}
		}

		[DebuggerBrowsable(DebuggerBrowsableState.Never)]
		[DebuggerHidden()]
		public string Xml {
			get {
				if (String.IsNullOrEmpty(this.Text))
					return String.Empty;
				try {
					_xmlError = String.Empty;
					XDocument doc = XDocument.Parse(this.Text);
					return doc.ToString();
				} catch (Exception ex) {
					if (ex.InnerException != null)
						_xmlError = ex.InnerException.Message;
					else
						_xmlError = ex.Message;
					return String.Empty;
				}
			}
		}
		
		string _xmlError = String.Empty;
		public string XmlError {
			get { return _xmlError; }
		}

		public override Font Font {
			get { return base.Font; }
			set {
				if (value != null && _cultureInfo != null && value.GdiCharSet != base.Font.GdiCharSet) {
					Font oldFont = base.Font;
					base.Font = new Font(value.FontFamily, value.SizeInPoints, value.Style,
										 GraphicsUnit.Point, value.GdiCharSet);
					oldFont.Dispose();
					return;
				} else {
					base.Font = value;
				}
			}
		}

		bool ctrlPressed = false;

		protected override void OnKeyDown(KeyEventArgs args) {
			if ((args.Modifiers & Keys.Control) != 0)
				ctrlPressed = true;
			if (this.ReadOnly && args.KeyCode == Keys.Delete) {
				args.Handled = true;
			}
			base.OnKeyDown(args);
		}

		protected override void OnKeyUp(KeyEventArgs args) {
			if ((args.Modifiers & Keys.Control) == 0)
				ctrlPressed = false;
			if (this.ReadOnly && args.KeyCode == Keys.Delete) {
				args.Handled = true;
			}
			base.OnKeyUp(args);
		}

		float zoomFactor = 1.0f;
		bool growing = false;
		
		void OnMouseWheel(object sender, MouseEventArgs args) {
			if (ctrlPressed) {
				int numberOfTextLinesToMove = args.Delta * SystemInformation.MouseWheelScrollLines / 120;
				int numberOfPixelsToMove = (int)(numberOfTextLinesToMove * this.Font.Size);
				if (numberOfPixelsToMove == 0)
					numberOfPixelsToMove = args.Delta > 0 ? 1 : -1;
				Rectangle rec = Screen.FromControl(this).Bounds;
				growing = numberOfPixelsToMove > 0;
				if (growing)
					this.ZoomFactor *= (float)(rec.Height - numberOfPixelsToMove) / rec.Height;
				else
					this.ZoomFactor *= (float)rec.Height / (rec.Height - numberOfPixelsToMove);
				zoomFactor = this.ZoomFactor; // backup the value, because stupid MS resets it to 1.0
			}
		}

		CultureInfo _cultureInfo;

		public void SetLanguage(string langCode, string langName) {
			if (langCode == null) langCode = String.Empty;
			if (langName == null) langName = String.Empty;

			CultureInfo cultureInfo = null;
			try {
				cultureInfo = CultureInfo.CreateSpecificCulture(String.Format("{0}-{1}", langCode, langCode));
				if (cultureInfo.IsNeutralCulture)
					cultureInfo = null;
			} catch {
				var cultures = CultureInfo.GetCultures(CultureTypes.SpecificCultures);
				cultureInfo = (from c in cultures
							   let subLanguage = c.Name.Substring(c.Name.Length - 2, 2)
							   where c.NativeName == langName ||
									 String.Compare(subLanguage, langCode, true) == 0 ||
									 String.Compare(c.Name, langCode, true) == 0
							   select c).FirstOrDefault();
			}

			if (_cultureInfo == null || cultureInfo == null || _cultureInfo.Name != cultureInfo.Name)
				_cultureInfo = cultureInfo;
			else return;

			int currCodepage = GetDefaultCodepage(Thread.CurrentThread.CurrentCulture);
			if (currCodepage == 0 || cultureInfo == null) {
				var languageSelection = new LanguageSelection();
				if (languageSelection.ShowDialog() == DialogResult.OK) {
					langCode = languageSelection.GetSelectedLangCode();
					cultureInfo = CultureInfo.CreateSpecificCulture(langCode);
				} else return;
			}
			
			int gdiCharSet = GetFontCharset(cultureInfo.TextInfo.ANSICodePage);
			
			this.Clear(); // Clear the content
			if (this.Rtf == null) // Sometimes happens, setting to empty, fills in default Rtf
				this.Rtf = String.Empty;
				
			this.Rtf = this.Rtf.Replace("ansicpg" + currCodepage.ToString(),
										"ansicpg" + cultureInfo.TextInfo.ANSICodePage.ToString());
			this.Rtf = this.Rtf.Replace("deflang" + Thread.CurrentThread.CurrentCulture.LCID.ToString(),
										"deflang" + cultureInfo.LCID.ToString());
			this.Rtf = this.Rtf.Replace("fcharset" + this.Font.GdiCharSet.ToString(),
										"fcharset" + gdiCharSet.ToString());
			this.Font = new Font(this.Font.FontFamily, this.Font.SizeInPoints, this.Font.Style,
								 GraphicsUnit.Point, (byte)gdiCharSet);
		}

		private int GetDefaultCodepage(CultureInfo ci) {
			if (ci.IsNeutralCulture)
				ci = null;
			return ci == null ? 0 : ci.TextInfo.ANSICodePage;
		}

		private int GetFontCharset(int codePage) {
			switch (codePage) {
				case 932:
				case 943:
					return 128;		// ShiftJIS
				case 1361:
					return 130;		// Johab
				case 949:
					return 129;		// Hangul
				case 950:
					return 134;		// GB2312
				case 936:
					return 136;		// Chinese BIG5
				case 1253:
					return 161;		// Greek
				case 1254:
					return 162;		// Turkish
				case 1258:
					return 163;		// Vietnamese
				case 1255:
					return 177;		// Hebrew
				case 1256:
					return 178;		// Arabic
				case 862:
					return 181;		// Hebrew user
				case 1257:
					return 186;		// Baltic
				case 1251:
					return 204;		// Russian
				case 874:
					return 222;		// Thai
				case 852:
					return 238;		// Eastern European
				default:
					// return 0;	// ANSI charset
					return 1;		// Default charset
			}
		}
	}
}
