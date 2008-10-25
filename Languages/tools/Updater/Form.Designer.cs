namespace ShareazaDialogUpdater
{
	using System;
	using System.Drawing;
	using System.Windows.Forms;
	using System.ComponentModel;
	
	partial class form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing) {
			if (disposing && (components != null)) {
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent() {
			this.components = new System.ComponentModel.Container();
			System.Windows.Forms.TableLayoutPanel tableTopAll;
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(form));
			System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
			System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
			System.Windows.Forms.TableLayoutPanel tableTopRight;
			System.Windows.Forms.TableLayoutPanel tableTopLeft;
			System.Windows.Forms.Label lblEnOld;
			System.Windows.Forms.Label lblEnNew;
			this.splitVertical = new System.Windows.Forms.SplitContainer();
			this.richEnOld = new XmlViewBox();
			this.contextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.undoItem = new System.Windows.Forms.ToolStripMenuItem();
			this.redoItem = new System.Windows.Forms.ToolStripMenuItem();
			this.cutItem = new System.Windows.Forms.ToolStripMenuItem();
			this.copyItem = new System.Windows.Forms.ToolStripMenuItem();
			this.pasteItem = new System.Windows.Forms.ToolStripMenuItem();
			this.deleteItem = new System.Windows.Forms.ToolStripMenuItem();
			this.selectAllItem = new System.Windows.Forms.ToolStripMenuItem();
			this.richEnNew = new XmlViewBox();
			this.txtEnNew = new System.Windows.Forms.TextBox();
			this.btnEnNew = new System.Windows.Forms.Button();
			this.txtEnOld = new System.Windows.Forms.TextBox();
			this.btnEnOld = new System.Windows.Forms.Button();
			this.tableBottomAll = new System.Windows.Forms.TableLayoutPanel();
			this.btnDoWork = new System.Windows.Forms.Button();
			this.splitHorizontal = new System.Windows.Forms.SplitContainer();
			this.richTranslation = new XmlViewBox();
			this.cmbDialogs = new System.Windows.Forms.ComboBox();
			this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.menuStrip = new System.Windows.Forms.MenuStrip();
			this.changeFontToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.fontDialog = new System.Windows.Forms.FontDialog();
			tableTopAll = new System.Windows.Forms.TableLayoutPanel();
			toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			tableTopRight = new System.Windows.Forms.TableLayoutPanel();
			tableTopLeft = new System.Windows.Forms.TableLayoutPanel();
			lblEnOld = new System.Windows.Forms.Label();
			lblEnNew = new System.Windows.Forms.Label();
			tableTopAll.SuspendLayout();
			this.splitVertical.Panel1.SuspendLayout();
			this.splitVertical.Panel2.SuspendLayout();
			this.splitVertical.SuspendLayout();
			this.contextMenu.SuspendLayout();
			tableTopRight.SuspendLayout();
			tableTopLeft.SuspendLayout();
			this.tableBottomAll.SuspendLayout();
			this.splitHorizontal.Panel1.SuspendLayout();
			this.splitHorizontal.Panel2.SuspendLayout();
			this.splitHorizontal.SuspendLayout();
			this.menuStrip.SuspendLayout();
			this.SuspendLayout();
			// 
			// tableTopAll
			// 
			resources.ApplyResources(tableTopAll, "tableTopAll");
			tableTopAll.Controls.Add(tableTopRight, 1, 1);
			tableTopAll.Controls.Add(tableTopLeft, 0, 1);
			tableTopAll.Controls.Add(lblEnOld, 0, 0);
			tableTopAll.Controls.Add(lblEnNew, 1, 0);
			tableTopAll.Name = "tableTopAll";
			// 
			// splitVertical
			// 
			this.splitVertical.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			resources.ApplyResources(this.splitVertical, "splitVertical");
			this.splitVertical.Name = "splitVertical";
			// 
			// splitVertical.Panel1
			// 
			this.splitVertical.Panel1.Controls.Add(this.richEnOld);
			resources.ApplyResources(this.splitVertical.Panel1, "splitVertical.Panel1");
			// 
			// splitVertical.Panel2
			// 
			this.splitVertical.Panel2.Controls.Add(this.richEnNew);
			resources.ApplyResources(this.splitVertical.Panel2, "splitVertical.Panel2");
			// 
			// richEnOld
			// 
			this.richEnOld.AcceptsTab = true;
			resources.ApplyResources(this.richEnOld, "richEnOld");
			this.richEnOld.BackColor = System.Drawing.SystemColors.Window;
			this.richEnOld.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.richEnOld.ContextMenuStrip = this.contextMenu;
			this.richEnOld.Name = "richEnOld";
			this.richEnOld.ReadOnly = true;
			this.richEnOld.TabStop = false;
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
			resources.ApplyResources(this.contextMenu, "contextMenu");
			this.contextMenu.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenu_Opening);
			this.contextMenu.Closing += new System.Windows.Forms.ToolStripDropDownClosingEventHandler(this.contextMenu_Closing);
			// 
			// undoItem
			// 
			resources.ApplyResources(this.undoItem, "undoItem");
			this.undoItem.Name = "undoItem";
			this.undoItem.Overflow = System.Windows.Forms.ToolStripItemOverflow.AsNeeded;
			this.undoItem.Click += new System.EventHandler(this.undoItem_Click);
			// 
			// redoItem
			// 
			this.redoItem.Name = "redoItem";
			resources.ApplyResources(this.redoItem, "redoItem");
			this.redoItem.Click += new System.EventHandler(this.redoItem_Click);
			// 
			// toolStripSeparator1
			// 
			toolStripSeparator1.Name = "toolStripSeparator1";
			resources.ApplyResources(toolStripSeparator1, "toolStripSeparator1");
			// 
			// cutItem
			// 
			this.cutItem.Name = "cutItem";
			resources.ApplyResources(this.cutItem, "cutItem");
			this.cutItem.Click += new System.EventHandler(this.cutItem_Click);
			// 
			// copyItem
			// 
			this.copyItem.Name = "copyItem";
			resources.ApplyResources(this.copyItem, "copyItem");
			this.copyItem.Click += new System.EventHandler(this.copyItem_Click);
			// 
			// pasteItem
			// 
			this.pasteItem.Name = "pasteItem";
			resources.ApplyResources(this.pasteItem, "pasteItem");
			this.pasteItem.Click += new System.EventHandler(this.pasteItem_Click);
			// 
			// deleteItem
			// 
			this.deleteItem.Name = "deleteItem";
			resources.ApplyResources(this.deleteItem, "deleteItem");
			this.deleteItem.Click += new System.EventHandler(this.deleteItem_Click);
			// 
			// toolStripSeparator2
			// 
			toolStripSeparator2.Name = "toolStripSeparator2";
			resources.ApplyResources(toolStripSeparator2, "toolStripSeparator2");
			// 
			// selectAllItem
			// 
			this.selectAllItem.Name = "selectAllItem";
			resources.ApplyResources(this.selectAllItem, "selectAllItem");
			this.selectAllItem.Click += new System.EventHandler(this.selectAllItem_Click);
			// 
			// richEnNew
			// 
			this.richEnNew.AcceptsTab = true;
			resources.ApplyResources(this.richEnNew, "richEnNew");
			this.richEnNew.BackColor = System.Drawing.SystemColors.Window;
			this.richEnNew.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.richEnNew.ContextMenuStrip = this.contextMenu;
			this.richEnNew.Name = "richEnNew";
			this.richEnNew.ReadOnly = true;
			this.richEnNew.TabStop = false;
			// 
			// tableTopRight
			// 
			resources.ApplyResources(tableTopRight, "tableTopRight");
			tableTopRight.Controls.Add(this.txtEnNew, 0, 0);
			tableTopRight.Controls.Add(this.btnEnNew, 1, 0);
			tableTopRight.Name = "tableTopRight";
			// 
			// txtEnNew
			// 
			resources.ApplyResources(this.txtEnNew, "txtEnNew");
			this.txtEnNew.Name = "txtEnNew";
			this.txtEnNew.ReadOnly = true;
			// 
			// btnEnNew
			// 
			resources.ApplyResources(this.btnEnNew, "btnEnNew");
			this.btnEnNew.Name = "btnEnNew";
			this.btnEnNew.UseVisualStyleBackColor = true;
			this.btnEnNew.Click += new System.EventHandler(this.btnEnNew_Click);
			// 
			// tableTopLeft
			// 
			resources.ApplyResources(tableTopLeft, "tableTopLeft");
			tableTopLeft.Controls.Add(this.txtEnOld, 0, 0);
			tableTopLeft.Controls.Add(this.btnEnOld, 1, 0);
			tableTopLeft.Name = "tableTopLeft";
			// 
			// txtEnOld
			// 
			resources.ApplyResources(this.txtEnOld, "txtEnOld");
			this.txtEnOld.Name = "txtEnOld";
			this.txtEnOld.ReadOnly = true;
			// 
			// btnEnOld
			// 
			resources.ApplyResources(this.btnEnOld, "btnEnOld");
			this.btnEnOld.Name = "btnEnOld";
			this.btnEnOld.UseVisualStyleBackColor = true;
			this.btnEnOld.Click += new System.EventHandler(this.btnEnOld_Click);
			// 
			// lblEnOld
			// 
			resources.ApplyResources(lblEnOld, "lblEnOld");
			lblEnOld.Name = "lblEnOld";
			// 
			// lblEnNew
			// 
			resources.ApplyResources(lblEnNew, "lblEnNew");
			lblEnNew.Name = "lblEnNew";
			// 
			// tableBottomAll
			// 
			resources.ApplyResources(this.tableBottomAll, "tableBottomAll");
			this.tableBottomAll.Controls.Add(this.btnDoWork, 0, 2);
			this.tableBottomAll.Controls.Add(this.splitHorizontal, 0, 0);
			this.tableBottomAll.Controls.Add(this.cmbDialogs, 1, 2);
			this.tableBottomAll.Name = "tableBottomAll";
			// 
			// btnDoWork
			// 
			resources.ApplyResources(this.btnDoWork, "btnDoWork");
			this.btnDoWork.Name = "btnDoWork";
			this.btnDoWork.UseVisualStyleBackColor = true;
			this.btnDoWork.Click += new System.EventHandler(this.btnDoWork_Click);
			// 
			// splitHorizontal
			// 
			this.splitHorizontal.BackColor = System.Drawing.SystemColors.Window;
			this.splitHorizontal.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.tableBottomAll.SetColumnSpan(this.splitHorizontal, 2);
			resources.ApplyResources(this.splitHorizontal, "splitHorizontal");
			this.splitHorizontal.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
			this.splitHorizontal.Name = "splitHorizontal";
			// 
			// splitHorizontal.Panel1
			// 
			this.splitHorizontal.Panel1.Controls.Add(this.splitVertical);
			// 
			// splitHorizontal.Panel2
			// 
			this.splitHorizontal.Panel2.Controls.Add(this.richTranslation);
			this.tableBottomAll.SetRowSpan(this.splitHorizontal, 2);
			// 
			// richTranslation
			// 
			this.richTranslation.AcceptsTab = true;
			resources.ApplyResources(this.richTranslation, "richTranslation");
			this.richTranslation.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.richTranslation.ContextMenuStrip = this.contextMenu;
			this.richTranslation.Name = "richTranslation";
			this.richTranslation.TabStop = false;
			this.richTranslation.TextChanged += new System.EventHandler(this.richTranslation_TextChanged);
			// 
			// cmbDialogs
			// 
			resources.ApplyResources(this.cmbDialogs, "cmbDialogs");
			this.cmbDialogs.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cmbDialogs.FormattingEnabled = true;
			this.cmbDialogs.Name = "cmbDialogs";
			this.cmbDialogs.SelectedIndexChanged += new System.EventHandler(this.cmbDialogs_SelectedIndexChanged);
			// 
			// openFileDialog
			// 
			resources.ApplyResources(this.openFileDialog, "openFileDialog");
			// 
			// menuStrip
			// 
			this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.changeFontToolStripMenuItem});
			resources.ApplyResources(this.menuStrip, "menuStrip");
			this.menuStrip.Name = "menuStrip";
			// 
			// changeFontToolStripMenuItem
			// 
			this.changeFontToolStripMenuItem.Name = "changeFontToolStripMenuItem";
			resources.ApplyResources(this.changeFontToolStripMenuItem, "changeFontToolStripMenuItem");
			this.changeFontToolStripMenuItem.Click += new System.EventHandler(this.changeFontToolStripMenuItem_Click);
			// 
			// form
			// 
			resources.ApplyResources(this, "$this");
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.menuStrip);
			this.Controls.Add(tableTopAll);
			this.Controls.Add(this.tableBottomAll);
			this.DoubleBuffered = true;
			this.KeyPreview = true;
			this.MainMenuStrip = this.menuStrip;
			this.Name = "form";
			this.TopMost = true;
			this.ResizeBegin += new System.EventHandler(this.form_ResizeBegin);
			this.Resize += new System.EventHandler(this.form_Resize);
			tableTopAll.ResumeLayout(false);
			tableTopAll.PerformLayout();
			this.splitVertical.Panel1.ResumeLayout(false);
			this.splitVertical.Panel2.ResumeLayout(false);
			this.splitVertical.ResumeLayout(false);
			this.contextMenu.ResumeLayout(false);
			tableTopRight.ResumeLayout(false);
			tableTopRight.PerformLayout();
			tableTopLeft.ResumeLayout(false);
			tableTopLeft.PerformLayout();
			this.tableBottomAll.ResumeLayout(false);
			this.splitHorizontal.Panel1.ResumeLayout(false);
			this.splitHorizontal.Panel2.ResumeLayout(false);
			this.splitHorizontal.ResumeLayout(false);
			this.menuStrip.ResumeLayout(false);
			this.menuStrip.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private SplitContainer splitHorizontal;
		private SplitContainer splitVertical;
		private TextBox txtEnOld;
		private Button btnEnOld;
		private TextBox txtEnNew;
		private Button btnEnNew;
		private OpenFileDialog openFileDialog;
		private Button btnDoWork;
		private TableLayoutPanel tableBottomAll;
		private ComboBox cmbDialogs;
		private XmlViewBox richTranslation;
		private XmlViewBox richEnOld;
		private XmlViewBox richEnNew;
		private ContextMenuStrip contextMenu;
		private ToolStripMenuItem undoItem;
		private ToolStripMenuItem cutItem;
		private ToolStripMenuItem copyItem;
		private ToolStripMenuItem pasteItem;
		private ToolStripMenuItem deleteItem;
		private ToolStripMenuItem selectAllItem;
		private ToolStripMenuItem redoItem;
		private MenuStrip menuStrip;
		private ToolStripMenuItem changeFontToolStripMenuItem;
		private FontDialog fontDialog;
	}
}

