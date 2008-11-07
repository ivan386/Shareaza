#region Using Directives
using System;
using System.IO;
using System.Security.Permissions;
using System.Text;
using System.Xml;
using System.Xml.Serialization;

#endregion

namespace ShareazaDialogUpdater
{
	public class XmlSerializerBase<T> where T : class
	{
		static XmlSerializer serializer;
		
		static XmlSerializerBase() {
			serializer = new XmlSerializer(typeof(T), new Type[] { typeof(XmlElement) });
			serializer.UnknownNode += new XmlNodeEventHandler(OnUnknownNode);
			serializer.UnknownAttribute += new XmlAttributeEventHandler(OnUnknownAttribute);
		}
		
		[PermissionSet(SecurityAction.Demand, Name = "FullTrust")]
		public static T Read(string filePath, out Exception exception) {
			XmlReader reader = null;
			var newSettings = new XmlReaderSettings()
			{
				CheckCharacters = false,
				IgnoreWhitespace = true,
				IgnoreComments = true,
				XmlResolver = null
			};

			FileStream stream = null;
			T objectTo = null;
			exception = null;
			try {
				stream = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.Read|FileShare.Delete);
				reader = XmlReader.Create(stream, newSettings);
				objectTo = (T)serializer.Deserialize(reader);
			} catch (Exception ex) {
				exception = ex;
			} finally {
				if (stream != null)
					stream.Close();
			}

			return objectTo;
		}

		public static T ReadString(string xml, out Exception exception) {
			XmlReader reader = null;
			var newSettings = new XmlReaderSettings()
			{
				CheckCharacters = false,
				IgnoreWhitespace = true,
				IgnoreComments = true,
				XmlResolver = null
			};

			StringReader stream = null;
			T objectTo = null;
			exception = null;
			try {
				stream = new StringReader(xml);
				reader = XmlReader.Create(stream, newSettings);
				objectTo = (T)serializer.Deserialize(reader);
			} catch (Exception ex) {
				exception = ex;
			} finally {
				if (stream != null)
					stream.Close();
			}

			return objectTo;
		}		

		[PermissionSet(SecurityAction.Demand, Name = "FullTrust")]
		public static void Write(T file, string filePath) {
			var xns = new XmlSerializerNamespaces();
			xns.Add(string.Empty, string.Empty);

			var newSettings = new XmlWriterSettings()
			{
				Indent = true,
				IndentChars = "\t",
				Encoding = new UTF8Encoding(false, false),
				CloseOutput = true,
				CheckCharacters = false,
				NewLineHandling = NewLineHandling.None,
			};

			XmlWriter writer = null;

			try {
				writer = XmlWriter.Create(filePath, newSettings);
				serializer.Serialize(writer, file, xns);
			} catch {
			} finally {
				if (writer != null)
					writer.Close();
			}
		}

		public static bool Write(T objectClass, out MemoryStream stream) {
			var xns = new XmlSerializerNamespaces();
			xns.Add(string.Empty, string.Empty);

			var newSettings = new XmlWriterSettings()
			{
				Indent = true,
				IndentChars = "\t",
				Encoding = new UTF8Encoding(false, false),
				CloseOutput = false,
				CheckCharacters = false,
				NewLineHandling = NewLineHandling.Replace,
				OmitXmlDeclaration = false,
			};

			stream = new MemoryStream();

			try {
				serializer.Serialize(stream, objectClass, xns);
				return true;
			} catch {
				stream.Close();
				stream = null;
				return false;
			}
		}		
		
		static void OnUnknownNode(object sender, XmlNodeEventArgs args) {
			// Ignore
		}
		
		static void OnUnknownAttribute(object sender, XmlAttributeEventArgs args) {
			// Ignore		
		}
	}
}
