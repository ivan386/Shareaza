using System;
using System.Linq;

namespace Updater.Common
{
	public partial class DialogPageViewImpl : AbstractGenericPageView<skinDialog>
	{
		public DialogPageViewImpl() {
			InitializeComponent();
		}

		protected override void UpdateElement(string elementId, out skinDialog dialog) {
			dialog = null;

			// Always not null
			skinDialog newEn = newEnList.FirstOrDefault(d => d.name == elementId);
			// Can be null
			skinDialog tr = updatesList.FirstOrDefault(d => d.name == elementId);
			if (tr == null) {
				updatesList.Add(newEn);
				dialog = newEn;
				return;
			} else if (oldEnList != null) {
				skinDialog oldEn = oldEnList.FirstOrDefault(d => d.name == elementId);
				int oldControlCount = oldEn == null || oldEn.controls == null ? 0 : oldEn.controls.Length;
				int trControlCount = tr.controls == null ? 0 : tr.controls.Length;
				if (oldEn == null || oldControlCount != trControlCount) {
					base.SetError(Settings.Default.UpdateMismatch);
					dialog = tr;
					return;
				}
				skinDialog updatedDialog = newEn.Clone();
				// Update caption
				if (newEn.caption != oldEn.caption)
					updatedDialog.caption = newEn.caption;
				else
					updatedDialog.caption = tr.caption;

				if (newEn.controls != null) {
					for (int i = 0; i < newEn.controls.Length; i++) {
						if (String.IsNullOrEmpty(newEn.controls[i].caption))
							continue; // remains the same in the cloned object
						// Well, it would be worth to check for changed spaces, since the
						// translation wouldn't change
						var oldMatches = oldEn.controls.Where(c => c.caption == newEn.controls[i].caption);
						if (!oldMatches.Any())
							continue;
						int oldIndex = Array.IndexOf(oldEn.controls, oldMatches.First());
						updatedDialog.controls[i] = tr.controls[oldIndex];
					}
				}
				dialog = updatedDialog;
			}
		}

		public override bool AutoSaveUpdates() {
			if (ElementIndex < 0 || !base.Dirty && String.IsNullOrEmpty(richUpdate.Text)) // can not be empty
				return true;
			string xml = richUpdate.Xml;
			skinDialog currDialog = newEnList[ElementIndex];

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
						if (testSkin.dialogs == null || testSkin.dialogs.Length == 0) {
							base.SetError(Settings.Default.InvalidXml);
							return false;
						}
						if (currDialog.cookie != testSkin.dialogs[0].cookie) {
							base.SetError(Settings.Default.InvalidCookie);
							return false;
						}
						if (currDialog.name != testSkin.dialogs[0].name) {
							base.SetError(Settings.Default.InvalidDialogName);
							return false;
						}
						if (!String.IsNullOrEmpty(currDialog.caption) &&
							String.IsNullOrEmpty(testSkin.dialogs[0].caption)) {
							base.SetError(Settings.Default.MissingCaption);
							return false;
						}
						if (testSkin.dialogs[0].Text != null) {
							base.SetError(Settings.Default.TextJunk);
							return false;
						}
						if (testSkin.dialogs[0].junk != null) {
							base.SetError(Settings.Default.ExtraDialogText);
							return false;
						}
						int count = currDialog.controls == null ? 0 : currDialog.controls.Length;
						int trCount = testSkin.dialogs[0].controls == null ? 0 : testSkin.dialogs[0].controls.Length;
						if (trCount != count) {
							base.SetError(Settings.Default.InvalidControlCount);
							return false;
						}
						for (int i = 0; i < count; i++) {
							if (testSkin.dialogs[0].controls[i].junk != null) {
								base.SetError(String.Format(Settings.Default.ExtraControlAttributes, i + 1));
								return false;
							}
							if (currDialog.controls[i].caption == String.Empty &&
								testSkin.dialogs[0].controls[i].caption != String.Empty ||
								currDialog.controls[i].caption != String.Empty &&
								testSkin.dialogs[0].controls[i].caption == String.Empty) {
								base.SetError(String.Format(Settings.Default.InvalidControl, i + 1));
								return false;
							}
						}
						base.Dirty = false;
					}
				}
			}

			if (!updatedXmlDic.ContainsKey(currDialog.name))
				updatedXmlDic.Add(currDialog.name, xml);
			else
				updatedXmlDic[currDialog.name] = xml;
			return true;
		}
	}
}
