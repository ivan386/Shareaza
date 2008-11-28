using System;
using System.Linq;

namespace Updater.Common
{
	public partial class ControlTipPageViewImpl : AbstractGenericPageView<skinControlTip>
	{
		public ControlTipPageViewImpl() {
			InitializeComponent();
		}

		protected override void UpdateElement(string elementId, out skinControlTip element) {
			element = null;
			skinControlTip newEn = newEnList.FirstOrDefault(d => d.id == elementId);
			skinControlTip tr = updatesList.FirstOrDefault(d => d.id == elementId);

			if (tr == null) {
				updatesList.Add(newEn);
				element = newEn;
				return;
			} else if (oldEnList != null) {
				skinControlTip updatedElement = newEn.Clone();
				skinControlTip oldEn = oldEnList.FirstOrDefault(d => d.id == elementId);
				if (oldEn == null) {
					base.SetError(Settings.Default.UpdateMismatch);
					element = tr;
					return;
				}				
				if (newEn.message != oldEn.message)
					updatedElement.message = newEn.message;
				else
					updatedElement.message = tr.message;
				element = updatedElement;
			}
		}

		public override bool AutoSaveUpdates() {
			if (ElementIndex < 0 || !base.Dirty && String.IsNullOrEmpty(richUpdate.Text)) // can not be empty
				return true;
			string xml = richUpdate.Xml;
			skinControlTip currTip = newEnList[ElementIndex];

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
						if (testSkin.controlTips == null || testSkin.controlTips.Length == 0) {
							base.SetError(Settings.Default.InvalidXml);
							return false;
						}
						if (currTip.id != testSkin.controlTips[0].id) {
							base.SetError(Settings.Default.InvalidId);
							return false;
						}
						if (testSkin.controlTips[0].Text != null) {
							base.SetError(Settings.Default.TextJunk);
							return false;
						}
						if (testSkin.controlTips[0].junk != null) {
							base.SetError(Settings.Default.ExtraText);
							return false;
						}
						base.Dirty = false;
					}
				}
			}

			if (!updatedXmlDic.ContainsKey(currTip.id))
				updatedXmlDic.Add(currTip.id, xml);
			else
				updatedXmlDic[currTip.id] = xml;
			return true;
		}		
	}
}
