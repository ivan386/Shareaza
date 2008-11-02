namespace ShareazaDialogUpdater
{
	partial class LanguageSelection
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

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
			System.Windows.Forms.Label lblText;
			System.Windows.Forms.Button btnOK;
			System.Windows.Forms.Button btnCancel;
			this.cmbLanguage = new System.Windows.Forms.ComboBox();
			lblText = new System.Windows.Forms.Label();
			btnOK = new System.Windows.Forms.Button();
			btnCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// lblText
			// 
			lblText.AutoSize = true;
			lblText.Location = new System.Drawing.Point(12, 9);
			lblText.Name = "lblText";
			lblText.Size = new System.Drawing.Size(262, 26);
			lblText.TabIndex = 0;
			lblText.Text = global::ShareazaDialogUpdater.Properties.Settings.Default.SelectLanguageExplanation;
			// 
			// btnOK
			// 
			btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			btnOK.Location = new System.Drawing.Point(75, 79);
			btnOK.Name = "btnOK";
			btnOK.Size = new System.Drawing.Size(54, 23);
			btnOK.TabIndex = 2;
			btnOK.Text = global::ShareazaDialogUpdater.Properties.Settings.Default.ButtonOk;
			btnOK.UseVisualStyleBackColor = true;
			// 
			// btnCancel
			// 
			btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			btnCancel.Location = new System.Drawing.Point(149, 79);
			btnCancel.Name = "btnCancel";
			btnCancel.Size = new System.Drawing.Size(58, 23);
			btnCancel.TabIndex = 3;
			btnCancel.Text = global::ShareazaDialogUpdater.Properties.Settings.Default.ButtonCancel;
			btnCancel.UseVisualStyleBackColor = true;
			// 
			// cmbLanguage
			// 
			this.cmbLanguage.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cmbLanguage.FormattingEnabled = true;
			this.cmbLanguage.Location = new System.Drawing.Point(12, 52);
			this.cmbLanguage.Name = "cmbLanguage";
			this.cmbLanguage.Size = new System.Drawing.Size(262, 21);
			this.cmbLanguage.Sorted = true;
			this.cmbLanguage.TabIndex = 1;
			// 
			// LanguageSelection
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(287, 110);
			this.ControlBox = false;
			this.Controls.Add(btnCancel);
			this.Controls.Add(btnOK);
			this.Controls.Add(this.cmbLanguage);
			this.Controls.Add(lblText);
			this.DataBindings.Add(new System.Windows.Forms.Binding("Text", global::ShareazaDialogUpdater.Properties.Settings.Default, "SelectLanguage", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "LanguageSelection";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = global::ShareazaDialogUpdater.Properties.Settings.Default.SelectLanguage;
			this.TopMost = true;
			this.Load += new System.EventHandler(this.LanguageSelection_Load);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ComboBox cmbLanguage;
	}
}