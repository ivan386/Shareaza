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
using System.Collections.Specialized;
using System.Xml.Linq;

namespace ShareazaDialogUpdater
{
	public partial class form : Form
	{
		float vertPerc;
		float horizPerc;
		List<skinDialog> oldEnList, newEnList, translation;
		StringDictionary updatedTranslation = new StringDictionary();
		StringDictionary finalTranslation = new StringDictionary();
		Func<skinDialog, bool> currDialogFunc = null;

		readonly string envelope = "<skin xmlns=\"http://www.shareaza.com/schemas/Skin.xsd\" version=\"1.0\">\r\n" +
								   "<dialogs>\r\n{0}\r\n" +
								   "</dialogs>\r\n" +
								   "</skin>";

		#region Initialization

		public form() {
			InitializeComponent();

			this.splitVertical.SplitterDistance = this.tableBottomAll.Width / 2 - this.tableBottomAll.Margin.Left;
			this.splitHorizontal.SplitterDistance = this.tableBottomAll.Height / 2 - this.tableBottomAll.Margin.Top;
			vertPerc = (float)this.splitVertical.SplitterDistance / this.tableBottomAll.Width;
			horizPerc = (float)this.splitHorizontal.SplitterDistance / this.tableBottomAll.Height;
			deleteItem.Enabled = false; // Don't send Delete keys
			lblStatus.Text = String.Empty;
			currDialogFunc = dialog => dialog.name == (string)cmbDialogs.Items[cmbDialogs.SelectedIndex];
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

		private void btnEnOld_Click(object sender, EventArgs e) {
			skinManifest manifest;
			List<skinDialog> list = new List<skinDialog>();
			if (GetDialogList(txtEnOld, ref list, out manifest)) {
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
				if (translation != null)
					UpdateBoxes();
			}
			exportChangesToolStripMenuItem.Enabled = translation != null;
		}

		#endregion

		int lastIndex = -1;
		private void cmbDialogs_SelectedIndexChanged(object sender, EventArgs e) {
			if (lastIndex == cmbDialogs.SelectedIndex)
				return; // stupid, why to call it when the index doesn't change?
			if (_dirty && !AutoSaveTranslation()) {
				var result = MessageBox.Show("Your changes are incorrect!\r\n\r\nDo you want to revert to automatic XML?",
											 "Error", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
				string dialogName = (string)cmbDialogs.Items[lastIndex];
				if (result == DialogResult.No) {
					if (updatedTranslation.ContainsKey(dialogName))
						updatedTranslation[dialogName] = richTranslation.Text;
					else
						updatedTranslation.Add(dialogName, richTranslation.Text);
					cmbDialogs.SelectedIndex = lastIndex;
					return;
				} else {
					updatedTranslation.Remove(dialogName);
				}
			}
			UpdateBoxes();
			lastIndex = cmbDialogs.SelectedIndex;
		}

		void UpdateBoxes() {
			lblStatus.Text = String.Empty;
			if (newEnList == null)
				return;
			skinDialog newDialog = newEnList.FirstOrDefault(currDialogFunc);

			if (oldEnList != null) {
				skinDialog oldEn = oldEnList.FirstOrDefault(dialog => dialog.name == newDialog.name);
				richEnOld.Text = GetXml(oldEn);
			}
			if (newEnList != null) {
				skinDialog newEn = newEnList.FirstOrDefault(dialog => dialog.name == newDialog.name);
				richEnNew.Text = GetXml(newEn);
			}
			if (translation != null) {
				if (updatedTranslation.ContainsKey(newDialog.name)) {
					richTranslation.Text = updatedTranslation[newDialog.name];
				} else {
					skinDialog tr;
					UpdateDialogTranslation(newDialog.name, out tr);
					richTranslation.Text = GetXml(tr);
					updatedTranslation.Add(newDialog.name, richTranslation.Text);
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
				int oldControlCount = oldEn == null || oldEn.controls == null ? 0 : oldEn.controls.Length;
				int trControlCount = tr.controls == null ? 0 : tr.controls.Length;
				if (oldEn == null || oldControlCount != trControlCount) {
					lblStatus.Text = "The old english file doesn't match your translation file!";
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
						var matched = sameControls.FirstOrDefault(s => s.NewControl.Equals(newEn.controls[i]));
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

		bool _dirty = false;
		private void richTranslation_TextChanged(object sender, EventArgs e) {
			if (lastIndex == cmbDialogs.SelectedIndex)
				_dirty = true;
		}

		bool AutoSaveTranslation() {
			if (!_dirty && String.IsNullOrEmpty(richTranslation.Text)) // can not be empty
				return true;
			string xml = richTranslation.Xml;
			skinDialog currDialog = newEnList[lastIndex];

			if (_dirty) {
				if (String.IsNullOrEmpty(xml)) {
					lblStatus.Text = richTranslation.XmlError;
					return false;
				} else {
					Exception exeption;
					skin testSkin = XmlSerializerBase<skin>.ReadString(String.Format(envelope, xml),
																	   out exeption);
					if (testSkin == null || exeption != null) {
						lblStatus.Text = exeption.InnerException != null ? exeption.InnerException.Message :
																		   exeption.Message;
						return false;
					} else {
						if (testSkin.dialogs == null || testSkin.dialogs.Length == 0) {
							lblStatus.Text = "Invalid XML";
							return false;
						}
						if (currDialog.cookie != testSkin.dialogs[0].cookie) {
							lblStatus.Text = "Cookie is invalid";
							return false;
						}
						if (currDialog.name != testSkin.dialogs[0].name) {
							lblStatus.Text = "Dialog name is invalid";
							return false;
						}
						int count = currDialog.controls == null ? 0 : currDialog.controls.Length;
						int trCount = testSkin.dialogs[0].controls == null ? 0 : testSkin.dialogs[0].controls.Length;
						if (trCount != count) {
							lblStatus.Text = "<control> count is invalid";
							return false;
						}
						for (int i = 0; i < count; i++) {
							if (currDialog.controls[i].caption == String.Empty &&
								testSkin.dialogs[0].controls[i].caption != String.Empty ||
								currDialog.controls[i].caption != String.Empty &&
								testSkin.dialogs[0].controls[i].caption == String.Empty) {
								lblStatus.Text = String.Format("<control> #{0} is invalid", i + 1);
								return false;
							}
						}
						_dirty = false;
					}
				}
			}

			if (!updatedTranslation.ContainsKey(currDialog.name))
				updatedTranslation.Add(currDialog.name, xml);
			else
				updatedTranslation[currDialog.name] = xml;
			return true;
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
			if (fontDialog.ShowDialog() == DialogResult.OK) {
				richTranslation.Font = new Font(fontDialog.Font.FontFamily, fontDialog.Font.SizeInPoints,
												fontDialog.Font.Style, GraphicsUnit.Point,
												fontDialog.Font.GdiCharSet);
			}
		}

		private void exportChangesToolStripMenuItem_Click(object sender, EventArgs e) {
			if (translation == null)
				return;
			_dirty = true; // save the last window changes
			if (!AutoSaveTranslation())
				return;
			StringBuilder sb = new StringBuilder();
			foreach (var dialog in newEnList) {
				string xml = String.Empty;
				if (!updatedTranslation.ContainsKey(dialog.name)) {
					try {
						XDocument doc = XDocument.Parse(GetXml(dialog));
						xml = doc.ToString();
					} catch { }
				} else {
					xml = updatedTranslation[dialog.name];
				}
				if (!String.IsNullOrEmpty(xml)) {
					sb.Append("\r\n");
					sb.Append(xml);
				}
			}
			saveFileDialog.ShowDialog();
			if (!String.IsNullOrEmpty(saveFileDialog.FileName)) {
				try {
					using (var fs = new FileStream(saveFileDialog.FileName, FileMode.Create, FileAccess.Write))
					using (TextWriter writer = new StreamWriter(fs)) {
						writer.Write(envelope, sb.ToString());
						writer.Flush();
					}
				} catch (Exception ex) {
					MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}
		}
	}
}
