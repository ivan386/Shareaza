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

		#endregion

		#region Button Events

		bool showedInvalidWarning = false;

		private void btnEnOld_Click(object sender, EventArgs e) {
			skinManifest manifest;
			List<skinDialog> list = new List<skinDialog>();
			if (GetDialogList(txtEnOld, ref list, out manifest)) {
				showedInvalidWarning = false;
				if (manifest == null || manifest.lang != "en") {
					MessageBox.Show(Settings.Default.Error_NotEnglish, "Information", MessageBoxButtons.OK,
									MessageBoxIcon.Information);
					oldEnList = null;
					txtEnOld.Text = String.Empty;
					richEnOld.Clear();
				} else {
					oldEnList = list;
					if (oldEnList != null)
						UpdateBoxes();
				}
			}
		}

		private void btnEnNew_Click(object sender, EventArgs e) {
			skinManifest manifest;
			List<skinDialog> list = new List<skinDialog>();
			if (GetDialogList(txtEnNew, ref list, out manifest)) {
				if (manifest == null || manifest.lang != "en") {
					MessageBox.Show(Settings.Default.Error_NotEnglish, "Information", MessageBoxButtons.OK,
									MessageBoxIcon.Information);
					newEnList = null;
					txtEnNew.Text = String.Empty;
					richEnNew.Clear();
				} else {
					newEnList = list;
					if (newEnList != null) {
						cmbDialogs.Items.AddRange(newEnList.Select(s => s.name).ToArray());
						cmbDialogs.SelectedIndex = 0;
					}
				}
			}
		}

		private void btnDoWork_Click(object sender, EventArgs e) {
			skinManifest manifest;
			List<skinDialog> list = new List<skinDialog>();
			if (GetDialogList(null, ref list, out manifest)) {
				if (manifest != null)
					richTranslation.SetLanguage(manifest.lang, manifest.name);
				translation = list;
				showedInvalidWarning = false;
				if (translation != null)
					UpdateBoxes();
			}
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
				//if (richEnOld.Tag != null)
				//    richEnOld.ZoomFactor = (float)richEnOld.Tag;
			}
			if (newEnList != null) {
				skinDialog newEn = newEnList.FirstOrDefault(dialog => dialog.name == newDialog.name);
				richEnNew.Text = GetXml(newEn);
				//if (richEnNew.Tag != null)
				//    richEnNew.ZoomFactor = (float)richEnNew.Tag;
			}
			if (translation != null) {
				skinDialog tr;
				UpdateDialogTranslation(newDialog.name, out tr);
				richTranslation.Text = GetXml(tr);
				//if (richTranslation.Tag != null) {
				//    float oldValue = (float)richTranslation.Tag;
				//    richTranslation.ZoomFactor = oldValue;
				//    if (oldValue != richTranslation.ZoomFactor) { // wtf happens here ????
				//        richTranslation.ZoomFactor = oldValue;
				//        Trace.WriteLine(String.Format("Old value: {0}, new value: {1}",
				//                        (float)richTranslation.Tag, richTranslation.ZoomFactor));
				//    }
				//    richTranslation.Tag = richTranslation.ZoomFactor;
				//}
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
				skinDialog oldEn = oldEnList.Where(d => d.name == dialogName).DefaultIfEmpty().First();
				int oldControlCount = oldEn == null || oldEn.controls == null ? 0 : oldEn.controls.Length;
				int trControlCount = tr.controls == null ? 0 : tr.controls.Length;
				if (oldEn == null || oldControlCount != trControlCount) {
					if (!showedInvalidWarning) {
						MessageBox.Show("The old english file doesn't match your translation file!");
						showedInvalidWarning = true;
					}
					dialog = tr;
					return;
				}
				skinDialog updatedDialog = newEn.Clone();
				// Update caption
				if (newEn.caption != oldEn.caption)
					updatedDialog.caption = newEn.caption;
				else
					updatedDialog.caption = tr.caption;

				if (newEn.controls != null) {
					// Same controls, no translation updates are needed
					var sameControls = from cNew in newEn.controls
									   join cOld in oldEn.controls on cNew.caption equals cOld.caption
									   select new
									   {
										   NewControl = cNew,
										   OldIndex = Array.IndexOf(oldEn.controls, cOld),
										   NewIndex = Array.IndexOf(newEn.controls, cNew)
									   };
					for (int i = 0; i < newEn.controls.Length; i++) {
						var matched = sameControls.Where(s => s.NewControl.Equals(newEn.controls[i]))
												  .FirstOrDefault();
						if (matched != null) {
							updatedDialog.controls[matched.NewIndex] = tr.controls[matched.OldIndex];
						}
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

		/// <summary>
		/// If error happens, the list is null
		/// </summary>
		/// <returns>True, if a new list was loaded</returns>
		bool GetDialogList(TextBox pathToSet, ref List<skinDialog> list, out skinManifest manifest) {
			openFileDialog.FileName = String.Empty;
			openFileDialog.ShowDialog();
			if (!String.IsNullOrEmpty(openFileDialog.FileName)) {
				Exception exception;
				var skin = XmlSerializerBase<skin>.Read(openFileDialog.FileName, out exception);
				if (exception != null) {
					if (exception.InnerException != null)
						MessageBox.Show(exception.InnerException.Message, "Error", MessageBoxButtons.OK,
										MessageBoxIcon.Information);
					else
						MessageBox.Show(exception.Message, "Error", MessageBoxButtons.OK,
										MessageBoxIcon.Error);
					list = null;
				} else if (skin != null && skin.dialogs != null && skin.dialogs.Length > 0) {
					manifest = skin.manifest;
					list = skin.dialogs.OrderBy(s => s.name).ToList();
					if (pathToSet != null)
						pathToSet.Text = openFileDialog.FileName;
					return true;
				}
			}
			manifest = null;
			return false;
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
				cutItem.Enabled = !box.ReadOnly;
				copyItem.Enabled = true;
				selectAllItem.Enabled = box.Text.Length != box.SelectionLength;
				deleteItem.Enabled = !box.ReadOnly;
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

		private void changeFontToolStripMenuItem_Click(object sender, EventArgs e) {
			// The dialog doesn't display GdiCharSet as selected (Script)
			using (fontDialog.Font = (Font)richTranslation.Font.Clone()) {
				if (fontDialog.ShowDialog() == DialogResult.OK) {
					richTranslation.Font = new Font(fontDialog.Font.FontFamily, fontDialog.Font.SizeInPoints,
													fontDialog.Font.Style, GraphicsUnit.Point,
													fontDialog.Font.GdiCharSet);
				}
			}
		}
	}
}
