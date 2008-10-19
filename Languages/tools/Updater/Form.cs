using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using ShareazaDialogUpdater.Properties;
using System.Globalization;
using System.Threading;
using System.Diagnostics;

namespace ShareazaDialogUpdater
{
	public partial class form : Form
	{
		float vertPerc;
		float horizPerc;
		List<skinDialog> oldEnList, newEnList, translation;

		#region Initialization

		public form() {
			InitializeComponent();

			this.splitVertical.SplitterDistance = this.tableBottomAll.Width / 2 - this.tableBottomAll.Margin.Left;
			this.splitHorizontal.SplitterDistance = this.tableBottomAll.Height / 2 - this.tableBottomAll.Margin.Top;
			vertPerc = (float)this.splitVertical.SplitterDistance / this.tableBottomAll.Width;
			horizPerc = (float)this.splitHorizontal.SplitterDistance / this.tableBottomAll.Height;
			deleteItem.Enabled = false; // Don't send Delete keys

			richEnOld.MouseWheel += new MouseEventHandler(OnMouseWheel);
			richEnNew.MouseWheel += new MouseEventHandler(OnMouseWheel);
			richTranslation.MouseWheel += new MouseEventHandler(OnMouseWheel);

			// Turn off the crappy autofont detection
			richTranslation.LanguageOption = RichTextBoxLanguageOptions.UIFonts;

			richEnOld.DetectUrls = false;
			richEnNew.DetectUrls = false;
			richTranslation.DetectUrls = false;
		}

		#endregion

		#region Sizing

		private void form_ResizeBegin(object sender, EventArgs e) {
			vertPerc = (float)this.splitVertical.SplitterDistance / this.tableBottomAll.Width;
			horizPerc = (float)this.splitHorizontal.SplitterDistance / this.tableBottomAll.Height;
		}

		private void form_Resize(object sender, EventArgs e) {
			if (this.tableBottomAll.Width == 0 || this.tableBottomAll.Height == 0)
				return;
			this.splitVertical.SplitterDistance = (int)(this.tableBottomAll.Width * vertPerc);
			this.splitHorizontal.SplitterDistance = (int)(this.tableBottomAll.Height * horizPerc);
		}

		void OnMouseWheel(object sender, MouseEventArgs args) {
			if (ctrlPressed) {
				RichTextBox box = (RichTextBox)sender;
				int numberOfTextLinesToMove = args.Delta * SystemInformation.MouseWheelScrollLines / 120;
				int numberOfPixelsToMove = (int)(numberOfTextLinesToMove * box.Font.Size);
				if (numberOfPixelsToMove == 0)
					numberOfPixelsToMove = args.Delta > 0 ? 1 : -1;
				Rectangle rec = Screen.FromControl(box).Bounds;
				if (numberOfPixelsToMove > 0)
					box.ZoomFactor *= (float)(rec.Height - numberOfPixelsToMove) / rec.Height;
				else
					box.ZoomFactor *= (float)rec.Height / (rec.Height - numberOfPixelsToMove);
				box.Tag = box.ZoomFactor; // backup the value, because stupid MS resets it to 1.0
			}
		}

		#endregion

		#region Button Events

		private void btnEnOld_Click(object sender, EventArgs e) {
			skinManifest manifest;
			oldEnList = GetDialogList(txtEnOld, out manifest);
			if (oldEnList != null && manifest.lang != "en") {
				MessageBox.Show(Settings.Default.Error_NotEnglish, "Information", MessageBoxButtons.OK,
								MessageBoxIcon.Information);
				oldEnList = null;
				txtEnOld.Text = String.Empty;
			}
			if (oldEnList != null)
				UpdateBoxes();
		}

		private void btnEnNew_Click(object sender, EventArgs e) {
			skinManifest manifest;
			newEnList = GetDialogList(txtEnNew, out manifest);
			if (oldEnList != null && manifest.lang != "en") {
				MessageBox.Show(Settings.Default.Error_NotEnglish, "Information", MessageBoxButtons.OK,
								MessageBoxIcon.Information);
				newEnList = null;
				txtEnNew.Text = String.Empty;
			}
			if (newEnList != null) {
				cmbDialogs.Items.AddRange(newEnList.Select(s => s.name).ToArray());
				cmbDialogs.SelectedIndex = 0;
			}
		}

		private void btnDoWork_Click(object sender, EventArgs e) {
			skinManifest manifest;
			translation = GetDialogList(null, out manifest);
			if (manifest != null)
				UpdateRtfLanguage(richTranslation, manifest);
			if (translation != null) 
				UpdateBoxes();
		}

		#endregion

		private void cmbDialogs_SelectedIndexChanged(object sender, EventArgs e) {
			UpdateBoxes();
		}

		void UpdateBoxes() {
			if (newEnList == null)
				return;
			var match = newEnList.Where(s => s.name == (string)cmbDialogs.Items[cmbDialogs.SelectedIndex]);
			skinDialog newDialog = match.First();

			if (oldEnList != null) {
				skinDialog oldEn = oldEnList.FirstOrDefault(dialog => dialog.name == newDialog.name);
				richEnOld.Text = GetXml(oldEn);
				// Fix the resetting of ZoomFactor, when the text is empty
				if (richEnOld.Tag != null)
					richEnOld.ZoomFactor = (float)richEnOld.Tag;
			}
			if (newEnList != null) {
				skinDialog newEn = newEnList.FirstOrDefault(dialog => dialog.name == newDialog.name);
				richEnNew.Text = GetXml(newEn);
				if (richEnNew.Tag != null)
					richEnNew.ZoomFactor = (float)richEnNew.Tag;
			}
			if (translation != null) {
				skinDialog tr;
				UpdateDialogTranslation(newDialog.name, out tr);
				richTranslation.Text = GetXml(tr);
				if (richTranslation.Tag != null) {
					float oldValue = (float)richTranslation.Tag;
					richTranslation.ZoomFactor = oldValue;
					if (oldValue != richTranslation.ZoomFactor) { // wtf happens here ????
						richTranslation.ZoomFactor = oldValue;
						Trace.WriteLine(String.Format("Old value: {0}, new value: {1}",
										(float)richTranslation.Tag, richTranslation.ZoomFactor));
					}
					richTranslation.Tag = richTranslation.ZoomFactor;
				}
			}
		}
		
		void UpdateDialogTranslation(string dialogName, out skinDialog dialog) {
			// Always not null
			skinDialog newEn = newEnList.FirstOrDefault(d => d.name == dialogName);
			// Can be null
			skinDialog tr = translation.FirstOrDefault(d => d.name == dialogName);
			if (tr == null) {
				translation.Add(newEn);
				dialog = newEn;
				return;
			} else {
				skinDialog oldEn = oldEnList.FirstOrDefault(d => d.name == dialogName);
				if (oldEn == null || oldEn.control.Length != tr.control.Length) {
					MessageBox.Show("The old english file doesn't match your translation file!");
					dialog = tr;
					return;
				}
				skinDialog updatedDialog = newEn.Clone();
				// Update caption
				if (newEn.caption != oldEn.caption)
					updatedDialog.caption = newEn.caption;
				else
					updatedDialog.caption = tr.caption;
				// Same controls, no translation updates are needed
				var sameControls = from cNew in newEn.control
								   join cOld in oldEn.control on cNew.caption equals cOld.caption
								   select new { 
									  NewControl = cNew,
									  OldIndex = Array.IndexOf(oldEn.control, cOld),
									  NewIndex = Array.IndexOf(newEn.control, cNew)
								   };
				for (int i = 0; i < newEn.control.Length; i++) {
					var matched = sameControls.Where(s => s.NewControl.Equals(newEn.control[i]))
											  .FirstOrDefault();
					if (matched != null) {
						updatedDialog.control[matched.NewIndex] = tr.control[matched.OldIndex];
					}
				}
				dialog = updatedDialog;
			}
		}

		string GetXml(skinDialog dialog) {
			if (dialog == null) return String.Empty;
			MemoryStream stream;
			string xml = String.Empty;

			if (XmlSerializerBase<skinDialog>.Write(dialog, out stream)) {
				byte[] bytes = stream.ToArray();
				try {
					xml = new String((new UTF8Encoding()).GetChars(stream.ToArray()));
					xml = xml.Replace(" xmlns=\"http://www.shareaza.com/schemas/Skin.xsd\"", String.Empty);
				} catch { }
				stream.Close();
			}
			return xml;
		}

		List<skinDialog> GetDialogList(TextBox pathToSet, out skinManifest manifest) {
			openFileDialog.FileName = String.Empty;
			openFileDialog.ShowDialog();
			if (pathToSet != null)
				pathToSet.Text = openFileDialog.FileName;
			manifest = null;
			if (!String.IsNullOrEmpty(openFileDialog.FileName)) {
				var skin = XmlSerializerBase<skin>.Read(openFileDialog.FileName);
				if (skin != null && skin.dialogs != null && skin.dialogs.Length > 0) {
					manifest = skin.manifest;
					return skin.dialogs.OrderBy(s => s.name).ToList();
				}
			}
			return null;
		}

		private void richTranslation_TextChanged(object sender, EventArgs e) {

		}

		#region Context Menu

		private void contextMenu_Opening(object sender, CancelEventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			box.Focus();
			IDataObject iData = Clipboard.GetDataObject();
			if (!box.ReadOnly && iData.GetDataPresent(DataFormats.Text)) {
				string text = (string)iData.GetData(DataFormats.Text);
				pasteItem.Enabled = !String.IsNullOrEmpty(text);
			} else {
				pasteItem.Enabled = false;
			}
			if (box.SelectionLength == 0) {
				cutItem.Enabled = false;
				copyItem.Enabled = false;
				selectAllItem.Enabled = box.Text.Length > 0;
				deleteItem.Enabled = false;
			} else {
				cutItem.Enabled = true;
				copyItem.Enabled = true;
				selectAllItem.Enabled = box.Text.Length != box.SelectionLength;
				deleteItem.Enabled = true;
			}
			undoItem.Enabled = box.CanUndo;
			redoItem.Enabled = box.CanRedo;
		}

		private void cutItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			box.Cut();
		}

		private void copyItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			box.Copy();
		}

		private void pasteItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			IDataObject iData = Clipboard.GetDataObject();
			if (iData.GetDataPresent(DataFormats.Text))
				box.Paste();
		}

		private void deleteItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			if (box == null)
				box = richTranslation; // No other should send this event
			IDataObject iData = Clipboard.GetDataObject();
			if (box != null && box.SelectionLength > 0) {
				if (iData.GetDataPresent(DataFormats.Text)) {
					object oldContent = iData.GetData(DataFormats.Text);
					box.Cut();
					iData.SetData(oldContent);
				} else {
					box.Cut();
				}
			}
			deleteItem.Enabled = false;
		}

		private void selectAllItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			if (box.CanSelect)
				box.SelectAll();
		}

		private void undoItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			if (box.CanUndo)
				box.Undo();
		}

		private void redoItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)contextMenu.SourceControl;
			if (box.CanRedo)
				box.Redo();
		}

		private void contextMenu_Closing(object sender, ToolStripDropDownClosingEventArgs e) {
			cutItem.Enabled = false;
			copyItem.Enabled = false;
			pasteItem.Enabled = false;
			selectAllItem.Enabled = false;
			deleteItem.Enabled = false;
			undoItem.Enabled = false;
			redoItem.Enabled = false;
		}

		#endregion

		bool ctrlPressed = false;

		private void form_KeyDown(object sender, KeyEventArgs e) {
			if ((e.Modifiers & Keys.Control) != 0)
				ctrlPressed = true;
		}

		private void form_KeyUp(object sender, KeyEventArgs e) {
			if ((e.Modifiers & Keys.Control) == 0)
				ctrlPressed = false;
		}

		private void UpdateRtfLanguage(RichTextBox richBox, skinManifest manifest) {
			CultureInfo cultureInfo = null;
			try {
				cultureInfo = CultureInfo.CreateSpecificCulture(String.Format("{0}-{1}", manifest.lang, manifest.lang));
				if (cultureInfo.IsNeutralCulture)
					cultureInfo = null;
			} catch {
				var cultures = CultureInfo.GetCultures(CultureTypes.SpecificCultures);
				cultureInfo = (from c in cultures
							   let subLanguage = c.Name.Substring(c.Name.Length - 2, 2)
							   where c.NativeName == manifest.name ||
									 String.Compare(subLanguage, manifest.lang, true) == 0 ||
									 String.Compare(c.Name, manifest.lang, true) == 0
							   select c).FirstOrDefault();
			}
			int currCodepage = GetDefaultCodepage(Thread.CurrentThread.CurrentCulture);
			if (currCodepage == 0 || cultureInfo == null) {
				var languageSelection = new LanguageSelection();
				if (languageSelection.ShowDialog() == DialogResult.OK) {
					string langCode = languageSelection.GetSelectedLangCode();
					cultureInfo = CultureInfo.CreateSpecificCulture(langCode);
				} else return;
			}
			richBox.Clear(); // Clear the content
			richBox.Rtf = richBox.Rtf.Replace("ansicpg" + currCodepage.ToString(),
											  "ansicpg" + cultureInfo.TextInfo.ANSICodePage.ToString());
			richBox.Rtf = richBox.Rtf.Replace("deflang" + Thread.CurrentThread.CurrentCulture.LCID.ToString(),
											  "deflang" + cultureInfo.LCID.ToString());
			int gdiCharSet = GetFontCharset(cultureInfo.TextInfo.ANSICodePage);
			Font oldFont = richBox.Font;
			richBox.Font = new Font(richBox.Font.FontFamily, richBox.Font.SizeInPoints, richBox.Font.Style,
									GraphicsUnit.Point, (byte)gdiCharSet);
			oldFont.Dispose();
			richBox.Rtf = richBox.Rtf.Replace("fcharset" + this.Font.GdiCharSet.ToString(),
											  "fcharset" + gdiCharSet.ToString());
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

		private void changeFontToolStripMenuItem_Click(object sender, EventArgs e) {
			// The dialog doesn't display GdiCharSet as selected (Script)
			using (fontDialog.Font = (Font)richTranslation.Font.Clone()) {
				if (fontDialog.ShowDialog() == DialogResult.OK) {
					// Create new font, since the GdiCharSet can not be changes otherwise
					Font oldFont = richTranslation.Font;
					richTranslation.Font = new Font(fontDialog.Font.FontFamily, fontDialog.Font.SizeInPoints,
													fontDialog.Font.Style, GraphicsUnit.Point, 
													fontDialog.Font.GdiCharSet);
					oldFont.Dispose();
				}
			}
		}
	}
}
