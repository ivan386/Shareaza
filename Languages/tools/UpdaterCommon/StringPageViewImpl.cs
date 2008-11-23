using System;
using System.Linq;

namespace Updater.Common
{
	public partial class StringPageViewImpl : AbstractGenericPageView<skinString>
	{
		public StringPageViewImpl() {
			InitializeComponent();
		}

		protected override void UpdateElement(string elementId, out skinString element) {
			element = null;
			skinString newEn = newEnList.FirstOrDefault(d => d.id == elementId);
			skinString tr = updatesList.FirstOrDefault(d => d.id == elementId);
			
			if (tr == null) {
				updatesList.Add(newEn);
				element = newEn;
				return;
			} else if (oldEnList != null) {
				skinString updatedElement = newEn.Clone();
				skinString oldEn = oldEnList.FirstOrDefault(d => d.id == elementId);
				if (oldEn == null) {
					base.SetError(Settings.Default.UpdateMismatch);
					element = tr;
					return;
				}				
				if (newEn.value != oldEn.value)
					updatedElement.value = newEn.value;
				else
					updatedElement.value = tr.value;
				element = updatedElement;		
			}		
		}

		public override bool AutoSaveUpdates() {
			if (ElementIndex < 0 || !base.Dirty && String.IsNullOrEmpty(richUpdate.Text)) // can not be empty
				return true;
			string xml = richUpdate.Xml;
			skinString currString = newEnList[ElementIndex];

			if (base.Dirty) {
				if (String.IsNullOrEmpty(xml)) {
					base.SetError(richUpdate.XmlError);
					return false;
				} else {
					Exception exeption;
					skin testSkin = XmlSerializerBase<skin>.ReadString(String.Format(Envelope, xml),
																	   out exeption);
					if (testSkin == null || exeption != null) {
						base.SetError(exeption.InnerException != null ? exeption.InnerException.Message :
																		exeption.Message);
						return false;
					} else {
						if (testSkin.strings == null || testSkin.strings.Length == 0) {
							base.SetError(Settings.Default.InvalidXml);
							return false;
						}
						if (currString.id != testSkin.strings[0].id) {
							base.SetError(Settings.Default.InvalidId);
							return false;
						}
						if (testSkin.strings[0].Text != null) {
							base.SetError(Settings.Default.TextJunk);
							return false;
						}
						if (testSkin.strings[0].junk != null) {
							base.SetError(Settings.Default.ExtraText);
							return false;
						}
						base.Dirty = false;											
					}
				}			
			}
			
			if (!updatedXmlDic.ContainsKey(currString.id))
				updatedXmlDic.Add(currString.id, xml);
			else
				updatedXmlDic[currString.id] = xml;
			return true;
		}
	}
}
