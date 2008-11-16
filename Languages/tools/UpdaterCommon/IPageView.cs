using System;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;

namespace Updater.Common
{
	public interface INamedElement
	{
		string Id { get; }
	}

	interface IPageView
	{
		int ElementIndex { get; }
		string NewFilePath { get; set; }
		string OldFilePath { get; set; }
		string UpdatedFilePath { get; set; }
		Font UpdatePaneFont { get; set; }
		bool IsReadyForExport { get; }
		bool Dirty { get; set; }
		
		event EventHandler<PageViewErrorArgs> OnError;
		event EventHandler<ExportStatusChangedArgs> OnUpdate;
		void BeginResize();
		void UpdatePanes();

		bool AutoSaveUpdates();
	}
}
