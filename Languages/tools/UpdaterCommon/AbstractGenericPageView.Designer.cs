namespace Updater.Common
{
	using System.Windows.Forms;
	using System.ComponentModel.Design;
	
	public partial class AbstractGenericPageView<T>
	{
		private System.ComponentModel.IContainer components = null;

		protected override void Dispose(bool disposing) {
			if (disposing && (components != null)) {
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		protected void InitializeComponent() {
			this.components = new System.ComponentModel.Container();
			System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
			System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
			this.tableBottomAll = new System.Windows.Forms.TableLayoutPanel();
			this.splitHorizontal = new System.Windows.Forms.SplitContainer();
			this.splitVertical = new System.Windows.Forms.SplitContainer();
			this.contextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.undoItem = new System.Windows.Forms.ToolStripMenuItem();
			this.redoItem = new System.Windows.Forms.ToolStripMenuItem();
			this.cutItem = new System.Windows.Forms.ToolStripMenuItem();
			this.copyItem = new System.Windows.Forms.ToolStripMenuItem();
			this.pasteItem = new System.Windows.Forms.ToolStripMenuItem();
			this.deleteItem = new System.Windows.Forms.ToolStripMenuItem();
			this.selectAllItem = new System.Windows.Forms.ToolStripMenuItem();
			this.cmbElements = new System.Windows.Forms.ComboBox();
			this.richEnOld = new Updater.Common.XmlViewBox();
			this.richEnNew = new Updater.Common.XmlViewBox();
			this.richUpdate = new Updater.Common.XmlViewBox();
			toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			this.tableBottomAll.SuspendLayout();
			this.splitHorizontal.Panel1.SuspendLayout();
			this.splitHorizontal.Panel2.SuspendLayout();
			this.splitHorizontal.SuspendLayout();
			this.splitVertical.Panel1.SuspendLayout();
			this.splitVertical.Panel2.SuspendLayout();
			this.splitVertical.SuspendLayout();
			this.contextMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// toolStripSeparator1
			// 
			toolStripSeparator1.Name = "toolStripSeparator1";
			toolStripSeparator1.Size = new System.Drawing.Size(128, 6);
			// 
			// toolStripSeparator2
			// 
			toolStripSeparator2.Name = "toolStripSeparator2";
			toolStripSeparator2.Size = new System.Drawing.Size(128, 6);
			// 
			// tableBottomAll
			// 
			this.tableBottomAll.ColumnCount = 2;
			this.tableBottomAll.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableBottomAll.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableBottomAll.Controls.Add(this.splitHorizontal, 0, 0);
			this.tableBottomAll.Controls.Add(this.cmbElements, 1, 2);
			this.tableBottomAll.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableBottomAll.Location = new System.Drawing.Point(0, 0);
			this.tableBottomAll.Margin = new System.Windows.Forms.Padding(0);
			this.tableBottomAll.Name = "tableBottomAll";
			this.tableBottomAll.RowCount = 3;
			this.tableBottomAll.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 65.60693F));
			this.tableBottomAll.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 34.39306F));
			this.tableBottomAll.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 28F));
			this.tableBottomAll.Size = new System.Drawing.Size(488, 403);
			this.tableBottomAll.TabIndex = 1;
			this.tableBottomAll.Resize += new System.EventHandler(this.tableBottomAll_Resize);
			// 
			// splitHorizontal
			// 
			this.splitHorizontal.BackColor = System.Drawing.SystemColors.Window;
			this.splitHorizontal.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.tableBottomAll.SetColumnSpan(this.splitHorizontal, 2);
			this.splitHorizontal.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitHorizontal.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
			this.splitHorizontal.Location = new System.Drawing.Point(3, 3);
			this.splitHorizontal.Name = "splitHorizontal";
			this.splitHorizontal.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitHorizontal.Panel1
			// 
			this.splitHorizontal.Panel1.Controls.Add(this.splitVertical);
			// 
			// splitHorizontal.Panel2
			// 
			this.splitHorizontal.Panel2.Controls.Add(this.richUpdate);
			this.tableBottomAll.SetRowSpan(this.splitHorizontal, 2);
			this.splitHorizontal.Size = new System.Drawing.Size(482, 368);
			this.splitHorizontal.SplitterDistance = 212;
			this.splitHorizontal.TabIndex = 0;
			// 
			// splitVertical
			// 
			this.splitVertical.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.splitVertical.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitVertical.Location = new System.Drawing.Point(0, 0);
			this.splitVertical.Name = "splitVertical";
			// 
			// splitVertical.Panel1
			// 
			this.splitVertical.Panel1.Controls.Add(this.richEnOld);
			this.splitVertical.Panel1.RightToLeft = System.Windows.Forms.RightToLeft.No;
			this.splitVertical.Panel1MinSize = 0;
			// 
			// splitVertical.Panel2
			// 
			this.splitVertical.Panel2.Controls.Add(this.richEnNew);
			this.splitVertical.Panel2.RightToLeft = System.Windows.Forms.RightToLeft.No;
			this.splitVertical.Panel2MinSize = 0;
			this.splitVertical.Size = new System.Drawing.Size(482, 212);
			this.splitVertical.SplitterDistance = 239;
			this.splitVertical.TabIndex = 0;
			// 
			// contextMenu
			// 
			this.contextMenu.ImageScalingSize = new System.Drawing.Size(0, 0);
			this.contextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.undoItem,
            this.redoItem,
            toolStripSeparator1,
            this.cutItem,
            this.copyItem,
            this.pasteItem,
            this.deleteItem,
            toolStripSeparator2,
            this.selectAllItem});
			this.contextMenu.Name = "contextMenu";
			this.contextMenu.ShowImageMargin = false;
			this.contextMenu.ShowItemToolTips = false;
			this.contextMenu.Size = new System.Drawing.Size(132, 170);
			this.contextMenu.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenu_Opening);
			this.contextMenu.Closing += new System.Windows.Forms.ToolStripDropDownClosingEventHandler(this.contextMenu_Closing);
			// 
			// undoItem
			// 
			this.undoItem.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
			this.undoItem.Name = "undoItem";
			this.undoItem.Overflow = System.Windows.Forms.ToolStripItemOverflow.AsNeeded;
			this.undoItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Z)));
			this.undoItem.Size = new System.Drawing.Size(131, 22);
			this.undoItem.Text = "Undo";
			this.undoItem.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.undoItem.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
			this.undoItem.Click += new System.EventHandler(this.undoItem_Click);
			// 
			// redoItem
			// 
			this.redoItem.Name = "redoItem";
			this.redoItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.R)));
			this.redoItem.Size = new System.Drawing.Size(131, 22);
			this.redoItem.Text = "Redo";
			this.redoItem.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.redoItem.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
			this.redoItem.Click += new System.EventHandler(this.redoItem_Click);
			// 
			// cutItem
			// 
			this.cutItem.Name = "cutItem";
			this.cutItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.X)));
			this.cutItem.Size = new System.Drawing.Size(131, 22);
			this.cutItem.Text = "Cut";
			this.cutItem.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.cutItem.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
			this.cutItem.Click += new System.EventHandler(this.cutItem_Click);
			// 
			// copyItem
			// 
			this.copyItem.Name = "copyItem";
			this.copyItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.C)));
			this.copyItem.Size = new System.Drawing.Size(131, 22);
			this.copyItem.Text = "Copy";
			this.copyItem.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.copyItem.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
			this.copyItem.Click += new System.EventHandler(this.copyItem_Click);
			// 
			// pasteItem
			// 
			this.pasteItem.Name = "pasteItem";
			this.pasteItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.V)));
			this.pasteItem.Size = new System.Drawing.Size(131, 22);
			this.pasteItem.Text = "Paste";
			this.pasteItem.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.pasteItem.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
			this.pasteItem.Click += new System.EventHandler(this.pasteItem_Click);
			// 
			// deleteItem
			// 
			this.deleteItem.Name = "deleteItem";
			this.deleteItem.ShortcutKeys = System.Windows.Forms.Keys.Delete;
			this.deleteItem.Size = new System.Drawing.Size(131, 22);
			this.deleteItem.Text = "Delete";
			this.deleteItem.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.deleteItem.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
			this.deleteItem.Click += new System.EventHandler(this.deleteItem_Click);
			// 
			// selectAllItem
			// 
			this.selectAllItem.Name = "selectAllItem";
			this.selectAllItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.A)));
			this.selectAllItem.Size = new System.Drawing.Size(131, 22);
			this.selectAllItem.Text = "Select All";
			this.selectAllItem.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.selectAllItem.TextImageRelation = System.Windows.Forms.TextImageRelation.TextAboveImage;
			this.selectAllItem.Click += new System.EventHandler(this.selectAllItem_Click);
			// 
			// cmbElements
			// 
			this.cmbElements.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.cmbElements.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cmbElements.FormattingEnabled = true;
			this.cmbElements.Location = new System.Drawing.Point(315, 379);
			this.cmbElements.Name = "cmbElements";
			this.cmbElements.Size = new System.Drawing.Size(170, 21);
			this.cmbElements.TabIndex = 3;
			this.cmbElements.SelectedIndexChanged += new System.EventHandler(this.cmbElements_SelectedIndexChanged);
			// 
			// richEnOld
			// 
			this.richEnOld.AcceptsTab = true;
			this.richEnOld.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.richEnOld.BackColor = System.Drawing.SystemColors.Window;
			this.richEnOld.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.richEnOld.ContextMenuStrip = this.contextMenu;
			this.richEnOld.Font = new System.Drawing.Font("Verdana", 8.25F);
			this.richEnOld.ImeMode = System.Windows.Forms.ImeMode.On;
			this.richEnOld.Location = new System.Drawing.Point(0, 0);
			this.richEnOld.Margin = new System.Windows.Forms.Padding(0);
			this.richEnOld.Name = "richEnOld";
			this.richEnOld.ReadOnly = true;
			this.richEnOld.Size = new System.Drawing.Size(238, 211);
			this.richEnOld.TabIndex = 0;
			this.richEnOld.TabStop = false;
			this.richEnOld.Text = "";
			this.richEnOld.WordWrap = false;
			// 
			// richEnNew
			// 
			this.richEnNew.AcceptsTab = true;
			this.richEnNew.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.richEnNew.BackColor = System.Drawing.SystemColors.Window;
			this.richEnNew.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.richEnNew.ContextMenuStrip = this.contextMenu;
			this.richEnNew.Font = new System.Drawing.Font("Verdana", 8.25F);
			this.richEnNew.ImeMode = System.Windows.Forms.ImeMode.On;
			this.richEnNew.Location = new System.Drawing.Point(-1, 0);
			this.richEnNew.Margin = new System.Windows.Forms.Padding(0);
			this.richEnNew.Name = "richEnNew";
			this.richEnNew.ReadOnly = true;
			this.richEnNew.Size = new System.Drawing.Size(239, 211);
			this.richEnNew.TabIndex = 0;
			this.richEnNew.TabStop = false;
			this.richEnNew.Text = "";
			this.richEnNew.WordWrap = false;
			// 
			// richTranslation
			// 
			this.richUpdate.AcceptsTab = true;
			this.richUpdate.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.richUpdate.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.richUpdate.ContextMenuStrip = this.contextMenu;
			this.richUpdate.Font = new System.Drawing.Font("Verdana", 8.25F);
			this.richUpdate.ImeMode = System.Windows.Forms.ImeMode.On;
			this.richUpdate.Location = new System.Drawing.Point(0, 0);
			this.richUpdate.Margin = new System.Windows.Forms.Padding(0);
			this.richUpdate.Name = "richTranslation";
			this.richUpdate.Size = new System.Drawing.Size(480, 150);
			this.richUpdate.TabIndex = 0;
			this.richUpdate.TabStop = false;
			this.richUpdate.Text = "";
			this.richUpdate.WordWrap = false;
			this.richUpdate.TextChanged += new System.EventHandler(this.richTranslation_TextChanged);
			// 
			// AbstractGenericPageView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.tableBottomAll);
			this.Name = "AbstractGenericPageView";
			this.Size = new System.Drawing.Size(488, 403);
			this.tableBottomAll.ResumeLayout(false);
			this.splitHorizontal.Panel1.ResumeLayout(false);
			this.splitHorizontal.Panel2.ResumeLayout(false);
			this.splitHorizontal.ResumeLayout(false);
			this.splitVertical.Panel1.ResumeLayout(false);
			this.splitVertical.Panel2.ResumeLayout(false);
			this.splitVertical.ResumeLayout(false);
			this.contextMenu.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private TableLayoutPanel tableBottomAll;
		private SplitContainer splitHorizontal;
		private SplitContainer splitVertical;
		private ComboBox cmbElements;
		public XmlViewBox richEnOld;
		public XmlViewBox richEnNew;
		public XmlViewBox richUpdate;
		private ContextMenuStrip contextMenu;
		private ToolStripMenuItem undoItem;
		private ToolStripMenuItem redoItem;
		private ToolStripMenuItem cutItem;
		private ToolStripMenuItem copyItem;
		private ToolStripMenuItem pasteItem;
		private ToolStripMenuItem deleteItem;
		private ToolStripMenuItem selectAllItem;
	}
}
