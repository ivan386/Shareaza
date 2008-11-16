namespace Updater.Common
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
			System.Windows.Forms.TableLayoutPanel tableTopAll;
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(form));
			System.Windows.Forms.TableLayoutPanel tableTopRight;
			System.Windows.Forms.TableLayoutPanel tableTopLeft;
			System.Windows.Forms.Label lblEnOld;
			System.Windows.Forms.Label lblEnNew;
			System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
			System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
			System.Windows.Forms.StatusStrip statusStrip;
			this.txtEnNew = new System.Windows.Forms.TextBox();
			this.btnEnNew = new System.Windows.Forms.Button();
			this.txtEnOld = new System.Windows.Forms.TextBox();
			this.btnEnOld = new System.Windows.Forms.Button();
			this.lblStatus = new System.Windows.Forms.ToolStripStatusLabel();
			this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.menuStrip = new System.Windows.Forms.MenuStrip();
			this.changeFontToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.exportChangesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.fontDialog = new System.Windows.Forms.FontDialog();
			this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
			this.btnDoWork = new System.Windows.Forms.Button();
			this.editor = new Updater.Common.EditorTabControl();
			tableTopAll = new System.Windows.Forms.TableLayoutPanel();
			tableTopRight = new System.Windows.Forms.TableLayoutPanel();
			tableTopLeft = new System.Windows.Forms.TableLayoutPanel();
			lblEnOld = new System.Windows.Forms.Label();
			lblEnNew = new System.Windows.Forms.Label();
			toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			statusStrip = new System.Windows.Forms.StatusStrip();
			tableTopAll.SuspendLayout();
			tableTopRight.SuspendLayout();
			tableTopLeft.SuspendLayout();
			statusStrip.SuspendLayout();
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
			lblEnOld.Text = global::Updater.Common.Properties.Settings.Default.StepOneExplanation;
			// 
			// lblEnNew
			// 
			resources.ApplyResources(lblEnNew, "lblEnNew");
			lblEnNew.Name = "lblEnNew";
			lblEnNew.Text = global::Updater.Common.Properties.Settings.Default.StepTwoExplanation;
			// 
			// toolStripSeparator1
			// 
			toolStripSeparator1.Name = "toolStripSeparator1";
			resources.ApplyResources(toolStripSeparator1, "toolStripSeparator1");
			// 
			// toolStripSeparator2
			// 
			toolStripSeparator2.Name = "toolStripSeparator2";
			resources.ApplyResources(toolStripSeparator2, "toolStripSeparator2");
			// 
			// statusStrip
			// 
			statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.lblStatus});
			resources.ApplyResources(statusStrip, "statusStrip");
			statusStrip.Name = "statusStrip";
			// 
			// lblStatus
			// 
			this.lblStatus.ForeColor = System.Drawing.Color.Red;
			this.lblStatus.Margin = new System.Windows.Forms.Padding(10, 3, 0, 2);
			this.lblStatus.Name = "lblStatus";
			resources.ApplyResources(this.lblStatus, "lblStatus");
			this.lblStatus.Spring = true;
			// 
			// openFileDialog
			// 
			resources.ApplyResources(this.openFileDialog, "openFileDialog");
			// 
			// menuStrip
			// 
			this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.changeFontToolStripMenuItem,
            this.exportChangesToolStripMenuItem});
			resources.ApplyResources(this.menuStrip, "menuStrip");
			this.menuStrip.Name = "menuStrip";
			// 
			// changeFontToolStripMenuItem
			// 
			this.changeFontToolStripMenuItem.Name = "changeFontToolStripMenuItem";
			resources.ApplyResources(this.changeFontToolStripMenuItem, "changeFontToolStripMenuItem");
			this.changeFontToolStripMenuItem.Text = global::Updater.Common.Properties.Settings.Default.ChangeFont;
			this.changeFontToolStripMenuItem.Click += new System.EventHandler(this.changeFontToolStripMenuItem_Click);
			// 
			// exportChangesToolStripMenuItem
			// 
			resources.ApplyResources(this.exportChangesToolStripMenuItem, "exportChangesToolStripMenuItem");
			this.exportChangesToolStripMenuItem.Name = "exportChangesToolStripMenuItem";
			this.exportChangesToolStripMenuItem.Text = global::Updater.Common.Properties.Settings.Default.ExportChanges;
			this.exportChangesToolStripMenuItem.Click += new System.EventHandler(this.exportChangesToolStripMenuItem_Click);
			// 
			// saveFileDialog
			// 
			this.saveFileDialog.DefaultExt = "xml";
			resources.ApplyResources(this.saveFileDialog, "saveFileDialog");
			// 
			// btnDoWork
			// 
			resources.ApplyResources(this.btnDoWork, "btnDoWork");
			this.btnDoWork.Name = "btnDoWork";
			this.btnDoWork.Text = global::Updater.Common.Properties.Settings.Default.StepThreeExplanation;
			this.btnDoWork.UseVisualStyleBackColor = true;
			this.btnDoWork.Click += new System.EventHandler(this.btnDoWork_Click);
			// 
			// editor
			// 
			resources.ApplyResources(this.editor, "editor");
			this.editor.Name = "editor";
			this.editor.NewFilePath = "";
			this.editor.OldFilePath = "";
			this.editor.UpdatedFilePath = null;
			this.editor.UpdatePaneFont = new System.Drawing.Font("Verdana", 8.25F);
			// 
			// form
			// 
			resources.ApplyResources(this, "$this");
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.btnDoWork);
			this.Controls.Add(this.editor);
			this.Controls.Add(statusStrip);
			this.Controls.Add(this.menuStrip);
			this.Controls.Add(tableTopAll);
			this.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::Updater.Common.Properties.Settings.Default, "FormTitle", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
			this.DoubleBuffered = true;
			this.MainMenuStrip = this.menuStrip;
			this.Name = "form";
			this.Text = global::Updater.Common.Properties.Settings.Default.FormTitle;
			this.ResizeBegin += new System.EventHandler(this.form_ResizeBegin);
			tableTopAll.ResumeLayout(false);
			tableTopAll.PerformLayout();
			tableTopRight.ResumeLayout(false);
			tableTopRight.PerformLayout();
			tableTopLeft.ResumeLayout(false);
			tableTopLeft.PerformLayout();
			statusStrip.ResumeLayout(false);
			statusStrip.PerformLayout();
			this.menuStrip.ResumeLayout(false);
			this.menuStrip.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private TextBox txtEnOld;
		private Button btnEnOld;
		private TextBox txtEnNew;
		private Button btnEnNew;
		private OpenFileDialog openFileDialog;
		private MenuStrip menuStrip;
		private ToolStripMenuItem changeFontToolStripMenuItem;
		private FontDialog fontDialog;
		private ToolStripStatusLabel lblStatus;
		private ToolStripMenuItem exportChangesToolStripMenuItem;
		private SaveFileDialog saveFileDialog;
		private Button btnDoWork;
		private EditorTabControl editor;
	}
}

