using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows.Forms;

namespace Updater.Common
{
	public partial class LanguageSelection : Form
	{
		Dictionary<int, string> langMap = new Dictionary<int, string>();

		public LanguageSelection() {
			InitializeComponent();
			var cultures = CultureInfo.GetCultures(CultureTypes.SpecificCultures);
			var cultureInfos = (from c in cultures
								select new { Name = c.NativeName, Code = c.Name }).OrderBy(i => i.Name);
			foreach (var info in cultureInfos) {
				string firstChar = info.Name[0].ToString().ToUpper(CultureInfo.GetCultureInfo(info.Code));
				int idx = cmbLanguage.Items.Add(firstChar + info.Name.Substring(1));
				langMap.Add(idx, info.Code);
			}
		}

		private void LanguageSelection_Load(object sender, EventArgs e) {
			cmbLanguage.SelectedIndex = 0;
		}

		public string GetSelectedLangCode() {
			return langMap[cmbLanguage.SelectedIndex];
		}
	}
}
