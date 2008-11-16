using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Updater.Common
{
	public partial class EditorTabControl : UserControl
	{
		public EditorTabControl() {
			InitializeComponent();
			this.SuspendLayout();

			Dictionary<string, Type> mappings = ImplementedViews.GetElementMappings();
			int pageIndex = 0;
			foreach (KeyValuePair<string, Type> mapping in mappings) {
				UserControl view = ImplementedViews.GetView(mapping.Value) as UserControl;
				if (view == null) continue;
				tabControl.TabPages.Add(mapping.Key);
				view.Dock = DockStyle.Fill;
				tabControl.TabPages[pageIndex++].Controls.Add(view);
			}
			this.ResumeLayout();

			foreach (TabPage page in tabControl.TabPages) {
				if (page.Controls.Count == 0) continue;
				IPageView view = (IPageView)page.Controls[0];
				view.OnError += new EventHandler<PageViewErrorArgs>(OnPageError);
				view.OnUpdate += new EventHandler<ExportStatusChangedArgs>(OnViewUpdate);
			}

			tabControl.TabIndexChanged += new EventHandler(OnTabIndexChanged);
		}

		public event EventHandler<PageViewErrorArgs> OnError;

		void OnPageError(object sender, PageViewErrorArgs args) {
			if (OnError != null)
				OnError(sender, args);
		}

		string _oldFilePath, _newFilePath, _updatedFilePath;

		public string OldFilePath {
			get {
				TabPage page = tabControl.SelectedTab;
				IPageView view = (IPageView)page.Controls[0];
				return view.OldFilePath;
			}

			set {
				if (value == null)
					value = String.Empty;
				if (value.CompareTo(_oldFilePath) != 0) {
					bool failed = false;
					foreach (TabPage page in tabControl.TabPages) {
						IPageView view = (IPageView)page.Controls[0];
						view.OldFilePath = value;
						if (String.IsNullOrEmpty(view.OldFilePath)) {
							failed = true;
							break;
						}
					}
					if (failed) {
						foreach (TabPage page in tabControl.TabPages) {
							IPageView view = (IPageView)page.Controls[0];
							view.OldFilePath = _oldFilePath;
						}
					} else {
						_oldFilePath = value;
					}
				}
			}
		}

		public string NewFilePath {
			get {
				TabPage page = tabControl.SelectedTab;
				IPageView view = (IPageView)page.Controls[0];
				return view.NewFilePath;
			}

			set {
				if (value == null)
					value = String.Empty;
				if (value.CompareTo(_newFilePath) != 0) {
					bool failed = false;
					foreach (TabPage page in tabControl.TabPages) {
						IPageView view = (IPageView)page.Controls[0];
						view.NewFilePath = value;
						if (String.IsNullOrEmpty(view.NewFilePath)) {
							failed = true;
							break;
						}
					}
					if (failed) {
						foreach (TabPage page in tabControl.TabPages) {
							IPageView view = (IPageView)page.Controls[0];
							view.NewFilePath = _newFilePath;
						}
					} else {
						_newFilePath = value;
					}
				}
			}
		}

		public string UpdatedFilePath {
			get {
				TabPage page = tabControl.SelectedTab;
				IPageView view = (IPageView)page.Controls[0];
				return view.UpdatedFilePath;
			}

			set {
				if (value == null)
					value = String.Empty;
				if (value.CompareTo(_updatedFilePath) != 0) {
					bool failed = false;
					foreach (TabPage page in tabControl.TabPages) {
						IPageView view = (IPageView)page.Controls[0];
						view.UpdatedFilePath = value;
						if (String.IsNullOrEmpty(view.UpdatedFilePath)) {
							failed = true;
							break;
						}
					}
					if (failed) {
						foreach (TabPage page in tabControl.TabPages) {
							IPageView view = (IPageView)page.Controls[0];
							view.UpdatedFilePath = _updatedFilePath;
						}
					} else {
						_updatedFilePath = value;
					}
				}
			}
		}

		public Font UpdatePaneFont {
			get {
				TabPage page = tabControl.SelectedTab;
				IPageView view = (IPageView)page.Controls[0];
				return view.UpdatePaneFont;
			}

			set {
				foreach (TabPage page in tabControl.TabPages) {
					IPageView view = (IPageView)page.Controls[0];
					view.UpdatePaneFont = value;
				}
			}
		}

		bool _readyForExport = false;
		public event EventHandler<ExportStatusChangedArgs> OnExportStatusChanged;

		private void OnViewUpdate(object sender, ExportStatusChangedArgs args) {
			IPageView view = (IPageView)sender;
			TabPage currTab = tabControl.SelectedTab;
			IPageView currView = (IPageView)currTab.Controls[0];
			if (view.Equals(currView) && _readyForExport != args.IsReadyForExport) {
				_readyForExport = args.IsReadyForExport;
				if (OnExportStatusChanged != null)
					OnExportStatusChanged(this, args);
			}
		}

		public void BeginResize() {
			foreach (TabPage page in tabControl.TabPages) {
				IPageView view = (IPageView)page.Controls[0];
				view.BeginResize();
			}
		}

		void OnTabIndexChanged(object sender, EventArgs args) {
			// tabControl.SelectedTab.Controls[
		}
	}
}
