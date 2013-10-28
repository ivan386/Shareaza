//
// FictionBookReader.cs
//
// Copyright (c) Shareaza Development Team, 2008.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#region Using directives
using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Serialization;
using Microsoft.Win32;
using Schemas;
using Enc = System.Drawing.Imaging.Encoder;

#endregion

namespace Shareaza
{
	[Guid("1B5C019B-CAD0-4590-9264-870284468393")]
	[ClassInterface(ClassInterfaceType.None)] // Explicit interface
	// [ProgId("Shareaza.FictionBookReader")]
	public class FictionBookReader : ILibraryBuilderPlugin, IImageServicePlugin
	{
		static readonly string builderPath = @"SOFTWARE\Shareaza\Shareaza\Plugins\LibraryBuilder";
		static readonly string imgServPath = @"SOFTWARE\Shareaza\Shareaza\Plugins\ImageService";
		static readonly string readerGuid = @"{1B5C019B-CAD0-4590-9264-870284468393}";

		#region ILibraryBuilderPlugin Members

		[SecurityPermission(SecurityAction.Demand)]
		public void Process(IntPtr hFile, string sFile, ISXMLElement pXML) {
			bool success = true;
			try {
				success = (Path.GetExtension(sFile) == @".fb2") && hFile != IntPtr.Zero;
			} catch { }
			if (!success)
				Marshal.ThrowExceptionForHR(Hresults.E_INVALIDARG);

			XmlReader reader = null;
			FictionBook book = null;
			Stream fs = null;
			try {
				XmlSerializer s = new XmlSerializer(typeof(FictionBook));
				s.UnknownNode += new XmlNodeEventHandler(OnUnknownNode);
				s.UnknownAttribute += new XmlAttributeEventHandler(OnUnknownAttribute);
				fs = new FileStream(sFile, FileMode.Open, FileAccess.Read);
				reader = new XmlTextReader(fs);
				book = (FictionBook)s.Deserialize(reader);
			} catch (Exception e) {
				Trace.WriteLine(e.Message);
				Exception inner = e.InnerException;
				while (inner != null) {
					Trace.WriteLine(inner.Message);
					inner = inner.InnerException;
				}
			} finally {
				if (reader != null) reader.Close();
				if (fs != null) fs.Close();
			}

			if (book == null) {
				Marshal.ThrowExceptionForHR(Hresults.E_FAIL);
			}

			int result = Hresults.S_OK;
			try {
				CreateShareazaXml(book, ref pXML);
			} catch (Exception e) {
				Trace.WriteLine(e.Message);
				Exception inner = e.InnerException;
				while (inner != null) {
					Trace.WriteLine(inner.Message);
					inner = inner.InnerException;
				}
				result = Marshal.GetHRForException(e);
			}

			if (result != 0) {
				Trace.WriteLine("Error HRESULT=" + result.ToString());
				Marshal.ThrowExceptionForHR(result);
			}
		}

		#endregion

		#region Routine of FictionBook XML convertion to Shareaza format

		void CreateShareazaXml(FictionBook book, ref ISXMLElement pXML) {
			ShareazaBook sbook = new ShareazaBook(book);
			Books sBooks = new Books();
			sBooks.books[0] = sbook;

			MemoryStream ms = null;
			XmlWriter writer = null;
			string finalXml = String.Empty;

			try {
				XmlSerializer s = new XmlSerializer(typeof(Books));
				s.UnknownNode += new XmlNodeEventHandler(OnUnknownNode);
				s.UnknownAttribute += new XmlAttributeEventHandler(OnUnknownAttribute);

				ms = new MemoryStream();

				UTF8Encoding utf8 = new UTF8Encoding(false, false);
				writer = new XmlTextWriter(ms, utf8);

				XmlSerializerNamespaces xsn = new XmlSerializerNamespaces();
				// Don't add any prefixes
				xsn.Add("xsi", "http://www.w3.org/2001/XMLSchema-instance");
				s.Serialize(writer, sBooks, xsn);

				// Start modifying the resulting XML
				XmlDocument doc = new XmlDocument();
				ms.Position = 0;
				doc.Load(ms);
				XmlAttribute schema = doc.CreateAttribute("xsi", "noNamespaceSchemaLocation",
														  "http://www.w3.org/2001/XMLSchema-instance");
				schema.Value = Books.URI;
				doc.DocumentElement.SetAttributeNode(schema);
				// Truncate the serialization result and overwrite it with our modified XML
				ms.SetLength(0);
				ms.Position = 0;
				writer = new XmlTextWriter(ms, utf8);
				doc.Save(writer);

				char[] buffer = Encoding.UTF8.GetChars(ms.ToArray());
				finalXml = new string(buffer);
			} catch (Exception e) {
				Trace.WriteLine(e.Message);
				Exception inner = e.InnerException;
				while (inner != null) {
					Trace.WriteLine(inner.Message);
					inner = inner.InnerException;
				}
			} finally {
				if (writer != null) writer.Close();
				if (ms != null) ms.Close();
			}
			if (!String.IsNullOrEmpty(finalXml)) {
				Trace.WriteLine(finalXml);
				ISXMLElement newXML = pXML.FromString(finalXml);
				if (newXML != null) {
					pXML.Elements.Attach(newXML);
				}
			}
		}

		#endregion

		#region XML reading events (Debug)

		protected void OnUnknownNode(object sender, XmlNodeEventArgs e) {
			Trace.WriteLine("Unknown Node:" + e.Name + "\t" + e.Text);
		}

		protected void OnUnknownAttribute(object sender, XmlAttributeEventArgs e) {
			XmlAttribute attr = e.Attr;
			Trace.WriteLine("Unknown attribute " + attr.Name + "='" + attr.Value + "'");
		}

		#endregion

		#region COM registration

		[ComRegisterFunction()]
		public static void RegisterFunction(Type t) {
			try {
				using (RegistryKey regHKLM = Registry.LocalMachine) {
					using (RegistryKey subKey = regHKLM.CreateSubKey(builderPath)) {
						subKey.SetValue(@".fb2", readerGuid);
					}
					using (RegistryKey subKey = regHKLM.CreateSubKey(imgServPath)) {
						subKey.SetValue(@".fb2", readerGuid);
					}
				}
			} catch (Exception ex) {
				MessageBox.Show(ex.Message, "Cannot Register",
								MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		[ComUnregisterFunction()]
		public static void UnregisterFunction(Type t) {
			try {
				using (RegistryKey regHKLM = Registry.LocalMachine) {
					using (RegistryKey subKey = regHKLM.OpenSubKey(builderPath)) {
						subKey.DeleteValue(@".fb2");
					}
					using (RegistryKey subKey = regHKLM.OpenSubKey(imgServPath)) {
						subKey.DeleteValue(@".fb2");
					}
				}
			} catch (Exception ex) {
				MessageBox.Show(ex.Message, "Cannot delete a value.",
								MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		#endregion

		#region IImageServicePlugin Members

		public void LoadFromFile(string sFile, ref IMAGESERVICEDATA pParams, out Array ppImage) {
			ppImage = null;
			bool success = true;
			try {
				success = (Path.GetExtension(sFile) == @".fb2");
			} catch { }
			if (!success)
				Marshal.ThrowExceptionForHR(Hresults.E_INVALIDARG);
			XmlReader reader = null;
			FictionBookImage image = null;
			Stream fs = null;
			try {
				XmlSerializer s = new XmlSerializer(typeof(FictionBookImage));
				s.UnknownNode += new XmlNodeEventHandler(OnUnknownNode);
				s.UnknownAttribute += new XmlAttributeEventHandler(OnUnknownAttribute);
				fs = new FileStream(sFile, FileMode.Open, FileAccess.Read);
				reader = new XmlTextReader(fs);
				image = (FictionBookImage)s.Deserialize(reader);
			} catch (Exception e) {
				Trace.WriteLine(e.Message);
				Exception inner = e.InnerException;
				while (inner != null) {
					Trace.WriteLine(inner.Message);
					inner = inner.InnerException;
				}
			} finally {
				if (reader != null) reader.Close();
				if (fs != null) fs.Close();
			}

			FictionBookBinary binary = GetCover(image);
			if (binary == null) {
				Marshal.ThrowExceptionForHR(Hresults.E_FAIL);
				return;
			}

			MemoryStream msIn = new MemoryStream(binary.Value);
			try {
				using (Bitmap inImage = (Bitmap)Image.FromStream(msIn))
				using (Bitmap outBitmap = new Bitmap(inImage.Width, inImage.Height,
													 PixelFormat.Format24bppRgb)) {
					using (Graphics g = Graphics.FromImage(outBitmap)) {
						g.DrawImage(inImage, new Point(0, 0));
					}
					Rectangle rect = new Rectangle(0, 0, outBitmap.Width, outBitmap.Height);
					BitmapData bmpData = outBitmap.LockBits(rect, ImageLockMode.ReadOnly,
															outBitmap.PixelFormat);

					// Declare an array to hold the bytes of the bitmap.
					int nBytes = bmpData.Stride * outBitmap.Height;
					byte[] bytes = new byte[nBytes];

					// Swap Red and Blue bytes
					unsafe {
						byte* pData = (byte*)bmpData.Scan0;
						int offset = bmpData.Stride - outBitmap.Width * 3;
						if (offset < 0) {
							Marshal.ThrowExceptionForHR(Hresults.E_FAIL);
							return;						
						}
						byte red;

						for (int y = 0; y < outBitmap.Height; ++y) {
							for (int x = 0; x < outBitmap.Width; ++x) {
								red = pData[2];
								pData[2] = pData[0];
								pData[0] = red;
								pData += 3;
							}
							pData += offset;
						}
					}
					Marshal.Copy(bmpData.Scan0, bytes, 0, nBytes);
					ppImage = bytes;

					pParams.nComponents = 3;
					pParams.nWidth = outBitmap.Width;
					pParams.nHeight = outBitmap.Height;

					outBitmap.UnlockBits(bmpData);
				}
			} catch {
				Marshal.ThrowExceptionForHR(Hresults.E_FAIL);
			} finally {
				msIn.Close();
			}
		}

		private FictionBookBinary GetCover(FictionBookImage image) {
			if (image == null || image.binary == null ||
				image.description == null || image.description.titleinfo == null)
				return null;

			ImageType[] covers = image.description.titleinfo.coverpage;
			FictionBookBinary binary = null;
			if (covers != null) {
				foreach (ImageType cover in covers) {
					string href = cover.href.TrimStart(new char[] { '#' });
					if (!String.IsNullOrEmpty(href)) {
						Predicate<FictionBookBinary> findHref =
							new Predicate<FictionBookBinary>(delegate(FictionBookBinary fbb)
							{
								return fbb.Id == href;
							});
						binary = Array.Find(image.binary, findHref);
						if (binary != null) {
							if (binary.Value == null ||
								(binary.ContentType != @"image/png" &&
								 binary.ContentType != @"image/jpeg" &&
								 binary.ContentType != @"image/bmp" &&
								 binary.ContentType != @"image/gif")) {
								binary = null;
								continue;
							} else
								break;
						}
					}
				}
			}
			return binary;
		}

		public void LoadFromMemory(string sType, Array pMemory,
								   ref IMAGESERVICEDATA pParams, out Array ppImage) {
			ppImage = null;
			Marshal.ThrowExceptionForHR(Hresults.E_NOTIMPL);
		}

		public void SaveToFile(string sFile, ref IMAGESERVICEDATA pParams, Array pImage) {
			Marshal.ThrowExceptionForHR(Hresults.E_NOTIMPL);
		}

		public void SaveToMemory(string sType, out Array ppMemory,
								 ref IMAGESERVICEDATA pParams, Array pImage) {
			ppMemory = null;
			Marshal.ThrowExceptionForHR(Hresults.E_NOTIMPL);
		}

		#endregion

		private ImageCodecInfo GetEncoderInfo(string mimeType) {
			ImageCodecInfo[] codecs = ImageCodecInfo.GetImageEncoders();
			for (int i = 0; i < codecs.Length; i++)
				if (codecs[i].MimeType == mimeType)
					return codecs[i];
			return null;
		}
	}
}
