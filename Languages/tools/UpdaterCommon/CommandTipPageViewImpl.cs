using System;
using System.Linq;

namespace Updater.Common
{
	public partial class CommandTipPageViewImpl : AbstractGenericPageView<skinCommandTip>
	{
		public CommandTipPageViewImpl() {
			InitializeComponent();
		}

		protected override void UpdateElement(string elementId, out skinCommandTip element) {
			element = null;
			skinCommandTip newEn = newEnList.FirstOrDefault(d => d.id == elementId);
			skinCommandTip tr = updatesList.FirstOrDefault(d => d.id == elementId);

			if (tr == null) {
				updatesList.Add(newEn);
				element = newEn;
				return;
			} else if (oldEnList != null) {
				skinCommandTip updatedElement = newEn.Clone();
				skinCommandTip oldEn = oldEnList.FirstOrDefault(d => d.id == elementId);
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
			skinCommandTip currTip = newEnList[ElementIndex];

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
						if (testSkin.commandTips == null || testSkin.commandTips.Length == 0) {
							base.SetError(Settings.Default.InvalidXml);
							return false;
						}
						if (currTip.id != testSkin.commandTips[0].id) {
							base.SetError(Settings.Default.InvalidId);
							return false;
						}
						if (testSkin.commandTips[0].Text != null) {
							base.SetError(Settings.Default.TextJunk);
							return false;
						}
						if (testSkin.commandTips[0].junk != null) {
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
