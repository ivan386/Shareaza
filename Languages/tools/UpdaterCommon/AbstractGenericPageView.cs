using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Text;
using System.Drawing;
using System.ComponentModel;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Linq;

namespace Updater.Common
{
	public abstract partial class AbstractGenericPageView<T> : UserControl, IPageView where T : class, INamedElement
	{
		float vertPerc;
		float horizPerc;
		protected List<T> oldEnList, newEnList, updatesList;
		protected StringDictionary updatedXmlDic = new StringDictionary();
		Func<T, bool> currElementFunc = null;

		static readonly string envelopeFormat = "<skin xmlns=\"http://www.shareaza.com/schemas/Skin.xsd\" version=\"1.0\">\r\n" +
												"<{0}>\r\n{1}\r\n" +
												"</{0}>\r\n" +
												"</skin>";
		public AbstractGenericPageView() {
			InitializeComponent();
			this.splitVertical.SplitterDistance = this.tableBottomAll.Width / 2 - this.tableBottomAll.Margin.Left;
			this.splitHorizontal.SplitterDistance = this.tableBottomAll.Height / 2 - this.tableBottomAll.Margin.Top;
			vertPerc = (float)this.splitVertical.SplitterDistance / this.tableBottomAll.Width;
			horizPerc = (float)this.splitHorizontal.SplitterDistance / this.tableBottomAll.Height;
			currElementFunc = element => ((INamedElement)element).Id == (string)cmbElements.Items[cmbElements.SelectedIndex];
			if (IsRunningInMono) {
				richEnOld.MouseClick += new MouseEventHandler(OnMouseClicked);
				richEnNew.MouseClick += new MouseEventHandler(OnMouseClicked);
				richUpdate.MouseClick += new MouseEventHandler(OnMouseClicked);
			}
		}

		public AbstractGenericPageView(string oldFilePath, string newFilePath, string translationFilePath)
			: this() {
			this.OldFilePath = oldFilePath;
			this.NewFilePath = newFilePath;
			this.UpdatedFilePath = translationFilePath;
		}

	
		public event EventHandler<PageViewErrorArgs> OnError;
		public event EventHandler<ExportStatusChangedArgs> OnUpdate;

		protected void SetError(string errorMessage) {
			if (OnError != null)
				OnError(this, new PageViewErrorArgs(errorMessage));
		}

		static bool _bMono = Type.GetType("Mono.Runtime") != null;
		public static bool IsRunningInMono {
			get { return _bMono; }
		}		
		
		int lastIndex = -1;
		public int ElementIndex {
			get { return lastIndex; }
		}

		private void cmbElements_SelectedIndexChanged(object sender, EventArgs e) {
			if (lastIndex == cmbElements.SelectedIndex)
				return; // stupid, why to call it when the index doesn't change?
			if (this.Dirty && !AutoSaveUpdates()) {
				var result = MessageBox.Show(Settings.Default.RevertQuestion,
											 Settings.Default.Error,
											 MessageBoxButtons.YesNo, MessageBoxIcon.Question);
				string elementName = (string)cmbElements.Items[lastIndex];
				if (result == DialogResult.No) {
					if (updatedXmlDic.ContainsKey(elementName))
						updatedXmlDic[elementName] = richUpdate.Text;
					else
						updatedXmlDic.Add(elementName, richUpdate.Text);
					cmbElements.SelectedIndex = lastIndex;
					return;
				} else {
					updatedXmlDic.Remove(elementName);
				}
			}
			UpdatePanes();
			lastIndex = cmbElements.SelectedIndex;
		}

		public void BeginResize() {
			vertPerc = (float)this.splitVertical.SplitterDistance / this.tableBottomAll.Width;
			horizPerc = (float)this.splitHorizontal.SplitterDistance / this.tableBottomAll.Height;
		}

		private void tableBottomAll_Resize(object sender, EventArgs e) {
			if (this.tableBottomAll.Width == 0 || this.tableBottomAll.Height == 0)
				return;
			this.SuspendLayout();
			this.splitVertical.SplitterDistance = (int)(this.tableBottomAll.Width * vertPerc);
			this.splitHorizontal.SplitterDistance = (int)(this.tableBottomAll.Height * horizPerc);
			this.ResumeLayout();
		}

		string GetXml(T element) {
			if (element == null) return String.Empty;
			MemoryStream stream;
			string xml = String.Empty;

			if (XmlSerializerBase<T>.Write(element, out stream)) {
				byte[] bytes = stream.ToArray();
				try {
					xml = new String((new UTF8Encoding()).GetChars(stream.ToArray()));
					xml = xml.Replace(" xmlns=\"http://www.shareaza.com/schemas/Skin.xsd\"", String.Empty);
					xml = xml.Replace("<?xml version=\"1.0\"?>\r\n", String.Empty);
				} catch { }
				stream.Close();
			}
			return xml;
		}

		protected bool GetElementList(string filePath, FileType fileType, ref List<T> list, out skinManifest manifest) {
			if (String.IsNullOrEmpty(filePath) || !File.Exists(filePath)) {
				manifest = null;
				return false;
			}

			Exception exception;
			var skin = XmlSerializerBase<skin>.Read(filePath, out exception);
			if (exception != null) {
				if (exception.InnerException != null)
					MessageBox.Show(exception.InnerException.Message, Settings.Default.Error,
									MessageBoxButtons.OK, MessageBoxIcon.Information);
				else
					MessageBox.Show(exception.Message, Settings.Default.Error,
									MessageBoxButtons.OK, MessageBoxIcon.Error);
				list = null;
			} else if (skin != null) {
				object[] elements = skin.GetElements(typeof(T));
				if (elements != null && elements.Length > 0) {
					manifest = skin.manifest;
					list = elements.Cast<T>().OrderBy(s => s.Id).ToList();
					
					XmlNameTable nameTable = skin.RootElement.OwnerDocument.NameTable;
					var nsmgr = new XmlNamespaceManager(nameTable);
					nsmgr.AddNamespace("ss", "http://www.shareaza.com/schemas/Skin.xsd");
					XmlElement root = skin.RootElement;
					
					foreach (var element in list) {
						if (!(element is INamedElement)) continue;
						INamedElement ne = (INamedElement)element;
						string nodeName = skin.GetElementName(typeof(T));
						string childName = skin.GetElementChildName(typeof(T));
						ne.NodeList = root.SelectNodes(String.Format("/ss:skin/ss:{0}/ss:{1}", 
																	 nodeName, childName), nsmgr);
					}
					
					if (fileType == FileType.OldEnglish)
						_oldFilePath = filePath;
					else if (fileType == FileType.NewEnglish)
						_newFilePath = filePath;
					else if (fileType == FileType.Updated)
						_updatedFilePath = filePath;
					return true;
				}
			}

			manifest = null;
			return false;
		}

		public bool Dirty { get; set; }

		private void richTranslation_TextChanged(object sender, EventArgs e) {
			if (lastIndex == cmbElements.SelectedIndex)
				this.Dirty = true;
		}

		public bool IsReadyForExport {
			get { return updatesList != null && newEnList != null && oldEnList != null; }
		}

		public void UpdatePanes() {
			if (OnUpdate != null)
				OnUpdate(this, new ExportStatusChangedArgs(IsReadyForExport));
				
			SetError(String.Empty);

			if (newEnList == null)
				return;
			T newElement = newEnList.FirstOrDefault(currElementFunc);

			if (oldEnList != null) {
				T oldEn = oldEnList.FirstOrDefault(element => element.Id == newElement.Id);
				richEnOld.Text = GetXml(oldEn);
			}
			if (newEnList != null) {
				T newEn = newEnList.FirstOrDefault(element => element.Id == newElement.Id);
				richEnNew.Text = GetXml(newEn);
			}
			if (updatesList != null) {
				if (updatedXmlDic.ContainsKey(newElement.Id)) {
					richUpdate.Text = updatedXmlDic[newElement.Id];
				} else if (oldEnList != null) {
					T tr;
					UpdateElement(newElement.Id, out tr);
					richUpdate.Text = GetXml(tr);
					updatedXmlDic.Add(newElement.Id, richUpdate.Text);
				}
			}
		}

		protected abstract void UpdateElement(string elementId, out T element);
		public abstract bool AutoSaveUpdates();

		string _oldFilePath, _newFilePath, _updatedFilePath;

		public string OldFilePath {
			get { return _oldFilePath; }
			set {
				if (value == null)
					value = String.Empty;
				if (value.CompareTo(_oldFilePath) != 0) {
					skinManifest manifest;
					List<T> list = new List<T>();
					if (GetElementList(value, FileType.OldEnglish, ref list, out manifest)) {
						if (manifest == null || manifest.lang != "en") {
							MessageBox.Show(Settings.Default.Error_NotEnglish,
											Settings.Default.Information, MessageBoxButtons.OK,
											MessageBoxIcon.Information);
							oldEnList = null;
							_oldFilePath = String.Empty;
							richEnOld.Clear();
						} else {
							_oldFilePath = value;
							oldEnList = list;
							if (oldEnList != null)
								UpdatePanes();
						}
					}
				}
			}
		}

		public string NewFilePath {
			get { return _newFilePath; }
			set {
				if (value == null)
					value = String.Empty;
				if (value.CompareTo(_newFilePath) != 0) {
					skinManifest manifest;
					List<T> list = new List<T>();
					if (GetElementList(value, FileType.NewEnglish, ref list, out manifest)) {
						if (manifest == null || manifest.lang != "en") {
							MessageBox.Show(Settings.Default.Error_NotEnglish,
											Settings.Default.Information, MessageBoxButtons.OK,
											MessageBoxIcon.Information);
							newEnList = null;
							_newFilePath = String.Empty;
							richEnNew.Clear();
						} else {
							newEnList = list;
							if (newEnList != null) {
								_newFilePath = value;
								cmbElements.Items.AddRange(newEnList.Select(s => s.Id).ToArray());
								cmbElements.SelectedIndex = 0;
							}
						}
					}
				}
			}
		}

		public string UpdatedFilePath {
			get { return _updatedFilePath; }
			set {
				if (value == null)
					value = String.Empty;
				if (value.CompareTo(_updatedFilePath) != 0) {
					skinManifest manifest;
					List<T> list = new List<T>();
					if (GetElementList(value, FileType.Updated, ref list, out manifest)) {
						if (manifest != null)
							richUpdate.SetLanguage(manifest.lang, manifest.name);
						updatesList = list;
						_updatedFilePath = value;
						if (updatesList != null) {
							updatedXmlDic.Clear();
							UpdatePanes();
						}
					}
				}
			}
		}

		public static string Envelope {
			get {
				string elementName = skin.GetElementName(typeof(T));
				return String.Format(envelopeFormat, elementName, @"{0}");
			}
		}
		
		public Font UpdatePaneFont {
			get { return richUpdate.Font; }
			set { richUpdate.Font = value;	}
		}

		#region Context Menu

		private void contextMenu_Opening(object sender, CancelEventArgs e) {
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
			box.Focus();
			if (!box.ReadOnly) {
				pasteItem.Enabled = !String.IsNullOrEmpty(Clipboard.GetText());
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
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
			box.Cut();
		}

		private void copyItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
			box.Copy();
		}

		private void pasteItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
			IDataObject iData = Clipboard.GetDataObject();
			if (iData.GetDataPresent(DataFormats.Text))
				box.Paste();
		}

		private void deleteItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
			if (box == null)
				box = richUpdate; // No other should send this event
			if (box != null && box.SelectionLength > 0)
				box.SelectedText = "";
			deleteItem.Enabled = false;
		}

		private void selectAllItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
			if (box.CanSelect)
				box.SelectAll();
		}

		private void undoItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
			if (box.CanUndo)
				box.Undo();
		}

		private void redoItem_Click(object sender, EventArgs e) {
			RichTextBox box = (RichTextBox)(contextMenu.SourceControl ?? contextMenu.Tag);
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

		private void OnMouseClicked(object sender, MouseEventArgs e) {
			if (IsRunningInMono)
				contextMenu.Tag = sender as Control;
		}
		
		public virtual string ExportChanges() {
			if (updatesList == null)
				return String.Empty;
			Dirty = true; // save the last window changes
			if (!AutoSaveUpdates())
				return String.Empty;
			StringBuilder sb = new StringBuilder();
			string rootName = skin.GetElementName(typeof(T));
			sb.Append("<");
			sb.Append(rootName);
			sb.Append(">");
			foreach (var element in newEnList) {
				string xml = String.Empty;
				if (updatedXmlDic.ContainsKey(element.Id)) {
					xml = updatedXmlDic[element.Id];
				} else if (oldEnList != null) {
					T tr;
					UpdateElement(element.Id, out tr);
					updatedXmlDic.Add(element.Id, GetXml(tr));
					xml = updatedXmlDic[element.Id];
				}
				if (!String.IsNullOrEmpty(xml)) {
					sb.Append("\r\n");
					sb.Append(xml);
				}
			}
			sb.Append("\r\n</");
			sb.Append(rootName);
			sb.Append(">");
			return sb.ToString();
		}
	}
}
