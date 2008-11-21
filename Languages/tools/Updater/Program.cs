using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Xml.Linq;
using System.Threading;

namespace Updater.Common
{
	static class Program
	{
		[STAThread]
		static void Main() {
			Application.ThreadException += new ThreadExceptionEventHandler(OnThreadException);
			AppDomain.CurrentDomain.UnhandledException +=
				new UnhandledExceptionEventHandler(OnUnhandledException);
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Form form = new MainForm();
			Application.Run(form);
		}

		static void OnUnhandledException(object sender, UnhandledExceptionEventArgs args) {
			if (args.IsTerminating) {
				Console.WriteLine(((Exception)args.ExceptionObject).StackTrace);
			}
		}

		static void OnThreadException(object sender, ThreadExceptionEventArgs args) {
			if (SystemInformation.UserInteractive) {
				using (ThreadExceptionDialog dialog = new ThreadExceptionDialog(args.Exception)) {
					if (dialog.ShowDialog() == DialogResult.Cancel)
						return;
				}
				Application.Exit();
				Environment.Exit(0);
			}
		}
		
		static bool bMono = Type.GetType("Mono.Runtime") != null;
		
		public static bool IsRunningInMono {
			get { return bMono; }
		}
	}
}
