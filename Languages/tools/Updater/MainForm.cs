using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using Updater.Common.Properties;
using System.Globalization;
using System.Threading;
using System.Diagnostics;
using System.Collections.Specialized;
using System.Xml;
using System.Xml.Linq;

namespace Updater.Common
{
	public partial class MainForm : Form
	{
		#region Initialization

		public MainForm() {
			InitializeComponent();
			
			// Fix column and row count under Mono
			tableTopAll.ColumnCount = 2;
			tableTopAll.RowCount = 2;
						
			lblStatus.Text = String.Empty;
			editor.OnError += delegate(object sender, PageViewErrorArgs args)
			{
				lblStatus.Text = args.Error;
			};
			editor.OnExportStatusChanged += delegate(object sender, ExportStatusChangedArgs args)
			{
				exportChangesToolStripMenuItem.Enabled = args.IsReadyForExport;
			};
		}

		#endregion

		#region Button Events

		private void btnEnOld_Click(object sender, EventArgs e) {
			openFileDialog.FileName = String.Empty;
			openFileDialog.ShowDialog();
			if (String.IsNullOrEmpty(openFileDialog.FileName))
				return;
			editor.OldFilePath = openFileDialog.FileName;
			txtEnOld.Text = editor.OldFilePath;
		}

		private void btnEnNew_Click(object sender, EventArgs e) {
			openFileDialog.FileName = String.Empty;
			openFileDialog.ShowDialog();
			if (String.IsNullOrEmpty(openFileDialog.FileName))
				return;
			editor.NewFilePath = openFileDialog.FileName;
			txtEnNew.Text = editor.NewFilePath;
		}

		private void btnDoWork_Click(object sender, EventArgs e) {
			openFileDialog.FileName = String.Empty;
			openFileDialog.ShowDialog();
			if (String.IsNullOrEmpty(openFileDialog.FileName))
				return;
			editor.UpdatedFilePath = openFileDialog.FileName;
		}

		#endregion

		private void changeFontToolStripMenuItem_Click(object sender, EventArgs e) {
			if (fontDialog.ShowDialog() == DialogResult.OK) {
				editor.UpdatePaneFont = new Font(fontDialog.Font.FontFamily, fontDialog.Font.SizeInPoints,
												  fontDialog.Font.Style, GraphicsUnit.Point,
												  fontDialog.Font.GdiCharSet);
			}
		}

		private void exportChangesToolStripMenuItem_Click(object sender, EventArgs e) {
			//if (translation == null)
			//    return;
			//_dirty = true; // save the last window changes
			//if (!AutoSaveTranslation())
			//    return;
			//StringBuilder sb = new StringBuilder();
			//foreach (var dialog in newEnList) {
			//    string xml = String.Empty;
			//    if (!updatedTranslation.ContainsKey(dialog.name)) {
			//        try {
			//            XElement elem = XElement.Parse(GetXml(dialog));
			//            xml = elem.ToString();
			//        } catch { }
			//    } else {
			//        xml = updatedTranslation[dialog.name];
			//    }
			//    if (!String.IsNullOrEmpty(xml)) {
			//        sb.Append("\r\n");
			//        sb.Append(xml);
			//    }
			//}
			//saveFileDialog.ShowDialog();
			//if (!String.IsNullOrEmpty(saveFileDialog.FileName)) {
			//    var newSettings = new XmlWriterSettings()
			//    {
			//        Indent = true,
			//        IndentChars = "\t",
			//        Encoding = new UTF8Encoding(false, false),
			//        CloseOutput = false,
			//        CheckCharacters = false,
			//        NewLineHandling = NewLineHandling.Replace,
			//    };
			//    try {
			//        using (var fs = new FileStream(saveFileDialog.FileName, FileMode.Create, FileAccess.Write))
			//        using (XmlWriter writer = XmlWriter.Create(fs, newSettings)) {
			//            XDocument doc = XDocument.Parse(String.Format(envelope, sb.ToString()));
			//            doc.WriteTo(writer);
			//            writer.Flush();
			//            fs.Flush();
			//        }
			//    } catch (Exception ex) {
			//        MessageBox.Show(ex.Message, Settings.Default.Error,
			//                        MessageBoxButtons.OK, MessageBoxIcon.Error);
			//    }
			//}
		}

		private void form_ResizeBegin(object sender, EventArgs e) {
			editor.BeginResize();
		}
	}
}
