using System;

namespace Updater.Common
{
	public enum FileType
	{
		OldEnglish,
		NewEnglish,
		Updated
	}

	public sealed class PageViewErrorArgs : EventArgs
	{
		public PageViewErrorArgs(string errorMessage) {
			this.Error = errorMessage;
		}

		public string Error { get; set; }
	}

	public sealed class ExportStatusChangedArgs : EventArgs
	{
		public ExportStatusChangedArgs(bool isReadyForExport) {
			this.IsReadyForExport = isReadyForExport;
		}

		public bool IsReadyForExport { get; set; }
	}	
}